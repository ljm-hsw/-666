#pragma once

#ifndef PIXEL_TOWN_DAY_LIMIT
#define PIXEL_TOWN_DAY_LIMIT 10
#endif

namespace pixel_town {

static_assert(PIXEL_TOWN_DAY_LIMIT == 5 || PIXEL_TOWN_DAY_LIMIT == 10,
              "PIXEL_TOWN_DAY_LIMIT must be 5 or 10");

[[nodiscard]] constexpr int configured_game_day_limit() noexcept {
    return PIXEL_TOWN_DAY_LIMIT;
}

[[nodiscard]] constexpr bool is_five_day_showcase_build() noexcept {
    return configured_game_day_limit() == 5;
}

[[nodiscard]] constexpr const char* game_plan_title() noexcept {
    return is_five_day_showcase_build() ? "像素小镇 · 五日展示版"
                                        : "像素小镇 · 十日经营计划";
}

[[nodiscard]] constexpr const char* game_plan_subtitle() noexcept {
    return is_five_day_showcase_build() ? "五日展示版本" : "十日经营计划";
}

[[nodiscard]] constexpr const char* game_plan_intro_notice() noexcept {
    return is_five_day_showcase_build() ? "镇长正在介绍五日展示计划。"
                                        : "镇长正在介绍十日计划。";
}

[[nodiscard]] constexpr const char* game_plan_completion_title() noexcept {
    return is_five_day_showcase_build() ? "五日展示完成" : "十日计划完成";
}

}  // namespace pixel_town
