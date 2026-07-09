#include <doctest/doctest.h>

#include <filesystem>
#include <fstream>

#include "locations/library_data.hpp"
#include "locations/library_rules.hpp"
#include "locations/library_ui.hpp"

namespace {

pixel_town::library::LibraryData create_test_data() {
    pixel_town::library::LibraryData data;

    data.categories = {
        {"history", "历史", "记录人类文明发展历程"},
        {"science", "科学", "探索自然规律"},
        {"literature", "文学", "人类情感表达"},
        {"art", "艺术", "视觉美学创造"},
        {"technology", "技术", "智慧与工具结合"},
    };

    data.questions = {
        {"人类第一次登月是在哪一年？", "history", "1969年", "回答正确！", "回答错误"},
        {"光速大约是多少公里每秒？", "science", "30万公里", "回答正确！", "回答错误"},
        {"《红楼梦》的作者是谁？", "literature", "曹雪芹", "回答正确！", "回答错误"},
        {"蒙娜丽莎是谁的作品？", "art", "达芬奇", "回答正确！", "回答错误"},
        {"计算机的基本组成部分有哪些？", "technology", "CPU内存等", "回答正确！", "回答错误"},
    };

    data.dialogue.greeting = "欢迎来到图书馆";
    data.dialogue.introduction = "书架按类别分的";
    data.dialogue.familiar_talk = "窗台晒着借书卡";
    data.dialogue.close_friend_talk = "管理员递给你一杯热茶";
    data.dialogue.old_map_reveal = "掉出一张旧集市地图";
    data.dialogue.borrow_card = "借书卡盒子里多了几张新记录";

    data.plot_events = {
        {"test_plot", "测试剧情", "测试描述", 2, 30, false},
    };

    data.welcome_message = "欢迎来到小镇图书馆";
    data.work_intro = "读者会提出各种问题";

    return data;
}

}  // namespace

TEST_CASE("Library data loading - valid format") {
    pixel_town::library::LibraryData data = create_test_data();
    CHECK(data.categories.size() == 5);
    CHECK(data.questions.size() == 5);
    CHECK(data.categories[0].id == "history");
    CHECK(data.categories[0].name == "历史");
    CHECK(data.categories[0].description == "记录人类文明发展历程");
    CHECK(data.questions[0].question == "人类第一次登月是在哪一年？");
    CHECK(data.questions[0].correct_category_id == "history");
    CHECK(data.questions[0].hint == "1969年");
    CHECK(data.questions[0].feedback_correct == "回答正确！");
    CHECK(data.questions[0].feedback_wrong == "回答错误");
    CHECK(data.dialogue.greeting == "欢迎来到图书馆");
    CHECK(data.dialogue.close_friend_talk == "管理员递给你一杯热茶");
    CHECK(data.plot_events.size() == 1);
    CHECK(data.plot_events[0].id == "test_plot");
    CHECK(data.plot_events[0].title == "测试剧情");
    CHECK(data.plot_events[0].required_visits == 2);
    CHECK(data.plot_events[0].required_knowledge == 30);
    CHECK(data.welcome_message == "欢迎来到小镇图书馆");
    CHECK(data.work_intro == "读者会提出各种问题");
}

TEST_CASE("Library data loading rejects questions with unknown categories") {
    const std::filesystem::path path =
        std::filesystem::current_path() / ".tmp-library-invalid-data.txt";
    {
        std::ofstream file(path);
        file << "CATEGORY history: 历史\n";
        file << "QUESTION science: 光速大约是多少？\n";
    }

    const auto result = pixel_town::library::load_library_data(path.string());
    std::filesystem::remove(path);

    CHECK_FALSE(result.success);
    CHECK(result.error_message.find("unknown category") != std::string::npos);
}

TEST_CASE("Library UI state starts from intro instructions") {
    const pixel_town::library::ui::LibraryUIState ui_state;

    CHECK(ui_state.scene_state == pixel_town::library::ui::LibrarySceneState::intro);
}

TEST_CASE("Library UI instructions can be reviewed from answering state") {
    pixel_town::library::ui::LibraryUIState ui_state;
    ui_state.scene_state = pixel_town::library::ui::LibrarySceneState::answering;

    pixel_town::library::ui::request_instruction_review(ui_state);

    CHECK(ui_state.scene_state == pixel_town::library::ui::LibrarySceneState::intro);
    CHECK(ui_state.return_to_answering_after_intro);

    pixel_town::library::ui::advance_from_intro(ui_state);

    CHECK(ui_state.scene_state == pixel_town::library::ui::LibrarySceneState::answering);
    CHECK_FALSE(ui_state.return_to_answering_after_intro);
}

