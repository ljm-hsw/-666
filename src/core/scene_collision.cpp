#include "core/scene_collision.hpp"

#include <algorithm>

namespace pixel_town {
namespace {

float right(SceneRect rect) {
    return rect.x + rect.width;
}

float bottom(SceneRect rect) {
    return rect.y + rect.height;
}

bool contains(SceneRect outer, SceneRect inner) {
    return inner.x >= outer.x && inner.y >= outer.y &&
           right(inner) <= right(outer) && bottom(inner) <= bottom(outer);
}

IndoorSceneLayout restaurant_layout() {
    return IndoorSceneLayout{
        Location::restaurant,
        960,
        540,
        SceneRect{12.0F, 78.0F, 936.0F, 442.0F},
        ScenePoint{330.0F, 466.0F},
        SceneRect{278.0F, 448.0F, 104.0F, 82.0F},
        {
            {"wall_top", {0.0F, 0.0F, 960.0F, 78.0F}},
            {"wall_left", {0.0F, 0.0F, 12.0F, 540.0F}},
            {"wall_right", {948.0F, 0.0F, 12.0F, 540.0F}},
            {"wall_bottom_left", {0.0F, 486.0F, 278.0F, 54.0F}},
            {"wall_bottom_right", {382.0F, 486.0F, 578.0F, 54.0F}},
            {"service_counter", {122.0F, 188.0F, 204.0F, 96.0F}},
            {"produce_display", {56.0F, 282.0F, 290.0F, 74.0F}},
            {"barrel_stack", {340.0F, 92.0F, 232.0F, 96.0F}},
            {"cold_counter", {664.0F, 124.0F, 188.0F, 128.0F}},
            {"center_table", {470.0F, 280.0F, 140.0F, 146.0F}},
            {"right_table", {748.0F, 280.0F, 154.0F, 146.0F}},
            {"umbrella_stand", {202.0F, 406.0F, 76.0F, 80.0F}},
        },
    };
}

IndoorSceneLayout home_layout() {
    return IndoorSceneLayout{
        Location::home,
        960,
        540,
        SceneRect{12.0F, 32.0F, 936.0F, 488.0F},
        ScenePoint{232.0F, 432.0F},
        SceneRect{178.0F, 404.0F, 110.0F, 126.0F},
        {
            {"wall_top", {0.0F, 0.0F, 960.0F, 32.0F}},
            {"wall_left", {0.0F, 0.0F, 12.0F, 540.0F}},
            {"wall_right", {948.0F, 0.0F, 12.0F, 540.0F}},
            {"wall_bottom_left", {0.0F, 454.0F, 178.0F, 86.0F}},
            {"wall_bottom_right", {288.0F, 454.0F, 672.0F, 86.0F}},
            {"bed_and_bedside", {16.0F, 62.0F, 184.0F, 158.0F}},
            {"wardrobe", {218.0F, 34.0F, 126.0F, 112.0F}},
            {"fireplace", {382.0F, 32.0F, 214.0F, 112.0F}},
            {"armchair", {336.0F, 116.0F, 84.0F, 104.0F}},
            {"center_table", {414.0F, 126.0F, 226.0F, 120.0F}},
            {"bookshelf", {762.0F, 34.0F, 182.0F, 114.0F}},
            {"writing_desk", {648.0F, 142.0F, 298.0F, 114.0F}},
            {"right_table", {726.0F, 270.0F, 116.0F, 138.0F}},
            {"sofa", {836.0F, 258.0F, 110.0F, 138.0F}},
        },
    };
}

IndoorSceneLayout convenience_store_layout() {
    return IndoorSceneLayout{
        Location::convenience_store,
        960,
        540,
        SceneRect{14.0F, 106.0F, 930.0F, 420.0F},
        ScenePoint{466.0F, 474.0F},
        SceneRect{394.0F, 400.0F, 142.0F, 140.0F},
        {
            {"wall_top", {0.0F, 0.0F, 960.0F, 106.0F}},
            {"wall_left", {0.0F, 0.0F, 14.0F, 540.0F}},
            {"wall_right", {944.0F, 0.0F, 16.0F, 540.0F}},
            {"wall_bottom_left", {0.0F, 452.0F, 394.0F, 88.0F}},
            {"wall_bottom_right", {536.0F, 452.0F, 424.0F, 88.0F}},
            {"stock_boxes", {38.0F, 48.0F, 274.0F, 112.0F}},
            {"shelf_left_top", {40.0F, 136.0F, 190.0F, 140.0F}},
            {"shelf_left_bottom", {40.0F, 272.0F, 190.0F, 148.0F}},
            {"shelf_middle", {326.0F, 132.0F, 116.0F, 292.0F}},
            {"basket_left_top", {228.0F, 184.0F, 54.0F, 92.0F}},
            {"basket_left_bottom", {228.0F, 338.0F, 54.0F, 86.0F}},
            {"freezer", {572.0F, 102.0F, 154.0F, 80.0F}},
            {"drink_cabinets", {726.0F, 50.0F, 202.0F, 134.0F}},
            {"basket_right", {878.0F, 182.0F, 54.0F, 88.0F}},
            {"checkout_counter", {604.0F, 286.0F, 340.0F, 106.0F}},
            {"umbrella_stand", {548.0F, 278.0F, 68.0F, 96.0F}},
        },
    };
}

IndoorSceneLayout tavern_layout() {
    return IndoorSceneLayout{
        Location::tavern,
        960,
        540,
        SceneRect{8.0F, 146.0F, 944.0F, 380.0F},
        ScenePoint{182.0F, 474.0F},
        SceneRect{136.0F, 402.0F, 92.0F, 138.0F},
        {
            {"wall_top", {0.0F, 0.0F, 960.0F, 146.0F}},
            {"wall_left", {0.0F, 0.0F, 8.0F, 540.0F}},
            {"wall_right", {952.0F, 0.0F, 8.0F, 540.0F}},
            {"wall_bottom_left", {0.0F, 454.0F, 136.0F, 54.0F}},
            {"wall_bottom_right", {228.0F, 454.0F, 732.0F, 54.0F}},
            {"bar_counter", {40.0F, 146.0F, 326.0F, 72.0F}},
            {"barrel_stack_left", {376.0F, 126.0F, 108.0F, 80.0F}},
            {"bottle_cabinet", {520.0F, 98.0F, 270.0F, 98.0F}},
            {"barrel_stack_right", {882.0F, 128.0F, 78.0F, 76.0F}},
            {"round_table_upper_left", {550.0F, 216.0F, 160.0F, 70.0F}},
            {"round_table_upper_right", {776.0F, 216.0F, 154.0F, 70.0F}},
            {"chess_table", {48.0F, 310.0F, 168.0F, 100.0F}},
            {"dice_table", {234.0F, 306.0F, 194.0F, 102.0F}},
            {"round_table_lower_left", {550.0F, 320.0F, 160.0F, 88.0F}},
            {"round_table_lower_right", {776.0F, 318.0F, 156.0F, 92.0F}},
            {"barrel_bottom_right", {896.0F, 404.0F, 64.0F, 58.0F}},
        },
    };
}

IndoorSceneLayout library_layout() {
    return IndoorSceneLayout{
        Location::library,
        960,
        540,
        SceneRect{30.0F, 80.0F, 900.0F, 430.0F},
        ScenePoint{118.0F, 430.0F},
        SceneRect{48.0F, 390.0F, 142.0F, 140.0F},
        {
            {"wall_top", {0.0F, 0.0F, 960.0F, 80.0F}},
            {"wall_left", {0.0F, 0.0F, 30.0F, 540.0F}},
            {"wall_right", {930.0F, 0.0F, 30.0F, 540.0F}},
            {"wall_bottom_left", {0.0F, 408.0F, 48.0F, 132.0F}},
            {"wall_bottom_right", {190.0F, 408.0F, 770.0F, 132.0F}},
            {"circulation_desk", {26.0F, 84.0F, 280.0F, 132.0F}},
            {"book_cart", {28.0F, 194.0F, 116.0F, 106.0F}},
            {"bookshelf_left", {510.0F, 80.0F, 96.0F, 180.0F}},
            {"bookshelf_middle", {626.0F, 80.0F, 100.0F, 180.0F}},
            {"bookshelf_right", {746.0F, 80.0F, 102.0F, 180.0F}},
            {"map_table", {86.0F, 302.0F, 238.0F, 96.0F}},
            {"reading_table_left", {380.0F, 294.0F, 314.0F, 114.0F}},
            {"reading_table_right", {704.0F, 294.0F, 222.0F, 114.0F}},
            {"right_plant", {906.0F, 92.0F, 44.0F, 60.0F}},
        },
    };
}

const IndoorSceneLayout& restaurant_scene() {
    static const IndoorSceneLayout layout = restaurant_layout();
    return layout;
}

const IndoorSceneLayout& home_scene() {
    static const IndoorSceneLayout layout = home_layout();
    return layout;
}

const IndoorSceneLayout& convenience_store_scene() {
    static const IndoorSceneLayout layout = convenience_store_layout();
    return layout;
}

const IndoorSceneLayout& tavern_scene() {
    static const IndoorSceneLayout layout = tavern_layout();
    return layout;
}

const IndoorSceneLayout& library_scene() {
    static const IndoorSceneLayout layout = library_layout();
    return layout;
}

}  // namespace

const IndoorSceneLayout* find_indoor_scene_layout(Location location) {
    switch (location) {
        case Location::restaurant:
            return &restaurant_scene();
        case Location::convenience_store:
            return &convenience_store_scene();
        case Location::home:
            return &home_scene();
        case Location::library:
            return &library_scene();
        case Location::tavern:
            return &tavern_scene();
    }
    return nullptr;
}

bool scene_rects_overlap(SceneRect left, SceneRect right_rect) noexcept {
    return left.x < right(right_rect) && right(left) > right_rect.x &&
           left.y < bottom(right_rect) && bottom(left) > right_rect.y;
}

bool scene_rect_contains(SceneRect rect, ScenePoint point) noexcept {
    return point.x >= rect.x && point.x <= right(rect) &&
           point.y >= rect.y && point.y <= bottom(rect);
}

SceneRect scene_actor_bounds(ScenePoint center, SceneSize size) noexcept {
    return SceneRect{center.x - size.width * 0.5F,
                     center.y - size.height * 0.5F,
                     size.width,
                     size.height};
}

bool scene_position_is_free(const IndoorSceneLayout& layout, ScenePoint center,
                            SceneSize size) noexcept {
    const SceneRect actor = scene_actor_bounds(center, size);
    if (!contains(layout.walkable_bounds, actor)) {
        return false;
    }
    for (const auto& collider : layout.static_colliders) {
        if (scene_rects_overlap(actor, collider.bounds)) {
            return false;
        }
    }
    return true;
}

ScenePoint move_with_scene_collisions(const IndoorSceneLayout& layout,
                                      ScenePoint current, ScenePoint delta,
                                      SceneSize size) noexcept {
    const float half_width = size.width * 0.5F;
    const float half_height = size.height * 0.5F;
    ScenePoint result = current;

    const float min_x = layout.walkable_bounds.x + half_width;
    const float max_x = right(layout.walkable_bounds) - half_width;
    float desired_x = std::clamp(current.x + delta.x, min_x, max_x);
    const SceneRect current_x_bounds = scene_actor_bounds(current, size);
    for (const auto& collider : layout.static_colliders) {
        const SceneRect obstacle = collider.bounds;
        const bool vertical_overlap =
            current_x_bounds.y < bottom(obstacle) &&
            bottom(current_x_bounds) > obstacle.y;
        if (!vertical_overlap) {
            continue;
        }
        if (delta.x > 0.0F && right(current_x_bounds) <= obstacle.x &&
            desired_x + half_width > obstacle.x) {
            desired_x = std::min(desired_x, obstacle.x - half_width);
        } else if (delta.x < 0.0F && current_x_bounds.x >= right(obstacle) &&
                   desired_x - half_width < right(obstacle)) {
            desired_x = std::max(desired_x, right(obstacle) + half_width);
        }
    }
    result.x = desired_x;

    const float min_y = layout.walkable_bounds.y + half_height;
    const float max_y = bottom(layout.walkable_bounds) - half_height;
    float desired_y = std::clamp(current.y + delta.y, min_y, max_y);
    const SceneRect current_y_bounds = scene_actor_bounds(result, size);
    for (const auto& collider : layout.static_colliders) {
        const SceneRect obstacle = collider.bounds;
        const bool horizontal_overlap =
            current_y_bounds.x < right(obstacle) &&
            right(current_y_bounds) > obstacle.x;
        if (!horizontal_overlap) {
            continue;
        }
        if (delta.y > 0.0F && bottom(current_y_bounds) <= obstacle.y &&
            desired_y + half_height > obstacle.y) {
            desired_y = std::min(desired_y, obstacle.y - half_height);
        } else if (delta.y < 0.0F && current_y_bounds.y >= bottom(obstacle) &&
                   desired_y - half_height < bottom(obstacle)) {
            desired_y = std::max(desired_y, bottom(obstacle) + half_height);
        }
    }
    result.y = desired_y;
    return result;
}

std::vector<std::string> validate_indoor_scene_layout(
    const IndoorSceneLayout& layout) {
    std::vector<std::string> errors;
    const SceneRect canvas{0.0F, 0.0F, static_cast<float>(layout.canvas_width),
                           static_cast<float>(layout.canvas_height)};
    if (layout.canvas_width <= 0 || layout.canvas_height <= 0) {
        errors.push_back("canvas dimensions must be positive");
    }
    if (layout.walkable_bounds.width <= 0.0F ||
        layout.walkable_bounds.height <= 0.0F ||
        !contains(canvas, layout.walkable_bounds)) {
        errors.push_back("walkable bounds must be positive and inside the canvas");
    }
    if (layout.exit_trigger.width <= 0.0F || layout.exit_trigger.height <= 0.0F ||
        !contains(canvas, layout.exit_trigger)) {
        errors.push_back("exit trigger must be positive and inside the canvas");
    }
    for (const auto& collider : layout.static_colliders) {
        if (collider.id.empty()) {
            errors.push_back("collider id must not be empty");
        }
        if (collider.bounds.width <= 0.0F || collider.bounds.height <= 0.0F ||
            !contains(canvas, collider.bounds)) {
            errors.push_back("collider must be positive and inside the canvas: " +
                             collider.id);
        }
    }
    if (!scene_position_is_free(layout, layout.player_spawn,
                                SceneSize{24.0F, 24.0F})) {
        errors.push_back("player spawn must be free for a 24x24 actor");
    }
    return errors;
}

}  // namespace pixel_town
