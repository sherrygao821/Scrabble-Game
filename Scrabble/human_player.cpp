#include "human_player.h"

#include "exceptions.h"
#include "formatting.h"
#include "move.h"
#include "place_result.h"
#include "rang.h"
#include "tile_kind.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <vector>

using namespace std;

// this is for the commands + display
// This method is fully implemented.
inline string& to_upper(string& str) {
    transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
}

// gets input from the player and returns a valid move
Move HumanPlayer::get_move(const Board& board, const Dictionary& dictionary) const {
    // TODO: begin your implementation here.
    print_hand(cout);      // print what the player has right now
    PlaceResult temp("");  // to take the PlaceResult from board::test_place
    Move move;             // the move the player inputs
    bool is_word = true;   // whether the dictionary contains the word
    bool command_errors;   // whether there are command errors

    cout << "please make your move, " << this->get_name() << endl;
    // use a do-while loop to deal with any command exceptions
    do {
        command_errors = false;
        // get the move command from the player
        string m_string;
        getline(cin, m_string);
        if (cin.fail()) {
            continue;
        }
        cin.clear();
        cin.sync();

        // use a try-catch block to get any errors from parsing the tiles and parsing the move
        try {
            move = parse_move(m_string);
        } catch (const CommandException& e) {
            cerr << "please enter: " << endl;
            command_errors = true;
            continue;
        }

        // the player chooses to pass/exchange
        if (move.kind == MoveKind::PASS || move.kind == MoveKind::EXCHANGE) {
            return move;
        }

        // use test_place to check whether this move can be placed on board
        temp = board.test_place(move);
        if (!temp.valid) {
            cerr << "the move entered cannot be placed on the board, please enter again: " << endl;
            continue;
        }

        // check whether the words are in the dictionary
        for (unsigned int i = 0; i < temp.words.size(); ++i) {
            if (!dictionary.is_word((temp.words)[i])) {
                cerr << "the word entered is not in the dictionary, please enter again: " << endl;
                is_word = false;
                continue;
            }
        }
    }
    // ask the player to enter again when there are any command errors/cannot place on board/not in the dictionary
    while ((!temp.valid || !is_word) || command_errors);
    return move;
}

// helper function: parse the tiles into a vector of tileKinds
vector<TileKind> HumanPlayer::parse_tiles(string& letters) const {
    // TODO: begin implementation here.
    stringstream ss(letters);
    char temp;                       // to get each letter in the letters string
    char assigned;                   // to get any assigned letter for ? letters
    vector<TileKind> convert_tiles;  // the vector of tileKinds to return

    while (ss >> temp) {
        // the case when this is a ? letter
        if (temp == TileKind::BLANK_LETTER) {
            ss >> assigned;
            if (ss.fail()) {
                throw CommandException("please assign a letter to the blank tile");
            }
            TileKind question_mark(TileKind::BLANK_LETTER, 0);
            // check whether the ? letter is in the collection
            try {
                question_mark = tiles.lookup_tile(temp);
            } catch (const out_of_range& e) {
                throw CommandException("you do not have a blank tile in the bag");
            }
            question_mark.assigned = tolower(assigned);
            convert_tiles.push_back(question_mark);
        } else {
            // check whether the letter is in the collection
            try {
                convert_tiles.push_back(tiles.lookup_tile(temp));
            } catch (const out_of_range& e) {
                throw CommandException("you do not have the tile in the bag");
            }
        }
    }
    return convert_tiles;
}

// helper function: parse the move_string and return the move the player indicates
Move HumanPlayer::parse_move(string& move_string) const {
    // TODO: begin implementation here.
    stringstream ss(move_string);
    string temp_kind;  // to hold the move type
    ss >> temp_kind;
    // the case pass
    if (to_upper(temp_kind) == "PASS") {
        return Move();
    }
    // the case exchange
    else if (to_upper(temp_kind) == "EXCHANGE") {
        string tiles_string;
        ss >> tiles_string;
        return Move(parse_tiles(tiles_string));
    }
    // the case place
    else {
        string tiles_string;
        char dir_temp;
        size_t row;
        size_t column;
        ss >> dir_temp >> row >> column >> tiles_string;
        // throw out the error if the player does not enter the correct parameters
        if (ss.fail()) {
            throw CommandException("the parameters of your move are not correct");
        }
        // make the direction object
        Direction dir;
        if (dir_temp == '-') {
            dir = Direction::ACROSS;
        } else if (dir_temp == '|') {
            dir = Direction::DOWN;
        } else {
            throw CommandException("the direction of your move is not valid");
        }
        return Move(parse_tiles(tiles_string), row - 1, column - 1, dir);
    }
}

// This function is fully implemented.
void HumanPlayer::print_hand(ostream& out) const {
    const size_t tile_count = tiles.count_tiles();
    const size_t empty_tile_count = this->get_hand_size() - tile_count;
    const size_t empty_tile_width = empty_tile_count * (SQUARE_OUTER_WIDTH - 1);

    for (size_t i = 0; i < HAND_TOP_MARGIN - 2; ++i) {
        out << endl;
    }

    out << repeat(SPACE, HAND_LEFT_MARGIN) << FG_COLOR_HEADING << "Your Hand: " << endl << endl;

    // Draw top line
    out << repeat(SPACE, HAND_LEFT_MARGIN) << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE;
    print_horizontal(tile_count, L_TOP_LEFT, T_DOWN, L_TOP_RIGHT, out);
    out << repeat(SPACE, empty_tile_width) << BG_COLOR_OUTSIDE_BOARD << endl;

    // Draw middle 3 lines
    for (size_t line = 0; line < SQUARE_INNER_HEIGHT; ++line) {
        out << FG_COLOR_LABEL << BG_COLOR_OUTSIDE_BOARD << repeat(SPACE, HAND_LEFT_MARGIN);
        for (auto it = tiles.cbegin(); it != tiles.cend(); ++it) {
            out << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE << I_VERTICAL << BG_COLOR_PLAYER_HAND;

            // Print letter
            if (line == 1) {
                out << repeat(SPACE, 2) << FG_COLOR_LETTER << (char)toupper(it->letter) << repeat(SPACE, 2);

                // Print score in bottom right
            } else if (line == SQUARE_INNER_HEIGHT - 1) {
                out << FG_COLOR_SCORE << repeat(SPACE, SQUARE_INNER_WIDTH - 2) << setw(2) << it->points;

            } else {
                out << repeat(SPACE, SQUARE_INNER_WIDTH);
            }
        }
        if (tiles.count_tiles() > 0) {
            out << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE << I_VERTICAL;
            out << repeat(SPACE, empty_tile_width) << BG_COLOR_OUTSIDE_BOARD << endl;
        }
    }

    // Draw bottom line
    out << repeat(SPACE, HAND_LEFT_MARGIN) << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE;
    print_horizontal(tile_count, L_BOTTOM_LEFT, T_UP, L_BOTTOM_RIGHT, out);
    out << repeat(SPACE, empty_tile_width) << rang::style::reset << endl;
}
