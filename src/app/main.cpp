#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <string>
#include <string_view>

#include <raylib.h>

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

}  // namespace

int main(int argc, char* argv[]) {
    constexpr auto display = pixel_town::default_display_config();
    const bool capture_prototype =
        argc == 2 && std::string_view{argv[1]} == "--capture-prototype";
    auto resources = pixel_town::validate_resources("assets", pixel_town::baseline_resource_manifest());

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(display.window_width, display.window_height, "Pixel Town: Ten-Day Plan - P0 Prototype");
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
        int codepoint_count = 0;
        int* codepoints = LoadCodepoints(pixel_town::visual_prototype_glyphs(), &codepoint_count);
        ui_font = LoadFontEx("assets/fonts/fusion-pixel-12px-proportional-zh_hans.ttf", 12,
                             codepoints, codepoint_count);
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
                 "font is missing required prototype glyphs", true});
        } else {
            SetTextureFilter(ui_font.texture, TEXTURE_FILTER_POINT);
        }
    }
    const std::string startup_stage = resources.can_start ? "visual_prototype" : "resource_error";
    const bool log_written =
        pixel_town::write_latest_log("logs/latest.log", PIXEL_TOWN_VERSION, startup_stage, resources);

    Texture2D tiny_farm_tiles{};
    const std::filesystem::path tiny_farm_tiles_path{
        "assets/textures/kenney_tiny_farm/Tilemap/tilemap_packed.png"};
    if (resources.can_start && std::filesystem::is_regular_file(tiny_farm_tiles_path)) {
        tiny_farm_tiles = LoadTexture(tiny_farm_tiles_path.string().c_str());
        if (tiny_farm_tiles.id != 0) {
            SetTextureFilter(tiny_farm_tiles, TEXTURE_FILTER_POINT);
        }
    }

    pixel_town::VisualPrototypeState prototype;
    bool capture_ready = capture_prototype;
    if (capture_prototype) {
        prototype.modal_open = false;
        std::error_code capture_error;
        std::filesystem::create_directories("prototype-captures", capture_error);
        capture_ready =
            !capture_error && std::filesystem::is_directory("prototype-captures", capture_error);
        if (!capture_ready) {
            TraceLog(LOG_WARNING, "CAPTURE: Could not create prototype-captures directory");
        }
    }
    const std::array<const char*, 4> capture_paths{
        "prototype-captures/variant-a.png",
        "prototype-captures/variant-b.png",
        "prototype-captures/variant-c.png",
        "prototype-captures/modal.png",
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

        if (resources.can_start && log_written) {
            pixel_town::update_visual_prototype(prototype, logical_mouse);
        }
        BeginTextureMode(canvas);
        if (resources.can_start && log_written) {
            pixel_town::draw_visual_prototype(ui_font, town_marker, tiny_farm_tiles, prototype,
                                              resources.audio_enabled, logical_mouse);
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

        if (capture_prototype && !capture_ready) {
            break;
        }
        if (capture_ready && ++capture_delay >= 3) {
            const char* capture_path = capture_paths[capture_index];
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
            } else if (capture_index == capture_paths.size()) {
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
    if (tiny_farm_tiles.id != 0) {
        UnloadTexture(tiny_farm_tiles);
    }
    if (canvas_loaded) {
        UnloadRenderTexture(canvas);
    }
    CloseWindow();
    return 0;
}
