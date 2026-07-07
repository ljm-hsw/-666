#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <string>
#include <string_view>

#include <raylib.h>

#include "app/game_flow.hpp"
#include "app/visual_prototype.hpp"
#include "core/display_config.hpp"
#include "io/resource_diagnostics.hpp"
#include "io/startup_log.hpp"

namespace {

void draw_resource_error(const pixel_town::ResourceReport& report, bool log_written) {
    ClearBackground(Color{47, 30, 35, 255});
    DrawText("PIXEL TOWN COULD NOT START", 34, 34, 24, Color{255, 205, 122, 255});
    DrawText("Required resources are missing or invalid.", 34, 76, 18, RAYWHITE);

    int line_y = 116;
    for (const pixel_town::ResourceIssue& issue : report.issues) {
        if (!issue.required) {
            continue;
        }
        const std::string line = "- " + issue.relative_path.generic_string() + ": " + issue.message;
        DrawText(line.c_str(), 46, line_y, 16, Color{255, 154, 142, 255});
        line_y += 24;
    }

    if (!log_written) {
        DrawText("- logs/latest.log: could not write startup log", 46, line_y, 16,
                 Color{255, 154, 142, 255});
        line_y += 24;
    }
    DrawText("Restore the listed files, then restart the application.", 34, line_y + 24, 16,
             Color{205, 211, 215, 255});
}

bool complete_day_with_rest(pixel_town::GameSession& session, pixel_town::Location location) {
    if (!session.enter_location(location)) {
        return false;
    }
    if (session.start_location() == 0) {
        return false;
    }
    if (!session.apply_action_result(session.simulated_success_result()).accepted) {
        return false;
    }
    if (!session.apply_action_result(session.home_rest_result()).accepted) {
        return false;
    }
    return session.finish_day_summary();
}

void advance_to_placeholder_ending(pixel_town::GameAppState& state) {
    state.has_session = true;
    state.session = pixel_town::GameSession::new_game();
    for (int day = 1; day <= 10 && !state.session.is_ended(); ++day) {
        if (!complete_day_with_rest(state.session, pixel_town::Location::restaurant)) {
            break;
        }
    }
    state.notice = "十日计划完成。";
}

}  // namespace

