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

    session_state_.scattered_books.clear();
    session_state_.misplaced_books.clear();

    int id_counter = 1;
    for (auto& book : data_.books) {
        Book book_copy = book;
        if (book_copy.id.empty()) {
            book_copy.id = "book_" + std::to_string(id_counter++);
        }
        if (book.is_scattered) {
            session_state_.scattered_books.push_back(book_copy);
        } else if (book.is_misplaced) {
            session_state_.misplaced_books.push_back(book_copy);
        }
    }

    session_state_.total_scattered = static_cast<int>(session_state_.scattered_books.size());
    session_state_.total_misplaced = static_cast<int>(session_state_.misplaced_books.size());
    session_state_.current_scattered_index = 0;

    npc_interaction_.current_plot_status = PlotStatus::not_available;
    npc_interaction_.current_plot_title.clear();
    npc_interaction_.current_plot_description.clear();
}

void LibraryRuleEngine::pick_up_book(const std::string& book_id) {
    if (!session_state_.is_active || session_state_.is_completed || session_state_.holding_book) {
        return;
    }

    for (const auto& book : session_state_.scattered_books) {
        if (book.id == book_id) {
            session_state_.selected_book_id = book_id;
            session_state_.holding_book = true;
            session_state_.held_book_category = book.category_id;
            session_state_.held_book_title = book.title;
            break;
        }
    }
}

void LibraryRuleEngine::put_down_book() {
    session_state_.holding_book = false;
    session_state_.selected_book_id.clear();
    session_state_.held_book_category.clear();
    session_state_.held_book_title.clear();
}

void LibraryRuleEngine::pick_misplaced_book(const std::string& book_id) {
    if (!session_state_.is_active || session_state_.is_completed || session_state_.holding_book) {
        return;
    }

    for (auto it = session_state_.misplaced_books.begin(); it != session_state_.misplaced_books.end(); ++it) {
        if (it->id == book_id) {
            session_state_.selected_book_id = it->id;
            session_state_.holding_book = true;
            session_state_.held_book_category = it->category_id;
            session_state_.held_book_title = it->title;
            session_state_.misplaced_books.erase(it);
            break;
        }
    }
}

bool LibraryRuleEngine::place_book_on_shelf(const std::string& shelf_id) {
    if (!session_state_.is_active || session_state_.is_completed || !session_state_.holding_book) {
        return false;
    }

    std::string correct_category_id;
    for (const auto& shelf : data_.shelves) {
        if (shelf.id == shelf_id) {
            correct_category_id = shelf.category_id;
            break;
        }
    }

    last_answer_correct_ = (session_state_.held_book_category == correct_category_id);

    if (last_answer_correct_) {
        session_state_.correct_count++;
        session_state_.current_combo++;
        if (session_state_.current_combo > session_state_.max_combo) {
            session_state_.max_combo = session_state_.current_combo;
        }
        
        bool was_scattered = false;
        for (const auto& book : session_state_.scattered_books) {
            if (book.id == session_state_.selected_book_id) {
                was_scattered = true;
                break;
            }
        }
        
        if (was_scattered) {
            session_state_.placed_count++;
            if (session_state_.current_scattered_index < static_cast<int>(session_state_.scattered_books.size()) - 1) {
                session_state_.current_scattered_index++;
            }
        } else {
            session_state_.corrected_count++;
        }
    } else {
        session_state_.wrong_count++;
        session_state_.current_combo = 0;
    }

    put_down_book();

    if (session_state_.placed_count >= session_state_.total_scattered && 
        session_state_.corrected_count >= session_state_.total_misplaced) {
        session_state_.is_completed = true;
        session_state_.is_active = false;
    }

    return last_answer_correct_;
}

bool LibraryRuleEngine::pick_from_shelf(int book_index) {
    if (!session_state_.is_active || session_state_.is_completed || session_state_.holding_book) {
        return false;
    }

    if (book_index < 0 || book_index >= static_cast<int>(session_state_.misplaced_books.size())) {
        return false;
    }

    const Book& book = session_state_.misplaced_books[book_index];
    session_state_.selected_book_id = book.id;
    session_state_.holding_book = true;
    session_state_.held_book_category = book.category_id;
    session_state_.held_book_title = book.title;

    session_state_.misplaced_books.erase(session_state_.misplaced_books.begin() + book_index);

    return true;
}

bool LibraryRuleEngine::is_session_active() const { return session_state_.is_active; }

bool LibraryRuleEngine::is_session_completed() const { return session_state_.is_completed; }

