#pragma once

#include <string>

#include <raylib.h>

#include "locations/library_rules.hpp"
#include "locations/library_scene.hpp"

namespace pixel_town::library::ui {

enum class LibrarySceneState {
    room_view,
    intro,
    npc_talk,
    plot_event,
    organizing,
    feedback,
    map_reveal,
    summary,
    exit,
};

struct FeedbackData {
    std::string book_title;
    std::string correct_category_name;
    std::string wrong_category_name;
    bool is_correct{false};
    int knowledge_reward{0};
};

struct LibraryUIState {
    LibrarySceneState scene_state{LibrarySceneState::room_view};
    bool show_instructions{true};
    std::string selected_shelf_id;
    bool last_answer_correct{false};
    int feedback_timer{0};
    std::string current_plot_event_id;
    FeedbackData feedback_data;
    float transition_progress{0.0F};
    bool is_transitioning{false};
    std::string clicked_npc_id;
    int selected_book_index{-1};
    bool is_holding_book{false};
    std::string held_book_title;
    std::string held_book_category;
    int hovered_shelf_index{-1};
    int hovered_book_index{-1};
    std::string highlighted_misplaced;
};

struct LibraryRenderConfig {
    int logical_width{960};
    int logical_height{540};
};

void draw_library_scene(const LibraryRuleEngine& engine, const LibraryUIState& ui_state,
                        const LibraryScene& scene, const LibraryRenderConfig& render_config, const Font& font, Vector2 logical_mouse);

void draw_library_room_scene(const LibraryScene& scene, const LibraryUIState& ui_state,
                             const LibraryRenderConfig& render_config, const Font& font, Vector2 logical_mouse);

void update_library_ui(LibraryRuleEngine& engine, LibraryUIState& ui_state, LibraryScene& scene);

[[nodiscard]] bool handle_library_input(LibraryRuleEngine& engine, LibraryUIState& ui_state, 
                                        LibraryScene& scene, const LibraryRenderConfig& render_config, Vector2 logical_mouse);

}  // namespace pixel_town::library::ui