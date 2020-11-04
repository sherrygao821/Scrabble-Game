#include "computer_player.h"

#include <iostream>
#include <memory>
#include <string>

void ComputerPlayer::left_part(
        Board::Position anchor_pos,
        std::string partial_word,
        Move partial_move,
        std::shared_ptr<Dictionary::TrieNode> node,
        size_t limit,
        TileCollection& remaining_tiles,
        std::vector<Move>& legal_moves,
        const Board& board) const {
    // HW5: IMPLEMENT THIS
    extend_right(anchor_pos, partial_word, partial_move, node, remaining_tiles, legal_moves, board);
    if (limit == 0)  // base case when the maximum numbers of letters are used for the left part
        return;
    if (node->nexts.empty())
        return;
    for (auto iter = node->nexts.cbegin(); iter != node->nexts.cend(); iter++) {
        char letter = iter->first;  // get the letter of the ith child node
        bool have_blank = false;
        // check to see if the player has that letter in the tile collection
        try {  // if the player has it, remove it
            remaining_tiles.lookup_tile(letter);
        } catch (const std::out_of_range& e) {  // if not, try the next child node
            try {
                remaining_tiles.lookup_tile(TileKind::BLANK_LETTER);
                have_blank = true;
            } catch (const std::out_of_range& e) {
                continue;
            }
        }
        // udpate the partial_word and the partial move
        TileKind temp_tile_kind(letter, 0);
        if (have_blank) {
            temp_tile_kind = remaining_tiles.lookup_tile(TileKind::BLANK_LETTER);
            temp_tile_kind.assigned = letter;
        } else {
            temp_tile_kind = remaining_tiles.lookup_tile(letter);
        }
        // update tiles in the bag, partial_word, and partial_move
        remaining_tiles.remove_tile(temp_tile_kind);
        partial_word.push_back(letter);
        partial_move.tiles.push_back(temp_tile_kind);
        if (partial_move.direction == Direction::ACROSS) {
            partial_move.column--;
        } else {
            partial_move.row--;
        }
        // the recursive case
        left_part(anchor_pos, partial_word, partial_move, iter->second, limit - 1, remaining_tiles, legal_moves, board);
        // undo what has been done (partial_word and move)
        partial_word.pop_back();
        remaining_tiles.add_tile(temp_tile_kind);
        if (partial_move.direction == Direction::ACROSS) {
            partial_move.column++;
        } else {
            partial_move.row++;
        }
        partial_move.tiles.pop_back();
    }
}

void ComputerPlayer::extend_right(
        Board::Position square,
        std::string partial_word,
        Move partial_move,
        std::shared_ptr<Dictionary::TrieNode> node,
        TileCollection& remaining_tiles,
        std::vector<Move>& legal_moves,
        const Board& board) const {
    // HW5: IMPLEMENT THIS
    // base case when square is out of bound
    if (!board.is_in_bounds(square)) {
        if (node->is_final) {
            legal_moves.push_back(partial_move);
        }
        return;
    }

    // check whether the current word is final
    if (node->is_final) {
        legal_moves.push_back(partial_move);
    }

    // the recursive case when the next square is not vacant
    if (board.in_bounds_and_has_tile(square)) {
        auto it = node->nexts.find(board.letter_at(square));
        if (it == node->nexts.cend())
            return;
        Board::Position next_square = square;
        if (partial_move.direction == Direction::ACROSS) {
            next_square.column++;
        } else {
            next_square.row++;
        }
        extend_right(next_square, partial_word, partial_move, it->second, remaining_tiles, legal_moves, board);
    }

    // the recursive case when the next squre is vacant
    for (auto iter = node->nexts.cbegin(); iter != node->nexts.cend(); iter++) {
        char letter = iter->first;  // get the letter of the ith child node
        // check to see if the player has that letter in the tile collection
        bool have_blank = false;
        // check to see if the player has that letter in the tile collection
        try {  // if the player has it, remove it
            remaining_tiles.lookup_tile(letter);
        } catch (const std::out_of_range& e) {  // if not, try the next child node
            try {
                remaining_tiles.lookup_tile(TileKind::BLANK_LETTER);
                have_blank = true;
            } catch (const std::out_of_range& e) {
                continue;
            }
        }

        // udpate the partial_word and the partial move
        TileKind temp_tile_kind(letter, 0);
        if (have_blank) {
            temp_tile_kind = remaining_tiles.lookup_tile(TileKind::BLANK_LETTER);
            temp_tile_kind.assigned = letter;
        } else {
            temp_tile_kind = remaining_tiles.lookup_tile(letter);
        }
        remaining_tiles.remove_tile(temp_tile_kind);
        // udpate the partial_word and the partial move
        partial_word.push_back(letter);
        partial_move.tiles.push_back(temp_tile_kind);
        Board::Position next_square = square;
        if (partial_move.direction == Direction::ACROSS) {
            next_square.column++;
        } else {
            next_square.row++;
        }

        // the recursive case
        extend_right(next_square, partial_word, partial_move, iter->second, remaining_tiles, legal_moves, board);
        // undo what has been done (partial_word and move)
        partial_word.pop_back();
        partial_move.tiles.pop_back();
        remaining_tiles.add_tile(temp_tile_kind);
    }
}

