#include "player.h"

using namespace std;

// TODO: implement member functions
// Adds points to player's score
void Player::add_points(size_t points) { this->points += points; }

// Subtracts points from player's score
void Player::subtract_points(size_t points) { this->points -= points; }

// get the corrent points of the player
size_t Player::get_points() const { return this->points; }

// get the name of the player
const std::string& Player::get_name() const { return this->name; }

// Returns the number of tiles in a player's hand.
size_t Player::count_tiles() const { return this->tiles.count_tiles(); }

// Removes tiles from player's hand.
void Player::remove_tiles(const std::vector<TileKind>& tiles) {
    for (unsigned int i = 0; i < tiles.size(); ++i) {
        this->tiles.remove_tile(tiles[i]);
    }
}

// Adds tiles to player's hand.
void Player::add_tiles(const std::vector<TileKind>& tiles) {
    for (unsigned int i = 0; i < tiles.size(); ++i) {
        this->tiles.add_tile(tiles[i]);
    }
}

// Checks if player has a matching tile.
bool Player::has_tile(TileKind tile) {
    auto iter = this->tiles.cbegin();
    while (iter != this->tiles.cend()) {
        if (*iter == tile) {
            return true;
        }
        iter++;
    }
    return false;
}

// Returns the total points of all tiles in the players hand.
unsigned int Player::get_hand_value() const { return this->tiles.total_points(); }

size_t Player::get_hand_size() const { return this->tiles.count_tiles(); }
