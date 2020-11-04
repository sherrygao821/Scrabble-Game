#include "board.h"

#include "board_square.h"
#include "exceptions.h"
#include "formatting.h"
#include <fstream>
#include <iomanip>

using namespace std;

bool Board::Position::operator==(const Board::Position& other) const {
    return this->row == other.row && this->column == other.column;
}

bool Board::Position::operator!=(const Board::Position& other) const {
    return this->row != other.row || this->column != other.column;
}

Board::Position Board::Position::translate(Direction direction) const { return this->translate(direction, 1); }

Board::Position Board::Position::translate(Direction direction, ssize_t distance) const {
    if (direction == Direction::DOWN) {
        return Board::Position(this->row + distance, this->column);
    } else {
        return Board::Position(this->row, this->column + distance);
    }
}

Board Board::read(const string& file_path) {
    ifstream file(file_path);
    if (!file) {
        throw FileException("cannot open board file!");
    }

    size_t rows;
    size_t columns;
    size_t starting_row;
    size_t starting_column;
    file >> rows >> columns >> starting_row >> starting_column;
    Board board(rows, columns, starting_row, starting_column);

    // TODO: complete implementation of reading in board from file here.
    for (unsigned int i = 0; i < rows; ++i) {
        std::vector<BoardSquare> temp;
        for (unsigned int j = 0; j < columns; ++j) {
            char step;
            file >> step;
            // check the abnormality when there are not enough steps
            if (file.fail()) {
                throw FileException("the board file is not correct!");
            }
            // check different conditions and read into the vector with their respective multipliers
            if (step == '.') {
                temp.push_back(BoardSquare(1, 1));
            } else if (step == '2') {
                temp.push_back(BoardSquare(2, 1));
            } else if (step == '3') {
                temp.push_back(BoardSquare(3, 1));
            } else if (step == 'd') {
                temp.push_back(BoardSquare(1, 2));
            } else if (step == 't') {
                temp.push_back(BoardSquare(1, 3));
            } else {
                throw FileException("the board file is not correct!");
            }
        }
        // push back each row
        board.squares.push_back(temp);
    }
    file.close();
    return board;
}

size_t Board::get_move_index() const { return this->move_index; }

