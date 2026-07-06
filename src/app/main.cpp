#include <algorithm>

#include <raylib.h>

#include "core/display_config.hpp"

namespace {

void draw_baseline_scene() {
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
    DrawText("PIXEL TOWN", 16, 18, 26, Color{48, 58, 64, 255});
    DrawText("DAY 1 / 10", 18, 50, 16, Color{48, 58, 64, 255});
}

}  // namespace

int main() {
    constexpr auto display = pixel_town::default_display_config();

    InitWindow(display.window_width, display.window_height, "Pixel Town: Ten-Day Plan");
    SetTargetFPS(60);

    RenderTexture2D canvas = LoadRenderTexture(display.logical_width, display.logical_height);
    SetTextureFilter(canvas.texture, TEXTURE_FILTER_POINT);

    while (!WindowShouldClose()) {
        BeginTextureMode(canvas);
        draw_baseline_scene();
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

    UnloadRenderTexture(canvas);
    CloseWindow();
    return 0;
}
