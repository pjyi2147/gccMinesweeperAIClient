#pragma once

class Tile
{
private:
  // universal
  bool _mine = false;				// is it mine?
  bool _revealed = false;			// is it revealed?
  bool _flagged = false;

  int _neighborCount = 0;			// how many mines near the tile?
  int _col;
  int _row;

  // for Client (AI)
  bool _done = false;				// AI finished?

public:
  // universal
  Tile();
  Tile(int col, int row);

  void printTile();

  bool isMine();
  bool isRevealed();
  bool isFlagged();
  
  int returnRow();
  int returnCol();
  int returnNeighborCount();

  void setMine();
  void setFlag();
  void setNeighborCount(int count);
  void setReveal();

  // for Client
  Tile(Tile& t);

  bool isDone();

  void setFlag(bool set);
  void setDone(bool done);
};