PlaceResult Board::test_place(const Move& move) const {
    // TODO: complete implementation here
    // check for the validity of placing the word on the board
    bool flag = false;                    // this is for checking whether there are adjacent letters
    bool flag_start = false;              // this is for checking whether the first words covers the starting position
    string temp("");                      // this is for the "main" word the user inputs
    vector<string> valid_words;           // this contains all valid words formed("main" word and extra words formed)
    Position pos(move.row, move.column);  // current position
    int temp_pts = 0, temp_multi = 1, final_pts = 0;  // these are for calculating the points

    for (unsigned int i = 0; i < move.tiles.size(); ++i) {
        // take care of the blank letters
        char every_letter;
        if (move.tiles[i].letter == TileKind::BLANK_LETTER) {
            every_letter = move.tiles[i].assigned;
        } else {
            every_letter = move.tiles[i].letter;
        }

        // check whether or not this is the first move
        if (!move_index) {
            // check whether or not this move starts at the correct position
            if (pos == start) {
                flag_start = true;
            }
            flag = true;
        }

        // check whether the current tile is in bound and occupied
        if (!is_in_bounds(pos)) {
            return PlaceResult("this position is out of bounds, please enter again");
        }
        if (in_bounds_and_has_tile(pos)) {
            if (i == 0) {
                return PlaceResult("this is an invalid move");
            }
            flag = true;
            TileKind occupied = squares[pos.row][pos.column].get_tile_kind();
            if (occupied.letter == TileKind::BLANK_LETTER) {
                temp.push_back(occupied.assigned);
            } else {
                temp.push_back(occupied.letter);
            }
            // NOTICE: only the letters entered by the user counts word/letter premium
            temp_pts += (squares[pos.row][pos.column].get_tile_kind()).points;
            pos = pos.translate(move.direction);
            i--;
            continue;
        }

        // check the situation when the first letter the player inputs is not the first letter of the word
        if (i == 0) {
            string before_first_letter("");
            Position temp_pos(pos.row, pos.column);
            temp_pos = temp_pos.translate(move.direction, -1);
            while (in_bounds_and_has_tile(temp_pos)) {
                flag = true;
                // check whether it is a blank letter ahead
                if (at(temp_pos).get_tile_kind().letter == TileKind::BLANK_LETTER) {
                    before_first_letter.push_back(at(temp_pos).get_tile_kind().assigned);
                } else {
                    before_first_letter.push_back(at(temp_pos).get_tile_kind().letter);
                }
                // update the points and push the letters into temp
                temp_pts += (squares[temp_pos.row][temp_pos.column].get_tile_kind()).points;
                temp_pos = temp_pos.translate(move.direction, -1);
            }
            // since this is before the given letter, we need to reverse it
            if (before_first_letter.size() != 0) {
                for (unsigned int j = 0; j < before_first_letter.size(); ++j) {
                    temp.push_back(before_first_letter[before_first_letter.size() - 1 - j]);
                }
                flag = true;
            }
        }

        // check for other words and push it into the vector of words
        string other_words("");
        int other_pts = 0;
        int other_multi = 1;
        // checking whether or not there is a letter on the up or left direction
        Position up_left(pos.row, pos.column);
        up_left = up_left.translate(!move.direction, -1);
        string reverse("");
        while (in_bounds_and_has_tile(up_left)) {
            flag = true;
            if (at(up_left).get_tile_kind().letter == TileKind::BLANK_LETTER) {
                reverse.push_back(at(up_left).get_tile_kind().assigned);
            } else {
                reverse.push_back(at(up_left).get_tile_kind().letter);
            }
            other_pts += (squares[up_left.row][up_left.column].get_tile_kind()).points;
            up_left = up_left.translate(!move.direction, -1);
        }
        // since this is on the up/left the of letter, we need to reverse it
        if (reverse.size() != 0) {
            for (unsigned int j = 0; j < reverse.size(); ++j) {
                other_words.push_back(reverse[reverse.size() - 1 - j]);
            }
        }

        // add the current letter into other_words if there is any adjacent letters
        Position down_right(pos.row, pos.column);
        down_right = down_right.translate(!move.direction);
        if (other_words.size() != 0 || in_bounds_and_has_tile(down_right)) {
            other_words.push_back(every_letter);
            other_pts += move.tiles[i].points * squares[pos.row][pos.column].letter_multiplier;
            other_multi *= squares[pos.row][pos.column].word_multiplier;
        }

        // checking whether or not there is a letter on the down or right direction
        while (in_bounds_and_has_tile(down_right)) {
            flag = true;
            if (at(down_right).get_tile_kind().letter == TileKind::BLANK_LETTER) {
                other_words.push_back(at(down_right).get_tile_kind().assigned);
            } else {
                other_words.push_back(at(down_right).get_tile_kind().letter);
            }
            other_pts += (squares[down_right.row][down_right.column].get_tile_kind()).points;
            down_right = down_right.translate(!move.direction);
        }

        // adding other words formed into the vector
        final_pts += other_pts * other_multi;
        if (other_words.size() != 0) {
            valid_words.push_back(other_words);
        }

        // push back the current letter into temp(the "main" word)
        temp.push_back(every_letter);
        temp_pts += move.tiles[i].points * squares[pos.row][pos.column].letter_multiplier;
        temp_multi *= squares[pos.row][pos.column].word_multiplier;

        // check the situation when the last letter the player inputs is not the last letter of the word
        if (i == move.tiles.size() - 1) {
            Position temp_pos(pos.row, pos.column);
            temp_pos = temp_pos.translate(move.direction);
            while (in_bounds_and_has_tile(temp_pos)) {
                flag = true;
                if (at(temp_pos).get_tile_kind().letter == TileKind::BLANK_LETTER) {
                    temp.push_back(at(temp_pos).get_tile_kind().assigned);
                } else {
                    temp.push_back(at(temp_pos).get_tile_kind().letter);
                }
                temp_pts += (squares[temp_pos.row][temp_pos.column].get_tile_kind()).points;
                temp_pos = temp_pos.translate(move.direction);
            }
        }

        // update the position according to respective direction
        pos = pos.translate(move.direction);
    }
    // return the false PlaceResult due to no adjacent letters
    if (!flag) {
        return PlaceResult("no adjacent letters for this word");
    }
    // return the false PlaceResult when the first word does not covers the starting position
    if (flag_start == false && !move_index) {
        return PlaceResult("the starting move does not start on the starting position");
    }

    // push back the main string when the size of the string is more than 1 letter
    if (temp.size() > 1) {
        valid_words.push_back(temp);
    }
    // subtract the repetitive pts added earily for the single character
    else {
        temp_pts = 0;
    }

    // return the false PlaceResult when there are no valid words formed
    if (valid_words.size() == 0) {
        return PlaceResult("no valid words formed");
    }

    // return the correct PlaceResult
    final_pts += temp_pts * temp_multi;
    // 50 pts bonus points
    return PlaceResult(valid_words, final_pts);
}

