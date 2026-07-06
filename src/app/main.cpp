#include <algorithm>
#include <string>

#include <raylib.h>

#include "core/display_config.hpp"
#include "io/resource_diagnostics.hpp"
#include "io/startup_log.hpp"

namespace {

void draw_baseline_scene(const Texture2D& town_marker, bool audio_enabled) {
    constexpr Color sky{127, 195, 219, 255};
    constexpr Color grass{102, 168, 89, 255};
    constexpr Color road{104, 105, 112, 255};
    constexpr Color wall{238, 199, 126, 255};
    constexpr Color roof{169, 70, 64, 255};
    constexpr Color window{91, 155, 213, 255};

    ClearBackground(sky);
    DrawCircle(545, 65, 28.0F, Color{255, 226, 118, 255});
    DrawRectangle(0, 205, 640, 155, grass);
    DrawRectangle(0, 275, 640, 85, road);
    DrawRectangle(0, 294, 640, 4, Color{229, 210, 142, 255});
    DrawRectangle(0, 332, 640, 4, Color{229, 210, 142, 255});

    DrawTriangle(Vector2{70.0F, 145.0F}, Vector2{205.0F, 145.0F}, Vector2{137.0F, 82.0F}, roof);
    DrawRectangle(82, 145, 112, 104, wall);
    DrawRectangle(126, 198, 25, 51, Color{112, 73, 61, 255});
    DrawRectangle(96, 165, 22, 22, window);
    DrawRectangle(159, 165, 22, 22, window);

    DrawRectangle(362, 151, 158, 98, Color{216, 176, 103, 255});
    DrawRectangle(350, 137, 182, 19, Color{89, 119, 80, 255});
    DrawRectangle(378, 181, 34, 68, Color{112, 73, 61, 255});
    DrawRectangle(438, 174, 59, 37, window);
    DrawText("STORE", 428, 113, 20, Color{55, 63, 67, 255});

    DrawRectangle(263, 172, 12, 78, Color{101, 71, 51, 255});
    DrawCircle(269, 150, 35.0F, Color{54, 127, 68, 255});
    DrawTextureEx(town_marker, Vector2{249.0F, 126.0F}, 0.0F, 4.0F, WHITE);
    DrawText("PIXEL TOWN", 16, 18, 26, Color{48, 58, 64, 255});
    DrawText("DAY 1 / 10", 18, 50, 16, Color{48, 58, 64, 255});
    if (!audio_enabled) {
        DrawText("AUDIO: MUTED", 508, 338, 10, Color{220, 220, 220, 255});
    }
}

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

int main() {
    constexpr auto display = pixel_town::default_display_config();
    auto resources = pixel_town::validate_resources("assets", pixel_town::baseline_resource_manifest());

    InitWindow(display.window_width, display.window_height, "Pixel Town: Ten-Day Plan");
    SetTargetFPS(60);

    RenderTexture2D canvas = LoadRenderTexture(display.logical_width, display.logical_height);
    SetTextureFilter(canvas.texture, TEXTURE_FILTER_POINT);
    Texture2D town_marker{};
    if (resources.can_start) {
        town_marker = LoadTexture("assets/textures/town_marker.png");
        if (town_marker.id == 0) {
            resources.can_start = false;
            resources.issues.push_back(
                {"textures/town_marker.png", "raylib could not load resource", true});
        } else {
            SetTextureFilter(town_marker, TEXTURE_FILTER_POINT);
        }
    }
    const std::string startup_stage = resources.can_start ? "baseline_scene" : "resource_error";
    const bool log_written =
        pixel_town::write_latest_log("logs/latest.log", PIXEL_TOWN_VERSION, startup_stage, resources);

    while (!WindowShouldClose()) {
        BeginTextureMode(canvas);
        if (resources.can_start && log_written) {
            draw_baseline_scene(town_marker, resources.audio_enabled);
        } else {
            draw_resource_error(resources, log_written);
        }
        EndTextureMode();

        const float scale = std::min(
            static_cast<float>(GetScreenWidth()) / static_cast<float>(display.logical_width),
            static_cast<float>(GetScreenHeight()) / static_cast<float>(display.logical_height));
        const float width = static_cast<float>(display.logical_width) * scale;
        const float height = static_cast<float>(display.logical_height) * scale;

        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexturePro(
            canvas.texture,
            Rectangle{0.0F, 0.0F, static_cast<float>(display.logical_width),
                      -static_cast<float>(display.logical_height)},
            Rectangle{(static_cast<float>(GetScreenWidth()) - width) / 2.0F,
                      (static_cast<float>(GetScreenHeight()) - height) / 2.0F, width, height},
            Vector2{0.0F, 0.0F}, 0.0F, WHITE);
        EndDrawing();
    }

    if (town_marker.id != 0) {
        UnloadTexture(town_marker);
    }
    UnloadRenderTexture(canvas);
    CloseWindow();
    return 0;
}
