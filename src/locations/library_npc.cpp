#include "locations/library_npc.hpp"

#include <cmath>
#include <random>
#include <cinttypes>

namespace pixel_town::library {

namespace {

std::uint64_t xorshift64(std::uint64_t& seed) {
    std::uint64_t x = seed;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    seed = x;
    return x;
}

float random_float(std::uint64_t& seed) {
    return static_cast<float>(xorshift64(seed)) / static_cast<float>(UINT64_C(0xFFFFFFFFFFFFFFFF));
}

float distance(const Vector2& a, const Vector2& b) {
    const float dx = a.x - b.x;
    const float dy = a.y - b.y;
    return std::sqrt(dx * dx + dy * dy);
}

float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

Vector2 lerp(const Vector2& a, const Vector2& b, float t) {
    return {lerp(a.x, b.x, t), lerp(a.y, b.y, t)};
}

}  // namespace

NpcManager::NpcManager() = default;

void NpcManager::add_npc(const NpcData& npc_data) {
    npc_data_list_.push_back(npc_data);
    NpcState state;
    state.npc_id = npc_data.id;
    state.position = npc_data.patrol_area.bounds.width > 0 ? 
        Vector2{npc_data.patrol_area.bounds.x + npc_data.patrol_area.bounds.width / 2,
                npc_data.patrol_area.bounds.y + npc_data.patrol_area.bounds.height / 2} :
        Vector2{320.0F, 180.0F};
    state.target_position = state.position;
    state.is_moving = false;
    state.move_progress = 0.0F;
    state.move_timer = 0;
    npc_states_.push_back(state);
}

void NpcManager::remove_npc(const std::string& npc_id) {
    for (size_t i = 0; i < npc_data_list_.size(); ++i) {
        if (npc_data_list_[i].id == npc_id) {
            npc_data_list_.erase(npc_data_list_.begin() + i);
            npc_states_.erase(npc_states_.begin() + i);
            break;
        }
    }
}

const std::vector<NpcState>& NpcManager::get_npc_states() const {
    return npc_states_;
}

NpcState* NpcManager::get_npc_state(const std::string& npc_id) {
    for (auto& state : npc_states_) {
        if (state.npc_id == npc_id) {
            return &state;
        }
    }
    return nullptr;
}

const NpcData* NpcManager::get_npc_data(const std::string& npc_id) const {
    for (const auto& data : npc_data_list_) {
        if (data.id == npc_id) {
            return &data;
        }
    }
    return nullptr;
}

void NpcManager::update(float delta_time, const std::vector<Rectangle>& obstacles) {
    obstacles_ = obstacles;

    for (size_t i = 0; i < npc_states_.size(); ++i) {
        if (i >= npc_data_list_.size()) break;

        update_behavior(npc_states_[i], npc_data_list_[i]);
        update_npc_position(npc_states_[i], npc_data_list_[i], delta_time);
    }
}

void NpcManager::set_obstacles(const std::vector<Rectangle>& obstacles) {
    obstacles_ = obstacles;
}

bool NpcManager::is_npc_clicked(const Vector2& click_position, std::string& out_npc_id) const {
    for (size_t i = 0; i < npc_states_.size(); ++i) {
        const float dist = distance(click_position, npc_states_[i].position);
        if (dist <= npc_data_list_[i].interaction_radius) {
            out_npc_id = npc_states_[i].npc_id;
            return true;
        }
    }
    return false;
}

void NpcManager::update_npc_position(NpcState& state, const NpcData& data, float delta_time) {
    if (!state.is_moving) return;

    const float dist = distance(state.position, state.target_position);
    if (dist < 5.0F) {
        state.position = state.target_position;
        state.is_moving = false;
        state.move_progress = 0.0F;
        return;
    }

    const float move_speed = data.speed * delta_time * 60.0F;
    const float move_ratio = move_speed / dist;
    
    Vector2 new_position = lerp(state.position, state.target_position, move_ratio);

    if (check_obstacle_collision(new_position, obstacles_)) {
        state.is_moving = false;
        state.move_progress = 0.0F;
        return;
    }

    if (check_npc_collision(state, new_position)) {
        state.is_moving = false;
        state.move_progress = 0.0F;
        return;
    }

    state.position = new_position;
    state.move_progress += move_ratio;

    const float dx = state.target_position.x - state.position.x;
    const float dy = state.target_position.y - state.position.y;
    state.rotation = std::atan2(dy, dx);
}

void NpcManager::update_behavior(NpcState& state, const NpcData& data) {
    state.move_timer++;

    if (state.is_moving) return;

    if (state.move_timer < 120) return;
    state.move_timer = 0;

    switch (data.behavior) {
        case NpcBehavior::idle:
            break;
        case NpcBehavior::random_walk:
            generate_random_target(state, data);
            break;
        case NpcBehavior::patrol:
            generate_patrol_target(state, data);
            break;
        case NpcBehavior::follow_path:
            generate_patrol_target(state, data);
            break;
    }
}

void NpcManager::generate_random_target(NpcState& state, const NpcData& data) {
    const Rectangle& bounds = data.patrol_area.bounds;
    
    std::uint64_t seed = static_cast<std::uint64_t>(std::hash<std::string>{}(state.npc_id)) + 
                         static_cast<std::uint64_t>(state.move_timer);
    
    float target_x = bounds.x + random_float(seed) * bounds.width;
    float target_y = bounds.y + random_float(seed) * bounds.height;

    state.target_position = {target_x, target_y};
    state.is_moving = true;
    state.move_progress = 0.0F;
}

void NpcManager::generate_patrol_target(NpcState& state, const NpcData& data) {
    if (data.patrol_area.waypoints.empty()) {
        generate_random_target(state, data);
        return;
    }

    std::uint64_t seed = static_cast<std::uint64_t>(std::hash<std::string>{}(state.npc_id)) + 
                         static_cast<std::uint64_t>(state.move_timer);
    
    const size_t waypoint_index = static_cast<size_t>(random_float(seed) * data.patrol_area.waypoints.size());
    state.target_position = data.patrol_area.waypoints[waypoint_index];
    state.is_moving = true;
    state.move_progress = 0.0F;
}

CollisionResult NpcManager::check_collision(const Rectangle& a, const Rectangle& b) const {
    CollisionResult result;
    result.collided = false;

    const float a_right = a.x + a.width;
    const float a_bottom = a.y + a.height;
    const float b_right = b.x + b.width;
    const float b_bottom = b.y + b.height;

    if (a.x < b_right && a_right > b.x && a.y < b_bottom && a_bottom > b.y) {
        result.collided = true;

        const float overlap_x = std::min(a_right, b_right) - std::max(a.x, b.x);
        const float overlap_y = std::min(a_bottom, b_bottom) - std::max(a.y, b.y);

        if (overlap_x < overlap_y) {
            result.normal = {a.x < b.x ? -1.0F : 1.0F, 0.0F};
            result.penetration_depth = overlap_x;
        } else {
            result.normal = {0.0F, a.y < b.y ? -1.0F : 1.0F};
            result.penetration_depth = overlap_y;
        }
    }

    return result;
}

bool NpcManager::check_obstacle_collision(const Vector2& position, const std::vector<Rectangle>& obstacles) const {
    const float npc_radius = 15.0F;
    const Rectangle npc_rect = {position.x - npc_radius, position.y - npc_radius, 
                                npc_radius * 2, npc_radius * 2};

    for (const auto& obstacle : obstacles) {
        if (check_collision(npc_rect, obstacle).collided) {
            return true;
        }
    }
    return false;
}

bool NpcManager::check_npc_collision(const NpcState& state, const Vector2& new_position) const {
    const float npc_radius = 15.0F;
    const Rectangle npc_rect = {new_position.x - npc_radius, new_position.y - npc_radius, 
                                npc_radius * 2, npc_radius * 2};

    for (const auto& other : npc_states_) {
        if (other.npc_id == state.npc_id) continue;

        const Rectangle other_rect = {other.position.x - npc_radius, other.position.y - npc_radius, 
                                      npc_radius * 2, npc_radius * 2};

        if (check_collision(npc_rect, other_rect).collided) {
            return true;
        }
    }
    return false;
}

}  // namespace pixel_town::library