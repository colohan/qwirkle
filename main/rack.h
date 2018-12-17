#ifndef RACK_H
#define RACK_H

#include <deque>
#include <iostream>

#include "bag.h"

class Rack {
 public:
  Rack(Bag* pbag) : pbag_(pbag) {
    populate();
  }

  Bag* bag() const { return pbag_; }

  void populate() {
    while(tiles_.size() < 6 &&
	  pbag_->tiles_left() > 0) {
      tiles_.push_back(pbag_->pick_tile());
    }
  }

  const std::vector<Tile> getTiles() const {
    std::vector<Tile> result(tiles_.begin(), tiles_.end());
    return result;
  }

  const size_t size() { return tiles_.size(); }

  void removeTile(Tile t) {
    for (auto tile = tiles_.begin(); tile != tiles_.end(); tile++) {
      if (*tile == t) {
	tiles_.erase(tile);
	break;
      }
    }
  }

  void print() const {
    for(auto t = tiles_.begin(); t != tiles_.end(); t++) {
      std::cout << " ";
      t->print();
    }
    std::cout << std::endl;
    int i=0;
    for(auto t = tiles_.begin(); t != tiles_.end(); t++, i++) {
      std::cout << std::setw(2) << i << " ";
    }
    std::cout << std::endl;
  }

 private:
  Bag* pbag_;
  std::deque<Tile> tiles_;
};

#endif // RACK_H
