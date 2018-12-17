#ifndef BAG_H
#define BAG_H

#include <algorithm>
#include <deque>

#include "boardstate.h"

class Bag {
 public:
  Bag() {
    std::vector<Tile::Color> colors =
      {Tile::red, Tile::cyan, Tile::yellow,
       Tile::green, Tile::blue, Tile::violet};
    std::vector<Tile::Shape> shapes =
      { Tile::circle, Tile::x, Tile::diamond,
	Tile::square, Tile::starburst, Tile::cross };
    for (auto c = colors.begin(); c != colors.end(); c++) {
      for (auto s = shapes.begin(); s != shapes.end(); s++) {
	for (int i = 0; i < 3; i++) {
	  tiles_.push_back(Tile(*c, *s));
	}
      }
    }
  }

  void shuffle() {
    std::random_shuffle(tiles_.begin(), tiles_.end());
  }

  size_t tiles_left() const {
    return tiles_.size();
  }

  Tile pick_tile() {
    Tile tile = tiles_.front();
    tiles_.pop_front();
    return tile;
  }

  void return_tile(Tile tile) {
    tiles_.push_front(tile);
    shuffle();
  }

  void return_tiles(std::vector<Tile> tiles) {
    std::for_each(tiles.begin(), tiles.end(),
		  [this](Tile t){return_tile(t);});
  }

 private:
  std::deque<Tile> tiles_;
};

#endif // BAG_H
