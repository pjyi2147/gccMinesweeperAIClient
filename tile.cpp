#include <iostream>
#include <vector>
#include <string>

#include "tile.h"

using namespace std;
/// Constructors
// No variables
Tile::Tile()
{
  this->_col = 0;
  this->_row = 0;
}

// with variables
Tile::Tile(int col, int row)
{
  this->_col = col;
  this->_row = row;
}

// print the tile status
void Tile::printTile()
{
  std::cout << "|";
  if (this->_revealed && this->_mine) cout << "M";
  else if (this->_revealed)
    cout << this->_neighborCount;
  else if (this->_flagged) cout << "F";			// Flag
  else cout << "C";								// Covered 
}

/// is series
// is it mine?
bool Tile::isMine()
{
  return this->_mine;
}

// is it revealed?
bool Tile::isRevealed()
{
  return this->_revealed;
}

// is it Flagged?
bool Tile::isFlagged()
{
  return this->_flagged;
}

/// return series
// return its row
int Tile::returnRow()
{
  return this->_row;
}

// return its column
int Tile::returnCol()
{
  return this->_col;
}

// return neighborCount;
int Tile::returnNeighborCount()
{
  return this->_neighborCount;
}

/// set series
// set Mine
void Tile::setMine()
{
  this->_mine = true;
}

// set Flag
void Tile::setFlag()
{
  // if flagged unflag, if unflagged flag
  if (this->_flagged) this->_flagged = false;
  else this->_flagged = true;
}

// set NeighborCount
void Tile::setNeighborCount(int count)
{
  this->_neighborCount = count;
}

// set reveal
void Tile::setReveal()
{
  this->_revealed = true;
}

/// for Client
// copy constructor
Tile::Tile(Tile& t)
{
  this->_mine = t.isMine();
  this->_revealed = t.isRevealed();
  this->_flagged = t.isFlagged();

  this->_neighborCount = t.returnNeighborCount();
  this->_done = t.isDone();
  
  this->_col = t.returnCol();
  this->_row = t.returnRow();
}

// is it done? (AI) 
bool Tile::isDone()
{
  return this->_done;
}

// set done
void Tile::setDone(bool done)
{
  this->_done = done;
}

// set flag to which I want
void Tile::setFlag(bool set)
{
  this->_flagged = set;
}