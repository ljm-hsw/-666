#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace pixel_town::library {

struct BookCategory {
    std::string id;
    std::string name;
    std::string description;
};

struct ReaderQuestion {
    std::string question;
    std::string correct_category_id;
    std::string hint;
    std::string feedback_correct;
    std::string feedback_wrong;
};

struct NpcDialogue {
    std::string greeting;
    std::string introduction;
    std::string familiar_talk;
    std::string close_friend_talk;
    std::string old_map_reveal;
    std::string borrow_card;
    std::string map_gift;
    std::string daily_remark_morning;
    std::string daily_remark_afternoon;
};

struct PlotEvent {
    std::string id;
    std::string title;
    std::string description;
    int required_visits{0};
    int required_knowledge{0};
    bool has_triggered{false};
};

struct LibraryData {
    std::vector<BookCategory> categories;
    std::vector<ReaderQuestion> questions;
    NpcDialogue dialogue;
    std::vector<PlotEvent> plot_events;
    std::string welcome_message;
    std::string work_intro;
};

struct LoadResult {
    bool success{false};
    std::string error_message;
    LibraryData data;
};

[[nodiscard]] LoadResult load_library_data(const std::string& file_path);
[[nodiscard]] std::string collect_all_text_characters(const LibraryData& data);

}  // namespace pixel_town::library