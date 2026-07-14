#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "locations/library_data.hpp"
#include "locations/library_work_result.hpp"

namespace pixel_town::library {

struct PlayerState {
    int money{0};
    int stamina{0};
    int reputation{0};
    int knowledge{0};
    int mood{0};
};

struct DailyContext {
    int day{1};
    uint64_t random_seed{0};
    int library_visits{0};
    int current_knowledge{0};
};

struct LibraryConfig {
    int questions_per_session{5};
    int correct_knowledge_reward{10};
    int correct_reputation_reward{3};
    int wrong_knowledge_penalty{0};
    int wrong_reputation_penalty{0};
    int stamina_cost{15};
    int base_mood_change{5};
    int correct_mood_bonus{5};
    int wrong_mood_penalty{-3};
    int combo_reputation_bonus{2};
    int money_per_correct{5};
    int money_bonus_for_combo{3};
    int knowledge_threshold_for_map{30};
    int visits_threshold_for_map{2};
    int knowledge_threshold_for_borrow_card{50};
    int visits_threshold_for_borrow_card{5};
    int knowledge_threshold_for_close_friend{80};
    int visits_threshold_for_close_friend{8};
};

enum class NpcRelationship {
    stranger,
    familiar,
    close_friend
};

enum class PlotStatus {
    not_available,
    available,
    triggered
};

struct NpcInteraction {
    NpcRelationship relationship{NpcRelationship::stranger};
    bool map_revealed{false};
    bool borrow_card_given{false};
    bool close_friend_unlocked{false};
    std::string current_dialogue;
    std::string current_plot_title;
    std::string current_plot_description;
    PlotStatus current_plot_status{PlotStatus::not_available};
};

struct SessionState {
    int current_question_index{0};
    int correct_count{0};
    int wrong_count{0};
    int current_combo{0};
    int max_combo{0};
    std::vector<bool> answers;
    bool is_active{false};
    bool is_completed{false};
    bool is_in_intro{true};
    bool is_in_npc_talk{false};
    bool is_in_plot_event{false};
};

class LibraryRuleEngine {
public:
    explicit LibraryRuleEngine(const LibraryData& data, const LibraryConfig& config);

    void start_session(const DailyContext& context);

    void select_category(const std::string& category_id);

    [[nodiscard]] bool is_session_active() const;

    [[nodiscard]] bool is_session_completed() const;

    [[nodiscard]] const SessionState& get_session_state() const;

    [[nodiscard]] const ReaderQuestion& get_current_question() const;

    [[nodiscard]] const std::vector<BookCategory>& get_categories() const;

    [[nodiscard]] bool was_last_answer_correct() const;

    [[nodiscard]] LibraryWorkResult finish_session() const;

    void give_up();

    [[nodiscard]] NpcInteraction& get_npc_interaction();

    [[nodiscard]] const NpcInteraction& get_npc_interaction() const;

    void update_npc_relationship(int knowledge, int visits);

    [[nodiscard]] bool should_reveal_map(int knowledge, int visits) const;

    void reveal_map();

    void grant_borrow_card();

    [[nodiscard]] bool should_unlock_close_friend(int knowledge, int visits) const;

    void unlock_close_friend();

    [[nodiscard]] const NpcDialogue& get_dialogue() const;

    [[nodiscard]] const LibraryData& get_data() const;

    [[nodiscard]] const LibraryConfig& get_config() const;

    [[nodiscard]] const DailyContext& get_current_context() const;

    void check_plot_events(int knowledge, int visits);

    void trigger_plot_event(const std::string& event_id);

    [[nodiscard]] bool has_pending_plot_event() const;

private:
    LibraryData data_;
    LibraryConfig config_;
    SessionState session_state_;
    DailyContext current_context_;
    std::vector<ReaderQuestion> shuffled_questions_;
    bool last_answer_correct_{false};
    bool gave_up_{false};
    NpcInteraction npc_interaction_;
    std::vector<std::string> triggered_plot_events_;

    void shuffle_questions(uint64_t seed);

    [[nodiscard]] int compute_knowledge_change() const;

    [[nodiscard]] int compute_reputation_change() const;

    [[nodiscard]] int compute_mood_change() const;

    [[nodiscard]] std::string generate_summary() const;

    [[nodiscard]] std::string generate_narrative_echo() const;
};

[[nodiscard]] LibraryConfig default_library_config();

}  // namespace pixel_town::library
