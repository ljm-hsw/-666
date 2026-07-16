// 图书馆 NPC 的只读展示状态和位置管理。
#include "locations/library_npc.hpp"

#include <algorithm>
#include <cstddef>
#include <cmath>

namespace pixel_town::library {

void NpcManager::add_npc(const NpcData& npc_data) {
    npc_data_list_.push_back(npc_data);
    npc_states_.push_back(
        NpcState{npc_data.id, npc_data.fixed_position, 0.0F});
}

void NpcManager::remove_npc(const std::string& npc_id) {
    for (std::size_t index = 0; index < npc_data_list_.size(); ++index) {
        if (npc_data_list_[index].id == npc_id) {
            npc_data_list_.erase(npc_data_list_.begin() +
                                 static_cast<std::ptrdiff_t>(index));
            npc_states_.erase(npc_states_.begin() +
                              static_cast<std::ptrdiff_t>(index));
            return;
        }
    }
}

const std::vector<NpcState>& NpcManager::get_npc_states() const {
    return npc_states_;
}

const NpcData* NpcManager::get_npc_data(const std::string& npc_id) const {
    for (const auto& data : npc_data_list_) {
        if (data.id == npc_id) {
            return &data;
        }
    }
    return nullptr;
}

void NpcManager::update(float delta_time) {
    if (delta_time <= 0.0F) {
        return;
    }
    const float bounded_delta = std::min(delta_time, 1.0F);
    for (auto& state : npc_states_) {
        state.idle_animation_seconds += bounded_delta;
    }
}

bool NpcManager::is_npc_clicked(Vector2 click_position,
                                std::string& out_npc_id) const {
    for (std::size_t index = 0; index < npc_states_.size(); ++index) {
        const float dx = click_position.x - npc_states_[index].position.x;
        const float dy = click_position.y - npc_states_[index].position.y;
        if (std::sqrt(dx * dx + dy * dy) <=
            npc_data_list_[index].interaction_radius) {
            out_npc_id = npc_states_[index].npc_id;
            return true;
        }
    }
    return false;
}

}  // namespace pixel_town::library
