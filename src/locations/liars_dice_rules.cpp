#include "locations/liars_dice_rules.hpp"

namespace pixel_town {

std::array<int, LiarsDiceGame::kDiceCount> LiarsDiceGame::roll_dice(unsigned int& seed) {
    std::array<int, kDiceCount> dice{};
    for (int i = 0; i < kDiceCount; ++i) {
        seed = seed * 1103515245U + 12345U;
        dice[i] = static_cast<int>((seed >> 16) % kMaxFace) + 1;
    }
    return dice;
}

LiarsDiceGame::LiarsDiceGame(unsigned int seed) : seed_(seed) {
    roll_round_dice();
}

const std::array<int, LiarsDiceGame::kDiceCount>& LiarsDiceGame::player_dice() const {
    return player_dice_;
}

const std::array<int, LiarsDiceGame::kDiceCount>& LiarsDiceGame::computer_dice() const {
    return computer_dice_;
}

int LiarsDiceGame::bid_count() const { return bid_count_; }
int LiarsDiceGame::bid_face() const { return bid_face_; }
bool LiarsDiceGame::is_player_turn() const { return player_turn_; }
bool LiarsDiceGame::is_round_over() const { return round_over_; }
bool LiarsDiceGame::is_game_over() const { return game_over_; }
bool LiarsDiceGame::player_won() const { return player_won_; }
bool LiarsDiceGame::dice_revealed() const { return dice_revealed_; }
int LiarsDiceGame::player_dice_count() const { return player_dice_count_; }
int LiarsDiceGame::computer_dice_count() const { return computer_dice_count_; }

ChallengeInitiator LiarsDiceGame::who_challenged() const { return who_challenged_; }
LiarsDiceParticipant LiarsDiceGame::round_loser() const { return round_loser_; }
int LiarsDiceGame::actual_count() const { return actual_count_; }
bool LiarsDiceGame::bid_was_valid() const { return bid_was_valid_; }

std::optional<LiarsDiceBid> LiarsDiceGame::minimum_legal_bid() const {
    if (round_over_ || game_over_) return std::nullopt;
    const int max_count = player_dice_count_ + computer_dice_count_;
    if (bid_count_ == 0) return LiarsDiceBid{1, kMinFace};
    if (bid_face_ < kMaxFace) return LiarsDiceBid{bid_count_, bid_face_ + 1};
    if (bid_count_ < max_count) return LiarsDiceBid{bid_count_ + 1, kMinFace};
    return std::nullopt;
}

bool LiarsDiceGame::is_bid_legal(int count, int face) const {
    if (count < 1 || count > player_dice_count_ + computer_dice_count_) return false;
    if (face < kMinFace || face > kMaxFace) return false;
    if (bid_count_ == 0) return true;
    if (count > bid_count_) return true;
    if (count == bid_count_ && face > bid_face_) return true;
    return false;
}

int LiarsDiceGame::count_face(int face) const {
    int total = 0;
    for (int i = 0; i < player_dice_count_; ++i) {
        const int die = player_dice_[i];
        if (die == face || (face != 1 && die == 1)) ++total;
    }
    for (int i = 0; i < computer_dice_count_; ++i) {
        const int die = computer_dice_[i];
        if (die == face || (face != 1 && die == 1)) ++total;
    }
    return total;
}

void LiarsDiceGame::roll_round_dice() {
    player_dice_ = roll_dice(seed_);
    computer_dice_ = roll_dice(seed_);
}

void LiarsDiceGame::resolve_challenge(ChallengeInitiator challenger) {
    dice_revealed_ = true;
    round_over_ = true;
    who_challenged_ = challenger;
    actual_count_ = count_face(bid_face_);
    bid_was_valid_ = (actual_count_ >= bid_count_);

    const bool player_loses =
        (challenger == ChallengeInitiator::player && bid_was_valid_) ||
        (challenger == ChallengeInitiator::computer && !bid_was_valid_);
    if (player_loses) {
        --player_dice_count_;
        round_loser_ = LiarsDiceParticipant::player;
        next_round_player_starts_ = true;
    } else {
        --computer_dice_count_;
        round_loser_ = LiarsDiceParticipant::computer;
        next_round_player_starts_ = false;
    }

    game_over_ = player_dice_count_ == 0 || computer_dice_count_ == 0;
    player_won_ = game_over_ && computer_dice_count_ == 0;
}

bool LiarsDiceGame::player_bid(int count, int face) {
    if (!player_turn_ || round_over_ || game_over_) return false;
    if (!is_bid_legal(count, face)) return false;
    bid_count_ = count;
    bid_face_ = face;
    player_turn_ = false;
    return true;
}

bool LiarsDiceGame::player_challenge() {
    if (!player_turn_ || round_over_ || game_over_) return false;
    if (bid_count_ == 0) return false;
    resolve_challenge(ChallengeInitiator::player);
    return true;
}

bool LiarsDiceGame::computer_act() {
    if (player_turn_ || round_over_ || game_over_) return false;

    if (bid_count_ == 0) {
        int best_face = 1;
        int best_cnt = 0;
        for (int f = 1; f <= kMaxFace; ++f) {
            int cnt = 0;
            for (int i = 0; i < computer_dice_count_; ++i) {
                const int die = computer_dice_[i];
                if (die == f || (f != 1 && die == 1)) ++cnt;
            }
            if (cnt > best_cnt) {
                best_cnt = cnt;
                best_face = f;
            }
        }
        bid_count_ = 1;
        bid_face_ = best_face;
        player_turn_ = true;
        return true;
    }

    int my_count = 0;
    for (int i = 0; i < computer_dice_count_; ++i) {
        const int die = computer_dice_[i];
        if (die == bid_face_ || (bid_face_ != 1 && die == 1)) ++my_count;
    }
    const int max_possible = my_count + player_dice_count_;

    if (bid_count_ > max_possible || bid_count_ > my_count + 2 ||
        !minimum_legal_bid().has_value()) {
        resolve_challenge(ChallengeInitiator::computer);
        return true;
    }

    if (bid_face_ < kMaxFace) {
        bid_face_ = bid_face_ + 1;
    } else {
        bid_count_ = bid_count_ + 1;
        bid_face_ = 1;
    }
    player_turn_ = true;
    return true;
}

bool LiarsDiceGame::start_next_round() {
    if (!round_over_ || game_over_) return false;

    bid_count_ = 0;
    bid_face_ = kMinFace;
    player_turn_ = next_round_player_starts_;
    round_over_ = false;
    dice_revealed_ = false;
    who_challenged_ = ChallengeInitiator::none;
    round_loser_ = LiarsDiceParticipant::none;
    actual_count_ = 0;
    bid_was_valid_ = false;
    roll_round_dice();
    return true;
}

}  // namespace pixel_town
