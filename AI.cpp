#include <iostream>
#include <string>
#include <sstream>
#include <ctime>
#include <vector>
#include <list>

#include "json.hpp"
#include "minesweeper.h"

using namespace std;
using json = nlohmann::json;

/// math functions
// compare function for min value of map
bool compare(pair<int, double> i, pair<int, double> j) {
  return i.second < j.second;
}

// n choose k function
unsigned nChoosek(unsigned n, unsigned k)
{
  if (k > n) return 0;
  if (k * 2 > n) k = n - k; // nCk == nC(n-k) choose smaller one
  if (k == 0) return 1;

  int result = n;
  for (int i = 2; i <= k; ++i) 
  {
    result *= (n - i + 1);
    result /= i;
  }
  return result;
}

/// Click script generator functions
// simple click 
string simpleClick(int col, int row)
{
  stringstream s;
  s << "E " << to_string(col) << " " << to_string(row);
  return s.str();
}

// double click
string doubleClick(int col, int row)
{
  stringstream s;
  s << "D " << to_string(col) << " " << to_string(row);
  return s.str();
}

// flag click
string flagClick(int col, int row)
{
    stringstream s;
    s << "F " << to_string(col) << " " << to_string(row);
    return s.str();
}

/// Basic AI methods 
// Flag a tile when it is 100% sure of being mined.
void basicFlagging(MineSweeper* m, vector<string>* orders)
{
  int _col = m->returnCol(), _row = m->returnRow();
  for (int r = 0; r < _row; ++r)
  {
      for (int c = 0; c < _col; ++c)
      {
          // when the tile is done or covered, then continue;
          if (m->returnDone(c, r) || m->returnCovered(c, r))
              continue;

          // the number of covered is the same as the neighborcount
          if (m->returnNeighborCount(c, r) == m->countCovered(c, r))
          {
              // to examine 8 adjacent tiles
              for (int yoff = -1; yoff <= 1; ++yoff)
              {
                  for (int xoff = -1; xoff <= 1; ++xoff)
                  {
                      if (xoff != 0 || yoff != 0)
                      {
                          int col = c + xoff;
                          int row = r + yoff;

                          if (col > -1 && col < _col
                              && row > -1 && row < _row)
                          {
                              // covered and not flagged 
                              // flag the tile!
                              if (m->returnCovered(col, row) && 
                                  !m->returnFlagged(col, row))
                              {
                                  orders->push_back(flagClick(col, row));

                                  m->setFlag(col, row);
                                  m->setDone(col, row, true);
                              
                                  cout << "Flagged col: " << col 
                                        << " row: " << row << endl;
                              }
                          }
                      }
                  }
              }
          }
      }
  }
}

// order to double click when a open tile has 
// the exact number of flags neighboring the tile
void basicDoubleClicking(MineSweeper* m, vector<string>* orders)
{
    int _col = m->returnCol(), _row = m->returnRow();
    for (int r = 0; r < _row; ++r)
    {
        for (int c = 0; c < _col; ++c)
        {
            // when the tile is done or covered, then continue;
            if (m->returnDone(c, r) || m->returnCovered(c, r))
                continue;

            // the number of flagged is the same as the neighborcount
            if (m->returnNeighborCount(c, r) == m->countFlag(c, r))
            {
                orders->push_back(doubleClick(c, r));

                cout << "Double-Clicked col: " << c << " row: " << r << endl;
                m->setDone(c, r, true);
            }
        }
    }
}

/// Get something functions
// get empty tile (covered, not flagged)
vector<int> getEmptyTiles(MineSweeper* m)
{
    int _row = m->returnRow(), _col = m->returnCol();
    vector<int> EmptyTiles;
    for (int r = 0; r < _row; ++r)
    {
        for (int c = 0; c < _col; ++c)
        {
            // not flagged and is covered
            if (!m->returnFlagged(c, r) && m->returnCovered(c, r))
            {
                EmptyTiles.push_back(r*_col + c);
            }
        }
    }
    return EmptyTiles;
}

// get the prob of guessing random
double getRandomGuessProb(MineSweeper* m)
{
    double numEmptyTiles = getEmptyTiles(m).size();
    double mineNum = m->returnMineNum();
    double numFlags = m->countAllFlagged();

    return (mineNum - numFlags) / numEmptyTiles;
}

