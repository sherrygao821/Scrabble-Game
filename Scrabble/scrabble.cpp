#include "scrabble.h"

#include "formatting.h"
#include <iomanip>
#include <iostream>
#include <map>

using namespace std;

// Given to you. this does not need to be changed
Scrabble::Scrabble(const ScrabbleConfig& config)
        : hand_size(config.hand_size),
          minimum_word_length(config.minimum_word_length),
          tile_bag(TileBag::read(config.tile_bag_file_path, config.seed)),
          board(Board::read(config.board_file_path)),
          dictionary(Dictionary::read(config.dictionary_file_path)) {}

void Scrabble::add_players() {
    // get the number of players and their names as well
    cout << "Please enter number of players: ";
    int num_players;
    cin >> num_players;
    while (cin.fail() || (num_players > 8 || num_players < 1)) {
        cout << "Please enter number of players: ";
        cin >> num_players;
    }
    cout << num_players << " players confirmed." << endl;
    cin.clear();
    cin.sync();

    // make shared_ptr and push it into the vector Players
    for (int i = 0; i < num_players; ++i) {
        string temp_name;
        cout << "Please enter name for " << i + 1 << ": " << endl;
        getline(cin, temp_name);
        getline(cin, temp_name);
        cin.clear();
        cin.sync();
        // specify the case when this is a computer player
        char is_computer;
        cout << "Is this player a computer? (y/n)" << endl;
        shared_ptr<Player> curr = make_shared<ComputerPlayer>(temp_name, hand_size);
        // check whether or not this is computer player
        while (cin >> is_computer) {
            if (!cin.fail()) {
                if (is_computer == 'n') {
                    num_human++;  // increment for further use: check the ending of the game
                    curr = make_shared<HumanPlayer>(temp_name, hand_size);
                    cin.clear();
                    cin.sync();
                    break;
                } else if (is_computer == 'y') {
                    num_computer++;
                    curr = make_shared<ComputerPlayer>(temp_name, hand_size);
                    cin.clear();
                    cin.sync();
                    break;
                }
            }
            cout << "Is this player a computer? (y/n)" << endl;
        }
        players.push_back(curr);
        cin.clear();
        cin.sync();
        // draw hand_size of tiles for each player
        players[i]->add_tiles(tile_bag.remove_random_tiles(hand_size));
    }
}

// Game Loop should cycle through players and get and execute that players move
// until the game is over.
void Scrabble::game_loop() {
    // TODO: implement this.
    unsigned int all_pass = 0;  // how many consecutive PASS the players have entered
    bool finish = false;        // whether or not the game is finished

    while (!finish) {
        // this is one round for all players
        for (unsigned int i = 0; i < players.size(); ++i) {
            bool move_errors;  // whether or not there are move errors
            if (finish) {
                break;
            }  // if the game is finished, leave the for loop
            do {
                move_errors = false;

                board.print(cout);  // print the board

                // get the valid move for this player
                Move curr_move = players[i]->get_move(board, dictionary);

                unsigned int points_gained = 0;

                // When the player demands exchange
                if (curr_move.kind == MoveKind::EXCHANGE) {
                    all_pass = 0;
                    // remove given tiles from the bag
                    try {
                        players[i]->remove_tiles(curr_move.tiles);
                    } catch (const MoveException& e) {
                        move_errors = true;
                        continue;
                    }
                    // draw equal number of new tiles
                    for (unsigned int k = 0; k < (curr_move.tiles).size(); ++k) {
                        tile_bag.add_tile((curr_move.tiles)[k]);
                    }
                    if (tile_bag.count_tiles() < (curr_move.tiles).size()) {
                        players[i]->add_tiles(tile_bag.remove_random_tiles(tile_bag.count_tiles()));
                    } else {
                        players[i]->add_tiles(tile_bag.remove_random_tiles((curr_move.tiles).size()));
                    }
                }

                // When the player demands place: remove given tiles from the bag + draw equal number of new tiles
                else if (curr_move.kind == MoveKind::PLACE) {
                    if (players[i]->is_human()) {
                        all_pass = 0;
                    }
                    // check minimal word length
                    PlaceResult temp = board.test_place(curr_move);
                    try {
                        // check for the case when a word formed is less than the minimum_word_length
                        for (unsigned int j = 0; j < (temp.words).size(); ++j) {
                            if ((temp.words)[i].size() < minimum_word_length) {
                                throw MoveException("move errors");
                            }
                        }
                    } catch (const MoveException& e) {
                        move_errors = true;
                        continue;
                    }
                    // check whether the player uses up all of the tiles at one round
                    if ((curr_move.tiles).size() == hand_size && players[i]->is_human()) {
                        players[i]->add_points(50);
                        points_gained += 50;
                    }
                    // place the move on the board
                    board.place(curr_move);
                    // update the points
                    points_gained = temp.points;
                    players[i]->add_points(points_gained);
                    // remove given tiles from the bag
                    try {
                        players[i]->remove_tiles(curr_move.tiles);
                    } catch (const MoveException& e) {
                        move_errors = true;
                        continue;
                    }
                    // draw equal number of new tiles
                    if (tile_bag.count_tiles() < (curr_move.tiles).size()) {
                        players[i]->add_tiles(tile_bag.remove_random_tiles(tile_bag.count_tiles()));
                    } else {
                        players[i]->add_tiles(tile_bag.remove_random_tiles((curr_move.tiles).size()));
                    }
                }

                // When the player demands to pass: update the all_pass index
                if (curr_move.kind == MoveKind::PASS && players[i]->is_human()) {
                    all_pass++;
                }

                cout << "You gained " << SCORE_COLOR << points_gained << rang::style::reset << " points!" << endl;
                cout << "Your current score: " << SCORE_COLOR << players[i]->get_points() << rang::style::reset << endl;
                // check whether the game finishes
                if ((players[i]->count_tiles() == 0 && tile_bag.count_tiles() == 0) || all_pass == (size_t)num_human) {
                    finish = true;
                    break;
                }
                cout << endl << "Press [enter] to continue";
                cin.ignore();
            } while (move_errors);
        }
    }
}

