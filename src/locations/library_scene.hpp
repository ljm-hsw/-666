#pragma once

#include <string>
#include <vector>

#include "locations/library_npc.hpp"

namespace pixel_town::library {

enum class SceneElementType {
    bookshelf,
    table,
    chair,
    desk,
    plant,
    window,
    door,
    floor_lamp,
    rug,
};

struct SceneElement {
    SceneElementType type;
    std::string id;
    Vector2 position;
    float width{0.0F};
    float height{0.0F};
    float rotation{0.0F};
    std::string texture_name;
    bool is_obstacle{true};
};

struct LibrarySceneLayout {
    float width{640.0F};
    float height{360.0F};
    std::vector<SceneElement> elements;
    std::vector<NpcData> npcs;
};

class LibraryScene {
public:
    LibraryScene();

    void initialize_default_layout();
    void load_layout(const LibrarySceneLayout& layout);

    [[nodiscard]] const std::vector<SceneElement>& get_elements() const;
    [[nodiscard]] const std::vector<Rectangle> get_obstacles() const;
    [[nodiscard]] const NpcManager& get_npc_manager() const;
    [[nodiscard]] NpcManager& get_npc_manager();

    void update(float delta_time);
    [[nodiscard]] bool handle_click(const Vector2& click_position, std::string& out_npc_id) const;

private:
    void add_bookshelf(float x, float y, float width, float height);
    void add_table(float x, float y, float width, float height);
    void add_chair(float x, float y, float width, float height);
    void add_desk(float x, float y, float width, float height);
    void add_plant(float x, float y);
    void add_window(float x, float y, float width, float height);
    void add_floor_lamp(float x, float y);
    void add_rug(float x, float y, float width, float height);
    void add_door(float x, float y, float width, float height);

    std::vector<SceneElement> elements_;
    NpcManager npc_manager_;
};

}  // namespace pixel_town::library