// get the open tiles for one tile
vector<int> getOpenedTilesForOne(MineSweeper* m, int col, int row) 
{
    vector<int> toReturn;
    int _col = m->returnCol(), _row = m->returnRow();
    for (int xoff = -1; xoff <= 1; ++xoff) 
    {
        for (int yoff = -1; yoff <= 1; ++yoff) 
        {
            int c = col + xoff;
            int r = row + yoff;
            if (c < _col && c > -1 && 
                r < _row && r > -1)
            {
                if (!m->returnCovered(c, r)) 
                { 
                    toReturn.push_back(r*m->returnCol() + c);
                }
            }
        }
    }
    return toReturn;
}

// get border tiles
vector<int> getBorderTiles(MineSweeper* m)
{
    int _row = m->returnRow(), _col = m->returnCol();
    vector<int> borderTiles;
    for (int r = 0; r < _row; ++r)
    {
        for (int c = 0; c < _col; ++c)
        {
            // when the tile is done or revealed or flagged, then continue;
            if (m->returnDone(c, r) || 
                !m->returnCovered(c, r) || 
                m->returnFlagged(c, r))
                continue;

            // examine 8 adjacent tiles
            for (int yoff = -1; yoff <= 1; ++yoff)
            {
                bool border = false;
                for (int xoff = -1; xoff <= 1; ++xoff)
                {
                    if (xoff != 0 || yoff != 0)	// exclude itself
                    {
                        int col = c + xoff;
                        int row = r + yoff;

                        // should not be out of bounds 
                        if (col > -1 && col < _col
                            && row > -1 && row < _row)
                        {
                            // if one of the neighboring tile is open
                            if (!m->returnCovered(col, row))
                            {
                                // then it is bordertile
                                borderTiles.push_back(r*_col + c);
                                border = true;
                                break;
                            }
                        }
                    }
                }
                // only one open tile is needed to determine
                if (border) break;	
            }
        }
    }
    return borderTiles;
}

// get connected border tiles (stores all section)
vector< vector<int> > getConnectedBorderTiles(MineSweeper *m)
{
    int _row = m->returnRow(), _col = m->returnCol();
    auto borderTiles = getBorderTiles(m);

    vector< vector<int> > connectedBorderTiles;

    vector<int> allfinished; 	// where the finished tiles go

    while (true)
    {
        list<int> queue;
        vector<int> section;

        for (auto& firstT: borderTiles)
        {
            // search bordertiles already in allfinished
            if (std::find(allfinished.begin(), allfinished.end(), firstT) 
                == allfinished.end())
            {
                // if not, put it in queue
                queue.push_back(firstT);
                break;
            }
        }
        
        if (queue.size() == 0)		// All section finished
            break;

        while(queue.size() != 0)	// section is not finished
        {
            int curTile = queue.front();
            queue.pop_front();

            int curCol = curTile % _col;
            int curRow = curTile / _col;
            
            section.push_back(curTile);
            allfinished.push_back(curTile);

            for (auto& tile: borderTiles)
            {
                int tileCol = tile % _col;
                int tileRow = tile / _col;

                bool isConnected = false;
                
                // if the tile is already in section
                if (std::find(section.begin(), section.end(), tile) 
                    != section.end())
                    // pass the tile 
                    continue;

                // the distance should be less than 2 to have common open tile 
                if (abs(curCol - tileCol) <= 2 || abs(curRow - tileRow) <= 2)
                {
                    for (int r = 0; r < _row; ++r)
                    {
                        for (int c = 0; c < _col; ++c)
                        {
                            if (!m->returnCovered(c, r))
                            {
                                // the connected tiles should share 
                                // at least one open tile on the side
                                if (abs(curCol - c) <= 1 && 
                                    abs(curRow - r) <= 1 &&
                                    abs(tileCol - c) <= 1 && 
                                    abs(tileRow - r) <= 1)
                                {
                                    isConnected = true;
                                    break;
                                }
                            }
                        }
                        if (isConnected) break;
                    }
                }
                // tile is not connected, next tile called
                if (!isConnected) continue;		 

                // if the connected tile for the section is not in queue
                if (find(queue.begin(), queue.end(), tile) == queue.end())
                    // add the tile to queue and check for that one again	
                    queue.push_back(tile);
            }
        }
        // one section is finished, add section to the big array.
        connectedBorderTiles.push_back(section);
    }
    return connectedBorderTiles;
}

