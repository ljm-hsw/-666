#include "locations/library_scene.hpp"
#include <cmath>

namespace pixel_town::library {

void NpcManager::add_npc(const NpcData& data) {
    npcs_.push_back(data);
    NpcState state;
    state.npc_id = data.id;
    state.position = data.position;
    npc_states_.push_back(state);
}

const NpcData* NpcManager::get_npc_data(const std::string& id) const {
    for (const auto& npc : npcs_) {
        if (npc.id == id) {
            return &npc;
        }
    }
    return nullptr;
}

const std::vector<NpcState>& NpcManager::get_npc_states() const {
    return npc_states_;
}

std::vector<NpcState>& NpcManager::get_npc_states() {
    return npc_states_;
}

void NpcManager::update(float delta_time) {
    for (auto& state : npc_states_) {
        const auto* data = get_npc_data(state.npc_id);
        if (!data || data->waypoints.empty()) {
            continue;
        }

        if (state.current_waypoint >= static_cast<int>(data->waypoints.size())) {
            state.current_waypoint = 0;
        }

        Vector2 target = data->waypoints[state.current_waypoint];
        float dx = target.x - state.position.x;
        float dy = target.y - state.position.y;
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist < 5.0F) {
            state.current_waypoint++;
            state.is_moving = false;
        } else {
            state.is_moving = true;
            float move_speed = static_cast<float>(data->speed) * delta_time * 60.0F;
            state.position.x += (dx / dist) * move_speed;
            state.position.y += (dy / dist) * move_speed;
            state.rotation = std::atan2(dy, dx) * 180.0F / PI;
        }
    }
}

const std::vector<SceneElement>& LibraryScene::get_elements() const {
    return elements_;
}

std::vector<SceneElement>& LibraryScene::get_elements() {
    return elements_;
}

NpcManager& LibraryScene::get_npc_manager() {
    return npc_manager_;
}

const NpcManager& LibraryScene::get_npc_manager() const {
    return npc_manager_;
}

void LibraryScene::add_element(const SceneElement& element) {
    elements_.push_back(element);
}

void LibraryScene::update(float delta_time) {
    npc_manager_.update(delta_time);
}

bool LibraryScene::handle_click(Vector2 mouse_pos, std::string& out_npc_id) const {
    for (const auto& state : npc_manager_.get_npc_states()) {
        const auto* data = npc_manager_.get_npc_data(state.npc_id);
        if (!data) {
            continue;
        }
        
        float dx = mouse_pos.x - state.position.x;
        float dy = mouse_pos.y - state.position.y;
        float dist = std::sqrt(dx * dx + dy * dy);
        
        if (dist <= data->interaction_radius) {
            out_npc_id = state.npc_id;
            return true;
        }
    }
    return false;
}

}  // namespace pixel_town::library
