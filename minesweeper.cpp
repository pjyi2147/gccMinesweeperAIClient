#include <iostream>
#include <vector>
#include <ctime>
#include <sstream>

#include "tile.h"
#include "minesweeper.h"

/// Constructors
// constructor with variables
MineSweeper::MineSweeper(int col, int row, int mineNum)
{
	this->_mineNum = mineNum;
	this->_col = col;
	this->_row = row;

	this->_minefield = new Tile*[row];
	for (int r = 0; r < row; ++r)
	{
		this->_minefield[r] = new Tile[col];
	}

	for (int r = 0; r < row; ++r)
	{
		for (int c = 0; c < col; ++c)
		{
			this->_minefield[r][c] = Tile(c, r);
		}
	}
}

// deconstructor (memory deallocation)
MineSweeper::~MineSweeper()
{
	for (int r = 0; r < this->_row; ++r)
	{
		delete[] this->_minefield[r];
	}
	delete[] this->_minefield;
}

/// Game functions
// create the minefield!
void MineSweeper::createMinefield(int col, int row)
{
	vector<int> options;
	for (int r = 0; r < this->_row; ++r)
	{
		for (int c = 0; c < this->_col; ++c)
		{
			if (!(r == row && c == col))
			{
				options.push_back(r * this->_col + c);
			}
		}
	}

	for (int i = 0; i < this->_mineNum; ++i)
	{
		if (options.size() > 0)
		{
			srand(time(NULL));
			int k = (rand() % options.size());
			auto pair1 = options[k];
			this->_minefield[pair1 / this->_col][pair1 % this->_col].setMine();
			options.erase(options.begin() + k);
		}
	}

	for (int r = 0; r < this->_row; ++r)
	{
		for (int c = 0; c < this->_col; ++c)
		{
			this->countMine(c, r);
		}
	}

	this->revealTile(col, row);
}

// print the minefield!
void MineSweeper::printMineField()
{
	for (int c = 0; c < this->_col * 2 + 1; ++c)
	{
		cout << "-";
	}
	cout << endl;
	for (int r = 0; r < this->_row; ++r)
	{
		for (int c = 0; c < this->_col; ++c)
		{
			this->_minefield[r][c].printTile();
		}
		cout << "|" << endl;
		for (int c = 0; c < this->_col * 2 + 1; ++c)
		{
			cout << "-";
		}
		cout << endl;
	}
}

// End the game!
void MineSweeper::EndGame(bool win)
{
	for (int r = 0; r < this->_row; ++r)
	{
		for (int c = 0; c < this->_col; ++c)
		{
			this->_minefield[r][c].setReveal();
		}
	}

	this->printMineField();

	if (win)
	{
		cout << "Won!" << endl;
	}
	else
	{
		cout << "Lost..." << endl;
	}
	this->setGameEnd(true);
}

// Check if the game is finished and won
void MineSweeper::checkWin()
{
	bool win = true;
	for (int r = 0; r < this->_row; ++r)
	{
		for (int c = 0; c < this->_col; ++c)
		{
			if (!this->_minefield[r][c].isRevealed()
				&& !this->_minefield[r][c].isMine())
				win = false;
		}
	}
	if (win)
	{
		this->setGameEnd(true);
		this->setWin(true);
	}
}

/// count series
// count nearing mines for a tile
void MineSweeper::countMine(int col, int row)
{
	// if the tile is mine
	if (this->_minefield[row][col].isMine())
	{
		this->_minefield[row][col].setNeighborCount(-1);
		return;
	}

	// not mine
	int total = 0;
	for (int yoff = -1; yoff <= 1; ++yoff)
	{
		for (int xoff = -1; xoff <= 1; ++xoff)
		{
			int c = col + xoff;
			int r = row + yoff;

			if (c > -1 && c < this->_col
				&& r > -1 && r < this->_row)
			{
				if (this->_minefield[r][c].isMine()) total += 1;
			}
		}
	}
	this->_minefield[row][col].setNeighborCount(total);
}

// count nearing flags for a tile
int MineSweeper::countFlag(int col, int row)
{
	int total = 0;
	for (int yoff = -1; yoff <= 1; ++yoff)
	{
		for (int xoff = -1; xoff <= 1; ++xoff)
		{
			if (xoff != 0 || yoff != 0)
			{
				int c = col + xoff;
				int r = row + yoff;

				if (c > -1 && c < this->_col
					&& r > -1 && r < this->_row)
				{
					if (this->_minefield[r][c].isFlagged()) total += 1;
				}
			}
		}
	}
	return total;
}

/// reveal series 
// reveal a single tile
void MineSweeper::revealTile(int col, int row)
{
	this->_minefield[row][col].setReveal();
	// if the Tile has mine
	if (this->_minefield[row][col].isMine())
	{
		this->setGameEnd(true);
		this->EndGame(false);
	}
	// if the neighbor count is == 0
	if (this->_minefield[row][col].returnNeighborCount() == 0
		&& !this->_minefield[row][col].isMine())
	{
		for (int yoff = -1; yoff <= 1; ++yoff)
		{
			for (int xoff = -1; xoff <= 1; ++xoff)
			{
				int c = col + xoff;
				int r = row + yoff;

				if (c > -1 && c < this->_col
					&& r > -1 && r < this->_row)
				{
					if (!this->_minefield[r][c].isRevealed())
						this->revealTile(c, r);
				}
			}
		}
	}
}