vector<vector<vector<int>>> getGroupedBorderTiles(MineSweeper *m)
{
    vector<vector<vector<int>>> groupedBorderTiles;
    int _col = m->returnCol(), _row = m->returnRow();
    auto connectedBorderTiles = getConnectedBorderTiles(m);
    for (auto& section : connectedBorderTiles) 
    {
        vector<vector<int>> oneSectionGroupedTiles;
        while (section.size() > 0) 
        {
            int firstTile = section.front();
            section.erase(section.begin());
            vector<int> groupedTiles;
            auto firstTileOpen = getOpenedTilesForOne(m, firstTile % _col, 
                                                      firstTile / _col);
            groupedTiles.push_back(firstTile);
            int i = 0;
            while (i < section.size())  
            {
                int tile = section[i];
                int tileCol = tile % _col, tileRow = tile / _col;
                if (getOpenedTilesForOne(m, tileCol, tileRow) == firstTileOpen) 
                {
                    groupedTiles.push_back(tile);
                    section.erase(section.begin() + i);
                }	
                else ++i;
            }
            oneSectionGroupedTiles.push_back(groupedTiles);
        }
        groupedBorderTiles.push_back(oneSectionGroupedTiles);
    }
    return groupedBorderTiles;
}

/// AI functions 
// Guess random!
void randomGuess(MineSweeper* m, vector<string>* orders)
{
    auto emptyTiles = getEmptyTiles(m);
    int mineNum = m->returnMineNum();
    int numFlags = m->countAllFlagged();

    srand(time(NULL));
    int randomEmptyTile = emptyTiles[rand() % emptyTiles.size()];
    int col = randomEmptyTile % m->returnCol();
    int row = randomEmptyTile / m->returnCol();
    orders->push_back(simpleClick(col, row));
    cout << "AI guessed Col: " << col << " Row: " 
         << row << " from the empty tiles" << endl;
    cout << "with " 
         << double(mineNum-numFlags) / double(emptyTiles.size()) * 100 
         << "% chance of the tile being mine." << endl;
}

