#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <deque>
#include <iomanip>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "bag.h"
#include "boardstate.h"
#include "rack.h"

// Parses the given command line.  Returns true on success.  All parameters are
// output parameters except for cmd.
bool parseCmd(const std::string& cmd,
	      char* direction_or_return,
	      int* x, int* y,
	      std::vector<int>* tile_nums,
	      int rack_size) {
  // Command syntax:
  // * h, v or r (horizontal, vertical, or return tiles)
  // * XX,YY; (starting square coordinates, only for h or v)
  // * tile,tile,tile,...
  // Examples:
  // h5,5;3,5,2,1  // Place tiles 3, 5, 2 and 1 starting horizontally at 5,5
  // r1,2,3        // Return tiles 1, 2 and 3 and draw new tiles
  
  if (cmd.size() == 0) {
    return false;
  }

  if (cmd[0] != 'h' &&
      cmd[0] != 'v' &&
      cmd[0] != 'r') {
    std::cout << "Missing h, v or r" << std::endl;
    return false;
  }
  *direction_or_return = cmd[0];
    
  std::string::const_iterator semicolon = cmd.begin();
  if (*direction_or_return == 'h' || *direction_or_return == 'v') {
    semicolon =
      std::find_if(cmd.begin(), cmd.end(), [](char c){return c==';';});

    if (semicolon == cmd.end()) {
      std::cout << "No semicolon found" << std::endl;
      return false;
    } 

    std::string::const_iterator first_comma =
      std::find_if(cmd.begin(), semicolon, [](char c){return c==',';});

    if (first_comma == semicolon) {
      std::cout << "No first comma found" << std::endl;
      return false;
    }

    std::stringstream sx(std::string(cmd.begin() + 1, first_comma));
    sx >> *x;
    std::stringstream sy(std::string(first_comma + 1, semicolon));
    sy >> *y;
  }

  std::string::const_iterator tile_string = semicolon;
  while (tile_string != cmd.end()) {
    tile_string++;
    std::string::const_iterator tile_comma =
      std::find_if(tile_string, cmd.end(), [](char c){return c==',';});
    int tile_num;
    std::stringstream st(std::string(tile_string, tile_comma));
    st >> tile_num;
    if (tile_num >= rack_size) {
      std::cout << "Invalid tile number" << std::endl;
      return false;
    }
    tile_nums->push_back(tile_num);
    tile_string = tile_comma;
  }

  return true;
}

// Given a board, the location of one tile, and whether that word is horizontal
// or vertical -- compute the score of playing that specified tile.
int scoreWord(BoardState board,
	      int x, int y,
	      bool horiz) {
  // Starting at (x,y), find the start of the word and iterate over it to
  // determine its length.
  int len = 0;

  // Rewind to the start of the word:
  if (horiz) {
    while (!board.isEmpty(x-1, y)) {
      x--;
    }
  } else {
    while (!board.isEmpty(x, y-1)) {
      y--;
    }
  }

  // Count the tiles in the word:
  while (!board.isEmpty(x, y)) {
    len++;
    if (horiz) {
      x++;
    } else {
      y++;
    }
  }

  // If the word is 6 long, then you have a Qwirkle and its score is doubled:
  return (len == 6) ? 12 : len;
}

// Compute the score of playing a particular set of tiles on the board.  You
// need to say whether those tiles are horizontally or vertically aligned (note
// that this doesn't matter if there is only one tile).
int scoreMove(BoardState board,
	      std::vector<std::pair<int, int>> tile_locations,
	      bool horiz) {
  int score = 0;

  // First compute the score of the word formed directly by putting down these
  // tiles.  Can start from any tile in the word, pick the first one
  // arbitrarily.
  int primary_score = scoreWord(board,
				tile_locations.begin()->first,
				tile_locations.begin()->second,
				horiz);
  if (primary_score > 1) {
    // Either this is the first move of the game and a single tile (which will
    // be handled by the caller), or a single tile was played and it only
    // generates multi-tile worlds in the other direction (and will be counted
    // below).
    score += primary_score;
  }
  
  // For each tile played compute the score of any words formed perpendicular to
  // the primary word.  Don't count single-tile words.
  for (auto i = tile_locations.begin(); i != tile_locations.end(); i++) {
    int secondary_score = scoreWord(board, i->first, i->second, !horiz);
    if (secondary_score > 1) {
      score += secondary_score;
    }
  }

  return score;
}

