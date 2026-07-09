#include "locations/library_scene.hpp"

namespace pixel_town::library {

LibraryScene::LibraryScene() {
    initialize_default_layout();
}

void LibraryScene::initialize_default_layout() {
    elements_.clear();

    add_window(50, 20, 120, 60);
    add_window(280, 20, 120, 60);
    add_window(510, 20, 120, 60);

    add_door(580, 280, 40, 80);

    add_bookshelf(30, 100, 140, 160);
    add_bookshelf(190, 100, 180, 180);
    add_bookshelf(400, 100, 140, 160);
    add_bookshelf(560, 100, 60, 160);

    add_plant(30, 270);
    add_plant(540, 270);

    NpcData librarian;
    librarian.id = "librarian";
    librarian.name = "管理员";
    librarian.speed = 0.4F;
    librarian.behavior = NpcBehavior::patrol;
    librarian.patrol_area.bounds = {50, 240, 150, 80};
    librarian.patrol_area.waypoints.push_back({80, 280});
    librarian.patrol_area.waypoints.push_back({180, 280});
    librarian.interaction_radius = 25.0F;

    npc_manager_.add_npc(librarian);
}

void LibraryScene::load_layout(const LibrarySceneLayout& layout) {
    elements_ = layout.elements;

    for (const auto& npc : layout.npcs) {
        npc_manager_.add_npc(npc);
    }
}

const std::vector<SceneElement>& LibraryScene::get_elements() const {
    return elements_;
}

const std::vector<Rectangle> LibraryScene::get_obstacles() const {
    std::vector<Rectangle> obstacles;
    for (const auto& element : elements_) {
        if (element.is_obstacle) {
            obstacles.push_back({element.position.x, element.position.y, 
                                element.width, element.height});
        }
    }
    return obstacles;
}

const NpcManager& LibraryScene::get_npc_manager() const {
    return npc_manager_;
}

NpcManager& LibraryScene::get_npc_manager() {
    return npc_manager_;
}

void LibraryScene::update(float delta_time) {
    npc_manager_.update(delta_time, get_obstacles());
}

bool LibraryScene::handle_click(const Vector2& click_position, std::string& out_npc_id) const {
    return npc_manager_.is_npc_clicked(click_position, out_npc_id);
}

void LibraryScene::add_bookshelf(float x, float y, float width, float height) {
    SceneElement element;
    element.type = SceneElementType::bookshelf;
    element.id = "bookshelf_" + std::to_string(elements_.size());
    element.position = {x, y};
    element.width = width;
    element.height = height;
    element.rotation = 0.0F;
    element.is_obstacle = true;
    elements_.push_back(element);
}

void LibraryScene::add_table(float x, float y, float width, float height) {
    SceneElement element;
    element.type = SceneElementType::table;
    element.id = "table_" + std::to_string(elements_.size());
    element.position = {x, y};
    element.width = width;
    element.height = height;
    element.rotation = 0.0F;
    element.is_obstacle = true;
    elements_.push_back(element);
}

void LibraryScene::add_chair(float x, float y, float width, float height) {
    SceneElement element;
    element.type = SceneElementType::chair;
    element.id = "chair_" + std::to_string(elements_.size());
    element.position = {x, y};
    element.width = width;
    element.height = height;
    element.rotation = 0.0F;
    element.is_obstacle = true;
    elements_.push_back(element);
}

void LibraryScene::add_desk(float x, float y, float width, float height) {
    SceneElement element;
    element.type = SceneElementType::desk;
    element.id = "desk_" + std::to_string(elements_.size());
    element.position = {x, y};
    element.width = width;
    element.height = height;
    element.rotation = 0.0F;
    element.is_obstacle = true;
    elements_.push_back(element);
}

void LibraryScene::add_plant(float x, float y) {
    SceneElement element;
    element.type = SceneElementType::plant;
    element.id = "plant_" + std::to_string(elements_.size());
    element.position = {x, y};
    element.width = 30.0F;
    element.height = 40.0F;
    element.rotation = 0.0F;
    element.is_obstacle = false;
    elements_.push_back(element);
}

void LibraryScene::add_window(float x, float y, float width, float height) {
    SceneElement element;
    element.type = SceneElementType::window;
    element.id = "window_" + std::to_string(elements_.size());
    element.position = {x, y};
    element.width = width;
    element.height = height;
    element.rotation = 0.0F;
    element.is_obstacle = false;
    elements_.push_back(element);
}

void LibraryScene::add_floor_lamp(float x, float y) {
    SceneElement element;
    element.type = SceneElementType::floor_lamp;
    element.id = "lamp_" + std::to_string(elements_.size());
    element.position = {x, y};
    element.width = 20.0F;
    element.height = 60.0F;
    element.rotation = 0.0F;
    element.is_obstacle = false;
    elements_.push_back(element);
}

void LibraryScene::add_rug(float x, float y, float width, float height) {
    SceneElement element;
    element.type = SceneElementType::rug;
    element.id = "rug_" + std::to_string(elements_.size());
    element.position = {x, y};
    element.width = width;
    element.height = height;
    element.rotation = 0.0F;
    element.is_obstacle = false;
    elements_.push_back(element);
}

void LibraryScene::add_door(float x, float y, float width, float height) {
    SceneElement element;
    element.type = SceneElementType::door;
    element.id = "door_" + std::to_string(elements_.size());
    element.position = {x, y};
    element.width = width;
    element.height = height;
    element.rotation = 0.0F;
    element.is_obstacle = false;
    elements_.push_back(element);
}

}  // namespace pixel_town::library