// using recusion find all the mine configurations
// that one section can possilby have 
void bruteRecurse(MineSweeper* m, 
                  map<int, vector<map<int, bool>>>* sectionSolutions, 
                  vector<int> section, 
                  bool borderOptimization, 
                  int depthK, 
                  int* functionCalls)
{
    ++(*functionCalls);
    int totalMineNum = m->returnMineNum();
    int flagCount = m->countAllFlagged();
    int _col = m->returnCol(), _row = m->returnRow();
    
    // total flagcount cannot exceed the total mineNum
    if (flagCount > totalMineNum) return;
    
    // if the flagcount is bigger than the neighborcount already?
    // then, return before it goes through more recursion.
    // for more optimization, the open tiles for the section can be
    // put into a container to be looped
    if (depthK != section.size()) {
        for (auto& anyTile : section)
        {
            int tileCol = anyTile % _col, tileRow = anyTile / _col;
            for (int yoff = -1; yoff <= 1; ++yoff)
            {
                for (int xoff = -1; xoff <= 1; ++xoff)
                {
                    int c = tileCol + xoff;
                    int r = tileRow + yoff;
                    // within in the range
                    if (c > -1 && c < _col &&
                        r > -1 && r < _row)
                    {
                        // should be revealed
                        if (!m->returnCovered(c, r))
                        {
                            // if the flagcount is bigger than the neighborcount?
                            // then, return before it goes through more recursion.
                            if (m->countFlag(c, r) > m->returnNeighborCount(c, r))
                                return;
                        }
                    }
                }
            }
        }
        int thisTile = section[depthK];
        int qCol = thisTile % _col;
        int qRow = thisTile / _col;

        m->setFlag(qCol, qRow, true);		// guess set
        bruteRecurse(m, sectionSolutions, section, 
            borderOptimization, depthK + 1, functionCalls);
        m->setFlag(qCol, qRow, false);		// guess unset
        bruteRecurse(m, sectionSolutions, section, 
            borderOptimization, depthK + 1, functionCalls);
    }

    // DFS Search
    // go to depth k and check it is correct
    else if (depthK == section.size())
    {
        for (auto& anyTile : section)
        {
            int tCol = anyTile % _col, tRow = anyTile / _col;
            for (int yoff = -1; yoff <= 1; ++yoff)
            {
                for (int xoff = -1; xoff <= 1; ++xoff)
                {
                    int c = tCol + xoff, r = tRow + yoff;
                    // within in the range
                    if (c > -1 && c < _col &&
                        r > -1 && r < _row)
                    {
                        // should be revealed
                        if (!m->returnCovered(c, r))
                        {
                            // if the counting is not the same?
                            // then it is not the possible answer.
                            if (m->countFlag(c, r) != m->returnNeighborCount(c, r))
                                return;
                        }
                    }
                }
            }
        }
        // when it is the end of the game and there isn't the same number of the flags
        // that totals to be the totalMineNum, then it is not a solution.
        if (!borderOptimization && m->countAllFlagged() != totalMineNum)
            return;

        map<int, bool> oneSectionSolution;	// store a one configuration

        int mineCountForSection = 0;
        for (auto& eachTile : section)
        {
            int tCol = eachTile % _col, tRow = eachTile / _col;
            bool flagged = m->returnFlagged(tCol, tRow);
            oneSectionSolution.insert(make_pair(eachTile, flagged));
            if (flagged) ++mineCountForSection;
        }

        auto mineCountFound = sectionSolutions->find(mineCountForSection);

        // if there isnt a vector for the mine count
        if (mineCountFound == sectionSolutions->end())
        {
            vector<map<int, bool>> vectorSectionSolution;
            vectorSectionSolution.push_back(oneSectionSolution);
            sectionSolutions->insert(make_pair(mineCountForSection, vectorSectionSolution));
        }
        // if there is
        else
        {
            mineCountFound->second.push_back(oneSectionSolution);
        }
        return;
    }
}










void recurseFinder(MineSweeper* mcopy, vector<vector<int>> section, 
    vector<vector<int>>* sectionSolution, vector<int>* groupSol, int depth)
{
    int _col = mcopy->returnCol(), _row = mcopy->returnRow();
    
    // excaper    
    if (depth == section.size()) 
    {
        for (auto& group : section) 
        {
            int tile = group[0];
            int tileRow = tile / _col, tileCol = tile % _col;

            auto openTiles = getOpenedTilesForOne(mcopy, tileCol, tileRow);
            for (auto& openTile : openTiles) 
            {
                int openTileRow = openTile / _col, openTileCol = openTile % _col;
                if (mcopy->countFlag(openTileCol, openTileRow) 
                    != mcopy->returnNeighborCount(openTileCol, openTileRow)) 
                {
                    return;
                }    
            }
        }
        
        // if not returned --> solution
        sectionSolution->push_back(*groupSol);
    }

    // recurser 
    else 
    {
        // Checker 
        for (auto& group : section) 
        {
            int tile = group[0];
            int tileRow = tile / _col, tileCol = tile % _col;

            auto openTiles = getOpenedTilesForOne(mcopy, tileCol, tileRow);
            for (auto& openTile : openTiles) 
            {
                int openTileRow = openTile / _col, openTileCol = openTile % _col;
                int countF = mcopy->countFlag(openTileCol, openTileRow);
                int countN = mcopy->returnNeighborCount(openTileCol, openTileRow);
                if (countF > countN)
                {
                    return;
                }    
            }
        }
    
        auto group1 = section[depth];

        for (int i = 0; i < group1.size() + 1; ++i) 
        {
            if (i == 0) 
            {
                groupSol->push_back(i);
                recurseFinder(mcopy, section, sectionSolution, 
                    groupSol, depth + 1);
                groupSol->pop_back();
            }
            else 
            {
                int tile1 = group1[i - 1];
                int tile1Row = tile1 / _col, tile1Col = tile1 % _col;
                mcopy->setFlag(tile1Col, tile1Row, true);
                groupSol->push_back(i);
                recurseFinder(mcopy, section, sectionSolution, 
                    groupSol, depth + 1);
                groupSol->pop_back();

                if (i == group1.size()) 
                {
                    for (int k = 0; k < group1.size(); ++k) 
                    {
                        int tilek = group1[k];
                        int tilekRow = tilek / _col, tilekCol = tilek % _col;
                        mcopy->setFlag(tilekCol, tilekRow, false);
                    }
                }
            }
        }
    }
}