bool runCmd(std::string cmd,
	    BoardState* board,
	    Rack* rack,
	    int* score,
	    bool first_move) {
  bool valid_move_played = false;

  int x, y;
  std::vector<int> tile_nums;
  char directive;
  if(parseCmd(cmd, &directive, &x, &y, &tile_nums, rack->size())) {

    // Convert tile numbers to actual tiles in the rack:
    std::vector<Tile> tiles;
    tiles.reserve(tile_nums.size());
    std::for_each(tile_nums.begin(), tile_nums.end(),
		  [&tiles, rack](int num)
		  {tiles.push_back(rack->getTiles()[num]);});

    if (directive == 'r') {
      // Return tiles from rack and get new ones from bag.
      if (!first_move) {
	// To avoid picking the same tile back out of the bag, we first take the
	// tiles out of our rack, then pick new tiles from the bag, then put the
	// returned tiles back in the bag.
	std::for_each(tiles.begin(), tiles.end(),
		      [&rack](Tile t){rack->removeTile(t);});
	rack->populate();
	rack->bag()->return_tiles(tiles);

	// Edge case: what if the bag didn't have enough tiles in it to replace
	// all the returned tiles?  We'll just refill our rack with a random
	// choice of our returned tiles:
	rack->populate();
      
	valid_move_played = true;

      } else {
	std::cout << "Can't return tiles on first move!" << std::endl;
      }

    } else {
      bool horiz = (directive == 'h');

      BoardState newboard(*board);

      // Is this move building on the tiles that have already been played?
      bool adjacent = false;

      std::vector<std::pair<int,int>> tile_locs;

      for (auto tile = tiles.begin(); tile != tiles.end(); tile++) {
	while (!newboard.isEmpty(x, y)) {
	  // Tried to put a tile on top of an existing tile, just skip over it
	  // and keep going.
	  if(horiz) {
	    x++;
	  } else {
	    y++;
	  }
	}

	if (board->isAdjacent(x, y)) {
	  adjacent = true;
	}
	newboard.insertTile(*tile, x, y);
	tile_locs.push_back(std::pair<int,int>(x, y));
      }

      if (newboard.isValidBoard() && (adjacent || first_move)) {
	int move_score = scoreMove(newboard, tile_locs, horiz);

	// Special case: if the player plays just 1 tile on the first move then
	// our scoring routine won't find any words > length 1 formed -- and
	// will assign a score of 0.  Be charitable and give it a score of 1.
	if (move_score == 0) {
	  assert(first_move);
	  move_score = 1;
	}

	std::cout << "User Move Score=" << move_score << std::endl;
	*score += move_score;

	*board=newboard;

	for (auto tile = tiles.begin(); tile != tiles.end(); tile++) {
	  rack->removeTile(*tile);
	}
      
	rack->populate();

	valid_move_played = true;

      } else {
	std::cout << "INVALID MOVE" << std::endl;
      }
    }
  }

  return valid_move_played;
}

void userTurn(BoardState* board,
	      Rack* rack,
	      int* score,
	      bool first_move) {
  bool valid_move_played = false;

  while (!valid_move_played && std::cin.good() && !std::cin.eof()) {
    std::cout << "> ";
    std::string cmd;
    std::getline(std::cin, cmd);

    valid_move_played = runCmd(cmd, board, rack, score, first_move);
  }
}

struct Move {
  Move(BoardState b, Rack r, int s) : newboard(b), depleted_rack(r), score(s) {}
  Move(BoardState b, Rack r) : newboard(b), depleted_rack(r), score(0) {}

  BoardState newboard;
  Rack depleted_rack;
  int score;
};
	    
// If we've started a move on the board, recursively evaluate all possible moves
// in a given direction (up, down, left or right) using the tiles we have left
// on our rack to find the best move.
Move bestMoveGivenPrefix(BoardState board,
			 Rack rack,
			 int x, int y,
			 std::vector<std::pair<int,int>> tile_locs,
			 int dx, int dy) {
  assert((dx ==  1 && dy ==  0) ||
	 (dx == -1 && dy ==  0) ||
	 (dx ==  0 && dy ==  1) ||
	 (dx ==  0 && dy == -1));

  bool horiz = (dx != 0);
  Move best_move(board, rack, scoreMove(board, tile_locs, horiz));

  while(!board.isEmpty(x, y)) {
    x += dx;
    y += dy;
  }

  // Loop through all the tiles on our rack and see if we can add any to the
  // board and improve our move.
  std::vector<Tile> rack_tiles(rack.getTiles());
  for (auto tile = rack_tiles.begin(); tile != rack_tiles.end(); tile++) {
    BoardState new_board(board);
    Rack new_rack(rack);
    std::vector<std::pair<int,int>> new_tile_locs(tile_locs);
    new_board.insertTile(*tile, x, y);
    new_rack.removeTile(*tile);
    if (new_board.isValidBoard()) {
      new_tile_locs.push_back(std::pair<int,int>(x, y));
      // We found a move we can make!  Recurse to see if there are more tiles we
      // can place.
      Move submove = bestMoveGivenPrefix(new_board, new_rack, x, y,
					 new_tile_locs, dx, dy);

      if (submove.score > best_move.score) {
	best_move = submove;
      }
    }
  }
  return best_move;
}
			 