// Performs final score subtraction. Players lose points for each tile in their
// hand. The player who cleared their hand receives all the points lost by the
// other players.
void Scrabble::final_subtraction(vector<shared_ptr<Player>>& plrs) {
    // TODO: implement this method.
    // Do not change the method signature.
    int total_pts = 0;      // the points to add to the winner if there is one
    int winner_index = -1;  // the one who uses up every letter

    // for each player, their hand_value needed to be subtracted
    for (unsigned int i = 0; i < plrs.size(); ++i) {
        if (plrs[i]->get_hand_value() == 0) {
            winner_index = i;
        }
        // if the player reaches a negative score, make it 0
        if (plrs[i]->get_hand_value() > plrs[i]->get_points()) {
            plrs[i]->subtract_points(plrs[i]->get_points());
        } else {
            plrs[i]->subtract_points(plrs[i]->get_hand_value());
        }
        total_pts += plrs[i]->get_hand_value();
    }

    // if there is a winner
    if (winner_index != -1) {
        plrs[winner_index]->add_points(total_pts);
    }
}

// You should not need to change this function.
void Scrabble::print_result() {
    // Determine highest score
    size_t max_points = 0;
    for (auto player : this->players) {
        if (player->get_points() > max_points) {
            max_points = player->get_points();
        }
    }

    // Determine the winner(s) indexes
    vector<shared_ptr<Player>> winners;
    for (auto player : this->players) {
        if (player->get_points() >= max_points) {
            winners.push_back(player);
        }
    }

    cout << (winners.size() == 1 ? "Winner:" : "Winners: ");
    for (auto player : winners) {
        cout << SPACE << PLAYER_NAME_COLOR << player->get_name();
    }
    cout << rang::style::reset << endl;

    // now print score table
    cout << "Scores: " << endl;
    cout << "---------------------------------" << endl;

    // Justify all integers printed to have the same amount of character as the high score, left-padding with spaces
    cout << setw(static_cast<uint32_t>(floor(log10(max_points) + 1)));

    for (auto player : this->players) {
        cout << SCORE_COLOR << player->get_points() << rang::style::reset << " | " << PLAYER_NAME_COLOR
             << player->get_name() << rang::style::reset << endl;
    }
}

// You should not need to change this.
void Scrabble::main() {
    add_players();
    game_loop();
    final_subtraction(this->players);
    print_result();
}
