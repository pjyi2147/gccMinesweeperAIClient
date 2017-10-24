#pragma once
#include <iostream>
#include <vector>

#include "tile.h"

using namespace std;

class MineSweeper
{
private:
  // universal
  int _col;
  int _row;
  int _mineNum;
  Tile** _minefield;

  bool _gameEnd = false;
  bool _win = false;
  
  // client only
  bool _done = false;
  
public:
  // universal
  MineSweeper(int col, int row, int mineNum);
  ~MineSweeper();

  void createMinefield(int col, int row);
  void printMineField();
  void EndGame(bool win);
  void checkWin();

  void countMine(int col, int row);
  int countFlag(int col, int row);

  void revealTile(int col, int row);
  void revealDoubleClick(int col, int row);

  void setGameEnd(bool finished);
  void setFlag(int col, int row);
  void setWin(bool win);

  bool returnGameEnd();
  int returnCol();
  int returnRow();
  int returnMineNum();
  bool returnWin();

  // Client only
  MineSweeper(MineSweeper& m);

  int countCovered(int col, int row);
  int countAllCovered();
  int countAllFlagged();

  void setFlag(int col, int row, bool flag);
  void setDone(int col, int row, bool done);
  void setReveal(int col, int row);
  void setMine(int col, int row);
  void setNeighborCount(int col, int row, int val);

  bool returnDone(int col, int row);
  bool returnCovered(int col, int row);
  int returnNeighborCount(int col, int row);
  bool returnFlagged(int col, int row);
  Tile& returnTile(int col, int row);

};
