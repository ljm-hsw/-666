// 图书整理纯规则会话。
//
// 规则维护“手中书”和每本书的归位状态。错误放置不能清空手中书，正确归位
// 最后一本时由规则明确返回 completed；UI 只展示 presentation，不自行判断任务
// 是否结束，也不负责计算奖励。
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "locations/library_work_result.hpp"

namespace pixel_town::library {

enum class OrganizingBookSource {
    scattered,
    misplaced,
};

struct OrganizingBookTask {
    std::string id;
    std::string title;
    std::string category_id;
    OrganizingBookSource source{OrganizingBookSource::scattered};
    std::string source_shelf_id;
    int x{0};
    int y{0};
};

struct OrganizingShelf {
    std::string id;
    std::string category_id;
    std::string name;
    int x{0};
    int y{0};
    int width{0};
    int height{0};
};

struct OrganizingConfig {
    int tasks_per_session{4};
    int money_per_book{4};
    int knowledge_per_book{5};
    int reputation_per_book{2};
    int stamina_cost{15};
    int base_mood_change{3};
    int wrong_mood_penalty{-1};
};

enum class OrganizingActionStatus {
    accepted,
    completed,
    inactive,
    already_holding,
    book_not_found,
    not_holding,
    shelf_not_found,
    wrong_shelf,
};

struct OrganizingActionFeedback {
    OrganizingActionStatus status{OrganizingActionStatus::inactive};
    std::string message;
};

struct OrganizingSessionState {
    bool is_active{false};
    bool is_completed{false};
    int completed_count{0};
    int wrong_count{0};
    std::string held_book_id;
    std::vector<bool> completed_tasks;
};

class LibraryOrganizingSession {
public:
    LibraryOrganizingSession(std::vector<OrganizingBookTask> tasks,
                             std::vector<OrganizingShelf> shelves,
                             OrganizingConfig config);

    void start(uint64_t seed);
    [[nodiscard]] OrganizingActionFeedback pick_up(const std::string& book_id);
    [[nodiscard]] OrganizingActionFeedback place_on_shelf(const std::string& shelf_id);
    void give_up();

    [[nodiscard]] const OrganizingSessionState& state() const;
    [[nodiscard]] const std::vector<OrganizingBookTask>& tasks() const;
    [[nodiscard]] const std::vector<OrganizingShelf>& shelves() const;
    [[nodiscard]] bool is_completed() const;
    [[nodiscard]] LibraryWorkResult finish_session() const;

private:
    std::vector<OrganizingBookTask> all_tasks_;
    std::vector<OrganizingBookTask> tasks_;
    std::vector<OrganizingShelf> shelves_;
    OrganizingConfig config_;
    OrganizingSessionState state_;
    bool gave_up_{false};
};

[[nodiscard]] OrganizingConfig default_organizing_config();

}  // namespace pixel_town::library