Move ComputerPlayer::get_move(const Board& board, const Dictionary& dictionary) const {
    std::vector<Move> legal_moves;
    std::vector<Board::Anchor> anchors = board.get_anchors();
    // HW5: IMPLEMENT THIS
    for (size_t i = 0; i < anchors.size(); i++) {
        std::string temp_word;
        std::vector<TileKind> temp_tiles;
        TileCollection temp_hand = tiles;
        Move temp_move(temp_tiles, anchors[i].position.row, anchors[i].position.column, anchors[i].direction);
        if (anchors[i].limit == 0) {
            Board::Position previous_pos(anchors[i].position.row, anchors[i].position.column);
            std::string before_anchor;
            bool flag = true;
            if (anchors[i].direction == Direction::ACROSS) {
                previous_pos.column--;
            } else {
                previous_pos.row--;
            }
            while (board.in_bounds_and_has_tile(previous_pos)) {
                before_anchor.push_back(board.letter_at(previous_pos));
                // update the column/row
                if (anchors[i].direction == Direction::ACROSS) {
                    previous_pos.column--;
                } else {
                    previous_pos.row--;
                }
            }
            std::shared_ptr<Dictionary::TrieNode> cur_dic_node = dictionary.get_root();
            // push them into partial_word and then update the dictionary node passed in
            for (size_t j = 0; j < before_anchor.size(); j++) {
                temp_word.push_back(before_anchor[before_anchor.size() - 1 - j]);
                auto it = cur_dic_node->nexts.find(before_anchor[before_anchor.size() - 1 - j]);
                if (it != cur_dic_node->nexts.cend()) {
                    cur_dic_node = it->second;
                } else {
                    flag = false;
                }
            }
            if (!flag)
                continue;
            extend_right(anchors[i].position, temp_word, temp_move, cur_dic_node, temp_hand, legal_moves, board);
        } else {
            left_part(
                    anchors[i].position,
                    temp_word,
                    temp_move,
                    dictionary.get_root(),
                    anchors[i].limit,
                    temp_hand,
                    legal_moves,
                    board);
        }
    }
    return get_best_move(legal_moves, board, dictionary);
}

Move ComputerPlayer::get_best_move(
        std::vector<Move> legal_moves, const Board& board, const Dictionary& dictionary) const {
    Move best_move = Move();  // Pass if no move found
    // HW5: IMPLEMENT THIS
    size_t highest_score = 0;
    // iterate through every legal_move and check for the one that has the highest score
    for (size_t i = 0; i < legal_moves.size(); i++) {
        PlaceResult temp_result = board.test_place(legal_moves[i]);
        if (temp_result.valid) {
            size_t j;
            for (j = 0; j < temp_result.words.size(); j++) {
                if (!dictionary.is_word(temp_result.words[j])) {  // make sure every word is in the dictionary
                    break;
                }
            }
            if (j == temp_result.words.size()) {                       // if every word is in the dictionary
                if (legal_moves[i].tiles.size() == get_hand_size()) {  // check bonus
                    temp_result.points += 50;
                }
                if (temp_result.points > highest_score) {  // update best move
                    highest_score = temp_result.points;
                    best_move = legal_moves[i];
                }
            }
        } else {
            continue;
        }
    }
    return best_move;
}