TEST_CASE("Library rule engine - start session") {
    pixel_town::library::LibraryData data = create_test_data();
    pixel_town::library::LibraryConfig config =
        pixel_town::library::default_library_config();
    pixel_town::library::LibraryRuleEngine engine(data, config);

    pixel_town::library::DailyContext context;
    context.day = 1;
    context.random_seed = 12345;
    engine.start_session(context);

    CHECK(engine.is_session_active());
    CHECK(!engine.is_session_completed());
    CHECK(engine.get_session_state().current_question_index == 0);
    CHECK(engine.get_session_state().correct_count == 0);
    CHECK(engine.get_session_state().wrong_count == 0);
}

TEST_CASE("Library rule engine - correct answer") {
    pixel_town::library::LibraryData data = create_test_data();
    pixel_town::library::LibraryConfig config =
        pixel_town::library::default_library_config();
    pixel_town::library::LibraryRuleEngine engine(data, config);

    pixel_town::library::DailyContext context;
    context.day = 1;
    context.random_seed = 12345;
    engine.start_session(context);

    const auto& current_question = engine.get_current_question();
    engine.select_category(current_question.correct_category_id);

    CHECK(engine.was_last_answer_correct());
    CHECK(engine.get_session_state().correct_count == 1);
    CHECK(engine.get_session_state().wrong_count == 0);
    CHECK(engine.get_session_state().current_combo == 1);
}

TEST_CASE("Library rule engine - wrong answer") {
    pixel_town::library::LibraryData data = create_test_data();
    pixel_town::library::LibraryConfig config =
        pixel_town::library::default_library_config();
    pixel_town::library::LibraryRuleEngine engine(data, config);

    pixel_town::library::DailyContext context;
    context.day = 1;
    context.random_seed = 12345;
    engine.start_session(context);

    const auto& current_question = engine.get_current_question();
    std::string wrong_category = "science";
    if (wrong_category == current_question.correct_category_id) {
        wrong_category = "history";
    }
    engine.select_category(wrong_category);

    CHECK(!engine.was_last_answer_correct());
    CHECK(engine.get_session_state().correct_count == 0);
    CHECK(engine.get_session_state().wrong_count == 1);
    CHECK(engine.get_session_state().current_combo == 0);
}

TEST_CASE("Library rule engine - complete session") {
    pixel_town::library::LibraryData data = create_test_data();
    pixel_town::library::LibraryConfig config =
        pixel_town::library::default_library_config();
    pixel_town::library::LibraryRuleEngine engine(data, config);

    pixel_town::library::DailyContext context;
    context.day = 1;
    context.random_seed = 12345;
    engine.start_session(context);

    for (int i = 0; i < config.questions_per_session; ++i) {
        const auto& question = engine.get_current_question();
        engine.select_category(question.correct_category_id);
    }

    CHECK(engine.is_session_completed());
    CHECK(!engine.is_session_active());
    CHECK(engine.get_session_state().correct_count == 5);
    CHECK(engine.get_session_state().wrong_count == 0);
}

TEST_CASE("Library rule engine - action result calculation") {
    pixel_town::library::LibraryData data = create_test_data();
    pixel_town::library::LibraryConfig config =
        pixel_town::library::default_library_config();
    pixel_town::library::LibraryRuleEngine engine(data, config);

    pixel_town::library::DailyContext context;
    context.day = 1;
    context.random_seed = 12345;
    engine.start_session(context);

    for (int i = 0; i < 3; ++i) {
        const auto& question = engine.get_current_question();
        engine.select_category(question.correct_category_id);
    }

    for (int i = 0; i < 2; ++i) {
        engine.select_category("wrong");
    }

    pixel_town::library::ActionResult result = engine.finish_session();

    CHECK(result.completed);
    CHECK(result.stamina_change == -config.stamina_cost);
    CHECK(result.knowledge_change == 3 * config.correct_knowledge_reward);
    CHECK(result.reputation_change == 3 * config.correct_reputation_reward + 3 * config.combo_reputation_bonus);
    CHECK(result.mood_change == config.base_mood_change + 3 * config.correct_mood_bonus +
                                     2 * config.wrong_mood_penalty);
}