PlaceResult Board::place(const Move& move) {
    // TODO: Complete implementation here
    Position pos(move.row, move.column);
    PlaceResult placeresult = test_place(move);
    if (!placeresult.valid) {
        return PlaceResult("this is not a valid move");
    }
    for (unsigned int i = 0; i < move.tiles.size(); ++i) {
        // check the case when the position is occupied
        if (in_bounds_and_has_tile(pos)) {
            i--;
            pos = pos.translate(move.direction);
            continue;
        }
        squares[pos.row][pos.column].set_tile_kind(move.tiles[i]);
        pos = pos.translate(move.direction);
    }
    move_index++;
    return placeresult;
}

// The rest of this file is provided for you. No need to make changes.

BoardSquare& Board::at(const Board::Position& position) { return this->squares.at(position.row).at(position.column); }

const BoardSquare& Board::at(const Board::Position& position) const {
    return this->squares.at(position.row).at(position.column);
}

bool Board::is_in_bounds(const Board::Position& position) const {
    return position.row < this->rows && position.column < this->columns;
}

bool Board::in_bounds_and_has_tile(const Position& position) const {
    return is_in_bounds(position) && at(position).has_tile();
}

void Board::print(ostream& out) const {
    // Draw horizontal number labels
    for (size_t i = 0; i < BOARD_TOP_MARGIN - 2; ++i) {
        out << std::endl;
    }
    out << FG_COLOR_LABEL << repeat(SPACE, BOARD_LEFT_MARGIN);
    const size_t right_number_space = (SQUARE_OUTER_WIDTH - 3) / 2;
    const size_t left_number_space = (SQUARE_OUTER_WIDTH - 3) - right_number_space;
    for (size_t column = 0; column < this->columns; ++column) {
        out << repeat(SPACE, left_number_space) << std::setw(2) << column + 1 << repeat(SPACE, right_number_space);
    }
    out << std::endl;

    // Draw top line
    out << repeat(SPACE, BOARD_LEFT_MARGIN);
    print_horizontal(this->columns, L_TOP_LEFT, T_DOWN, L_TOP_RIGHT, out);
    out << endl;

    // Draw inner board
    for (size_t row = 0; row < this->rows; ++row) {
        if (row > 0) {
            out << repeat(SPACE, BOARD_LEFT_MARGIN);
            print_horizontal(this->columns, T_RIGHT, PLUS, T_LEFT, out);
            out << endl;
        }

        // Draw insides of squares
        for (size_t line = 0; line < SQUARE_INNER_HEIGHT; ++line) {
            out << FG_COLOR_LABEL << BG_COLOR_OUTSIDE_BOARD;

            // Output column number of left padding
            if (line == 1) {
                out << repeat(SPACE, BOARD_LEFT_MARGIN - 3);
                out << std::setw(2) << row + 1;
                out << SPACE;
            } else {
                out << repeat(SPACE, BOARD_LEFT_MARGIN);
            }

            // Iterate columns
            for (size_t column = 0; column < this->columns; ++column) {
                out << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE << I_VERTICAL;
                const BoardSquare& square = this->squares.at(row).at(column);
                bool is_start = this->start.row == row && this->start.column == column;

                // Figure out background color
                if (square.word_multiplier == 2) {
                    out << BG_COLOR_WORD_MULTIPLIER_2X;
                } else if (square.word_multiplier == 3) {
                    out << BG_COLOR_WORD_MULTIPLIER_3X;
                } else if (square.letter_multiplier == 2) {
                    out << BG_COLOR_LETTER_MULTIPLIER_2X;
                } else if (square.letter_multiplier == 3) {
                    out << BG_COLOR_LETTER_MULTIPLIER_3X;
                } else if (is_start) {
                    out << BG_COLOR_START_SQUARE;
                }

                // Text
                if (line == 0 && is_start) {
                    out << "  \u2605  ";
                } else if (line == 0 && square.word_multiplier > 1) {
                    out << FG_COLOR_MULTIPLIER << repeat(SPACE, SQUARE_INNER_WIDTH - 2) << 'W' << std::setw(1)
                        << square.word_multiplier;
                } else if (line == 0 && square.letter_multiplier > 1) {
                    out << FG_COLOR_MULTIPLIER << repeat(SPACE, SQUARE_INNER_WIDTH - 2) << 'L' << std::setw(1)
                        << square.letter_multiplier;
                } else if (line == 1 && square.has_tile()) {
                    char l = square.get_tile_kind().letter == TileKind::BLANK_LETTER ? square.get_tile_kind().assigned
                                                                                     : ' ';
                    out << repeat(SPACE, 2) << FG_COLOR_LETTER << square.get_tile_kind().letter << l
                        << repeat(SPACE, 1);
                } else if (line == SQUARE_INNER_HEIGHT - 1 && square.has_tile()) {
                    out << repeat(SPACE, SQUARE_INNER_WIDTH - 1) << FG_COLOR_SCORE << square.get_points();
                } else {
                    out << repeat(SPACE, SQUARE_INNER_WIDTH);
                }
            }

            // Add vertical line
            out << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE << I_VERTICAL << BG_COLOR_OUTSIDE_BOARD << std::endl;
        }
    }

    // Draw bottom line
    out << repeat(SPACE, BOARD_LEFT_MARGIN);
    print_horizontal(this->columns, L_BOTTOM_LEFT, T_UP, L_BOTTOM_RIGHT, out);
    out << endl << rang::style::reset << std::endl;
}

