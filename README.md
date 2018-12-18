# qwirkle

A simple implementation of the board game Qwirkle.  Nothing fancy (the UI is
primitive at best).  The reason I created this was because I was curious about
how hard it would be to play against a greedy algorithm opponent.

The Game
========

- Players take turns.  This game simulates a two player game with a computer and
  human player.

- Each player has a rack of 6 tiles.  Tiles can be one of 6 colors, and one of 6
  shapes.  There are three of each tile.  Hence the rack is populated from a
  shared bag of 108 tiles.

- The human goes first (this varies from the "official" rules which has the
  player with the largest first move on their rack going first).

- On a turn you place as many tiles as you want on the playfield.  All tiles
  must be placed to make a single contiguous line, horizontally or vertically.
  You may use tiles already on the playfield to form your line.  All tiles in a
  every line must have something in common (either shape or color), and must
  differ in the other characteristic.

- You get one point for each tile in the contiguous line you placed (plus a
  bonus 6 points if you form a "Qwirkle" of 6 tiles in a line).  You also get
  one point for any perpendicular line you form or extend branching off of the
  line you placed (plus a 6 point bonus for a "Qwirkle").

- The game ends when the bag is empty and the first player runs out of tiles.

- A player can, if they choose, use their turn to exchange any tiles from their
  rack for random replacements from the bag.


This Implementation
===================

- It compiles with any C++17 compliant compliler (I've tested gcc and clang).

- When the game starts you are shown an empty game board and your rack.

- You place tiles by deciding if you want to place a line horizontally (h) from
  left to right or vertically (v) from top to bottom, choosing a starting
  location, and then choosing which tiles to place.  For example:

     ◆  ■  🞧  ●  ✖  ◆ 
     0  1  2  3  4  5 
    > h5,5;0,5

  Places the two diamond tiles next to each other at 5,5 and 5,6.

- You can return tiles to the bag and get new ones using the r command:

    > r1,3,4

  This command returns tiles 1, 3 and 4 to the bag from your rack.

- If you want to place tiles on either side of a tile already on the board, you
  just specify the starting location of your first tile and the program will
  automatically skip over existing tiles as it determines where to place your
  tiles.

Good luck!
