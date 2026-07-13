#include "locations/library_data.hpp"

#include <fstream>
#include <sstream>
#include <string>

namespace pixel_town::library {

namespace {

std::string trim(const std::string& s) {
    const auto start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    const auto end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

bool starts_with(const std::string& s, const std::string& prefix) {
    return s.size() >= prefix.size() && s.substr(0, prefix.size()) == prefix;
}

}  // namespace

LoadResult load_library_data(const std::string& file_path) {
    LoadResult result;
    std::ifstream file(file_path);
    if (!file.is_open()) {
        result.error_message = "Could not open file: " + file_path;
        return result;
    }

    std::string line;
    std::string current_category_id;
    std::string current_question_id;

    while (std::getline(file, line)) {
        const std::string trimmed = trim(line);
        if (trimmed.empty() || starts_with(trimmed, "//")) {
            continue;
        }

        if (starts_with(trimmed, "CATEGORY ")) {
            const std::string rest = trimmed.substr(9);
            const auto colon_pos = rest.find(':');
            if (colon_pos == std::string::npos) {
                result.error_message = "Invalid category format: " + line;
                return result;
            }
            current_category_id = trim(rest.substr(0, colon_pos));
            const std::string name = trim(rest.substr(colon_pos + 1));
            if (current_category_id.empty() || name.empty()) {
                result.error_message = "Empty category id or name: " + line;
                return result;
            }
            result.data.categories.push_back({current_category_id, name, ""});
        } else if (starts_with(trimmed, "CATEGORY_DESC ")) {
            const std::string desc = trim(trimmed.substr(15));
            if (!result.data.categories.empty()) {
                result.data.categories.back().description = desc;
            }
        } else if (starts_with(trimmed, "QUESTION ")) {
            const std::string rest = trimmed.substr(9);
            const auto colon_pos = rest.find(':');
            if (colon_pos == std::string::npos) {
                result.error_message = "Invalid question format: " + line;
                return result;
            }
            const std::string category_id = trim(rest.substr(0, colon_pos));
            const std::string question = trim(rest.substr(colon_pos + 1));
            if (category_id.empty() || question.empty()) {
                result.error_message = "Empty category id or question: " + line;
                return result;
            }
            result.data.questions.push_back({question, category_id, "", "", ""});
        } else if (starts_with(trimmed, "HINT ")) {
            const std::string hint = trim(trimmed.substr(5));
            if (!result.data.questions.empty()) {
                result.data.questions.back().hint = hint;
            }
        } else if (starts_with(trimmed, "FEEDBACK_CORRECT ")) {
            const std::string feedback = trim(trimmed.substr(17));
            if (!result.data.questions.empty()) {
                result.data.questions.back().feedback_correct = feedback;
            }
        } else if (starts_with(trimmed, "FEEDBACK_WRONG ")) {
            const std::string feedback = trim(trimmed.substr(15));
            if (!result.data.questions.empty()) {
                result.data.questions.back().feedback_wrong = feedback;
            }
        } else if (starts_with(trimmed, "DIALOGUE_GREETING ")) {
            result.data.dialogue.greeting = trim(trimmed.substr(18));
        } else if (starts_with(trimmed, "DIALOGUE_INTRO ")) {
            result.data.dialogue.introduction = trim(trimmed.substr(14));
        } else if (starts_with(trimmed, "DIALOGUE_FAMILIAR ")) {
            result.data.dialogue.familiar_talk = trim(trimmed.substr(17));
        } else if (starts_with(trimmed, "DIALOGUE_CLOSE_FRIEND ")) {
            result.data.dialogue.close_friend_talk = trim(trimmed.substr(21));
        } else if (starts_with(trimmed, "DIALOGUE_MAP ")) {
            result.data.dialogue.old_map_reveal = trim(trimmed.substr(13));
        } else if (starts_with(trimmed, "DIALOGUE_BORROW ")) {
            result.data.dialogue.borrow_card = trim(trimmed.substr(15));
        } else if (starts_with(trimmed, "DIALOGUE_MAP_GIFT ")) {
            result.data.dialogue.map_gift = trim(trimmed.substr(18));
        } else if (starts_with(trimmed, "DIALOGUE_REMARK_MORNING ")) {
            result.data.dialogue.daily_remark_morning = trim(trimmed.substr(24));
        } else if (starts_with(trimmed, "DIALOGUE_REMARK_AFTERNOON ")) {
            result.data.dialogue.daily_remark_afternoon = trim(trimmed.substr(26));
        } else if (starts_with(trimmed, "PLOT_EVENT ")) {
            const std::string rest = trimmed.substr(11);
            const auto colon_pos = rest.find(':');
            if (colon_pos == std::string::npos) {
                result.error_message = "Invalid plot event format: " + line;
                return result;
            }
            std::string event_id = trim(rest.substr(0, colon_pos));
            std::string event_title = trim(rest.substr(colon_pos + 1));
            result.data.plot_events.push_back({event_id, event_title, "", 0, 0, false});
        } else if (starts_with(trimmed, "PLOT_DESC ")) {
            const std::string desc = trim(trimmed.substr(10));
            if (!result.data.plot_events.empty()) {
                result.data.plot_events.back().description = desc;
            }
        } else if (starts_with(trimmed, "PLOT_REQUIREMENT ")) {
            const std::string rest = trimmed.substr(17);
            if (!result.data.plot_events.empty()) {
                auto& event = result.data.plot_events.back();
                const auto colon_pos = rest.find(':');
                if (colon_pos != std::string::npos) {
                    event.required_visits = std::stoi(trim(rest.substr(0, colon_pos)));
                    event.required_knowledge = std::stoi(trim(rest.substr(colon_pos + 1)));
                }
            }
        } else if (starts_with(trimmed, "WELCOME ")) {
            result.data.welcome_message = trim(trimmed.substr(9));
        } else if (starts_with(trimmed, "WORK_INTRO ")) {
            result.data.work_intro = trim(trimmed.substr(11));
        } else if (starts_with(trimmed, "BOOK ")) {
            const std::string rest = trimmed.substr(5);
            const auto colon_pos = rest.find(':');
            if (colon_pos == std::string::npos) {
                result.error_message = "Invalid book format: " + line;
                return result;
            }
            const std::string category_id = trim(rest.substr(0, colon_pos));
            const std::string title = trim(rest.substr(colon_pos + 1));
            if (category_id.empty() || title.empty()) {
                result.error_message = "Empty category id or book title: " + line;
                return result;
            }
            result.data.books.push_back({"", title, category_id, false, false, false, 0, 0, 0, 0, ""});
        } else if (starts_with(trimmed, "SCATTERED ")) {
            const std::string rest = trimmed.substr(10);
            const auto colon_pos = rest.find(':');
            if (colon_pos == std::string::npos) {
                result.error_message = "Invalid scattered book format: " + line;
                return result;
            }
            std::string book_title = trim(rest.substr(0, colon_pos));
            const std::string coords = trim(rest.substr(colon_pos + 1));
            const auto comma_pos = coords.find(',');
            if (comma_pos == std::string::npos) {
                result.error_message = "Invalid scattered book coordinates: " + line;
                return result;
            }
            int x = std::stoi(trim(coords.substr(0, comma_pos)));
            int y = std::stoi(trim(coords.substr(comma_pos + 1)));
            for (auto& book : result.data.books) {
                if (book.title == book_title) {
                    book.is_scattered = true;
                    book.x = x;
                    book.y = y;
                    break;
                }
            }
        } else if (starts_with(trimmed, "MISPLACED ")) {
            const std::string rest = trimmed.substr(10);
            const auto colon_pos = rest.find(':');
            if (colon_pos == std::string::npos) {
                result.error_message = "Invalid misplaced book format: " + line;
                return result;
            }
            std::string book_title = trim(rest.substr(0, colon_pos));
            const std::string shelf_id = trim(rest.substr(colon_pos + 1));
            for (auto& book : result.data.books) {
                if (book.title == book_title) {
                    book.is_misplaced = true;
                    book.current_shelf_id = shelf_id;
                    break;
                }
            }
        } else if (starts_with(trimmed, "SHELF ")) {
            const std::string rest = trimmed.substr(6);
            const auto colon_pos = rest.find(':');
            if (colon_pos == std::string::npos) {
                result.error_message = "Invalid shelf format: " + line;
                return result;
            }
            const std::string category_id = trim(rest.substr(0, colon_pos));
            const std::string name = trim(rest.substr(colon_pos + 1));
            if (category_id.empty() || name.empty()) {
                result.error_message = "Empty category id or shelf name: " + line;
                return result;
            }
            result.data.shelves.push_back({"shelf_" + category_id, category_id, name, 0, 0, 8});
        } else if (starts_with(trimmed, "DIALOGUE_WORK_ASSIGN ")) {
            result.data.dialogue.work_assign = trim(trimmed.substr(20));
        } else if (starts_with(trimmed, "DIALOGUE_WORK_COMPLETE ")) {
            result.data.dialogue.work_complete = trim(trimmed.substr(22));
        } else if (starts_with(trimmed, "DIALOGUE_WORK_PRAISE ")) {
            result.data.dialogue.work_praise = trim(trimmed.substr(19));
        } else if (starts_with(trimmed, "DIALOGUE_WORK_CRITICIZE ")) {
            result.data.dialogue.work_criticize = trim(trimmed.substr(22));
        }
    }

    if (result.data.categories.empty()) {
        result.error_message = "No categories defined in library data";
        return result;
    }

    if (result.data.books.empty()) {
        result.error_message = "No books defined in library data";
        return result;
    }

    if (result.data.shelves.empty()) {
        result.error_message = "No shelves defined in library data";
        return result;
    }

    for (const auto& q : result.data.questions) {
        bool found = false;
        for (const auto& c : result.data.categories) {
            if (c.id == q.correct_category_id) {
                found = true;
                break;
            }
        }
        if (!found) {
            result.error_message = "Question references unknown category: " + q.correct_category_id;
            return result;
        }
    }

    result.success = true;
    return result;
}

std::string collect_all_text_characters(const LibraryData& data) {
    std::string result;
    for (const auto& cat : data.categories) {
        result += cat.name;
        result += cat.description;
    }
    for (const auto& book : data.books) {
        result += book.title;
    }
    for (const auto& shelf : data.shelves) {
        result += shelf.name;
    }
    for (const auto& q : data.questions) {
        result += q.question;
        result += q.hint;
        result += q.feedback_correct;
        result += q.feedback_wrong;
    }
    result += data.dialogue.greeting;
    result += data.dialogue.introduction;
    result += data.dialogue.familiar_talk;
    result += data.dialogue.close_friend_talk;
    result += data.dialogue.old_map_reveal;
    result += data.dialogue.borrow_card;
    result += data.dialogue.map_gift;
    result += data.dialogue.daily_remark_morning;
    result += data.dialogue.daily_remark_afternoon;
    result += data.dialogue.work_assign;
    result += data.dialogue.work_complete;
    result += data.dialogue.work_praise;
    result += data.dialogue.work_criticize;
    for (const auto& event : data.plot_events) {
        result += event.title;
        result += event.description;
    }
    result += data.welcome_message;
    result += data.work_intro;
    return result;
}

}  // namespace pixel_town::library