// New Functions for HW5
char Board::letter_at(Position p) const {
    char temp = squares[p.row][p.column].get_tile_kind().letter;
    if (temp == TileKind::BLANK_LETTER) {
        temp = squares[p.row][p.column].get_tile_kind().assigned;
    }
    return temp;
}

bool Board::is_anchor_spot(Position p) const {
    if (!is_in_bounds(p))
        return false;
    else if (in_bounds_and_has_tile(p))
        return false;
    else if (move_index == 0 && p == start)
        return true;
    else if (  // check whether there are adjacent tiles
            in_bounds_and_has_tile(Position(p.row + 1, p.column))
            || in_bounds_and_has_tile(Position(p.row - 1, p.column))
            || in_bounds_and_has_tile(Position(p.row, p.column + 1))
            || in_bounds_and_has_tile(Position(p.row, p.column - 1)))
        return true;
    return false;
}

vector<Board::Anchor> Board::get_anchors() const {
    vector<Board::Anchor> all_anchors;
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < columns; j++) {
            if (!is_anchor_spot(Position(i, j)))
                continue;  // check whether this spot is a valid anchor spot
            // the first anchor for the ACROSS direction
            int limit = 0;
            Position left_cursor(i, j - 1);
            while (is_in_bounds(left_cursor) && !in_bounds_and_has_tile(left_cursor) && !is_anchor_spot(left_cursor)) {
                limit++;
                left_cursor.column--;
            }
            all_anchors.push_back(Anchor(Position(i, j), Direction::ACROSS, limit));
            // the second anchor for the DOWN direction
            limit = 0;
            Position up_cursor(i - 1, j);
            while (is_in_bounds(up_cursor) && !in_bounds_and_has_tile(up_cursor) && !is_anchor_spot(up_cursor)) {
                limit++;
                up_cursor.row--;
            }
            all_anchors.push_back(Anchor(Position(i, j), Direction::DOWN, limit));
        }
    }
    return all_anchors;
}
