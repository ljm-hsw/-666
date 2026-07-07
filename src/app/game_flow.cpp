#include "app/game_flow.hpp"

#include <array>
#include <string>

namespace pixel_town {
namespace {

constexpr Color ink{45, 52, 54, 255};
constexpr Color paper{250, 238, 203, 255};
constexpr Color cream{255, 248, 226, 255};
constexpr Color green{82, 137, 92, 255};
constexpr Color disabled{145, 143, 132, 255};
constexpr Color red{183, 83, 72, 255};
constexpr Color gold{224, 169, 74, 255};
constexpr Color shadow{39, 48, 53, 120};

constexpr std::array<Location, 5> map_locations{
    Location::restaurant, Location::convenience_store, Location::library, Location::tavern,
    Location::home};

constexpr std::array<const char*, 28> ui_texts{
    "像素小镇",
    "十日经营计划",
    "新游戏",
    "第 1 天",
    "第 2 天",
    "白天选择",
    "白天地点",
    "夜晚选择",
    "夜晚地点",
    "每日总结",
    "金钱",
    "体力",
    "声望",
    "知识",
    "心情",
    "餐馆",
    "便利店",
    "图书馆",
    "酒馆",
    "家",
    "进入地点",
    "返回地图",
    "开始模拟",
    "完成模拟",
    "主动放弃",
    "回家休息",
    "继续到下一天",
    "静音",
};

void text(const Font& font, const char* value, float x, float y, float size, Color color = ink) {
    DrawTextEx(font, value, Vector2{x, y}, size, 1.0F, color);
}

void text(const Font& font, const std::string& value, float x, float y, float size,
          Color color = ink) {
    text(font, value.c_str(), x, y, size, color);
}

void panel(Rectangle bounds, Color fill, Color border = ink) {
    DrawRectangleRec(Rectangle{bounds.x + 3, bounds.y + 3, bounds.width, bounds.height}, shadow);
    DrawRectangleRec(bounds, fill);
    DrawRectangleLinesEx(bounds, 2.0F, border);
}

bool clicked(Rectangle bounds, Vector2 mouse) {
    return CheckCollisionPointRec(mouse, bounds) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

bool activated(Rectangle bounds, Vector2 mouse, KeyboardKey key) {
    return clicked(bounds, mouse) || IsKeyPressed(key);
}

std::array<Rectangle, 5> location_bounds() {
    return {Rectangle{34, 122, 112, 52}, Rectangle{164, 122, 112, 52},
            Rectangle{294, 122, 112, 52}, Rectangle{424, 122, 112, 52},
            Rectangle{237, 198, 112, 52}};
}

Rectangle source_tile(int tile_index) {
    constexpr int columns = 12;
    constexpr int tile_size = 16;
    return Rectangle{static_cast<float>((tile_index % columns) * tile_size),
                     static_cast<float>((tile_index / columns) * tile_size),
                     tile_size, tile_size};
}

void draw_tile(const Texture2D& tiles, int tile_index, Rectangle destination) {
    if (tiles.id == 0) {
        return;
    }
    DrawTexturePro(tiles, source_tile(tile_index), destination, Vector2{0.0F, 0.0F}, 0.0F,
                   WHITE);
}

void draw_status(const Font& font, const GameSession& session, bool audio_enabled) {
    const auto& player = session.player();
    DrawRectangle(0, 0, 640, 48, Color{60, 79, 82, 255});
    text(font, "像素小镇", 16, 9, 18, RAYWHITE);
    text(font, std::string{"第 "} + std::to_string(session.day()) + " 天 · " +
                   phase_label(session.phase()),
         132, 14, 12, Color{255, 224, 154, 255});
    text(font,
         std::string{"金钱 "} + std::to_string(player.money) + "  体力 " +
             std::to_string(player.stamina) + "  声望 " + std::to_string(player.reputation),
         318, 6, 12, RAYWHITE);
    text(font,
         std::string{"知识 "} + std::to_string(player.knowledge) + "  心情 " +
             std::to_string(player.mood),
         318, 26, 12, RAYWHITE);
    if (!audio_enabled) {
        text(font, "静音", 590, 26, 12, Color{255, 208, 166, 255});
    }
}

void draw_title(const Font& font, Vector2 mouse) {
    ClearBackground(Color{37, 50, 57, 255});
    text(font, "像素小镇", 214, 78, 36, RAYWHITE);
    text(font, "十日经营计划", 226, 124, 18, Color{255, 224, 154, 255});
    const Rectangle start_button{244, 190, 152, 42};
    panel(start_button, CheckCollisionPointRec(mouse, start_button) ? cream : green);
    text(font, "新游戏", 294, 202, 14, CheckCollisionPointRec(mouse, start_button) ? ink : RAYWHITE);
    text(font, "Enter / 点击开始", 254, 254, 12, Color{205, 211, 215, 255});
}

void draw_map(const Font& font, const Texture2D& marker, const Texture2D& tiles,
              const GameAppState& state, bool audio_enabled, Vector2 mouse) {
    ClearBackground(Color{221, 211, 174, 255});
    draw_status(font, state.session, audio_enabled);
    panel(Rectangle{18, 62, 604, 230}, paper);
    DrawRectangle(28, 72, 584, 210, Color{144, 190, 119, 255});
    for (int x = 36; x < 600; x += 32) {
        draw_tile(tiles, 4, Rectangle{static_cast<float>(x), 82.0F, 16.0F, 16.0F});
    }
    DrawTextureEx(marker, Vector2{306, 162}, 0.0F, 2.0F, WHITE);

    const auto bounds = location_bounds();
    const std::array<Color, 5> colors{Color{231, 151, 103, 255}, gold,
                                      Color{161, 169, 196, 255}, Color{181, 122, 104, 255},
                                      Color{122, 176, 122, 255}};
    const std::array<int, 5> icons{84, 85, 96, 86, 72};
    for (std::size_t index = 0; index < map_locations.size(); ++index) {
        const auto permission = state.session.can_enter(map_locations[index]);
        const Rectangle button = bounds[index];
        const bool hovered = CheckCollisionPointRec(mouse, button);
        panel(button, permission.allowed ? (hovered ? cream : colors[index]) : disabled);
        text(font, location_label(map_locations[index]), button.x + 12, button.y + 9, 12,
             permission.allowed ? ink : Color{78, 78, 72, 255});
        text(font, permission.allowed ? "进入地点" : "不可进入", button.x + 12, button.y + 30, 12,
             permission.allowed ? ink : Color{78, 78, 72, 255});
        draw_tile(tiles, icons[index], Rectangle{button.x + 80, button.y + 10, 24, 24});
    }

    panel(Rectangle{28, 304, 584, 34}, Color{65, 91, 89, 245});
    text(font, state.notice, 42, 314, 12, RAYWHITE);
}

void draw_location(const Font& font, const GameAppState& state, Vector2 mouse) {
    ClearBackground(Color{215, 221, 194, 255});
    draw_status(font, state.session, true);
    panel(Rectangle{96, 78, 448, 210}, cream);
    const Location location = state.session.pending_location();
    text(font, location_label(location), 126, 106, 28, red);
    text(font,
         state.session.location_started() ? "地点已开始：完成模拟或主动放弃都会消耗本阶段。"
                                          : "尚未开始：现在返回地图不会消耗本阶段。",
         126, 154, 12, ink);

    const Rectangle back_button{126, 228, 112, 34};
    const Rectangle start_button{264, 228, 112, 34};
    const Rectangle abandon_button{402, 228, 112, 34};
    if (!state.session.location_started()) {
        panel(back_button, CheckCollisionPointRec(mouse, back_button) ? paper : Color{211, 202, 174, 255});
        text(font, "返回地图", 154, 238, 12, ink);
        panel(start_button, CheckCollisionPointRec(mouse, start_button) ? paper : green);
        text(font, "开始模拟", 292, 238, 12, RAYWHITE);
    } else {
        panel(start_button, CheckCollisionPointRec(mouse, start_button) ? paper : green);
        text(font, "完成模拟", 292, 238, 12, RAYWHITE);
        panel(abandon_button, CheckCollisionPointRec(mouse, abandon_button) ? paper : red);
        text(font, "主动放弃", 430, 238, 12, RAYWHITE);
    }
}

void draw_summary(const Font& font, const GameAppState& state, Vector2 mouse) {
    ClearBackground(Color{37, 50, 57, 255});
    draw_status(font, state.session, true);
    panel(Rectangle{90, 88, 460, 188}, cream);
    text(font, "每日总结", 124, 116, 24, red);
    text(font, state.session.last_summary(), 124, 162, 12, ink);
    text(font, "确认后进入下一游戏日。", 124, 190, 12, ink);
    const Rectangle next_button{242, 224, 156, 34};
    panel(next_button, CheckCollisionPointRec(mouse, next_button) ? paper : green);
    text(font, "继续到下一天", 280, 234, 12, RAYWHITE);
}

}  // namespace

const char* game_flow_glyphs() {
    static const std::string glyphs = [] {
        std::string result =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 "
            "·/：，。；“”+-";
        for (const char* value : ui_texts) {
            result += value;
        }
        result +=
            "点击开始第一天不可进入尚未地点已开始完成模拟或主动放弃都会消耗本阶段现在返回地图不会"
            "消耗本阶段今天的白天行动已经完成现在是白天回家休息只能在夜晚选择酒馆夜晚开放当前"
            "阶段不能进入今晚的行动已经完成酒馆玩法将在后续issue接入本切片先开放回家休息白天工作"
            "已经结束不能再进入该地点当前正在处理另一个阶段不能选择新地点餐馆模拟工作完成服务了"
            "午餐客流获得金钱与声望便利店模拟经营完成一次进货与销售结算图书馆帮助读者找书并提升"
            "知识行动完成回家休息恢复体力并结束今天主动放弃阶段已消耗本次无收益确认后进入下一游戏日";
        return result;
    }();
    return glyphs.c_str();
}

void update_game_flow(GameAppState& state, Vector2 logical_mouse) {
    const Rectangle start_button{244, 190, 152, 42};
    if (!state.has_session) {
        if (activated(start_button, logical_mouse, KEY_ENTER)) {
            state.has_session = true;
            state.session = GameSession::new_game();
            state.notice = "第 1 天开始：请选择一个白天工作地点。";
        }
        return;
    }

    if (state.session.phase() == GamePhase::day_choice ||
        state.session.phase() == GamePhase::night_choice) {
        const auto bounds = location_bounds();
        for (std::size_t index = 0; index < map_locations.size(); ++index) {
            if (!clicked(bounds[index], logical_mouse)) {
                continue;
            }
            const Location location = map_locations[index];
            const auto permission = state.session.can_enter(location);
            if (!permission.allowed) {
                state.notice = permission.reason;
                return;
            }
            if (location == Location::home) {
                const auto result = state.session.home_rest_result();
                const auto applied = state.session.apply_action_result(result);
                state.notice = applied.message;
                return;
            }
            if (state.session.enter_location(location)) {
                state.notice =
                    std::string{"已进入"} + location_label(location) + "，开始前可返回地图。";
            }
            return;
        }
        return;
    }

    if (state.session.phase() == GamePhase::day_location ||
        state.session.phase() == GamePhase::night_location) {
        const Rectangle back_button{126, 228, 112, 34};
        const Rectangle start_button_location{264, 228, 112, 34};
        const Rectangle abandon_button{402, 228, 112, 34};
        if (!state.session.location_started()) {
            if (activated(back_button, logical_mouse, KEY_ESCAPE)) {
                if (state.session.return_to_map()) {
                    state.notice = "已返回地图：阶段未消耗。";
                }
                return;
            }
            if (activated(start_button_location, logical_mouse, KEY_SPACE)) {
                if (state.session.start_location() != 0) {
                    state.notice = "地点已开始：完成或放弃都会消耗本阶段。";
                }
                return;
            }
        } else {
            if (activated(start_button_location, logical_mouse, KEY_SPACE)) {
                const auto applied =
                    state.session.apply_action_result(state.session.simulated_success_result());
                state.notice = applied.message;
                return;
            }
            if (clicked(abandon_button, logical_mouse)) {
                const auto applied =
                    state.session.apply_action_result(state.session.abandon_current_location());
                state.notice = applied.message;
                return;
            }
        }
        return;
    }

    if (state.session.phase() == GamePhase::day_summary) {
        const Rectangle next_button{242, 224, 156, 34};
        if (activated(next_button, logical_mouse, KEY_ENTER)) {
            if (state.session.finish_day_summary()) {
                state.notice = std::string{"第 "} + std::to_string(state.session.day()) +
                               " 天开始：请选择一个白天工作地点。";
            }
        }
    }
}

void draw_game_flow(const Font& font, const Texture2D& town_marker,
                    const Texture2D& kenney_tiles, const GameAppState& state,
                    bool audio_enabled, Vector2 logical_mouse) {
    if (!state.has_session) {
        draw_title(font, logical_mouse);
        return;
    }

    if (state.session.phase() == GamePhase::day_choice ||
        state.session.phase() == GamePhase::night_choice) {
        draw_map(font, town_marker, kenney_tiles, state, audio_enabled, logical_mouse);
    } else if (state.session.phase() == GamePhase::day_location ||
               state.session.phase() == GamePhase::night_location) {
        draw_location(font, state, logical_mouse);
    } else {
        draw_summary(font, state, logical_mouse);
    }

    if (!audio_enabled && state.has_session) {
        text(font, "静音", 590, 332, 12, red);
    }
}

}  // namespace pixel_town