int main(int argc, char* argv[]) {
    constexpr auto display = pixel_town::default_display_config();
    const bool capture_prototype =
        argc == 2 && std::string_view{argv[1]} == "--capture-prototype";
    const bool capture_game_flow =
        argc == 2 && std::string_view{argv[1]} == "--capture-game-flow";
    auto resources = pixel_town::validate_resources("assets", pixel_town::baseline_resource_manifest());

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(display.window_width, display.window_height, "Pixel Town: Ten-Day Plan");
    SetWindowMinSize(display.logical_width, display.logical_height);
    SetTargetFPS(60);

    RenderTexture2D canvas = LoadRenderTexture(display.logical_width, display.logical_height);
    const bool canvas_loaded = canvas.texture.id != 0;
    if (canvas_loaded) {
        SetTextureFilter(canvas.texture, TEXTURE_FILTER_POINT);
    } else {
        resources.can_start = false;
        resources.issues.push_back(
            {"render_texture", "raylib could not create render target", true});
    }
    Texture2D town_marker{};
    Font ui_font{};
    if (resources.can_start) {
        town_marker = LoadTexture("assets/textures/town_marker.png");
        if (town_marker.id == 0) {
            resources.can_start = false;
            resources.issues.push_back(
                {"textures/town_marker.png", "raylib could not load resource", true});
        } else {
            SetTextureFilter(town_marker, TEXTURE_FILTER_POINT);
        }
        const std::string required_glyphs =
            std::string{pixel_town::visual_prototype_glyphs()} + pixel_town::game_flow_glyphs();
        int codepoint_count = 0;
        int* codepoints = LoadCodepoints(required_glyphs.c_str(), &codepoint_count);
        const int font_pixel_size = capture_prototype ? 12 : 20;
        ui_font = LoadFontEx("assets/fonts/fusion-pixel-12px-proportional-zh_hans.ttf",
                             font_pixel_size, codepoints, codepoint_count);
        bool required_glyphs_available = ui_font.texture.id != 0;
        if (required_glyphs_available) {
            for (int index = 0; index < codepoint_count; ++index) {
                const int glyph_index = GetGlyphIndex(ui_font, codepoints[index]);
                if (ui_font.glyphs[glyph_index].value != codepoints[index]) {
                    TraceLog(LOG_WARNING, "FONT: Missing required prototype glyph U+%04X",
                             codepoints[index]);
                    required_glyphs_available = false;
                }
            }
        }
        UnloadCodepoints(codepoints);
        if (!required_glyphs_available) {
            resources.can_start = false;
            resources.issues.push_back(
                {"fonts/fusion-pixel-12px-proportional-zh_hans.ttf",
                 "font is missing required game flow glyphs", true});
        } else {
            SetTextureFilter(ui_font.texture, TEXTURE_FILTER_POINT);
        }
    }
    const std::string startup_stage =
        resources.can_start ? (capture_prototype ? "visual_prototype" : "p1_game_flow")
                            : "resource_error";
    const bool log_written =
        pixel_town::write_latest_log("logs/latest.log", PIXEL_TOWN_VERSION, startup_stage, resources);

    Texture2D kenney_tiles{};
    Texture2D generated_buildings{};
    const std::array<std::filesystem::path, 2> kenney_tiles_paths{
        "assets/textures/kenney_tiny_town/Tilemap/tilemap_packed.png",
        "assets/textures/kenney_tiny_farm/Tilemap/tilemap_packed.png",
    };
    if (resources.can_start) {
        for (const std::filesystem::path& tiles_path : kenney_tiles_paths) {
            if (!std::filesystem::is_regular_file(tiles_path)) {
                continue;
            }
            kenney_tiles = LoadTexture(tiles_path.string().c_str());
            if (kenney_tiles.id != 0) {
                SetTextureFilter(kenney_tiles, TEXTURE_FILTER_POINT);
                break;
            }
        }
        if (std::filesystem::is_regular_file(
                "assets/textures/imagegen_buildings/p1_building_sprites.png")) {
            generated_buildings =
                LoadTexture("assets/textures/imagegen_buildings/p1_building_sprites.png");
            if (generated_buildings.id != 0) {
                SetTextureFilter(generated_buildings, TEXTURE_FILTER_POINT);
            }
        }
    }

    pixel_town::VisualPrototypeState prototype;
    pixel_town::GameAppState game_flow;
    bool capture_ready = capture_prototype || capture_game_flow;
    if (capture_ready) {
        prototype.modal_open = false;
        std::error_code capture_error;
        const std::filesystem::path capture_directory =
            capture_prototype ? "prototype-captures" : "game-flow-captures";
        std::filesystem::create_directories(capture_directory, capture_error);
        capture_ready =
            !capture_error && std::filesystem::is_directory(capture_directory, capture_error);
        if (!capture_ready) {
            TraceLog(LOG_WARNING, "CAPTURE: Could not create capture directory");
        }
    }
    const std::array<const char*, 4> prototype_capture_paths{
        "prototype-captures/variant-a.png",
        "prototype-captures/variant-b.png",
        "prototype-captures/variant-c.png",
        "prototype-captures/modal.png",
    };
    const std::array<const char*, 3> game_flow_capture_paths{
        "game-flow-captures/title.png",
        "game-flow-captures/map.png",
        "game-flow-captures/ending.png",
    };
    std::size_t capture_index = 0;
    int capture_delay = 0;
    while (!WindowShouldClose()) {
        if (!canvas_loaded) {
            BeginDrawing();
            draw_resource_error(resources, log_written);
            EndDrawing();
            if (capture_prototype) {
                break;
            }
            continue;
        }

        const float scale = std::max(
            1.0F,
            std::floor(std::min(
                static_cast<float>(GetScreenWidth()) / static_cast<float>(display.logical_width),
                static_cast<float>(GetScreenHeight()) / static_cast<float>(display.logical_height))));
        const float width = static_cast<float>(display.logical_width) * scale;
        const float height = static_cast<float>(display.logical_height) * scale;
        const float offset_x = (static_cast<float>(GetScreenWidth()) - width) / 2.0F;
        const float offset_y = (static_cast<float>(GetScreenHeight()) - height) / 2.0F;
        const Vector2 screen_mouse = GetMousePosition();
        const Vector2 logical_mouse{(screen_mouse.x - offset_x) / scale,
                                    (screen_mouse.y - offset_y) / scale};

        if (resources.can_start && log_written && !capture_game_flow) {
            if (capture_prototype) {
                pixel_town::update_visual_prototype(prototype, logical_mouse);
            } else {
                pixel_town::update_game_flow(game_flow, logical_mouse);
            }
        }
        BeginTextureMode(canvas);
        if (resources.can_start && log_written) {
            if (capture_prototype) {
                pixel_town::draw_visual_prototype(ui_font, town_marker, kenney_tiles, prototype,
                                                  resources.audio_enabled, logical_mouse);
            } else {
                pixel_town::draw_game_flow(ui_font, town_marker, kenney_tiles, generated_buildings,
                                           game_flow, resources.audio_enabled, logical_mouse);
            }
        } else {
            draw_resource_error(resources, log_written);
        }
        EndTextureMode();

        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexturePro(
            canvas.texture,
            Rectangle{0.0F, 0.0F, static_cast<float>(display.logical_width),
                      -static_cast<float>(display.logical_height)},
            Rectangle{offset_x, offset_y, width, height},
            Vector2{0.0F, 0.0F}, 0.0F, WHITE);
        EndDrawing();

        if ((capture_prototype || capture_game_flow) && !capture_ready) {
            break;
        }
        if (capture_ready && ++capture_delay >= 3) {
            const char* capture_path = capture_prototype ? prototype_capture_paths[capture_index]
                                                         : game_flow_capture_paths[capture_index];
            TakeScreenshot(capture_path);
            if (!std::filesystem::is_regular_file(capture_path)) {
                TraceLog(LOG_WARNING, "CAPTURE: Screenshot was not written: %s", capture_path);
            }
            ++capture_index;
            capture_delay = 0;
            if (capture_index == 1) {
                prototype.variant = 1;
            } else if (capture_index == 2) {
                prototype.variant = 2;
            } else if (capture_index == 3) {
                prototype.variant = 0;
                prototype.modal_open = true;
            }
            if (capture_game_flow && capture_index == 1) {
                game_flow.has_session = true;
                game_flow.session = pixel_town::GameSession::new_game();
                game_flow.notice = "第 1 天开始：请选择一个白天工作地点。";
            } else if (capture_game_flow && capture_index == 2) {
                advance_to_placeholder_ending(game_flow);
            }
            const std::size_t capture_count =
                capture_prototype ? prototype_capture_paths.size() : game_flow_capture_paths.size();
            if (capture_index == capture_count) {
                break;
            }
        }
    }

    if (town_marker.id != 0) {
        UnloadTexture(town_marker);
    }
    if (ui_font.texture.id != 0) {
        UnloadFont(ui_font);
    }
    if (kenney_tiles.id != 0) {
        UnloadTexture(kenney_tiles);
    }
    if (generated_buildings.id != 0) {
        UnloadTexture(generated_buildings);
    }
    if (canvas_loaded) {
        UnloadRenderTexture(canvas);
    }
    CloseWindow();
    return 0;
}