TEST_CASE("Library rule engine - give up") {
    pixel_town::library::LibraryData data = create_test_data();
    pixel_town::library::LibraryConfig config =
        pixel_town::library::default_library_config();
    pixel_town::library::LibraryRuleEngine engine(data, config);

    pixel_town::library::DailyContext context;
    context.day = 1;
    context.random_seed = 12345;
    engine.start_session(context);

    engine.give_up();

    CHECK(engine.is_session_completed());
    CHECK(!engine.is_session_active());

    pixel_town::library::ActionResult result = engine.finish_session();
    CHECK(result.gave_up);
    CHECK(result.stamina_change == 0);
    CHECK(result.knowledge_change == 0);
    CHECK(result.reputation_change == 0);
}

TEST_CASE("Library rule engine - deterministic randomness") {
    pixel_town::library::LibraryData data = create_test_data();
    pixel_town::library::LibraryConfig config =
        pixel_town::library::default_library_config();

    pixel_town::library::DailyContext context1;
    context1.day = 1;
    context1.random_seed = 12345;
    pixel_town::library::LibraryRuleEngine engine1(data, config);
    engine1.start_session(context1);

    pixel_town::library::DailyContext context2;
    context2.day = 1;
    context2.random_seed = 12345;
    pixel_town::library::LibraryRuleEngine engine2(data, config);
    engine2.start_session(context2);

    CHECK(engine1.get_current_question().question ==
          engine2.get_current_question().question);
}

TEST_CASE("Library rule engine - npc relationship progression") {
    pixel_town::library::LibraryData data = create_test_data();
    pixel_town::library::LibraryConfig config =
        pixel_town::library::default_library_config();
    pixel_town::library::LibraryRuleEngine engine(data, config);

    engine.update_npc_relationship(0, 0);
    CHECK(engine.get_npc_interaction().relationship ==
          pixel_town::library::NpcRelationship::stranger);

    engine.update_npc_relationship(30, 2);
    CHECK(engine.get_npc_interaction().relationship ==
          pixel_town::library::NpcRelationship::familiar);

    engine.update_npc_relationship(50, 5);
    CHECK(engine.get_npc_interaction().borrow_card_given);

    engine.update_npc_relationship(80, 8);
    CHECK(engine.get_npc_interaction().close_friend_unlocked);
}

TEST_CASE("Library rule engine - plot events") {
    pixel_town::library::LibraryData data = create_test_data();
    pixel_town::library::LibraryConfig config =
        pixel_town::library::default_library_config();
    pixel_town::library::LibraryRuleEngine engine(data, config);

    CHECK(!engine.has_pending_plot_event());

    engine.check_plot_events(25, 1);
    CHECK(!engine.has_pending_plot_event());

    engine.check_plot_events(30, 2);
    CHECK(engine.has_pending_plot_event());
    CHECK(engine.get_npc_interaction().current_plot_title == "测试剧情");

    engine.trigger_plot_event("test_plot");
    CHECK(!engine.has_pending_plot_event());
}

TEST_CASE("Library rule engine - map reveal conditions") {
    pixel_town::library::LibraryData data = create_test_data();
    pixel_town::library::LibraryConfig config =
        pixel_town::library::default_library_config();
    pixel_town::library::LibraryRuleEngine engine(data, config);

    CHECK(!engine.should_reveal_map(25, 1));
    CHECK(engine.should_reveal_map(30, 2));

    engine.reveal_map();
    CHECK(engine.get_npc_interaction().map_revealed);
    CHECK(!engine.should_reveal_map(30, 2));
}

TEST_CASE("Library rule engine exposes session context and config for UI decisions") {
    pixel_town::library::LibraryData data = create_test_data();
    pixel_town::library::LibraryConfig config =
        pixel_town::library::default_library_config();
    config.correct_knowledge_reward = 12;
    config.knowledge_threshold_for_map = 40;
    config.visits_threshold_for_map = 3;
    pixel_town::library::LibraryRuleEngine engine(data, config);

    pixel_town::library::DailyContext context;
    context.day = 4;
    context.random_seed = 12345;
    context.library_visits = 3;
    context.current_knowledge = 39;
    engine.start_session(context);

    CHECK(engine.get_config().correct_knowledge_reward == 12);
    CHECK(engine.get_current_context().day == 4);
    CHECK(engine.get_current_context().library_visits == 3);
    CHECK(engine.get_current_context().current_knowledge == 39);
    CHECK_FALSE(engine.should_reveal_map(engine.get_current_context().current_knowledge,
                                         engine.get_current_context().library_visits));

    context.current_knowledge = 40;
    engine.start_session(context);
    CHECK(engine.should_reveal_map(engine.get_current_context().current_knowledge,
                                   engine.get_current_context().library_visits));
}
