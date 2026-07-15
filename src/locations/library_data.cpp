#include "locations/library_data.hpp"

#include <algorithm>
#include <charconv>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

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

bool parse_nonnegative_int(std::string_view text, int& value) {
    if (text.empty()) {
        return false;
    }
    const char* begin = text.data();
    const char* end = begin + text.size();
    const auto parsed = std::from_chars(begin, end, value);
    return parsed.ec == std::errc{} && parsed.ptr == end && value >= 0;
}

std::vector<std::string> split_fields(const std::string& value, char separator) {
    std::vector<std::string> fields;
    std::size_t start = 0;
    while (start <= value.size()) {
        const std::size_t end = value.find(separator, start);
        fields.push_back(trim(value.substr(start, end - start)));
        if (end == std::string::npos) {
            break;
        }
        start = end + 1;
    }
    return fields;
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
            const std::string current_category_id = trim(rest.substr(0, colon_pos));
            const std::string name = trim(rest.substr(colon_pos + 1));
            if (current_category_id.empty() || name.empty()) {
                result.error_message = "Empty category id or name: " + line;
                return result;
            }
            for (const auto& category : result.data.categories) {
                if (category.id == current_category_id) {
                    result.error_message = "Duplicate category id: " + current_category_id;
                    return result;
                }
            }
            result.data.categories.push_back({current_category_id, name, ""});
        } else if (starts_with(trimmed, "CATEGORY_DESC ")) {
            const std::string desc = trim(trimmed.substr(15));
            if (result.data.categories.empty()) {
                result.error_message = "Category description has no category: " + line;
                return result;
            }
            result.data.categories.back().description = desc;
        } else if (starts_with(trimmed, "ORGANIZING_SHELF ")) {
            const auto fields = split_fields(
                trimmed.substr(std::string{"ORGANIZING_SHELF "}.size()), '|');
            if (fields.size() != 7) {
                result.error_message = "Invalid organizing shelf format: " + line;
                return result;
            }
            OrganizingShelf shelf;
            shelf.id = fields[0];
            shelf.category_id = fields[1];
            shelf.name = fields[2];
            if (shelf.id.empty() || shelf.category_id.empty() || shelf.name.empty() ||
                !parse_nonnegative_int(fields[3], shelf.x) ||
                !parse_nonnegative_int(fields[4], shelf.y) ||
                !parse_nonnegative_int(fields[5], shelf.width) ||
                !parse_nonnegative_int(fields[6], shelf.height) || shelf.width == 0 ||
                shelf.height == 0) {
                result.error_message = "Invalid organizing shelf values: " + line;
                return result;
            }
            const auto duplicate = std::find_if(
                result.data.organizing_shelves.begin(), result.data.organizing_shelves.end(),
                [&shelf](const auto& existing) { return existing.id == shelf.id; });
            if (duplicate != result.data.organizing_shelves.end()) {
                result.error_message = "Duplicate organizing shelf id: " + shelf.id;
                return result;
            }
            result.data.organizing_shelves.push_back(std::move(shelf));
        } else if (starts_with(trimmed, "ORGANIZING_BOOK ")) {
            const auto fields = split_fields(
                trimmed.substr(std::string{"ORGANIZING_BOOK "}.size()), '|');
            if (fields.size() != 7) {
                result.error_message = "Invalid organizing book format: " + line;
                return result;
            }
            OrganizingBookTask task;
            task.id = fields[0];
            task.title = fields[1];
            task.category_id = fields[2];
            if (fields[3] == "scattered") {
                task.source = OrganizingBookSource::scattered;
            } else if (fields[3] == "misplaced") {
                task.source = OrganizingBookSource::misplaced;
            } else {
                result.error_message = "Invalid organizing book source: " + line;
                return result;
            }
            task.source_shelf_id = fields[4] == "-" ? "" : fields[4];
            if (task.id.empty() || task.title.empty() || task.category_id.empty() ||
                !parse_nonnegative_int(fields[5], task.x) ||
                !parse_nonnegative_int(fields[6], task.y)) {
                result.error_message = "Invalid organizing book values: " + line;
                return result;
            }
            const auto duplicate = std::find_if(
                result.data.organizing_tasks.begin(), result.data.organizing_tasks.end(),
                [&task](const auto& existing) { return existing.id == task.id; });
            if (duplicate != result.data.organizing_tasks.end()) {
                result.error_message = "Duplicate organizing book id: " + task.id;
                return result;
            }
            result.data.organizing_tasks.push_back(std::move(task));
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
            if (result.data.questions.empty()) {
                result.error_message = "Hint has no question: " + line;
                return result;
            }
            result.data.questions.back().hint = hint;
        } else if (starts_with(trimmed, "FEEDBACK_CORRECT ")) {
            const std::string feedback = trim(trimmed.substr(17));
            if (result.data.questions.empty()) {
                result.error_message = "Correct feedback has no question: " + line;
                return result;
            }
            result.data.questions.back().feedback_correct = feedback;
        } else if (starts_with(trimmed, "FEEDBACK_WRONG ")) {
            const std::string feedback = trim(trimmed.substr(15));
            if (result.data.questions.empty()) {
                result.error_message = "Wrong feedback has no question: " + line;
                return result;
            }
            result.data.questions.back().feedback_wrong = feedback;
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
            if (event_id.empty() || event_title.empty()) {
                result.error_message = "Empty plot event id or title: " + line;
                return result;
            }
            for (const auto& event : result.data.plot_events) {
                if (event.id == event_id) {
                    result.error_message = "Duplicate plot event id: " + event_id;
                    return result;
                }
            }
            result.data.plot_events.push_back({event_id, event_title, "", 0, 0, false});
        } else if (starts_with(trimmed, "PLOT_DESC ")) {
            const std::string desc = trim(trimmed.substr(10));
            if (result.data.plot_events.empty()) {
                result.error_message = "Plot description has no event: " + line;
                return result;
            }
            result.data.plot_events.back().description = desc;
        } else if (starts_with(trimmed, "PLOT_REQUIREMENT ")) {
            const std::string rest = trimmed.substr(17);
            const auto colon_pos = rest.find(':');
            if (result.data.plot_events.empty() || colon_pos == std::string::npos) {
                result.error_message = "Invalid plot requirement format: " + line;
                return result;
            }
            int required_visits = 0;
            int required_knowledge = 0;
            const std::string visits_text = trim(rest.substr(0, colon_pos));
            const std::string knowledge_text = trim(rest.substr(colon_pos + 1));
            if (!parse_nonnegative_int(visits_text, required_visits) ||
                !parse_nonnegative_int(knowledge_text, required_knowledge)) {
                result.error_message = "Invalid plot requirement values: " + line;
                return result;
            }
            auto& event = result.data.plot_events.back();
            event.required_visits = required_visits;
            event.required_knowledge = required_knowledge;
        } else if (starts_with(trimmed, "WELCOME ")) {
            result.data.welcome_message = trim(trimmed.substr(8));
        } else if (starts_with(trimmed, "WORK_INTRO ")) {
            result.data.work_intro = trim(trimmed.substr(11));
        } else {
            result.error_message = "Unknown library data directive: " + line;
            return result;
        }
    }

    if (result.data.categories.empty()) {
        result.error_message = "No categories defined in library data";
        return result;
    }

    if (result.data.questions.empty()) {
        result.error_message = "No questions defined in library data";
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

    const auto category_exists = [&result](const std::string& category_id) {
        return std::any_of(result.data.categories.begin(), result.data.categories.end(),
                           [&category_id](const auto& category) {
                               return category.id == category_id;
                           });
    };
    for (const auto& shelf : result.data.organizing_shelves) {
        if (!category_exists(shelf.category_id)) {
            result.error_message =
                "Organizing shelf references unknown category: " + shelf.category_id;
            return result;
        }
    }
    for (const auto& task : result.data.organizing_tasks) {
        if (!category_exists(task.category_id)) {
            result.error_message =
                "Organizing book references unknown category: " + task.category_id;
            return result;
        }
        if (task.source == OrganizingBookSource::misplaced) {
            const bool shelf_exists = std::any_of(
                result.data.organizing_shelves.begin(), result.data.organizing_shelves.end(),
                [&task](const auto& shelf) { return shelf.id == task.source_shelf_id; });
            if (!shelf_exists) {
                result.error_message =
                    "Misplaced organizing book references unknown shelf: " +
                    task.source_shelf_id;
                return result;
            }
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
    for (const auto& shelf : data.organizing_shelves) {
        result += shelf.name;
    }
    for (const auto& task : data.organizing_tasks) {
        result += task.title;
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
    for (const auto& event : data.plot_events) {
        result += event.title;
        result += event.description;
    }
    result += data.welcome_message;
    result += data.work_intro;
    return result;
}

}  // namespace pixel_town::library
