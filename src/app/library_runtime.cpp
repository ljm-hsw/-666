// 图书馆模式生命周期和结算顺序；规则本身由 locations::library 模块提供。
#include "app/library_runtime.hpp"

#include <utility>

#include "app/location_result_adapter.hpp"

namespace pixel_town {
namespace {

LibraryStepResult step_result(LibraryStepStatus status, std::string notice) {
    return LibraryStepResult{status, std::move(notice)};
}

}  // namespace

void LibraryRuntime::close() {
    // close 只清理地点临时对象；核心阶段和玩家状态已经在 settle() 中完成收口。
    active_ = false;
    mode_ = LibraryRuntimeMode::selection;
    reader_.reset();
    organizing_.reset();
    active_result_id_ = 0;
}

LibraryStepResult LibraryRuntime::settle(
    GameSession& session, const library::LibraryWorkResult& work_result) {
    // 两种图书馆模式共享同一结算口；Adapter 做字段映射，Session 决定最终接受/拒绝。
    const auto applied = session.apply_action_result(
        library_action_result(work_result, active_result_id_, ActionSlot::day));
    feedback_ = applied.accepted ? work_result.summary : applied.message;
    if (!applied.accepted) {
        return step_result(LibraryStepStatus::rejected, feedback_);
    }
    close();
    return step_result(LibraryStepStatus::settled, feedback_);
}

LibraryOpenResult LibraryRuntime::open(GameSession& session,
                                       const library::LibraryData& data) {
    // 先校验 Session 的地点阶段，再分配结果 ID；数据加载成功不等于行动已开始。
    if (active_) {
        return {LibraryOpenStatus::already_active, "图书馆会话已经打开。"};
    }
    if (session.phase() != GamePhase::day_location ||
        !session.has_pending_location() ||
        session.pending_location() != Location::library) {
        return {LibraryOpenStatus::denied, "当前游戏阶段不能开始图书馆工作。"};
    }
    const int result_id = session.start_location();
    if (result_id == 0) {
        return {LibraryOpenStatus::denied, "图书馆地点会话启动失败。"};
    }

    data_ = data;
    const int completed_visits =
        session.location_visit_count(Location::library);
    visits_ = completed_visits + 1;
    map_revealed_ = map_revealed_ ||
                    completed_visits >=
                        library::default_library_config().visits_threshold_for_map;
    active_ = true;
    mode_ = LibraryRuntimeMode::selection;
    reader_.reset();
    organizing_.reset();
    active_result_id_ = result_id;
    feedback_ = "已进入图书馆，请选择读者咨询或书籍整理。";
    return {LibraryOpenStatus::opened, feedback_};
}

LibraryStepResult LibraryRuntime::step(GameSession& session,
                                       const LibraryIntent& intent) {
    // 每个 intent 只推进一个逻辑动作；完成/放弃最终都经过同一个 settle()。
    if (!active_) {
        return step_result(LibraryStepStatus::rejected, "图书馆会话尚未打开。");
    }
    if (session.phase() != GamePhase::day_location ||
        !session.has_pending_location() ||
        session.pending_location() != Location::library ||
        session.active_result_id() != active_result_id_) {
        return step_result(LibraryStepStatus::rejected,
                           "当前游戏阶段与图书馆会话不一致。");
    }

    if (intent.type == LibraryIntentType::abandon) {
        if (mode_ == LibraryRuntimeMode::reader_consultation && reader_) {
            reader_->give_up();
            return settle(session, reader_->finish_session());
        }
        if (mode_ == LibraryRuntimeMode::book_organizing && organizing_) {
            organizing_->give_up();
            return settle(session, organizing_->finish_session());
        }
        const auto applied =
            session.apply_action_result(session.abandon_current_location());
        feedback_ = applied.message;
        if (!applied.accepted) {
            return step_result(LibraryStepStatus::rejected, feedback_);
        }
        close();
        return step_result(LibraryStepStatus::settled, feedback_);
    }

    if (mode_ == LibraryRuntimeMode::selection) {
        if (intent.type == LibraryIntentType::select_reader) {
            reader_ = std::make_unique<library::LibraryRuleEngine>(
                data_, library::default_library_config());
            library::DailyContext context;
            context.day = session.day();
            context.random_seed = session.location_seed(
                Location::library, static_cast<unsigned int>(visits_));
            context.library_visits = visits_;
            context.current_knowledge = session.player().knowledge;
            context.map_revealed = map_revealed_;
            reader_->start_session(context);
            reader_->update_npc_relationship(session.player().knowledge, visits_);
            organizing_.reset();
            mode_ = LibraryRuntimeMode::reader_consultation;
            feedback_ = "已选择读者咨询：根据需求匹配书籍类别。";
            return step_result(LibraryStepStatus::changed, feedback_);
        }
        if (intent.type != LibraryIntentType::select_organizing) {
            feedback_ = "请选择读者咨询或书籍整理。";
            return step_result(LibraryStepStatus::rejected, feedback_);
        }
        if (data_.organizing_tasks.empty() || data_.organizing_shelves.empty()) {
            feedback_ = "图书馆整理数据不完整，无法开始。";
            return step_result(LibraryStepStatus::rejected, feedback_);
        }
        organizing_ = std::make_unique<library::LibraryOrganizingSession>(
            data_.organizing_tasks, data_.organizing_shelves,
            library::default_organizing_config());
        organizing_->start(session.location_seed(
            Location::library, static_cast<unsigned int>(visits_)));
        mode_ = LibraryRuntimeMode::book_organizing;
        reader_.reset();
        feedback_ = "已选择书籍整理：拿起待整理书，再点击正确书架。";
        return step_result(LibraryStepStatus::changed, feedback_);
    }

    if (mode_ == LibraryRuntimeMode::reader_consultation && reader_) {
        if (intent.type == LibraryIntentType::acknowledge_map) {
            if (!reader_->should_reveal_map(
                    reader_->get_current_context().current_knowledge,
                    reader_->get_current_context().library_visits)) {
                feedback_ = "旧地图已经收下，不会重复展示。";
                return step_result(LibraryStepStatus::rejected, feedback_);
            }
            reader_->reveal_map();
            map_revealed_ = true;
            feedback_ = "已收下旧集市地图。";
            return step_result(LibraryStepStatus::changed, feedback_);
        }
        if (intent.type == LibraryIntentType::answer_category) {
            if (!reader_->is_session_active()) {
                feedback_ = "本次读者咨询已经完成。";
                return step_result(LibraryStepStatus::rejected, feedback_);
            }
            reader_->select_category(intent.target_id);
            feedback_ = reader_->was_last_answer_correct() ? "回答正确。"
                                                           : "回答错误。";
            return step_result(LibraryStepStatus::changed, feedback_);
        }
        if (intent.type == LibraryIntentType::finish_reader) {
            if (!reader_->is_session_completed()) {
                feedback_ = "读者咨询尚未完成。";
                return step_result(LibraryStepStatus::rejected, feedback_);
            }
            return settle(session, reader_->finish_session());
        }
        return step_result(LibraryStepStatus::unchanged, feedback_);
    }

    if (mode_ != LibraryRuntimeMode::book_organizing || !organizing_) {
        return step_result(LibraryStepStatus::rejected,
                           "图书馆工作模式当前不接受该操作。");
    }

    if (intent.type == LibraryIntentType::pick_up_book) {
        if (intent.target_id.empty()) {
            feedback_ = "这里没有待整理的书，请点击带光圈的书本素材。";
            return step_result(LibraryStepStatus::unchanged, feedback_);
        }
        const auto result = organizing_->pick_up(intent.target_id);
        feedback_ = result.message;
        return step_result(result.status == library::OrganizingActionStatus::accepted
                               ? LibraryStepStatus::changed
                               : LibraryStepStatus::rejected,
                           feedback_);
    }
    if (intent.type == LibraryIntentType::place_on_shelf) {
        if (intent.target_id.empty()) {
            feedback_ = "请点击书架高亮区域完成归位。";
            return step_result(LibraryStepStatus::unchanged, feedback_);
        }
        const auto result = organizing_->place_on_shelf(intent.target_id);
        feedback_ = result.message;
        if (result.status != library::OrganizingActionStatus::accepted &&
            result.status != library::OrganizingActionStatus::completed) {
            return step_result(result.status == library::OrganizingActionStatus::wrong_shelf
                                   ? LibraryStepStatus::changed
                                   : LibraryStepStatus::rejected,
                               feedback_);
        }
        if (result.status != library::OrganizingActionStatus::completed) {
            return step_result(LibraryStepStatus::changed, feedback_);
        }

        return settle(session, organizing_->finish_session());
    }

    return step_result(LibraryStepStatus::unchanged, feedback_);
}

LibraryPresentation LibraryRuntime::presentation() const {
    LibraryPresentation result;
    result.active = active_;
    result.mode = mode_;
    result.feedback = feedback_;
    if (reader_) {
        LibraryReaderPresentation reader_view;
        reader_view.state = reader_->get_session_state();
        reader_view.categories = reader_->get_categories();
        reader_view.interaction = reader_->get_npc_interaction();
        reader_view.result = reader_->finish_session();
        reader_view.welcome_message = reader_->get_data().welcome_message.empty()
                                          ? reader_->get_dialogue().greeting
                                          : reader_->get_data().welcome_message;
        reader_view.work_intro = reader_->get_data().work_intro.empty()
                                     ? "读者会提出各种问题，你需要从书架上找到正确的书籍类别来回答"
                                     : reader_->get_data().work_intro;
        reader_view.introduction_dialogue = reader_->get_dialogue().introduction;
        reader_view.old_map_reveal_dialogue = reader_->get_dialogue().old_map_reveal;
        reader_view.correct_knowledge_reward =
            reader_->get_config().correct_knowledge_reward;
        reader_view.last_answer_correct = reader_->was_last_answer_correct();
        reader_view.pending_plot_event = reader_->has_pending_plot_event();
        const auto& context = reader_->get_current_context();
        reader_view.should_reveal_map = reader_->should_reveal_map(
            context.current_knowledge, context.library_visits);
        if (reader_->is_session_active()) {
            reader_view.current_question = reader_->get_current_question();
        }
        result.reader = std::move(reader_view);
    }
    if (organizing_) {
        result.organizing = LibraryOrganizingPresentation{
            organizing_->tasks(), organizing_->shelves(), organizing_->state(),
            feedback_};
    }
    return result;
}

}  // namespace pixel_town
