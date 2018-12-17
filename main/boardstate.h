#ifndef BOARDSTATE_H
#define BOARDSTATE_H

#include <assert.h>
#include <variant>
#include <deque>
#include <iostream>

using std::cout;
using std::endl;

class Tile {
 public:
  enum Color { red, cyan, yellow, green, blue, violet };
  enum Shape { circle, x, diamond, square, starburst, cross };

  Tile(Color color, Shape shape) : color_(color), shape_(shape) {}
  Color color() const { return color_; }
  Shape shape() const { return shape_; }

  bool operator==(const Tile& t) const {
    return (color_ == t.color_ && shape_ == t.shape_);
  }

  void print() const {
    switch(color()) {
    case red:
      cout << "\u001b[31m";
      break;
    case cyan:
      cout << "\u001b[36m";
      break;
    case yellow:
      cout << "\u001b[33m";
      break;
    case green:
      cout << "\u001b[32m";
      break;
    case blue:
      cout << "\u001b[34m";
      break;
    case violet:
      cout << "\u001b[35m";
      break;
    }
    switch(shape()) {
    case circle:
      cout << "● ";
      break;
    case x:
      cout << "✖ ";
      break;
    case diamond:
      cout << "◆ ";
      break;
    case square:
      cout << "■ ";
      break;
    case starburst:
      cout << "🟏 ";
      break;
    case cross:
      cout << "🞧 ";
      break;
    }
	
    cout << "\u001b[0m"; // Reset color
  }

 private:
  Color color_;
  Shape shape_;
};

class NoTile {};

// A particular board state.  The board is a 2D grid, indexed by arbitrary x&y
// coordinates.  Each square contains either a Tile or NoTile.
class BoardState {
 public:
 BoardState() :
  minx_(0), maxx_(0), miny_(0), maxy_(0) {}

  int minX() const { return minx_; }
  int maxX() const { return maxx_; }
  int minY() const { return miny_; }
  int maxY() const { return maxy_; }

  void insertTile(Tile tile, int x, int y) {
    resizeBoardToInclude(x, y);
    board_[y-miny_][x - minx_] = tile;
  }

  bool isEmpty(int x, int y) const {
    if (x < minx_ ||
	x >= maxx_ ||
	y < miny_ ||
	y >= maxy_) {
      return true;
    }
    return std::holds_alternative<NoTile>(board_[y-miny_][x-minx_]);
  }

  bool isAdjacent(int x, int y) const {
    return isEmpty(x, y) &&
      (!isEmpty(x-1, y) ||
       !isEmpty(x, y-1) ||
       !isEmpty(x+1, y) ||
       !isEmpty(x, y+1));
  }

  const Tile& getTile(int x, int y) const {
    assert(!isEmpty(x, y));
    return std::get<Tile>(board_[y-miny_][x-minx_]);
  }

  bool isValidBoard() const {
    // Each sequence of tiles separated by an empty space is a "word".  Tiles in
    // a word must all have the same color or the same shape.  A tile can't
    // repeat itself in a word.
    for (int y = miny_; y < maxy_; y++) {
      std::deque<Tile> word;
      for (int x = minx_; x < maxx_; x++) {
	if (!isEmpty(x, y)) {
	  word.push_back(getTile(x,y));
	} else {
	  if (word.size() > 0 && !validWord(word)) {
	    return false;
	  }
	  word.clear();
	}
      }
      if (word.size() > 0 && !validWord(word)) {
	return false;
      }
    }

    for (int x = minx_; x < maxx_; x++) {
      std::deque<Tile> word;
      for (int y = miny_; y < maxy_; y++) {
	if (!isEmpty(x, y)) {
	  word.push_back(getTile(x,y));
	} else {
	  if (word.size() > 0 && !validWord(word)) {
	    return false;
	  }
	  word.clear();
	}
      }
      if (word.size() > 0 && !validWord(word)) {
	return false;
      }
    }
    return true;
  }

  void print() const {
    cout << "    ";
    for (int x = minX(); x < maxX(); x++) {
      cout << " " << std::setfill(' ') << std::setw(3) << x;
    }
    cout << endl;

    for (int y = minY(); y < maxY(); y++) {
      cout << std::setfill(' ') << std::setw(3) << y << ":";

      for (int x = minX(); x < maxX(); x++) {
	cout << "  ";
	if (isEmpty(x, y)) {
	  cout << "--";

	} else {
	  Tile tile = getTile(x, y);
	  tile.print();
	}
      }
      cout << endl;
    }
  }

  // Currently no way of deleting a Tile.  Do we need that?

 private:
  void resizeBoardToInclude(int x, int y) {
    if (board_.size() == 0) {
      // First tile
      minx_ = x;
      maxx_ = x;
      miny_ = y;
      maxy_ = y;
    }

    while (y >= maxy_) {
      board_.push_back(std::deque<std::variant<NoTile, Tile>>());
      board_[maxy_-miny_].resize(maxx_-minx_, NoTile());
      maxy_++;
    }
    while (y < miny_) {
      board_.push_front(std::deque<std::variant<NoTile, Tile>>());
      board_[0].resize(maxx_-minx_, NoTile());
      miny_--;
    }
    while (x >= maxx_) {
      for (int iy = miny_; iy < maxy_; iy++) {
	board_[iy-miny_].push_back(NoTile());
      }
      maxx_++;
    }
    while (x < minx_) {
      for (int iy = miny_; iy < maxy_; iy++) {
	board_[iy-miny_].push_front(NoTile());
      }
      minx_--;
    }
  }

  static bool validWord(const std::deque<Tile>& word) {
    bool const_color = true;
    bool const_shape = true;
    Tile prev_tile = *word.begin();
    for (auto i = word.begin(); i != word.end(); i++) {
      const_color = const_color &&
	(prev_tile.color() == i->color());
      const_shape = const_shape &&
	(prev_tile.shape() == i->shape());
      prev_tile = *i;
    }
    if (const_color) {
      std::vector<bool> shape_found =
	{ false, false, false, false, false, false };
      for (auto i = word.begin(); i != word.end(); i++) {
	if (shape_found[i->shape()]) {
	  return false;
	}
	shape_found[i->shape()] = true;
      }
    }
    if (const_shape) {
      std::vector<bool> color_found =
	{ false, false, false, false, false, false };
      for (auto i = word.begin(); i != word.end(); i++) {
	if (color_found[i->color()]) {
	  return false;
	}
	color_found[i->color()] = true;
      }
    }
    
    return (const_color || const_shape);
  }

  int minx_, maxx_, miny_, maxy_;

  std::deque<std::deque<std::variant<NoTile, Tile>>> board_;
};

#endif // BOARDSTATE_H
