// 读者咨询规则实现；不绘制界面、不选择剧情、不直接写入会话。
#include "locations/library_rules.hpp"

#include <algorithm>
#include <cinttypes>
#include <sstream>
#include <string>

namespace pixel_town::library {

namespace {

uint64_t splitmix64(uint64_t& state) {
    uint64_t x = state;
    x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
    return state = x ^ (x >> 31);
}

template <typename T>
void shuffle_with_seed(std::vector<T>& vec, uint64_t seed) {
    for (size_t i = vec.size() - 1; i > 0; --i) {
        const size_t j = splitmix64(seed) % (i + 1);
        std::swap(vec[i], vec[j]);
    }
}

}  // namespace

LibraryRuleEngine::LibraryRuleEngine(const LibraryData& data, const LibraryConfig& config)
    : data_(data), config_(config) {}

void LibraryRuleEngine::start_session(const DailyContext& context) {
    current_context_ = context;
    session_state_ = SessionState{};
    session_state_.is_active = true;
    session_state_.is_in_intro = true;
    gave_up_ = false;

    shuffled_questions_ = data_.questions;
    shuffle_questions(context.random_seed);

    const size_t take_count =
        std::min(static_cast<size_t>(config_.questions_per_session), shuffled_questions_.size());
    shuffled_questions_.resize(take_count);

    npc_interaction_.current_plot_status = PlotStatus::not_available;
    npc_interaction_.current_plot_title.clear();
    npc_interaction_.current_plot_description.clear();
}

void LibraryRuleEngine::select_category(const std::string& category_id) {
    if (!session_state_.is_active || session_state_.is_completed) {
        return;
    }

    const auto& current_question = shuffled_questions_[session_state_.current_question_index];
    last_answer_correct_ = (category_id == current_question.correct_category_id);

    session_state_.answers.push_back(last_answer_correct_);
    if (last_answer_correct_) {
        session_state_.correct_count++;
        session_state_.current_combo++;
        if (session_state_.current_combo > session_state_.max_combo) {
            session_state_.max_combo = session_state_.current_combo;
        }
    } else {
        session_state_.wrong_count++;
        session_state_.current_combo = 0;
    }

    session_state_.current_question_index++;

    if (session_state_.current_question_index >=
        static_cast<int>(shuffled_questions_.size())) {
        session_state_.is_completed = true;
        session_state_.is_active = false;
    }
}

bool LibraryRuleEngine::is_session_active() const { return session_state_.is_active; }

bool LibraryRuleEngine::is_session_completed() const { return session_state_.is_completed; }

const SessionState& LibraryRuleEngine::get_session_state() const { return session_state_; }

const ReaderQuestion& LibraryRuleEngine::get_current_question() const {
    return shuffled_questions_[session_state_.current_question_index];
}

const std::vector<BookCategory>& LibraryRuleEngine::get_categories() const {
    return data_.categories;
}

bool LibraryRuleEngine::was_last_answer_correct() const { return last_answer_correct_; }

void LibraryRuleEngine::give_up() {
    gave_up_ = true;
    session_state_.is_completed = true;
    session_state_.is_active = false;
}

LibraryWorkResult LibraryRuleEngine::finish_session() const {
    LibraryWorkResult result;

    if (gave_up_ ||
        (session_state_.correct_count == 0 && session_state_.wrong_count == 0)) {
        result.gave_up = true;
        result.summary = "离开了图书馆，什么也没做。";
        return result;
    }

    result.completed = true;
    result.stamina_change = -config_.stamina_cost;
    result.knowledge_change = compute_knowledge_change();
    result.reputation_change = compute_reputation_change();
    result.mood_change = compute_mood_change();
    result.money_change = session_state_.correct_count * config_.money_per_correct +
                          session_state_.max_combo * config_.money_bonus_for_combo;
    result.summary = generate_summary();
    result.narrative_echo = generate_narrative_echo();

    if (npc_interaction_.current_plot_status == PlotStatus::available) {
        result.plot_triggered = true;
        result.plot_title = npc_interaction_.current_plot_title;
        result.plot_description = npc_interaction_.current_plot_description;
    }

    return result;
}

void LibraryRuleEngine::shuffle_questions(uint64_t seed) {
    shuffle_with_seed(shuffled_questions_, seed);
}

int LibraryRuleEngine::compute_knowledge_change() const {
    return session_state_.correct_count * config_.correct_knowledge_reward +
           session_state_.wrong_count * config_.wrong_knowledge_penalty;
}

int LibraryRuleEngine::compute_reputation_change() const {
    int reputation = session_state_.correct_count * config_.correct_reputation_reward +
                     session_state_.wrong_count * config_.wrong_reputation_penalty;
    reputation += session_state_.max_combo * config_.combo_reputation_bonus;
    return reputation;
}

int LibraryRuleEngine::compute_mood_change() const {
    int mood = config_.base_mood_change;
    mood += session_state_.correct_count * config_.correct_mood_bonus;
    mood += session_state_.wrong_count * config_.wrong_mood_penalty;
    return mood;
}

std::string LibraryRuleEngine::generate_summary() const {
    std::ostringstream oss;
    oss << "在图书馆工作了一天。";
    oss << "答对 " << session_state_.correct_count << " 题，答错 " << session_state_.wrong_count
        << " 题。";

    if (session_state_.max_combo > 1) {
        oss << "最高连续答对 " << session_state_.max_combo << " 题！";
    }

    int money_earned = session_state_.correct_count * config_.money_per_correct +
                       session_state_.max_combo * config_.money_bonus_for_combo;
    if (money_earned > 0) {
        oss << "获得 " << money_earned << " 金币。";
    }

    if (session_state_.correct_count >= static_cast<int>(shuffled_questions_.size()) * 3 / 4) {
        oss << "表现出色！";
    } else if (session_state_.correct_count >=
               static_cast<int>(shuffled_questions_.size()) / 2) {
        oss << "表现不错。";
    } else {
        oss << "还需要继续努力。";
    }

    return oss.str();
}

std::string LibraryRuleEngine::generate_narrative_echo() const {
    if (npc_interaction_.relationship == NpcRelationship::stranger) {
        return "";
    }

    if (npc_interaction_.map_revealed && !npc_interaction_.borrow_card_given) {
        return "你帮孩子找到了考试要用的书，借书卡又多盖了一个章。管理员把旧地图递给你，说可以先看看，不急着还。书页翻动的声音很轻，窗外的街也慢下来。";
    }

    if (npc_interaction_.borrow_card_given && !npc_interaction_.close_friend_unlocked) {
        return "借书卡盒子里多了几张新记录。管理员把旧地图收回去前，指了指边上的空白处，说以后可以把今年的事也写上去。";
    }

    if (npc_interaction_.close_friend_unlocked) {
        return "管理员递给你一杯热茶。窗外的梧桐树影落在书页上，今天的图书馆格外安静。";
    }

    return "";
}

NpcInteraction& LibraryRuleEngine::get_npc_interaction() { return npc_interaction_; }

const NpcInteraction& LibraryRuleEngine::get_npc_interaction() const { return npc_interaction_; }

void LibraryRuleEngine::update_npc_relationship(int knowledge, int visits) {
    if (visits >= config_.visits_threshold_for_close_friend &&
        knowledge >= config_.knowledge_threshold_for_close_friend) {
        npc_interaction_.relationship = NpcRelationship::close_friend;
        if (!npc_interaction_.close_friend_unlocked) {
            npc_interaction_.close_friend_unlocked = true;
            npc_interaction_.current_dialogue = data_.dialogue.close_friend_talk;
        }
    } else if (visits >= config_.visits_threshold_for_borrow_card &&
               knowledge >= config_.knowledge_threshold_for_borrow_card) {
        npc_interaction_.relationship = NpcRelationship::familiar;
        if (!npc_interaction_.borrow_card_given) {
            npc_interaction_.borrow_card_given = true;
            npc_interaction_.current_dialogue = data_.dialogue.borrow_card;
        }
    } else if (visits >= config_.visits_threshold_for_map &&
               knowledge >= config_.knowledge_threshold_for_map) {
        npc_interaction_.relationship = NpcRelationship::familiar;
        npc_interaction_.current_dialogue = data_.dialogue.familiar_talk;
    } else {
        npc_interaction_.relationship = NpcRelationship::stranger;
        npc_interaction_.current_dialogue = data_.dialogue.introduction;
    }
}

bool LibraryRuleEngine::should_reveal_map(int knowledge, int visits) const {
    return !npc_interaction_.map_revealed &&
           knowledge >= config_.knowledge_threshold_for_map &&
           visits >= config_.visits_threshold_for_map;
}

void LibraryRuleEngine::reveal_map() {
    npc_interaction_.map_revealed = true;
    npc_interaction_.current_dialogue = data_.dialogue.old_map_reveal;
}

void LibraryRuleEngine::grant_borrow_card() {
    npc_interaction_.borrow_card_given = true;
    npc_interaction_.current_dialogue = data_.dialogue.borrow_card;
}

bool LibraryRuleEngine::should_unlock_close_friend(int knowledge, int visits) const {
    return !npc_interaction_.close_friend_unlocked &&
           knowledge >= config_.knowledge_threshold_for_close_friend &&
           visits >= config_.visits_threshold_for_close_friend;
}

void LibraryRuleEngine::unlock_close_friend() {
    npc_interaction_.close_friend_unlocked = true;
    npc_interaction_.current_dialogue = data_.dialogue.close_friend_talk;
}

const NpcDialogue& LibraryRuleEngine::get_dialogue() const { return data_.dialogue; }

const LibraryData& LibraryRuleEngine::get_data() const { return data_; }

const LibraryConfig& LibraryRuleEngine::get_config() const { return config_; }

const DailyContext& LibraryRuleEngine::get_current_context() const { return current_context_; }

void LibraryRuleEngine::check_plot_events(int knowledge, int visits) {
    for (const auto& event : data_.plot_events) {
        bool already_triggered = false;
        for (const auto& triggered_id : triggered_plot_events_) {
            if (triggered_id == event.id) {
                already_triggered = true;
                break;
            }
        }

        if (!already_triggered && visits >= event.required_visits &&
            knowledge >= event.required_knowledge) {
            npc_interaction_.current_plot_status = PlotStatus::available;
            npc_interaction_.current_plot_title = event.title;
            npc_interaction_.current_plot_description = event.description;
            return;
        }
    }

    npc_interaction_.current_plot_status = PlotStatus::not_available;
}

void LibraryRuleEngine::trigger_plot_event(const std::string& event_id) {
    triggered_plot_events_.push_back(event_id);
    npc_interaction_.current_plot_status = PlotStatus::triggered;
}

bool LibraryRuleEngine::has_pending_plot_event() const {
    return npc_interaction_.current_plot_status == PlotStatus::available;
}

LibraryConfig default_library_config() {
    LibraryConfig config;
    config.questions_per_session = 5;
    config.correct_knowledge_reward = 10;
    config.correct_reputation_reward = 3;
    config.wrong_knowledge_penalty = 0;
    config.wrong_reputation_penalty = 0;
    config.stamina_cost = 15;
    config.base_mood_change = 5;
    config.correct_mood_bonus = 5;
    config.wrong_mood_penalty = -3;
    config.combo_reputation_bonus = 2;
    config.money_per_correct = 5;
    config.money_bonus_for_combo = 3;
    config.knowledge_threshold_for_map = 30;
    config.visits_threshold_for_map = 2;
    config.knowledge_threshold_for_borrow_card = 50;
    config.visits_threshold_for_borrow_card = 5;
    config.knowledge_threshold_for_close_friend = 80;
    config.visits_threshold_for_close_friend = 8;
    return config;
}

}  // namespace pixel_town::library
