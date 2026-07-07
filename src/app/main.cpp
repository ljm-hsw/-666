#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <string>
#include <string_view>

#include <raylib.h>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#elif defined(__linux__)
#include <unistd.h>
#endif

#include "app/game_flow.hpp"
#include "app/visual_prototype.hpp"
#include "core/display_config.hpp"
#include "core/interaction_runtime.hpp"
#include "io/app_settings.hpp"
#include "io/demo_preset.hpp"
#include "io/resource_diagnostics.hpp"
#include "io/save_game.hpp"
#include "io/startup_log.hpp"

namespace {

constexpr int legacy_ui_width = 640;
constexpr int legacy_ui_height = 360;

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

std::filesystem::path system_executable_path() {
#ifdef _WIN32
    std::wstring buffer(260, L'\0');
    DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    while (length == buffer.size()) {
        buffer.resize(buffer.size() * 2);
        length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    }
    if (length == 0) {
        return {};
    }
    buffer.resize(length);
    return std::filesystem::path{buffer};
#elif defined(__APPLE__)
    std::uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);
    if (size == 0) {
        return {};
    }
    std::string buffer(size, '\0');
    if (_NSGetExecutablePath(buffer.data(), &size) != 0) {
        return {};
    }
    buffer.resize(std::strlen(buffer.c_str()));
    return std::filesystem::path{buffer};
#elif defined(__linux__)
    std::array<char, 4096> buffer{};
    const ssize_t length = readlink("/proc/self/exe", buffer.data(), buffer.size() - 1);
    if (length <= 0) {
        return {};
    }
    return std::filesystem::path{std::string{buffer.data(), static_cast<std::size_t>(length)}};
#else
    return {};
#endif
}

std::filesystem::path directory_from_executable_path(const std::filesystem::path& executable_path) {
    std::error_code ignored;
    if (executable_path.empty()) {
        return std::filesystem::current_path(ignored);
    }

    std::filesystem::path absolute_path = std::filesystem::weakly_canonical(executable_path, ignored);
    if (ignored || absolute_path.empty()) {
        absolute_path = std::filesystem::absolute(executable_path, ignored);
    }
    if (ignored || absolute_path.empty()) {
        return std::filesystem::current_path(ignored);
    }
    return absolute_path.parent_path();
}

std::filesystem::path application_directory_from_argv(const char* executable_path) {
    const std::filesystem::path system_path = system_executable_path();
    if (!system_path.empty()) {
        return directory_from_executable_path(system_path);
    }
    if (executable_path == nullptr || std::string_view{executable_path}.empty()) {
        return directory_from_executable_path({});
    }
    return directory_from_executable_path(std::filesystem::path{executable_path});
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

bool export_canvas_capture(RenderTexture2D canvas, const char* capture_path, int output_width,
                           int output_height) {
    Image image = LoadImageFromTexture(canvas.texture);
    if (image.data == nullptr) {
        return false;
    }
    ImageFlipVertical(&image);
    ImageResizeNN(&image, output_width, output_height);
    const bool exported = ExportImage(image, capture_path);
    UnloadImage(image);
    return exported;
}

bool is_save_boundary(const pixel_town::GameSession& session) {
    const pixel_town::GamePhase phase = session.phase();
    return phase == pixel_town::GamePhase::day_choice ||
           phase == pixel_town::GamePhase::night_choice ||
           phase == pixel_town::GamePhase::day_summary ||
           phase == pixel_town::GamePhase::ending;
}

bool same_snapshot(const pixel_town::GameSessionSnapshot& left,
                   const pixel_town::GameSessionSnapshot& right) {
    return left.day == right.day && left.seed == right.seed &&
           left.next_result_id == right.next_result_id &&
           left.active_result_id == right.active_result_id && left.phase == right.phase &&
           left.player.money == right.player.money &&
           left.player.stamina == right.player.stamina &&
           left.player.reputation == right.player.reputation &&
           left.player.knowledge == right.player.knowledge &&
           left.player.mood == right.player.mood &&
           left.has_pending_location == right.has_pending_location &&
           left.pending_location == right.pending_location &&
           left.location_started == right.location_started &&
           left.day_action_done == right.day_action_done &&
           left.night_action_done == right.night_action_done &&
           left.last_summary == right.last_summary && left.main_ending == right.main_ending &&
           left.final_summary == right.final_summary &&
           left.applied_result_ids == right.applied_result_ids;
}

}  // namespace

