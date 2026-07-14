#pragma once

#include <string>
#include <vector>

#include <raylib.h>

namespace pixel_town::library {

using Vector2 = ::Vector2;
using Rectangle = ::Rectangle;

struct NpcData {
    std::string id;
    std::string name;
    Vector2 fixed_position{320.0F, 180.0F};
    float interaction_radius{20.0F};
};

struct NpcState {
    std::string npc_id;
    Vector2 position;
    float idle_animation_seconds{0.0F};
};

class NpcManager {
public:
    void add_npc(const NpcData& npc_data);
    void remove_npc(const std::string& npc_id);
    [[nodiscard]] const std::vector<NpcState>& get_npc_states() const;
    [[nodiscard]] const NpcData* get_npc_data(const std::string& npc_id) const;

    void update(float delta_time);
    [[nodiscard]] bool is_npc_clicked(Vector2 click_position,
                                      std::string& out_npc_id) const;

private:
    std::vector<NpcData> npc_data_list_;
    std::vector<NpcState> npc_states_;
};

}  // namespace pixel_town::library
