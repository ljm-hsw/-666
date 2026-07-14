#pragma once

#include <optional>
#include <string>
#include <vector>

#include "locations/library_organizing.hpp"
#include "locations/library_rules.hpp"

namespace pixel_town {

enum class LibraryRuntimeMode {
    selection,
    reader_consultation,
    book_organizing,
};

enum class LibraryIntentType {
    none,
    select_reader,
    select_organizing,
    pick_up_book,
    place_on_shelf,
    answer_category,
    finish_reader,
    abandon,
};

struct LibraryIntent {
    LibraryIntentType type{LibraryIntentType::none};
    std::string target_id;
};

struct LibraryOrganizingPresentation {
    std::vector<library::OrganizingBookTask> tasks;
    std::vector<library::OrganizingShelf> shelves;
    library::OrganizingSessionState state;
    std::string feedback;
};

struct LibraryReaderPresentation {
    library::SessionState state;
    std::optional<library::ReaderQuestion> current_question;
    std::vector<library::BookCategory> categories;
    library::NpcInteraction interaction;
    library::LibraryWorkResult result;
    std::string welcome_message;
    std::string work_intro;
    std::string introduction_dialogue;
    std::string old_map_reveal_dialogue;
    int correct_knowledge_reward{0};
    bool last_answer_correct{false};
    bool pending_plot_event{false};
    bool should_reveal_map{false};
};

struct LibraryPresentation {
    bool active{false};
    LibraryRuntimeMode mode{LibraryRuntimeMode::selection};
    std::string feedback;
    std::optional<LibraryReaderPresentation> reader;
    std::optional<LibraryOrganizingPresentation> organizing;
};

}  // namespace pixel_town
