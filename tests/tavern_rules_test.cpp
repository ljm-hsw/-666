#include <doctest/doctest.h>

#include "core/tavern_rules.hpp"

namespace pixel_town {
namespace {

TEST_CASE("bet_amount maps tier to config value") {
    TavernChallengeConfig config;
    CHECK(bet_amount(config, BetTier::low) == 10);
    CHECK(bet_amount(config, BetTier::medium) == 25);
    CHECK(bet_amount(config, BetTier::high) == 50);

    TavernChallengeConfig custom;
    custom.low_bet = 5;
    custom.medium_bet = 15;
    custom.high_bet = 30;
    CHECK(bet_amount(custom, BetTier::low) == 5);
    CHECK(bet_amount(custom, BetTier::medium) == 15);
    CHECK(bet_amount(custom, BetTier::high) == 30);
}

TEST_CASE("win produces expected net gain, mood boost, and win record") {
    PlayerState player;
    player.money = 50;
    TavernChallengeConfig config;

    const auto result = simulate_tavern_challenge(
        player, config, ChallengeType::gomoku, BetTier::low, ChallengeOutcome::win, 1);

    CHECK(result.delta.money == 20);
    CHECK(result.delta.mood == 8);
    CHECK(result.delta.stamina == 0);
    CHECK(result.delta.reputation == 0);
    CHECK(result.delta.knowledge == 0);
    CHECK(result.tavern_win_delta == 1);
    CHECK(result.tavern_loss_delta == 0);
    CHECK(result.slot == ActionSlot::night);
    CHECK(result.location == Location::tavern);
    CHECK(result.outcome == ActionOutcome::completed);
    CHECK(result.result_id == 1);

    const std::string& summary = result.summary;
    CHECK(summary.find("Win") != std::string::npos);
    CHECK(summary.find("Earned") != std::string::npos);
}

TEST_CASE("loss deducts bet and lowers mood") {
    PlayerState player;
    player.money = 50;
    TavernChallengeConfig config;

    const auto result = simulate_tavern_challenge(
        player, config, ChallengeType::liars_dice, BetTier::medium, ChallengeOutcome::loss, 2);

    CHECK(result.delta.money == -25);
    CHECK(result.delta.mood == -5);
    CHECK(result.tavern_win_delta == 0);
    CHECK(result.tavern_loss_delta == 1);
    CHECK(result.outcome == ActionOutcome::completed);

    const std::string& summary = result.summary;
    CHECK(summary.find("Loss") != std::string::npos);
    CHECK(summary.find("lost") != std::string::npos);
}

TEST_CASE("draw refunds bet with small mood gain") {
    PlayerState player;
    player.money = 50;
    TavernChallengeConfig config;

    const auto result = simulate_tavern_challenge(
        player, config, ChallengeType::gomoku, BetTier::high, ChallengeOutcome::draw, 3);

    CHECK(result.delta.money == 0);
    CHECK(result.delta.mood == 2);
    CHECK(result.tavern_win_delta == 0);
    CHECK(result.tavern_loss_delta == 0);
    CHECK(result.outcome == ActionOutcome::completed);

    const std::string& summary = result.summary;
    CHECK(summary.find("Draw") != std::string::npos);
    CHECK(summary.find("refunded") != std::string::npos);
}

TEST_CASE("bet exceeding player money returns abandoned outcome") {
    PlayerState player;
    player.money = 5;
    TavernChallengeConfig config;

    const auto result = simulate_tavern_challenge(
        player, config, ChallengeType::gomoku, BetTier::medium, ChallengeOutcome::win, 4);

    CHECK(result.outcome == ActionOutcome::abandoned);
    CHECK(result.delta.money == 0);
    CHECK(result.delta.stamina == 0);
    CHECK(result.delta.mood == 0);
    CHECK(result.tavern_win_delta == 0);
    CHECK(result.tavern_loss_delta == 0);

    const std::string& summary = result.summary;
    CHECK(summary.find("赌注不足") != std::string::npos);
}

TEST_CASE("label functions return distinct non-empty values") {
    CHECK(challenge_type_label(ChallengeType::gomoku) != nullptr);
    CHECK(challenge_type_label(ChallengeType::liars_dice) != nullptr);
    CHECK(std::string{challenge_type_label(ChallengeType::gomoku)} !=
          std::string{challenge_type_label(ChallengeType::liars_dice)});

    CHECK(bet_tier_label(BetTier::low) != nullptr);
    CHECK(bet_tier_label(BetTier::medium) != nullptr);
    CHECK(bet_tier_label(BetTier::high) != nullptr);
    CHECK(std::string{bet_tier_label(BetTier::low)} !=
          std::string{bet_tier_label(BetTier::medium)});
    CHECK(std::string{bet_tier_label(BetTier::medium)} !=
          std::string{bet_tier_label(BetTier::high)});

    CHECK(challenge_outcome_label(ChallengeOutcome::win) != nullptr);
    CHECK(challenge_outcome_label(ChallengeOutcome::loss) != nullptr);
    CHECK(challenge_outcome_label(ChallengeOutcome::draw) != nullptr);
}

TEST_CASE("result carries correct slot and location regardless of outcome") {
    PlayerState player;
    player.money = 100;
    TavernChallengeConfig config;

    for (auto outcome : {ChallengeOutcome::win, ChallengeOutcome::loss, ChallengeOutcome::draw}) {
        const auto result = simulate_tavern_challenge(
            player, config, ChallengeType::gomoku, BetTier::low, outcome, 7);
        CHECK(result.slot == ActionSlot::night);
        CHECK(result.location == Location::tavern);
        CHECK(result.result_id == 7);
    }
}

}  // namespace
}  // namespace pixel_town