// Given a starting location, find the best move possible that includes putting
// a tile at that location.  If no move is possible return a move with a score
// of 0.
Move bestMove(BoardState board,
	      Rack rack,
	      int x, int y) {
  Move move(board, rack);

  // A move is only possible if it starts next to existing tiles
  if (board.isEmpty(x, y) && board.isAdjacent(x, y)) {
    std::vector<Tile> rack_tiles(rack.getTiles());

    // Try every tile in this location to see what we can do:
    for (auto tile = rack_tiles.begin(); tile != rack_tiles.end(); tile++) {
      BoardState new_board(board);
      Rack new_rack(rack);
      new_board.insertTile(*tile, x, y);
      new_rack.removeTile(*tile);

      if (new_board.isValidBoard()) {
	// We found a move we can make!  Now explore in all four directions (up,
	// down, left and right) to find the highest scoring word we can build
	// in that direction.
	std::vector<std::pair<int,int>> tile_locs;
	tile_locs.push_back(std::pair<int,int>(x, y));

	Move right = bestMoveGivenPrefix(new_board, new_rack, x, y, tile_locs,
					 1, 0);
	if (right.score > move.score) {
	  move = right;
	}

	Move left = bestMoveGivenPrefix(new_board, new_rack, x, y, tile_locs,
					-1, 0);
	if (left.score > move.score) {
	  move = left;
	}

	Move down = bestMoveGivenPrefix(new_board, new_rack, x, y, tile_locs,
					0, 1);
	if (down.score > move.score) {
	  move = down;
	}

	Move up = bestMoveGivenPrefix(new_board, new_rack, x, y, tile_locs,
				      0, -1);
	if (up.score > move.score) {
	  move = up;
	}
      }
    }
  }

  return move;
}

void computerTurn(BoardState* board,
		  Rack* rack,
		  int* score) {
  // Approach: exhaustive search.  Start at each square in the board (taking the
  // extents of the current board and adding one to each edge).  For each
  // square, try to place each tile in our rack.  If a placement is legal, then
  // start searching in all four directions (horizontal and vertical) to see of
  // any additional tiles can be placed.  Once either no placement is possible
  // or the rack is exhausted, record the move required to get there.

  Move best_move(*board, *rack);
  for (int x = board->minX()-1; x <= board->maxX(); x++) {
    for (int y = board->minY()-1; y <= board->maxY(); y++) {
      Move best_move_at_location = bestMove(*board, *rack, x, y);
      if (best_move_at_location.score > best_move.score) {
	best_move = best_move_at_location;
      }
    }
  }

  if (best_move.score > 0) {
    std::cout << "Computer Move Score=" << best_move.score << std::endl;
    *score += best_move.score;
    *board = best_move.newboard;
    *rack = best_move.depleted_rack;
    rack->populate();

  } else {
    std::cout << "OH NO, NO MOVES POSSIBLE!  Exchanging entire rack."
	      << std::endl;
    Rack new_rack(rack->bag());
    // If the bag is almost empty it may not be possible to return the entire
    // rack (say, if the bag has 3 tiles and the rack has 6).  In that case this
    // code will do the slightly improper thing of returning 6 tiles and drawing
    // 3.  The subsequent populate will fix this up (and otherwise be a no-op).
    rack->bag()->return_tiles(rack->getTiles());
    new_rack.populate();  // Usually a no-op
    *rack = new_rack;
  }
}

int main() {
  std::srand(std::time(0));

  BoardState board;
  Bag bag;
  bag.shuffle();
  Rack user_rack(&bag);
  Rack computer_rack(&bag);
  bool first_move = true;
  int user_score = 0;
  int computer_score = 0;

  while(std::cin.good() && !std::cin.eof()) {
    std::cout << "Your score: " << user_score
	      << "    Computer score: " << computer_score
	      << "    Tiles left: " << bag.tiles_left()
	      << std::endl;
    board.print();
    std::cout << std::endl;
    user_rack.print();
    if (computer_rack.size() < 6) {
      std::cout << "COMPUTER HAS " << computer_rack.size()
		<< " TILES LEFT." << std::endl;
    }

    userTurn(&board, &user_rack, &user_score, first_move);
    first_move = false;

    if (!std::cin.good() || std::cin.eof()) {
      break;
    }

    if (user_rack.size() == 0) {
      user_score += 6;
      break;
    }

    computerTurn(&board, &computer_rack, &computer_score);

    if (computer_rack.size() == 0) {
      computer_score += 6;
      break;
    }
  }

  std::cout << "*** GAME OVER ****" << std::endl;
  std::cout << "Your score: " << user_score << "    "
       << "Computer score: " << computer_score << std::endl;
  board.print();
  std::cout << std::endl;
  user_rack.print();

  return 0;
}

