#pragma once

#include <array>
#include <optional>
#include <string>

#include "app/dialogue_runtime.hpp"
#include "core/game_session.hpp"
#include "core/location_story.hpp"
#include "core/tavern_rules.hpp"
#include "locations/gomoku_rules.hpp"
#include "locations/liars_dice_rules.hpp"
#include "locations/tavern_challenge_settlement.hpp"

namespace pixel_town {

enum class TavernScreen {
    lobby,
    challenge_select,
    npc_dialog,
    gomoku,
    liars_dice,
};

struct TavernCanvasPoint {
    float x{0.0F};
    float y{0.0F};
    bool valid{false};
};

struct TavernFrameInput {
    float elapsed_seconds{0.0F};
    bool updates_enabled{true};
    TavernCanvasPoint pointer;
    bool primary_pressed{false};
    bool escape_pressed{false};
    bool enter_pressed{false};
    bool space_pressed{false};
    int digit_pressed{0};
};

struct TavernGomokuPresentation {
    GomokuBoard board{};
    GomokuTurn turn{GomokuTurn::player};
    GomokuState state{GomokuState::playing};
};

struct TavernDiePresentation {
    bool active{false};
    std::optional<int> visible_face;
};

struct TavernLiarsDicePresentation {
    std::array<TavernDiePresentation, LiarsDiceGame::kDiceCount> player_dice;
    std::array<TavernDiePresentation, LiarsDiceGame::kDiceCount> computer_dice;
    int player_dice_count{LiarsDiceGame::kDiceCount};
    int computer_dice_count{LiarsDiceGame::kDiceCount};
    int current_bid_count{0};
    int current_bid_face{1};
    int proposed_bid_count{1};
    int proposed_bid_face{1};
    bool player_turn{true};
    bool round_over{false};
    bool game_over{false};
    bool player_won{false};
    int actual_count{0};
    bool bid_was_valid{false};
    LiarsDiceParticipant round_loser{LiarsDiceParticipant::none};
};

struct TavernPresentation {
    TavernScreen screen{TavernScreen::lobby};
    ChallengeType selected_challenge{ChallengeType::gomoku};
    BetTier selected_bet{BetTier::medium};
    int selected_bet_amount{0};
    std::array<int, 3> bet_amounts{};
    float bartender_animation_seconds{0.0F};
    std::string feedback;
    bool challenge_started{false};
    std::optional<TavernGomokuPresentation> gomoku;
    std::optional<TavernLiarsDicePresentation> liars_dice;
    std::optional<DialoguePresentation> dialogue;
};

enum class TavernOpenStatus {
    opened,
    denied,
    already_active,
};

struct TavernOpenResult {
    TavernOpenStatus status{TavernOpenStatus::denied};
    std::string message;
};

enum class TavernStepStatus {
    unchanged,
    changed,
    returned_to_map,
    settled,
    rejected,
};

struct TavernStepResult {
    TavernStepStatus status{TavernStepStatus::unchanged};
    std::optional<std::string> notice;
};

class TavernRuntime {
public:
    TavernRuntime();

    [[nodiscard]] TavernOpenResult open(GameSession& session);
    [[nodiscard]] TavernStepResult step(GameSession& session,
                                        const TavernFrameInput& input);
    [[nodiscard]] TavernPresentation presentation() const;
    [[nodiscard]] bool active() const noexcept { return active_; }

private:
    bool active_{false};
    TavernScreen screen_{TavernScreen::lobby};
    ChallengeType selected_challenge_{ChallengeType::gomoku};
    BetTier selected_bet_{BetTier::medium};
    GomokuGame gomoku_{};
    LiarsDiceGame liars_dice_{0U};
    float computer_timer_{0.0F};
    int bid_count_{1};
    int bid_face_{1};
    std::string feedback_;
    float npc_animation_timer_{0.0F};
    int active_result_id_{0};
    PlayerState player_at_start_{};
    std::optional<ActionResult> pending_settlement_;
    TavernChallengeConfig config_{};
    TavernChallengeSettlement settlement_;
    std::optional<DialogueScript> location_story_;
    DialogueRuntime dialogue_;

    void reset();
};

[[nodiscard]] const char* tavern_ui_glyphs();

}  // namespace pixel_town