vector<vector<vector<int>>> bruteSolFinder(MineSweeper* m, 
    vector<vector<vector<int>>> groupedTiles)
{
    vector<vector<vector<int>>> allSolutions;
    MineSweeper mcopy(*m);
    for (auto& section : groupedTiles) 
    {
        vector<vector<int>> sectionSolution;
        vector<int> groupSol;   
        recurseFinder(&mcopy, section, &sectionSolution, &groupSol, 0);
        allSolutions.push_back(sectionSolution);
        groupSol.clear();
    }
    return allSolutions;
}

map<int, double> bruteProbCal(MineSweeper* m, 
    vector<vector<vector<int>>> groupedTiles, vector<vector<vector<int>>> groupedSol)
{
    double p = getRandomGuessProb(m);
    int _col = m->returnCol(), _row = m->returnRow();

    vector<vector<vector<double>>> groupProb;
    map<int, double> tileProb;
    
    /// get section probabilities
    for (int i = 0; i < groupedSol.size(); ++i)			// section
    {
        double section_value_sum = 0;
        vector<double> sectionGroupSum;
        for (int j = 0; j < groupedSol[i].size(); ++j)		// # of solutions
        {
            double solutionPerProb = 1;
            for (int k = 0; k < groupedSol[i][j].size(); ++k)
            // group numbers;
            {
                int mineN = groupedSol[i][j][k];
                int groupSize = groupedTiles[i][k].size();
                solutionPerProb *= nChoosek(groupSize, mineN);
                int pmp = 0;
                while (pmp < groupSize)
                {
                    if (pmp < mineN) solutionPerProb *= p;
                    else solutionPerProb *= (1 - p);
                    ++pmp;
                }
            }
            sectionGroupSum.push_back(solutionPerProb);
            section_value_sum += solutionPerProb;
        }

        for (int j = 0; j < groupedSol[i].size(); ++j) 
        {
            for (int k = 0; k < groupedSol[i][j].size(); ++k) 
            {
                int mineN = groupedSol[i][j][k];
                int groupSize = groupedTiles[i][k].size();
                
                if (mineN == 0) 
                {
                    for (int tile : groupedTiles[i][k])
                    {
                        auto tileIterator = tileProb.find(tile);
                        if (tileIterator == tileProb.end())
                        {
                            tileProb[tile] = 0;
                        }
                    }					
                }
                else 
                {
                    for (int tile : groupedTiles[i][k])
                    {
                        auto tileIterator = tileProb.find(tile);
                        if (tileIterator == tileProb.end())
                        {
                            tileProb[tile] = sectionGroupSum[j] 
                                * double(mineN) / double(groupSize)
                                / section_value_sum;
                        }
                        else 
                        { 
                            tileProb[tile] += sectionGroupSum[j] 
                                * double(mineN) / double(groupSize)
                                / section_value_sum;
                        }
                    }
                }
            }
        }
    }
    return tileProb;
}





