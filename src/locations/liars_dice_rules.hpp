// 骗子骰子纯规则状态机：隐藏骰子、叫点、质疑和淘汰。
#pragma once

#include <array>
#include <optional>

namespace pixel_town {

enum class ChallengeInitiator { none, player, computer };
enum class LiarsDiceParticipant { none, player, computer };

struct LiarsDiceBid {
    int count{0};
    int face{1};
};

class LiarsDiceGame {
public:
    static constexpr int kDiceCount = 5;
    static constexpr int kMinFace = 1;
    static constexpr int kMaxFace = 6;
    static constexpr int kMaxBidCount = 10;

    explicit LiarsDiceGame(unsigned int seed);

    [[nodiscard]] const std::array<int, kDiceCount>& player_dice() const;
    [[nodiscard]] const std::array<int, kDiceCount>& computer_dice() const;
    [[nodiscard]] int bid_count() const;
    [[nodiscard]] int bid_face() const;
    [[nodiscard]] bool is_player_turn() const;
    [[nodiscard]] bool is_round_over() const;
    [[nodiscard]] bool is_game_over() const;
    [[nodiscard]] bool player_won() const;
    [[nodiscard]] bool dice_revealed() const;
    [[nodiscard]] int player_dice_count() const;
    [[nodiscard]] int computer_dice_count() const;

    [[nodiscard]] ChallengeInitiator who_challenged() const;
    [[nodiscard]] LiarsDiceParticipant round_loser() const;
    [[nodiscard]] int actual_count() const;
    [[nodiscard]] bool bid_was_valid() const;
    [[nodiscard]] std::optional<LiarsDiceBid> minimum_legal_bid() const;

    bool player_bid(int count, int face);
    bool player_challenge();
    bool computer_act();
    bool start_next_round();

private:
    std::array<int, kDiceCount> player_dice_{};
    std::array<int, kDiceCount> computer_dice_{};
    unsigned int seed_{0};
    int player_dice_count_{kDiceCount};
    int computer_dice_count_{kDiceCount};
    int bid_count_{0};
    int bid_face_{1};
    bool player_turn_{true};
    bool round_over_{false};
    bool game_over_{false};
    bool player_won_{false};
    bool dice_revealed_{false};
    ChallengeInitiator who_challenged_{ChallengeInitiator::none};
    LiarsDiceParticipant round_loser_{LiarsDiceParticipant::none};
    bool next_round_player_starts_{true};
    int actual_count_{0};
    bool bid_was_valid_{false};

    [[nodiscard]] bool is_bid_legal(int count, int face) const;
    [[nodiscard]] int count_face(int face) const;
    void roll_round_dice();
    void resolve_challenge(ChallengeInitiator challenger);
    static std::array<int, kDiceCount> roll_dice(unsigned int& seed);
};

}  // namespace pixel_town
