// 确定性书籍整理任务；最后一本正确归位时返回 completed 结果。
#include "locations/library_organizing.hpp"

#include <algorithm>
#include <cinttypes>
#include <utility>

namespace pixel_town::library {

namespace {

uint64_t splitmix64(uint64_t& state) {
    uint64_t value = state;
    value = (value ^ (value >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    value = (value ^ (value >> 27)) * UINT64_C(0x94d049bb133111eb);
    return state = value ^ (value >> 31);
}

void shuffle_tasks(std::vector<OrganizingBookTask>& tasks, uint64_t seed) {
    if (tasks.size() < 2) {
        return;
    }
    for (std::size_t index = tasks.size() - 1; index > 0; --index) {
        const std::size_t selected = splitmix64(seed) % (index + 1);
        std::swap(tasks[index], tasks[selected]);
    }
}

}  // namespace

LibraryOrganizingSession::LibraryOrganizingSession(
    std::vector<OrganizingBookTask> tasks, std::vector<OrganizingShelf> shelves,
    OrganizingConfig config)
    : all_tasks_(std::move(tasks)), shelves_(std::move(shelves)), config_(config) {}

void LibraryOrganizingSession::start(uint64_t seed) {
    tasks_ = all_tasks_;
    shuffle_tasks(tasks_, seed);
    if (tasks_.size() > static_cast<std::size_t>(std::max(0, config_.tasks_per_session))) {
        tasks_.resize(static_cast<std::size_t>(config_.tasks_per_session));
    }
    state_ = OrganizingSessionState{};
    gave_up_ = false;
    state_.is_active = !tasks_.empty();
    state_.is_completed = tasks_.empty();
    state_.completed_tasks.assign(tasks_.size(), false);
}

void LibraryOrganizingSession::give_up() {
    gave_up_ = true;
    state_.is_active = false;
    state_.is_completed = true;
    state_.held_book_id.clear();
}

OrganizingActionFeedback LibraryOrganizingSession::pick_up(const std::string& book_id) {
    if (!state_.is_active) {
        return {OrganizingActionStatus::inactive, "整理工作当前不可操作。"};
    }
    if (!state_.held_book_id.empty()) {
        return {OrganizingActionStatus::already_holding, "请先处理手中的书。"};
    }
    for (std::size_t index = 0; index < tasks_.size(); ++index) {
        if (tasks_[index].id == book_id && !state_.completed_tasks[index]) {
            state_.held_book_id = book_id;
            return {OrganizingActionStatus::accepted, "已拿起《" + tasks_[index].title + "》。"};
        }
    }
    return {OrganizingActionStatus::book_not_found, "这本书不在本次整理任务中。"};
}

OrganizingActionFeedback LibraryOrganizingSession::place_on_shelf(const std::string& shelf_id) {
    if (!state_.is_active) {
        return {OrganizingActionStatus::inactive, "整理工作当前不可操作。"};
    }
    if (state_.held_book_id.empty()) {
        return {OrganizingActionStatus::not_holding, "请先拿起一本待整理的书。"};
    }

    const auto shelf = std::find_if(shelves_.begin(), shelves_.end(),
                                    [&shelf_id](const OrganizingShelf& item) {
                                        return item.id == shelf_id;
                                    });
    if (shelf == shelves_.end()) {
        return {OrganizingActionStatus::shelf_not_found, "没有找到这个书架。"};
    }

    const auto task = std::find_if(tasks_.begin(), tasks_.end(), [this](const auto& item) {
        return item.id == state_.held_book_id;
    });
    if (task == tasks_.end() || task->category_id != shelf->category_id) {
        ++state_.wrong_count;
        return {OrganizingActionStatus::wrong_shelf, "分类不匹配，请换一个书架。"};
    }

    const auto index = static_cast<std::size_t>(std::distance(tasks_.begin(), task));
    state_.completed_tasks[index] = true;
    ++state_.completed_count;
    state_.held_book_id.clear();
    if (state_.completed_count == static_cast<int>(tasks_.size())) {
        state_.is_active = false;
        state_.is_completed = true;
        return {OrganizingActionStatus::completed,
                "分类正确，全部书籍已归位，正在结算。"};
    }
    return {OrganizingActionStatus::accepted, "分类正确，书籍已归位。"};
}

const OrganizingSessionState& LibraryOrganizingSession::state() const { return state_; }

const std::vector<OrganizingBookTask>& LibraryOrganizingSession::tasks() const { return tasks_; }

const std::vector<OrganizingShelf>& LibraryOrganizingSession::shelves() const { return shelves_; }

bool LibraryOrganizingSession::is_completed() const { return state_.is_completed; }

LibraryWorkResult LibraryOrganizingSession::finish_session() const {
    LibraryWorkResult result;
    if (gave_up_ || !state_.is_completed || state_.completed_count == 0) {
        result.gave_up = true;
        result.summary = "离开了图书馆，整理工作没有结算。";
        return result;
    }

    result.completed = true;
    result.money_change = state_.completed_count * config_.money_per_book;
    result.stamina_change = -config_.stamina_cost;
    result.reputation_change = state_.completed_count * config_.reputation_per_book;
    result.knowledge_change = state_.completed_count * config_.knowledge_per_book;
    result.mood_change = config_.base_mood_change +
                         state_.wrong_count * config_.wrong_mood_penalty;
    result.summary = "完成图书馆整理：归位 " +
                     std::to_string(state_.completed_count) + " 本，错误尝试 " +
                     std::to_string(state_.wrong_count) + " 次。";
    return result;
}

OrganizingConfig default_organizing_config() { return OrganizingConfig{}; }

}  // namespace pixel_town::library
