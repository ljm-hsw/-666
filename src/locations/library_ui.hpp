// 图书馆 raylib UI 绘制接口：只读取 presentation 和输入状态。
#pragma once

#include <string>
#include <vector>

#include <raylib.h>

#include "locations/library_runtime_contract.hpp"

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
    bool return_to_answering_after_intro{false};
    std::string selected_category_id;
    bool last_answer_correct{false};
    int feedback_timer{0};
    bool show_hint{false};
    std::string current_plot_event_id;
    FeedbackData feedback_data;
};

inline void request_instruction_review(LibraryUIState& ui_state) {
    ui_state.return_to_answering_after_intro = true;
    ui_state.scene_state = LibrarySceneState::intro;
}

inline void advance_from_intro(LibraryUIState& ui_state) {
    if (ui_state.return_to_answering_after_intro) {
        ui_state.return_to_answering_after_intro = false;
        ui_state.scene_state = LibrarySceneState::answering;
        return;
    }
    ui_state.scene_state = LibrarySceneState::npc_talk;
}

struct LibraryRenderConfig {
    int logical_width{640};
    int logical_height{360};
    Texture2D background{};
    Texture2D old_map{};
};

[[nodiscard]] std::vector<std::string> split_utf8_codepoints(
    const std::string& text);

void draw_library_scene(const LibraryReaderPresentation& presentation,
                        const LibraryUIState& ui_state,
                        const LibraryRenderConfig& render_config, const Font& font,
                        Vector2 logical_mouse);

void update_library_ui(const LibraryReaderPresentation& presentation,
                       LibraryUIState& ui_state);

[[nodiscard]] LibraryIntent handle_library_input(
    const LibraryReaderPresentation& presentation, LibraryUIState& ui_state,
    Vector2 logical_mouse);

}  // namespace pixel_town::library::ui
