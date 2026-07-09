#pragma once

#include <string>
#include <vector>

#include <raylib.h>

namespace pixel_town::library {

using Vector2 = ::Vector2;
using Rectangle = ::Rectangle;

enum class NpcBehavior {
    idle,
    random_walk,
    patrol,
    follow_path,
};

struct NpcPatrolArea {
    Rectangle bounds;
    std::vector<Vector2> waypoints;
};

struct NpcData {
    std::string id;
    std::string name;
    float speed{1.0F};
    NpcBehavior behavior{NpcBehavior::random_walk};
    NpcPatrolArea patrol_area;
    float interaction_radius{20.0F};
};

struct NpcState {
    std::string npc_id;
    Vector2 position;
    Vector2 velocity;
    Vector2 target_position;
    bool is_moving{false};
    float move_progress{0.0F};
    int move_timer{0};
    float rotation{0.0F};
};

struct CollisionResult {
    bool collided{false};
    Vector2 normal;
    float penetration_depth;
};

class NpcManager {
public:
    NpcManager();

    void add_npc(const NpcData& npc_data);
    void remove_npc(const std::string& npc_id);
    [[nodiscard]] const std::vector<NpcState>& get_npc_states() const;
    [[nodiscard]] NpcState* get_npc_state(const std::string& npc_id);
    [[nodiscard]] const NpcData* get_npc_data(const std::string& npc_id) const;

    void update(float delta_time, const std::vector<Rectangle>& obstacles);
    void set_obstacles(const std::vector<Rectangle>& obstacles);

    [[nodiscard]] bool is_npc_clicked(const Vector2& click_position, std::string& out_npc_id) const;

private:
    void update_npc_position(NpcState& state, const NpcData& data, float delta_time);
    void update_behavior(NpcState& state, const NpcData& data);
    void generate_random_target(NpcState& state, const NpcData& data);
    void generate_patrol_target(NpcState& state, const NpcData& data);

    [[nodiscard]] CollisionResult check_collision(const Rectangle& a, const Rectangle& b) const;
    [[nodiscard]] bool check_obstacle_collision(const Vector2& position, const std::vector<Rectangle>& obstacles) const;
    [[nodiscard]] bool check_npc_collision(const NpcState& state, const Vector2& new_position) const;

    std::vector<NpcData> npc_data_list_;
    std::vector<NpcState> npc_states_;
    std::vector<Rectangle> obstacles_;
};

}  // namespace pixel_town::library