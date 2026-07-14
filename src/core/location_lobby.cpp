#include "core/location_lobby.hpp"

namespace pixel_town {
namespace {

constexpr float design_width = 640.0F;
constexpr float design_height = 360.0F;

float right(LobbyRect rect) {
    return rect.x + rect.width;
}

float bottom(LobbyRect rect) {
    return rect.y + rect.height;
}

bool valid_rect(LobbyRect rect) {
    return rect.width > 0.0F && rect.height > 0.0F && rect.x >= 0.0F &&
           rect.y >= 0.0F && right(rect) <= design_width &&
           bottom(rect) <= design_height;
}

const LocationLobbySpec& restaurant_lobby() {
    static const LocationLobbySpec spec{
        Location::restaurant,
        "餐馆大厅",
        "餐馆老板（预留）",
        "进入餐馆工作",
        LobbyRect{72.0F, 78.0F, 128.0F, 100.0F},
        LobbyRect{364.0F, 312.0F, 110.0F, 34.0F},
        LobbyRect{486.0F, 312.0F, 138.0F, 34.0F},
    };
    return spec;
}

const LocationLobbySpec& store_lobby() {
    static const LocationLobbySpec spec{
        Location::convenience_store,
        "便利店大厅",
        "便利店店主（预留）",
        "开始经营",
        LobbyRect{420.0F, 108.0F, 130.0F, 100.0F},
        LobbyRect{364.0F, 312.0F, 110.0F, 34.0F},
        LobbyRect{486.0F, 312.0F, 138.0F, 34.0F},
    };
    return spec;
}

const LocationLobbySpec& library_lobby() {
    static const LocationLobbySpec spec{
        Location::library,
        "图书馆大厅",
        "图书馆管理员",
        "开始图书馆工作",
        LobbyRect{48.0F, 62.0F, 160.0F, 100.0F},
        LobbyRect{364.0F, 312.0F, 110.0F, 34.0F},
        LobbyRect{486.0F, 312.0F, 138.0F, 34.0F},
    };
    return spec;
}

const LocationLobbySpec& home_lobby() {
    static const LocationLobbySpec spec{
        Location::home,
        "家",
        "访客互动位（预留）",
        "准备休息",
        LobbyRect{468.0F, 152.0F, 104.0F, 110.0F},
        LobbyRect{364.0F, 312.0F, 110.0F, 34.0F},
        LobbyRect{486.0F, 312.0F, 138.0F, 34.0F},
    };
    return spec;
}

}  // namespace

const LocationLobbySpec* find_location_lobby_spec(Location location) {
    switch (location) {
        case Location::restaurant:
            return &restaurant_lobby();
        case Location::convenience_store:
            return &store_lobby();
        case Location::library:
            return &library_lobby();
        case Location::home:
            return &home_lobby();
        case Location::tavern:
            return nullptr;
    }
    return nullptr;
}

bool lobby_rects_overlap(LobbyRect left, LobbyRect right_rect) noexcept {
    return left.x < right(right_rect) && right(left) > right_rect.x &&
           left.y < bottom(right_rect) && bottom(left) > right_rect.y;
}

std::vector<std::string> validate_location_lobby_spec(
    const LocationLobbySpec& spec) {
    std::vector<std::string> errors;
    if (spec.title.empty()) {
        errors.push_back("title must not be empty");
    }
    if (spec.npc_label.empty()) {
        errors.push_back("npc label must not be empty");
    }
    if (spec.action_label.empty()) {
        errors.push_back("action label must not be empty");
    }
    if (!valid_rect(spec.npc_hotspot)) {
        errors.push_back("npc hotspot must be inside the design canvas");
    }
    if (!valid_rect(spec.back_button)) {
        errors.push_back("back button must be inside the design canvas");
    }
    if (!valid_rect(spec.action_button)) {
        errors.push_back("action button must be inside the design canvas");
    }
    return errors;
}

}  // namespace pixel_town
