#pragma once

#include <string>

#include <raylib.h>

#include "locations/library_rules.hpp"

namespace pixel_town::library::ui {

enum class LibrarySceneState {
    intro,
    npc_talk,
    plot_event,
    answering,
    feedback,
    map_reveal,
    summary,
    exit,
};

struct FeedbackData {
    std::string question;
    std::string feedback_correct;
    std::string feedback_wrong;
    std::string correct_category_name;
    bool is_correct{false};
    int knowledge_reward{0};
};

struct LibraryUIState {
    LibrarySceneState scene_state{LibrarySceneState::intro};
    bool show_instructions{true};
    std::string selected_category_id;
    bool last_answer_correct{false};
    int feedback_timer{0};
    bool show_hint{false};
    std::string current_plot_event_id;
    FeedbackData feedback_data;
};

struct LibraryRenderConfig {
    int logical_width{640};
    int logical_height{360};
};

void draw_library_scene(const LibraryRuleEngine& engine, const LibraryUIState& ui_state,
                        const LibraryRenderConfig& render_config, const Font& font, Vector2 logical_mouse);

void update_library_ui(LibraryRuleEngine& engine, LibraryUIState& ui_state);

[[nodiscard]] bool handle_library_input(LibraryRuleEngine& engine, LibraryUIState& ui_state, Vector2 logical_mouse);

}  // namespace pixel_town::library::ui