const SessionState& LibraryRuleEngine::get_session_state() const { return session_state_; }

const std::vector<Book>& LibraryRuleEngine::get_scattered_books() const {
    return session_state_.scattered_books;
}

const std::vector<Book>& LibraryRuleEngine::get_misplaced_books() const {
    return session_state_.misplaced_books;
}

const std::vector<BookCategory>& LibraryRuleEngine::get_categories() const {
    return data_.categories;
}

const std::vector<Shelf>& LibraryRuleEngine::get_shelves() const {
    return data_.shelves;
}

bool LibraryRuleEngine::was_last_answer_correct() const { return last_answer_correct_; }

bool LibraryRuleEngine::is_holding_book() const { return session_state_.holding_book; }

const std::string& LibraryRuleEngine::get_held_book_category() const {
    return session_state_.held_book_category;
}

const std::string& LibraryRuleEngine::get_held_book_title() const {
    return session_state_.held_book_title;
}

void LibraryRuleEngine::give_up() {
    session_state_.is_completed = true;
    session_state_.is_active = false;
}

ActionResult LibraryRuleEngine::finish_session() const {
    ActionResult result;

    if (session_state_.placed_count == 0 && session_state_.corrected_count == 0 && 
        session_state_.wrong_count == 0) {
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
                          session_state_.corrected_count * config_.money_per_correct +
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

int LibraryRuleEngine::compute_knowledge_change() const {
    return session_state_.correct_count * config_.correct_knowledge_reward +
           session_state_.corrected_count * config_.correct_knowledge_reward +
           session_state_.wrong_count * config_.wrong_knowledge_penalty;
}

int LibraryRuleEngine::compute_reputation_change() const {
    int reputation = session_state_.correct_count * config_.correct_reputation_reward +
                     session_state_.corrected_count * config_.correct_reputation_reward +
                     session_state_.wrong_count * config_.wrong_reputation_penalty;
    reputation += session_state_.max_combo * config_.combo_reputation_bonus;
    return reputation;
}

int LibraryRuleEngine::compute_mood_change() const {
    int mood = config_.base_mood_change;
    mood += session_state_.correct_count * config_.correct_mood_bonus;
    mood += session_state_.corrected_count * config_.correct_mood_bonus;
    mood += session_state_.wrong_count * config_.wrong_mood_penalty;
    return mood;
}

std::string LibraryRuleEngine::generate_summary() const {
    std::ostringstream oss;
    oss << "在图书馆整理了一天书籍。";
    
    if (session_state_.placed_count > 0) {
        oss << "整理了 " << session_state_.placed_count << " 本散落的书，";
    }
    if (session_state_.corrected_count > 0) {
        oss << "纠正了 " << session_state_.corrected_count << " 本放错的书，";
    }
    oss << "放错 " << session_state_.wrong_count << " 本。";

    if (session_state_.max_combo > 1) {
        oss << "最高连续整理正确 " << session_state_.max_combo << " 本！";
    }

    int money_earned = session_state_.correct_count * config_.money_per_correct +
                       session_state_.corrected_count * config_.money_per_correct +
                       session_state_.max_combo * config_.money_bonus_for_combo;
    if (money_earned > 0) {
        oss << "获得 " << money_earned << " 金币。";
    }

    int total_correct = session_state_.correct_count + session_state_.corrected_count;
    int total_books = session_state_.total_scattered + session_state_.total_misplaced;
    if (total_books > 0 && total_correct >= total_books * 3 / 4) {
        oss << "管理员对你的工作很满意！";
    } else if (total_books > 0 && total_correct >= total_books / 2) {
        oss << "工作表现不错。";
    } else {
        oss << "还需要继续熟悉图书分类。";
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
    config.books_per_session = 10;
    config.correct_knowledge_reward = 5;
    config.correct_reputation_reward = 2;
    config.wrong_knowledge_penalty = -2;
    config.wrong_reputation_penalty = -1;
    config.stamina_cost = 15;
    config.base_mood_change = 5;
    config.correct_mood_bonus = 3;
    config.wrong_mood_penalty = -2;
    config.combo_reputation_bonus = 1;
    config.money_per_correct = 8;
    config.money_bonus_for_combo = 5;
    config.knowledge_threshold_for_map = 30;
    config.visits_threshold_for_map = 2;
    config.knowledge_threshold_for_borrow_card = 50;
    config.visits_threshold_for_borrow_card = 5;
    config.knowledge_threshold_for_close_friend = 80;
    config.visits_threshold_for_close_friend = 8;
    return config;
}

}  // namespace pixel_town::library