// reveal the tiles that are not flagged
// safe of less or more flags
void MineSweeper::revealDoubleClick(int col, int row)
{
	// not revealed -> do nothing
	if (!this->_minefield[row][col].isRevealed()) return;

	// the flag count does not match the mine num -> do nothing
	if (this->countFlag(col, row) != this->_minefield[row][col].returnNeighborCount()) return;

	for (int yoff = -1; yoff <= 1; ++yoff)
	{
		for (int xoff = -1; xoff <= 1; ++xoff)
		{
			if (xoff != 0 || yoff != 0)
			{
				int c = col + xoff;
				int r = row + yoff;

				// within range
				if (c > -1 && c < this->_col 
					&& r > -1 && r < this->_row)
				{
					if (!this->_minefield[r][c].isFlagged())
						this->revealTile(c, r);
				}
			}
		}
	}
}

/// set series
// set GameEnd
void MineSweeper::setGameEnd(bool finished)
{
	this->_gameEnd = true;
}

// set a flag on a tile 
// if flagged unflag, unflagged flag
void MineSweeper::setFlag(int col, int row)
{
	if (!this->_minefield[row][col].isRevealed())
		this->_minefield[row][col].setFlag();
}

// set the game is won or lost
void MineSweeper::setWin(bool win)
{
	this->_win = win;
}

/// return series
// return MineNum (total number of mines)
int MineSweeper::returnMineNum()
{
	return this->_mineNum;
}

// return columns
int MineSweeper::returnCol()
{
	return this->_col;
}

// return row
int MineSweeper::returnRow()
{
	return this->_row;
}

// return gameEnd
bool MineSweeper::returnGameEnd()
{
	return this->_gameEnd;
}

// return win
bool MineSweeper::returnWin()
{
	return this->_win;
}


//// Client ONLY ////
/// constructor
// copy constructor
MineSweeper::MineSweeper(MineSweeper& m)
{
	this->_mineNum = m.returnMineNum();
	this->_col = m.returnCol();
	this->_row = m.returnRow();

	this->_minefield = new Tile*[_row];
	for (int r = 0; r < this->_row; ++r)
	{
		this->_minefield[r] = new Tile[_col];
	}

	for (int r = 0; r < this->_row; ++r)
	{
		for (int c = 0; c < this->_col; ++c)
		{    
			this->_minefield[r][c] = Tile(m.returnTile(c, r));
		}
	}
}

/// count series
// count nearing covered tiles
int MineSweeper::countCovered(int col, int row)
{
	int total = 0;
	for (int yoff = -1; yoff <= 1; ++yoff)
	{
		for (int xoff = -1; xoff <= 1; ++xoff)
		{
			if (xoff != 0 || yoff != 0)
			{
				int c = col + xoff;
				int r = row + yoff;

				if (c > -1 && c < this->returnCol()
					&& r > -1 && r < this->returnRow())
				{
					if (!this->_minefield[r][c].isRevealed()) total += 1;
				}
			}
		}
	}
	return total;
}

// count all covered tiles
int MineSweeper::countAllCovered()
{
	int total = 0;
	for (int r = 0; r < this->returnRow(); ++r)
	{
		for (int c = 0; c < this->returnCol(); ++c)
		{
			if (!this->_minefield[r][c].isRevealed())
				++total;
		}
	}
	return total;
}

// count all flagged tiles
int MineSweeper::countAllFlagged()
{
	int total = 0;
	for (int r = 0; r < this->returnRow(); ++r)
	{
		for (int c = 0; c < this->returnCol(); ++c)
		{
			if (this->_minefield[r][c].isFlagged())
				++total;
		}
	}
	return total;
}

/// set series
// set flag for a tile with value
void MineSweeper::setFlag(int col, int row, bool set)
{
	if (!this->_minefield[row][col].isRevealed())
		this->_minefield[row][col].setFlag(set);
	else cout << "You cannot flag tiles that are already open" << endl;
}

// set done for a tile with value 
void MineSweeper::setDone(int col, int row, bool done)
{
	this->_minefield[row][col].setDone(done);
}

// set reveal
void MineSweeper::setReveal(int col, int row) {
	this->_minefield[row][col].setReveal();
}

// set mine 
void MineSweeper::setMine(int col, int row) {
	this->_minefield[row][col].setMine();
}

// set neighborCount
void MineSweeper::setNeighborCount(int col, int row, int val) {
	this->_minefield[row][col].setNeighborCount(val);
}

/// return series
// return done
bool MineSweeper::returnDone(int col, int row)
{
	return this->_minefield[row][col].isDone();
}

// return covered 
bool MineSweeper::returnCovered(int col, int row)
{
	return !this->_minefield[row][col].isRevealed();
}

/// return series
// returnNeighborCount of a tile 
int MineSweeper::returnNeighborCount(int col, int row)
{
	return this->_minefield[row][col].returnNeighborCount();
}

// return whether the tile is flagged
bool MineSweeper::returnFlagged(int col, int row)
{
	return this->_minefield[row][col].isFlagged();
}

// return the reference of the tile
Tile& MineSweeper::returnTile(int col, int row)
{
	return this->_minefield[row][col];
}