int main(int argc, char* argv[]) {
    constexpr auto display = pixel_town::default_display_config();
    constexpr float legacy_ui_scale =
        static_cast<float>(display.logical_width) / static_cast<float>(legacy_ui_width);
    static_assert(display.logical_width * legacy_ui_height ==
                      display.logical_height * legacy_ui_width,
                  "legacy UI scale requires the same 16:9 aspect ratio");
    const auto demo_args = pixel_town::parse_demo_preset_args(argc, argv);
    const bool capture_prototype =
        !demo_args.requested && argc == 2 && std::string_view{argv[1]} == "--capture-prototype";
    const bool capture_game_flow =
        !demo_args.requested && argc == 2 && std::string_view{argv[1]} == "--capture-game-flow";
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
    pixel_town::DemoPresetLoadResult demo_load;
    std::string launch_note;
    if (demo_args.requested && !demo_args.error.empty()) {
        launch_note = "demo_preset_error=" + demo_args.error;
    } else if (demo_args.requested) {
        demo_load = pixel_town::load_demo_preset("assets/data", demo_args.id);
        launch_note = "demo_preset=" + demo_args.id + "," + demo_load.message;
    }

    const bool demo_loaded =
        demo_args.requested && demo_load.status == pixel_town::DemoPresetStatus::ok;
    const std::string startup_stage =
        resources.can_start
            ? (demo_args.requested ? (demo_loaded ? "demo_preset" : "demo_preset_error")
                                   : (capture_prototype ? "visual_prototype" : "p1_game_flow"))
            : "resource_error";
    const bool log_written =
        pixel_town::write_latest_log("logs/latest.log", PIXEL_TOWN_VERSION, startup_stage, resources,
                                     launch_note);

    Texture2D kenney_tiles{};
    Texture2D generated_full_map_scene{};
    Texture2D generated_map_background{};
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
                "assets/textures/imagegen_backgrounds/town_map_full_scene.png")) {
            generated_full_map_scene =
                LoadTexture("assets/textures/imagegen_backgrounds/town_map_full_scene.png");
            if (generated_full_map_scene.id != 0) {
                SetTextureFilter(generated_full_map_scene, TEXTURE_FILTER_POINT);
            }
        }
        if (std::filesystem::is_regular_file(
                "assets/textures/imagegen_backgrounds/town_map_background.png")) {
            generated_map_background =
                LoadTexture("assets/textures/imagegen_backgrounds/town_map_background.png");
            if (generated_map_background.id != 0) {
                SetTextureFilter(generated_map_background, TEXTURE_FILTER_POINT);
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
    const bool persistence_enabled = !capture_prototype && !capture_game_flow && !demo_args.requested;
    const std::filesystem::path application_directory = application_directory_from_argv(argv[0]);
    const std::filesystem::path save_path = pixel_town::default_save_path(application_directory);
    const std::filesystem::path settings_path =
        pixel_town::default_settings_path(application_directory);
    pixel_town::InteractionRuntime interaction_runtime;
    if (const auto loaded_settings = pixel_town::load_app_settings(settings_path);
        loaded_settings.has_value()) {
        interaction_runtime.set_muted(loaded_settings->muted);
    }
    bool has_persisted_snapshot = false;
    pixel_town::GameSessionSnapshot persisted_snapshot{};
    if (demo_args.requested) {
        game_flow.save_present = false;
        if (!demo_args.error.empty()) {
            game_flow.notice = "演示参数错误：" + demo_args.error;
        } else if (demo_loaded) {
            game_flow.has_session = true;
            game_flow.session = demo_load.session;
            game_flow.notice = "已加载演示预设：" + demo_args.id + "。正式存档不会被读取或覆盖。";
        } else {
            game_flow.notice = "演示预设加载失败：" + demo_load.message;
        }
    } else if (persistence_enabled) {
        const auto loaded = pixel_town::load_session(save_path);
        if (loaded.status == pixel_town::SaveStatus::ok) {
            game_flow.has_session = true;
            game_flow.save_present = true;
            game_flow.session = loaded.session;
            game_flow.notice = "已恢复最近的阶段边界。";
            persisted_snapshot = game_flow.session.snapshot();
            has_persisted_snapshot = true;
        } else if (loaded.status == pixel_town::SaveStatus::not_found) {
            game_flow.save_present = false;
        } else {
            game_flow.save_present = true;
            game_flow.notice = loaded.status == pixel_town::SaveStatus::incompatible_version
                                   ? "存档版本不兼容，原文件已保留。"
                                   : "存档损坏或缺字段，原文件已保留。";
        }
    }
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

        const pixel_town::IntegerViewport viewport =
            pixel_town::integer_scaled_viewport(display, GetScreenWidth(), GetScreenHeight());
        const float width = static_cast<float>(viewport.width);
        const float height = static_cast<float>(viewport.height);
        const float offset_x = static_cast<float>(viewport.x);
        const float offset_y = static_cast<float>(viewport.y);
        const Vector2 screen_mouse = GetMousePosition();
        const auto logical_point = pixel_town::screen_to_logical_point(
            viewport, static_cast<int>(screen_mouse.x), static_cast<int>(screen_mouse.y));
        const Vector2 logical_mouse =
            logical_point.has_value()
                ? Vector2{static_cast<float>(logical_point->x), static_cast<float>(logical_point->y)}
                : Vector2{-1.0F, -1.0F};
        const Vector2 legacy_ui_mouse{logical_mouse.x / legacy_ui_scale,
                                      logical_mouse.y / legacy_ui_scale};
        const auto interaction_frame = interaction_runtime.update(pixel_town::InteractionFrameInput{
            GetFrameTime(), IsKeyPressed(KEY_P), IsKeyPressed(KEY_M), IsWindowFocused(),
            IsWindowState(FLAG_WINDOW_MINIMIZED)});
        if (interaction_frame.mute_toggled &&
            !pixel_town::save_app_settings(
                settings_path, pixel_town::AppSettings{interaction_runtime.muted()}) &&
            game_flow.has_session) {
            game_flow.notice = "设置写入失败：静音只在本次运行中生效。";
        }
        const bool audio_enabled = resources.audio_enabled && !interaction_runtime.muted();

        if (resources.can_start && log_written && !capture_game_flow &&
            interaction_frame.game_updates_enabled) {
            if (capture_prototype) {
                pixel_town::update_visual_prototype(prototype, legacy_ui_mouse);
            } else {
                pixel_town::update_game_flow(game_flow, legacy_ui_mouse);
                if (persistence_enabled && game_flow.has_session &&
                    is_save_boundary(game_flow.session)) {
                    const auto current_snapshot = game_flow.session.snapshot();
                    if (!has_persisted_snapshot ||
                        !same_snapshot(persisted_snapshot, current_snapshot)) {
                        const auto save_result =
                            pixel_town::save_session_atomic(save_path, game_flow.session);
                        if (save_result.status == pixel_town::SaveStatus::ok) {
                            persisted_snapshot = current_snapshot;
                            has_persisted_snapshot = true;
                            game_flow.save_present = true;
                        } else {
                            game_flow.notice = "存档写入失败：请检查发布目录权限。";
                        }
                    }
                }
            }
        }
        BeginTextureMode(canvas);
        if (resources.can_start && log_written) {
            const Camera2D legacy_ui_camera{Vector2{0.0F, 0.0F}, Vector2{0.0F, 0.0F}, 0.0F,
                                            legacy_ui_scale};
            BeginMode2D(legacy_ui_camera);
            if (capture_prototype) {
                pixel_town::draw_visual_prototype(ui_font, town_marker, kenney_tiles, prototype,
                                                  audio_enabled, legacy_ui_mouse);
            } else {
                pixel_town::draw_game_flow(ui_font, town_marker, kenney_tiles,
                                           generated_full_map_scene, generated_map_background,
                                           generated_buildings, game_flow, audio_enabled,
                                           interaction_runtime.paused(), legacy_ui_mouse);
            }
            EndMode2D();
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
            if (!export_canvas_capture(canvas, capture_path, display.window_width,
                                       display.window_height) ||
                !std::filesystem::is_regular_file(capture_path)) {
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
    if (generated_full_map_scene.id != 0) {
        UnloadTexture(generated_full_map_scene);
    }
    if (generated_map_background.id != 0) {
        UnloadTexture(generated_map_background);
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
