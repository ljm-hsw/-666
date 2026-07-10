#include <doctest/doctest.h>

#include "core/game_session.hpp"
#include "locations/liars_dice_rules.hpp"
#include "core/tavern_rules.hpp"
#include "test_game_session_helpers.hpp"

namespace pixel_town {
namespace {

TEST_CASE("fixed seed produces repeatable dice") {
    LiarsDiceGame game1(42);
    LiarsDiceGame game2(42);

    for (int i = 0; i < LiarsDiceGame::kDiceCount; ++i) {
        CHECK(game1.player_dice()[i] == game2.player_dice()[i]);
        CHECK(game1.computer_dice()[i] == game2.computer_dice()[i]);
    }
}

TEST_CASE("different seeds produce different dice") {
    LiarsDiceGame game1(42);
    LiarsDiceGame game2(99);

    bool any_different = false;
    for (int i = 0; i < LiarsDiceGame::kDiceCount; ++i) {
        if (game1.player_dice()[i] != game2.player_dice()[i]) any_different = true;
    }
    CHECK(any_different);
}

TEST_CASE("initial state has no bid and player turn") {
    LiarsDiceGame game(42);
    CHECK(game.is_player_turn());
    CHECK_FALSE(game.is_game_over());
    CHECK(game.bid_count() == 0);
    CHECK_FALSE(game.dice_revealed());
}

TEST_CASE("first legal bid succeeds") {
    LiarsDiceGame game(42);
    CHECK(game.player_bid(2, 3));
    CHECK(game.bid_count() == 2);
    CHECK(game.bid_face() == 3);
    CHECK_FALSE(game.is_player_turn());
}

TEST_CASE("bid with count zero rejected") {
    LiarsDiceGame game(42);
    CHECK_FALSE(game.player_bid(0, 3));
    CHECK(game.bid_count() == 0);
}

TEST_CASE("bid with invalid face rejected") {
    LiarsDiceGame game(42);
    CHECK_FALSE(game.player_bid(1, 0));
    CHECK_FALSE(game.player_bid(1, 7));
    CHECK(game.bid_count() == 0);
}

TEST_CASE("larger count bid succeeds") {
    LiarsDiceGame game(42);
    REQUIRE(game.player_bid(2, 4));
    REQUIRE(game.computer_act());
    REQUIRE(game.is_player_turn());
    CHECK(game.player_bid(3, 1));
}

TEST_CASE("same count larger face bid succeeds") {
    LiarsDiceGame game(42);
    REQUIRE(game.player_bid(2, 2));
    REQUIRE(game.computer_act());
    REQUIRE(game.is_player_turn());
    int cb_cnt = game.bid_count();
    int cb_face = game.bid_face();
    bool ok = false;
    if (cb_face < 6)
        ok = game.player_bid(cb_cnt, cb_face + 1);
    else
        ok = game.player_bid(cb_cnt + 1, 1);
    CHECK(ok);
}

TEST_CASE("minimum legal bid advances beyond the current computer bid") {
    LiarsDiceGame game(42);
    REQUIRE(game.player_bid(1, 1));
    REQUIRE(game.computer_act());
    REQUIRE_FALSE(game.is_game_over());
    REQUIRE(game.is_player_turn());

    const auto next_bid = game.minimum_legal_bid();
    REQUIRE(next_bid.has_value());
    CHECK((next_bid->count > game.bid_count() ||
           (next_bid->count == game.bid_count() && next_bid->face > game.bid_face())));
    CHECK(game.player_bid(next_bid->count, next_bid->face));
}

TEST_CASE("non-increasing bid rejected") {
    LiarsDiceGame game(42);
    REQUIRE(game.player_bid(3, 2));
    REQUIRE(game.computer_act());
    REQUIRE(game.is_player_turn());
    int cb_cnt = game.bid_count();
    int cb_face = game.bid_face();
    if (cb_face > 1) CHECK_FALSE(game.player_bid(cb_cnt, cb_face - 1));
    if (cb_cnt > 1) CHECK_FALSE(game.player_bid(cb_cnt - 1, 6));
    CHECK(game.bid_count() == cb_cnt);
    CHECK(game.bid_face() == cb_face);
}

TEST_CASE("challenge with no bid rejected") {
    LiarsDiceGame game(42);
    CHECK_FALSE(game.player_challenge());
    CHECK_FALSE(game.is_game_over());
}

TEST_CASE("challenge resolves correctly with reveal") {
    LiarsDiceGame game(2026);
    int f = game.player_dice()[0];
    REQUIRE(game.player_bid(1, f));
    REQUIRE(game.computer_act());
    if (!game.is_round_over()) {
        REQUIRE(game.is_player_turn());
        REQUIRE(game.player_challenge());
    }
    CHECK(game.is_round_over());
    CHECK(game.dice_revealed());
}

TEST_CASE("player challenge assigns the round loss from the revealed bid") {
    LiarsDiceGame game(2026);
    REQUIRE(game.player_bid(1, 1));
    REQUIRE(game.computer_act());
    REQUIRE_FALSE(game.is_round_over());
    REQUIRE(game.is_player_turn());
    REQUIRE(game.player_challenge());

    CHECK(game.is_round_over());
    CHECK(game.who_challenged() == ChallengeInitiator::player);
    CHECK(game.round_loser() == (game.bid_was_valid() ? LiarsDiceParticipant::player
                                                       : LiarsDiceParticipant::computer));
}

TEST_CASE("computer action is legal bid or challenge") {
    for (unsigned int seed = 1; seed <= 20; ++seed) {
        LiarsDiceGame game(seed);
        REQUIRE(game.player_bid(2, 3));
        int prev_count = game.bid_count();
        int prev_face = game.bid_face();
        REQUIRE(game.computer_act());
        if (game.is_round_over()) {
            CHECK(game.dice_revealed());
        } else {
            CHECK(game.is_player_turn());
            bool raised = (game.bid_count() > prev_count) ||
                          (game.bid_count() == prev_count && game.bid_face() > prev_face);
            CHECK(raised);
        }
    }
}

TEST_CASE("computer action deterministic for same seed") {
    LiarsDiceGame game1(77);
    LiarsDiceGame game2(77);
    REQUIRE(game1.player_bid(3, 4));
    REQUIRE(game2.player_bid(3, 4));
    CHECK(game1.computer_act());
    CHECK(game2.computer_act());
    CHECK(game1.is_round_over() == game2.is_round_over());
    CHECK(game1.is_game_over() == game2.is_game_over());
    CHECK(game1.bid_count() == game2.bid_count());
    CHECK(game1.bid_face() == game2.bid_face());
}

TEST_CASE("no actions after a round ends") {
    LiarsDiceGame game(42);
    REQUIRE(game.player_bid(9, 6));
    REQUIRE(game.computer_act());
    REQUIRE(game.is_round_over());
    CHECK_FALSE(game.player_bid(1, 1));
    CHECK_FALSE(game.player_challenge());
    CHECK_FALSE(game.computer_act());
}

TEST_CASE("computer bids on its first turn") {
    LiarsDiceGame game(42);
    REQUIRE(game.player_bid(1, 3));
    REQUIRE(game.computer_act());
    CHECK(game.bid_count() >= 1);
    CHECK(game.bid_face() >= 1);
    CHECK(game.bid_face() <= 6);
    CHECK(game.is_player_turn());
    CHECK_FALSE(game.is_game_over());
}

TEST_CASE("bid with count above 10 rejected") {
    LiarsDiceGame game(42);
    CHECK_FALSE(game.player_bid(11, 3));
    CHECK(game.bid_count() == 0);
}

TEST_CASE("bid with count 10 allowed") {
    LiarsDiceGame game(42);
    CHECK(game.player_bid(10, 3));
    CHECK(game.bid_count() == 10);
}

TEST_CASE("player challenge sets who_challenged and settlement info") {
    LiarsDiceGame game(2026);
    REQUIRE(game.player_bid(1, 1));
    REQUIRE(game.computer_act());
    REQUIRE_FALSE(game.is_round_over());
    REQUIRE(game.is_player_turn());
    REQUIRE(game.player_challenge());

    REQUIRE(game.is_round_over());
    CHECK(game.who_challenged() == ChallengeInitiator::player);
    CHECK(game.actual_count() >= 0);
    CHECK(game.actual_count() <= 10);
    bool consistent = (game.actual_count() >= game.bid_count()) == game.bid_was_valid();
    CHECK(consistent);
}

TEST_CASE("computer challenge sets settlement info correctly") {
    LiarsDiceGame game(100);
    REQUIRE(game.player_bid(9, 6));
    REQUIRE(game.computer_act());

    REQUIRE(game.is_round_over());
    CHECK(game.who_challenged() == ChallengeInitiator::computer);
    CHECK(game.actual_count() >= 0);
    CHECK((game.actual_count() >= game.bid_count()) == game.bid_was_valid());
    CHECK(game.round_loser() == (game.bid_was_valid() ? LiarsDiceParticipant::computer
                                                       : LiarsDiceParticipant::player));
}

TEST_CASE("who_challenged is none before game over") {
    LiarsDiceGame game(42);
    CHECK(game.who_challenged() == ChallengeInitiator::none);
    CHECK(game.actual_count() == 0);
    CHECK_FALSE(game.bid_was_valid());
}

TEST_CASE("ones are wild for a normal face") {
    LiarsDiceGame game(1);
    int expected = 0;
    for (int die : game.player_dice()) {
        if (die == 1 || die == 6) ++expected;
    }
    for (int die : game.computer_dice()) {
        if (die == 1 || die == 6) ++expected;
    }

    REQUIRE(game.player_bid(LiarsDiceGame::kMaxBidCount, 6));
    REQUIRE(game.computer_act());
    REQUIRE(game.is_round_over());
    CHECK(game.actual_count() == expected);
}

TEST_CASE("ones are not wild when the bid face is one") {
    LiarsDiceGame game(1);
    int expected = 0;
    for (int die : game.player_dice()) {
        if (die == 1) ++expected;
    }
    for (int die : game.computer_dice()) {
        if (die == 1) ++expected;
    }

    REQUIRE(game.player_bid(LiarsDiceGame::kMaxBidCount, 1));
    REQUIRE(game.computer_act());
    REQUIRE(game.is_round_over());
    CHECK(game.actual_count() == expected);
}

TEST_CASE("round loser loses one die and starts the next round") {
    LiarsDiceGame game(42);
    REQUIRE(game.player_bid(LiarsDiceGame::kMaxBidCount, 6));
    REQUIRE(game.computer_act());

    REQUIRE(game.is_round_over());
    CHECK_FALSE(game.is_game_over());
    CHECK(game.player_dice_count() == 4);
    CHECK(game.computer_dice_count() == 5);
    REQUIRE(game.start_next_round());
    CHECK_FALSE(game.is_round_over());
    CHECK(game.is_player_turn());
    CHECK(game.bid_count() == 0);
    CHECK_FALSE(game.player_bid(LiarsDiceGame::kMaxBidCount, 6));
}

TEST_CASE("computer round loss removes a die and gives computer the next opening") {
    bool found_computer_loss = false;

    for (unsigned int seed = 1; seed <= 200 && !found_computer_loss; ++seed) {
        for (int opening_face = 1; opening_face < LiarsDiceGame::kMaxFace; ++opening_face) {
            LiarsDiceGame game(seed);
            REQUIRE(game.player_bid(1, opening_face));
            REQUIRE(game.computer_act());
            REQUIRE_FALSE(game.is_round_over());
            REQUIRE(game.player_challenge());

            if (game.round_loser() == LiarsDiceParticipant::computer) {
                CHECK(game.player_dice_count() == 5);
                CHECK(game.computer_dice_count() == 4);
                REQUIRE(game.start_next_round());
                CHECK_FALSE(game.is_player_turn());
                found_computer_loss = true;
                break;
            }
        }
    }

    CHECK(found_computer_loss);
}

TEST_CASE("match ends only after one side loses all dice") {
    LiarsDiceGame game(42);

    for (int expected_dice = 4; expected_dice >= 0; --expected_dice) {
        const int total_dice = game.player_dice_count() + game.computer_dice_count();
        REQUIRE(game.player_bid(total_dice, 6));
        REQUIRE(game.computer_act());
        REQUIRE(game.is_round_over());
        CHECK(game.player_dice_count() == expected_dice);

        if (expected_dice > 0) {
            CHECK_FALSE(game.is_game_over());
            REQUIRE(game.start_next_round());
        }
    }

    CHECK(game.is_game_over());
    CHECK_FALSE(game.player_won());
    CHECK_FALSE(game.start_next_round());
}

TEST_CASE("simple player and computer policies always finish a match") {
    LiarsDiceGame game(77);
    int actions = 0;

    while (!game.is_game_over() && actions < 100) {
        if (game.is_round_over()) {
            REQUIRE(game.start_next_round());
        } else if (game.is_player_turn()) {
            if (game.bid_count() == 0) {
                const auto bid = game.minimum_legal_bid();
                REQUIRE(bid.has_value());
                REQUIRE(game.player_bid(bid->count, bid->face));
            } else {
                REQUIRE(game.player_challenge());
            }
        } else {
            REQUIRE(game.computer_act());
        }
        ++actions;
    }

    CHECK(game.is_game_over());
    CHECK(actions < 100);
    CHECK((game.player_dice_count() == 0 || game.computer_dice_count() == 0));
    CHECK(game.player_won() == (game.computer_dice_count() == 0));
}

TEST_CASE("completed liars dice match settles the active tavern location once") {
    GameSession session = GameSession::new_game(42);
    REQUIRE(session.enter_location(Location::restaurant));
    REQUIRE(session.start_location() != 0);
    REQUIRE(session.apply_action_result(test_support::completed_location_result(session)).accepted);
    REQUIRE(session.enter_location(Location::tavern));
    const int result_id = session.start_location();
    REQUIRE(result_id != 0);

    LiarsDiceGame game(42);
    while (!game.is_game_over()) {
        const int total_dice = game.player_dice_count() + game.computer_dice_count();
        REQUIRE(game.player_bid(total_dice, 6));
        REQUIRE(game.computer_act());
        if (!game.is_game_over()) REQUIRE(game.start_next_round());
    }

    const TavernChallengeConfig config;
    const auto result = simulate_tavern_challenge(
        session.player(), config, ChallengeType::liars_dice, BetTier::low,
        game.player_won() ? ChallengeOutcome::win : ChallengeOutcome::loss, result_id);
    REQUIRE(session.apply_action_result(result).accepted);

    CHECK(session.phase() == GamePhase::day_summary);
    CHECK(session.tavern_losses() == 1);
    CHECK_FALSE(session.apply_action_result(result).accepted);
}

}  // namespace
}  // namespace pixel_town
