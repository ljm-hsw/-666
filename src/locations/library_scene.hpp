#pragma once

#include <string>
#include <vector>

#include <raylib.h>

namespace pixel_town::library {

enum class SceneElementType {
    bookshelf,
    plant,
    window,
    door
};

struct SceneElement {
    SceneElementType type;
    std::string id;
    Vector2 position;
    float width{0.0F};
    float height{0.0F};
    float rotation{0.0F};
    float scale{1.0F};
    Color tint{WHITE};
    std::string texture_name;
};

struct NpcData {
    std::string id;
    std::string name;
    Vector2 position;
    float interaction_radius{30.0F};
    int speed{2};
    std::vector<Vector2> waypoints;
};

struct NpcState {
    std::string npc_id;
    Vector2 position;
    float rotation{0.0F};
    int current_waypoint{0};
    bool is_moving{false};
    float move_progress{0.0F};
};

class NpcManager {
public:
    void add_npc(const NpcData& data);
    [[nodiscard]] const NpcData* get_npc_data(const std::string& id) const;
    [[nodiscard]] const std::vector<NpcState>& get_npc_states() const;
    [[nodiscard]] std::vector<NpcState>& get_npc_states();
    void update(float delta_time);

private:
    std::vector<NpcData> npcs_;
    std::vector<NpcState> npc_states_;
};

class LibraryScene {
public:
    [[nodiscard]] const std::vector<SceneElement>& get_elements() const;
    [[nodiscard]] std::vector<SceneElement>& get_elements();
    [[nodiscard]] NpcManager& get_npc_manager();
    [[nodiscard]] const NpcManager& get_npc_manager() const;
    void add_element(const SceneElement& element);
    void update(float delta_time);
    [[nodiscard]] bool handle_click(Vector2 mouse_pos, std::string& out_npc_id) const;

private:
    std::vector<SceneElement> elements_;
    NpcManager npc_manager_;
};

}  // namespace pixel_town::library