// The master function (calculate probablity)
void bruteSolver(MineSweeper* m, vector<string>* orders)
{
    int _col = m->returnCol(), _row = m->returnRow();
    int bruteRecurseCalls = 0;

    vector<vector<int>> connectedTiles;
    auto emptyTiles = getEmptyTiles(m);	

    vector<map<int, vector<map<int, bool>>>> bigSolutions;
    
    bool borderOptimization = false;		// true => bordertile divided by sections

    // if there is less than 23 tiles left
    if (emptyTiles.size() < 23)
    {
        // then brute force all left tiles
        connectedTiles.push_back(emptyTiles);
    }
    else
    {
        // divide bordertile by sections
        borderOptimization = true;
        connectedTiles = getConnectedBorderTiles(m);
    }





    auto groupedTiles = getGroupedBorderTiles(m);
    auto allSolutions = bruteSolFinder(m, groupedTiles);
    auto groupedProb = bruteProbCal(m, groupedTiles, allSolutions);
    cout << "group Solving done" << endl;
    cout << endl;
    

    
    
    
    int borderSectionSize = connectedTiles.size();
    if (borderSectionSize == 0)
    {
        cout << "Something went wrong... there is no bordertiles" << endl;
        cout << "Therefore, AI will guess random empty tile" << endl;
        randomGuess(m, orders);
        // system("pause");
        return; // something is wrong...
    }

    for (int s = 0; s < borderSectionSize; ++s)
    {
        // a set of solutions for a section
        map<int, vector<map<int, bool>>> sectionSolutions;

        // not to cause a problem, create a copy of m
        MineSweeper mcopy(*m);

        int sectionBlockSize = connectedTiles[s].size();
        cout << "Size of section: " << sectionBlockSize << endl;
         
        // start bruteforcing
        if (sectionBlockSize >= 30 && s == borderSectionSize - 1
            && bigSolutions.size() == 0)
        {
            cout << "This recursion may take extreme amount of time..." << endl;
            cout << "And there no solutions possible" << endl;
            cout << "It may be a better to guess..." << endl;
            cout << "Will you guess? y or n: ";
            string yesOrNo;
            getline(cin, yesOrNo);
            while(yesOrNo != "y" && yesOrNo != "n") 
            {
                cout << "Invalid input. Try again." << endl;
                cout << "Will you guess? y or n: ";
                getline(cin, yesOrNo);
            }
            if (yesOrNo == "y") 
            {
                randomGuess(m, orders);
                return;
            }
            else if (yesOrNo == "n")
            {
                bruteRecurse(&mcopy, &sectionSolutions, connectedTiles[s],
                    borderOptimization, 0, &bruteRecurseCalls);
            }
        }
        else if (sectionBlockSize >= 30 && borderSectionSize > 1)
        {
            cout << "This recursion will take extreme amount of time..." << endl;
            cout << "But there are more than one section" << endl;
            cout << "Will you pass? y or n: ";
            string yesOrNo;
            getline(cin, yesOrNo);
            while(yesOrNo != "y" && yesOrNo != "n") 
            {
                cout << "Invalid. Try again." << endl;
                cout << "Will you pass? y or n: ";
                getline(cin, yesOrNo);
            }
            if (yesOrNo == "y") 
            {
                continue;
            }				
            else if (yesOrNo == "n")
            {
                bruteRecurse(&mcopy, &sectionSolutions, connectedTiles[s],
                    borderOptimization, 0, &bruteRecurseCalls);	
            }		
        }
        else 
        {
            bruteRecurse(&mcopy, &sectionSolutions, connectedTiles[s],
                borderOptimization, 0, &bruteRecurseCalls);
        }
        bigSolutions.push_back(sectionSolutions);
    }

    /// probability calculation 
    // count how many times there were flags on each tile
    vector<map<int, map<int, int>>> sectionSum;
    for (auto& sectionSol : bigSolutions)
    {
        map<int, map<int, int>> oneKsum;
        for (auto& oneKmanySolutions : sectionSol)
        {
            int mineNumK = oneKmanySolutions.first;
            map<int, int> oneKcount;
            auto oneKmanySolutionsvector = oneKmanySolutions.second;
            for (auto oneSolution : oneKmanySolutionsvector)
            {
                for (auto tile : oneSolution)
                {
                    int location = tile.first;
                    bool isMine = tile.second;
                    auto finditerator = oneKcount.find(tile.first);
                    // if first time
                    if (finditerator == oneKcount.end())
                    {
                        oneKcount[location] = 0;
                        if (isMine) ++oneKcount[location];
                    }
                    // if second time
                    else if (isMine) ++(finditerator->second);
                }
            }
            oneKsum[mineNumK] = oneKcount;
        }
        sectionSum.push_back(oneKsum);
    }

    for (auto& p : sectionSum)
    {
        for (auto& q : p)
        {
            int mineNumK = q.first;
            auto tileMinecountmap = q.second;
            q.second[9999] = 0;
            for (auto& r : tileMinecountmap)
            {
                if (r.first != 9999)
                    q.second[9999] += r.second;
            }
        }
    }

    double randomProb = getRandomGuessProb(m);
    // section more than one
    if (sectionSum.size() > 1)
    {
        map<int, double> probStore;
        vector<int> sectionSolutionTotalStore;
        for (auto& p : bigSolutions)
        {
            int solutionTotal = 0;
            for (auto& q : p)
            {
                for (auto& r : q.second)
                    ++solutionTotal;
            }
            sectionSolutionTotalStore.push_back(solutionTotal);
        }

        for (int p = 0; p < sectionSum.size(); ++p)
        {
            for (auto& q : sectionSum[p])
            {
                int mineNumK = q.first;
                auto solutionSets = q.second;
                int solKsize = q.second.size();
                for (auto& r : solutionSets)
                {
                    int tile = r.first;
                    if (tile != 9999)
                    {
                        int timesMines = r.second;
                        auto tileIterator = probStore.find(tile);
                        if (tileIterator == probStore.end())
                        {
                            probStore[tile] = 0;
                            probStore[tile] += double(timesMines) / double(solutionSets[9999])
                                * double(bigSolutions[p][mineNumK].size()) 
                                / double(sectionSolutionTotalStore[p])
                                * mineNumK;
                        }
                        else
                        {
                            probStore[tile] += double(timesMines) / double(solutionSets[9999])
                                * double(bigSolutions[p][mineNumK].size()) 
                                / double(sectionSolutionTotalStore[p])
                                * mineNumK;
                        }
                    }
                }
            }
        }
        auto minProbTile = *min_element(probStore.begin(), probStore.end(), compare);
        
        if (minProbTile.second == 0)
        {
            for (auto& p : probStore)
            {
                if (p.second == 0)
                {
                    int chosenTileCol = p.first % _col;
                    int chosenTileRow = p.first / _col;
                    std::cout << "AI chosed Col: " << chosenTileCol << " Row: "
                        << chosenTileRow << " with the chance of "
                        << minProbTile.second * 100 << "%"
                        << " the tile being mine" << std::endl;
                    orders->push_back(simpleClick(chosenTileCol, chosenTileRow));
                }
            }
        }
        else if (minProbTile.second < randomProb) {
            int chosenTileCol = minProbTile.first % _col;
            int chosenTileRow = minProbTile.first / _col;
            std::cout << "AI chosed Col: " << chosenTileCol << " Row: " 
                << chosenTileRow << " with the chance of "
                << minProbTile.second * 100 << "%"
                << " the tile being mine" << std::endl;
            orders->push_back(simpleClick(chosenTileCol, chosenTileRow));
        }
        else
        {
            cout << "The chance of the random guess is less than" << endl
                << "calculated border tiles." << endl
                << "Random: " << randomProb * 100 << "%"
                << " Border: " << minProbTile.second * 100 << "%" << endl;
            cout << "Therefore, we choose randomly" << endl;
            randomGuess(m, orders);
        }
    }
    // only one section
    else
    {
        auto manySol = bigSolutions[0];
        map<int, double> probStore;
        int solutionTotal = 0;				// how many total solutions are there?
        for (auto& mk : manySol)
        {
            for (auto& n : mk.second)
            {
                ++solutionTotal;
            }
        }

        // real probablity calculation
        for (auto& p : sectionSum)
        {
            for (auto& q : p)
            {
                int mineNumK = q.first;
                auto solutionSets = q.second;
                int solKsize = q.second.size();
                for (auto& r : solutionSets)
                {
                    int tile = r.first;
                    if (tile != 9999) {
                        int timesMines = r.second;
                        auto tileIterator = probStore.find(tile);
                        if (tileIterator == probStore.end())
                        {
                            probStore[tile] = 0;
                            probStore[tile] += double(timesMines) / double(solutionSets[9999])
                                * double(manySol[mineNumK].size()) / double(solutionTotal)
                                * mineNumK;
                        }
                        else
                        {
                            probStore[tile] += double(timesMines) / double(solutionSets[9999])
                                * double(manySol[mineNumK].size()) / double(solutionTotal)
                                * mineNumK;
                        }
                    }
                }
            }
        }
        auto minProbTile = *min_element(probStore.begin(), probStore.end(), compare);

        if (minProbTile.second == 0)
        {
            for (auto& p : probStore)
            {
                if (p.second == 0)
                {
                    int chosenTileCol = p.first % _col;
                    int chosenTileRow = p.first / _col;
                    std::cout << "AI chosed Col: " << chosenTileCol << " Row: "
                        << chosenTileRow << " with the chance of "
                        << minProbTile.second * 100 << "%"
                        << " the tile being mine" << std::endl;
                    orders->push_back(simpleClick(chosenTileCol, chosenTileRow));
                }
            }
        }
        else if (minProbTile.second < randomProb) {
            int chosenTileCol = minProbTile.first % _col;
            int chosenTileRow = minProbTile.first / _col;
            std::cout << "AI chosed Col: " << chosenTileCol << " Row: " 
                << chosenTileRow << " with the chance of "
                << minProbTile.second * 100 << "%"
                << " the tile being mine" << std::endl;
            orders->push_back(simpleClick(chosenTileCol, chosenTileRow));
        }
        else
        {
            cout << "The chance of the random guess is less than" << endl
                << "calculated border tiles." << endl
                << "Random: " << randomProb * 100 << "%"
                << " Border: " << minProbTile.second * 100 << "%" << endl;
            cout << "Therefore, we choose randomly" << endl;
            randomGuess(m, orders);
        }
    }
    cout << endl << "bruteRecurse was called " << bruteRecurseCalls << " times. " << endl << endl;
    
    /// legacy probability	
    /*
    vector< map<int, int>> sectionSum;
    for (auto& sectionSol : bigSolutions)
    {
        map<int, int> sectionCount;
        for (auto& solution : sectionSol)
        {
            for (auto& tile : solution)
            {
                int location = tile.first;
                bool isMine = tile.second;
                auto finditerator = sectionCount.find(tile.first);
                // if first time
                if (finditerator == sectionCount.end())
                {
                    sectionCount[location] = 0;
                    if (isMine) ++sectionCount[location];
                }
                // if second time
                else if (isMine) ++(finditerator->second);
            }
        }
        sectionSum.push_back(sectionCount);
    }
    // would this the right way to calculate the probablity?
    for (auto& section : sectionSum)
    {
        section[9999] = 0;
        for (auto& location : section)
        {
            if (location.first != 9999)
                section[9999] += location.second;
        }
    }
    
    // when there is more than one section
    if (borderSectionSize > 1)
    {
        vector<int> tileStore;			// store tile
        vector<double> probStore;		// store probability
        for (auto& section : sectionSum)
        {
            auto min = *min_element(section.begin(), section.end(), compare);
            int total = section[9999];
            tileStore.push_back(min.first);
            probStore.push_back(double(min.second) / double(total) * 100);
        }
        int dis = distance(probStore.begin(), min_element(probStore.begin(), probStore.end()));
        int tile = tileStore[dis];
        int col = tile % m->returnCol(), row = tile / m->returnCol();
        std::cout << "AI chosed Col: " << col << " Row: " << row << " with the chance of "
            << probStore[dis] << "%"
            << " the tile being mine" << std::endl;
        orders->push_back(simpleClick(col, row));
    }
    // one section
    else
    {
        auto section = sectionSum[0];
        auto min = *min_element(section.begin(), section.end(), compare);
        int col = min.first % m->returnCol(), row = min.first / m->returnCol();
        std::cout << "AI chosed Col: " << col << " Row: " << row << " with the chance of " 
            << double(min.second) / double(section[9999]) * 100 << "%" 
            << " the tile being mine" << std::endl;
        orders->push_back(simpleClick(col, row));
    }
    */
}

// the wrapper
void AI(MineSweeper* m, json* to_send)
{
    // store scripts
    vector<string> scripts;
    
    // basic flagging
    basicFlagging(m, &scripts);

    // Double clicking
    basicDoubleClicking(m, &scripts);

    if (scripts.size() == 0)
    {
        cout << "Guesses start..." << endl;
        bruteSolver(m, &scripts);
    }

    (*to_send)["orders"] = scripts;
}