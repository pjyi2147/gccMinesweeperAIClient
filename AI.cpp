#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <ctime>
#include <list>

#include "json.hpp"
#include "minesweeper.h"


using namespace std;
using json = nlohmann::json;

unsigned nChoosek(unsigned n, unsigned k)
{
	if (k > n) return 0;
	if (k * 2 > n) /*return*/ k = n - k;  //remove the commented section
	if (k == 0) return 1;

	int result = n;
	for (int i = 2; i <= k; ++i) {
		result *= (n - i + 1);
		result /= i;
	}
	return result;
}

bool compare(pair<int, double> i, pair<int, double> j) {
	return i.second < j.second;
}

string doubleClick(int col, int row)
{
	stringstream s;
	s << "D " << to_string(col) << " " << to_string(row);
	return s.str();
}

string simpleClick(int col, int row)
{
	stringstream s;
	s << "E " << to_string(col) << " " << to_string(row);
	return s.str();
}

string flagClick(int col, int row)
{
	stringstream s;
	s << "F " << to_string(col) << " " << to_string(row);
	return s.str();
}

void basicFlagging(MineSweeper* m, vector<string>* orders)
{
	for (int r = 0; r < m->returnRow(); ++r)
	{
		for (int c = 0; c < m->returnCol(); ++c)
		{
			// when the tile is done or covered, then continue;
			if (m->returnDone(c, r) || m->returnCovered(c, r))
				continue;

			// the number of covered is the same as the neighborcount
			if (m->returnNeighborCount(c, r) != 0 && m->returnNeighborCount(c, r) == m->countCovered(c, r))
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

							if (col > -1 && col < m->returnCol()
								&& row > -1 && row < m->returnRow())
							{
								// covered and not flagged 
								// flag the tile!
								if (m->returnCovered(col, row) && !m->returnFlagged(col, row))
								{
									orders->push_back(flagClick(col, row));

									cout << "Flagged col: " << col << " row: " << row << endl;
									m->setFlag(col, row);
									// set done
									m->setDone(col, row, true);
								}
							}
						}
					}
				}
			}
		}
	}
}

void basicDoubleClicking(MineSweeper* m, vector<string>* orders)
{
	for (int r = 0; r < m->returnRow(); ++r)
	{
		for (int c = 0; c < m->returnCol(); ++c)
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

vector<int> getBorderTiles(MineSweeper* m)
{
	vector<int> borderTiles;
	for (int r = 0; r < m->returnRow(); ++r)
	{
		for (int c = 0; c < m->returnCol(); ++c)
		{
			// when the tile is done or revealed or flagged, then continue;
			if (m->returnDone(c, r) || !m->returnCovered(c, r) || m->returnFlagged(c, r))
				continue;

			// examine 8 adjacent tiles
			for (int yoff = -1; yoff <= 1; ++yoff)
			{
				bool border = false;
				for (int xoff = -1; xoff <= 1; ++xoff)
				{
					if (xoff != 0 || yoff != 0)
					{
						int col = c + xoff;
						int row = r + yoff;

						// should not be out of bounds 
						if (col > -1 && col < m->returnCol()
							&& row > -1 && row < m->returnRow())
						{
							// if one of the neighboring tile is open
							if (!m->returnCovered(col, row))
							{
								// then it is bordertile
								borderTiles.push_back(r*m->returnCol() + c);
								border = true;
								break;
							}
						}
					}
				}
				if (border) break;
			}
		}
	}
	return borderTiles;
}

vector<int> getEmptyTiles(MineSweeper* m)
{
	vector<int> EmptyTiles;
	int mineRow = m->returnRow(), mineCol = m->returnCol();
	for (int r = 0; r < mineRow; ++r)
	{
		for (int c = 0; c < mineCol; ++c)
		{
			// not flagged and is covered
			if (!m->returnFlagged(c, r) && m->returnCovered(c, r))
			{
				EmptyTiles.push_back(r*m->returnCol() + c);
			}
		}
	}
	return EmptyTiles;
}

vector< vector<int> > getConnectedBorderTiles(MineSweeper *m, vector<int> borderTiles)
{
	vector< vector<int> > k;
	vector<int> covered;

	while (true)
	{
		list<int> queue;
		vector<int> finishedRegion;

		for (auto& firstT: borderTiles)
		{
			if (std::find(covered.begin(), covered.end(), firstT) == covered.end()) {
				queue.push_back(firstT);
				break;
			}
		}
		
		if (queue.size() == 0)
			break;

		while(queue.size() != 0)
		{
			int curTile = queue.front();
			queue.pop_front();

			int curCol = curTile % m->returnCol();
			int curRow = curTile / m->returnCol();
			
			finishedRegion.push_back(curTile);
			covered.push_back(curTile);

			for (auto& tile: borderTiles)
			{
				int tileCol = tile % m->returnCol();
				int tileRow = tile / m->returnCol();

				bool isConnected = false;
				
				if (std::find(finishedRegion.begin(), finishedRegion.end(), tile) != finishedRegion.end())
					continue;

				// the distance should be less than 2 to have common open tile 
				if (abs(curCol - tileCol) <= 2 || abs(curRow - tileRow) <= 2)
				{
					for (int r = 0; r < m->returnRow(); ++r)
					{
						for (int c = 0; c < m->returnCol(); ++c)
						{
							if (!m->returnCovered(c, r))
							{
								if (abs(curCol - c) <= 1 && abs(curRow - r) <= 1 &&
									abs(tileCol - c) <= 1 && abs(tileRow - r) <= 1)
								{
									isConnected = true;
									break;
								}
							}
						}
						if (isConnected) break;
					}
				}

				if (!isConnected) continue;

				if (find(queue.begin(), queue.end(), tile) == queue.end())
					queue.push_back(tile);
			}
		}
		k.push_back(finishedRegion);
	}
	return k;
}

double getRandomGuessProb(MineSweeper* m)
{
	int numEmptyTiles = getEmptyTiles(m).size();
	int mineNum = m->returnMineNum();
	int numFlags = m->countAllFlagged();

	return double(mineNum - numFlags) / double(numEmptyTiles);
}

void randomGuess(MineSweeper* m, vector<string>* orders)
{
	vector<int> emptyTiles = getEmptyTiles(m);
	int mineNum = m->returnMineNum();
	int numFlags = m->countAllFlagged();
	srand(time(NULL));
	int k = emptyTiles[rand() % emptyTiles.size()];
	int col = k % m->returnCol(), row = k / m->returnCol();
	orders->push_back(simpleClick(col, row));
	cout << "AI guessed Col: " << col << " Row: " << row << " from the empty tiles" << endl;
	cout << "with " << double(mineNum-numFlags) / double(emptyTiles.size()) * 100 << "% chance of the tile being mine." << endl;
}

void tankRecurse(vector<int> section, MineSweeper* m, int k, 
	bool borderOptimization, map<int, vector<map<int, bool>>>* solutions, int* test)
{
	++(*test);
	int totalMineNum = m->returnMineNum();
	int flagCount = m->countAllFlagged();
	int mineCol = m->returnCol(), mineRow = m->returnRow();
	
	// total flagcount cannot exceed the total mineNum
	if (flagCount > totalMineNum) return;
	
	// DFS Search
	// go to depth k and check it is correct
	if (k == section.size())
	{
		for (auto& tile : section)
		{
			int tCol = tile % mineCol, tRow = tile / mineCol;
			for (int yoff = -1; yoff <= 1; ++yoff)
			{
				for (int xoff = -1; xoff <= 1; ++xoff)
				{
					int c = tCol + xoff, r = tRow + yoff;
					// within in the range
					if (c > -1 && c < mineCol &&
						r > -1 && r < mineRow)
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

		map<int, bool> solution;
		int mineCountForSection = 0;
		for (auto& i : section)
		{
			int col = i % m->returnCol(), row = i / m->returnCol();
			bool flagged = m->returnFlagged(col, row);
			solution.insert(make_pair(i, flagged));
			if (flagged) ++mineCountForSection;
		}
		auto mineCountFound = solutions->find(mineCountForSection);
		// if there isnt a vector for the mine count
		if (mineCountFound == solutions->end())
		{
			vector<map<int, bool>> kool;
			kool.push_back(solution);
			solutions->insert(make_pair(mineCountForSection, kool));
		}
		// if there is
		else
		{
			mineCountFound->second.push_back(solution);
		}

		return;
	}

	// if the flagcount is bigger than the neighborcount already?
	// then, return before it goes through more recursion.
	// for more optimization, the open tiles for the section can be
	// put into a container to be looped
	for (auto& tile1 : section)
	{
		int tCol1 = tile1 % mineCol, tRow1 = tile1 / mineCol;
		for (int yoff = -1; yoff <= 1; ++yoff)
		{
			for (int xoff = -1; xoff <= 1; ++xoff)
			{
				int c = tCol1 + xoff;
				int r = tRow1 + yoff;
				// within in the range
				if (c > -1 && c < mineCol &&
					r > -1 && r < mineRow)
				{
					// should be revealed
					if (!m->returnCovered(c, r))
					{
						// if the flagcount is bigger than the neighborcount already?
						// then, return before it goes through more recursion.
						if (m->countFlag(c, r) > m->returnNeighborCount(c, r))
							return;
					}
				}
			}
		}
	}
	int location = section[k];
	int qCol = location % m->returnCol();
	int qRow = location / m->returnCol();

	m->setFlag(qCol, qRow, true);
	tankRecurse(section, m, k + 1, borderOptimization, solutions, test);
	m->setFlag(qCol, qRow, false);
	tankRecurse(section, m, k + 1, borderOptimization, solutions, test);
}

void tankSolver(MineSweeper* m, vector<string>* orders)
{
	auto borderTiles = getBorderTiles(m);
	auto emptyTiles = getEmptyTiles(m);
	int test = 0;
	vector< vector<int> > connectedTiles;
	vector<map<int, vector<map<int, bool>>>> bigSolutions;
	bool borderOptimization = false;
	// endGame tactics
	if (emptyTiles.size() < 23)
	{
		connectedTiles.push_back(emptyTiles);
	}
	else
	{
		borderOptimization = true;
		connectedTiles = getConnectedBorderTiles(m, borderTiles);
	}
	int conSize = connectedTiles.size();
	if (conSize == 0) {
		cout << "Something went wrong... there is no bordertiles" << endl;
		cout << "Therefore, AI will guess random empty tile" << endl;
		randomGuess(m, orders);
		system("pause");
		return; // something is wrong...
	}

	for (int s = 0; s < conSize; ++s)
	{
		// a set of solutions for a section
		map<int, vector<map<int, bool>>> solutions;

		// not to cause a problem, create a copy of m
		auto&& mcopy = MineSweeper(*m);
		int sectionSize = connectedTiles[s].size();
		cout << "Size of section: " << sectionSize << endl;
		if (sectionSize >= 17 && sectionSize < 25) {
			cout << "This recursion will take some time..." << endl;
			tankRecurse(connectedTiles[s], &mcopy, 0, borderOptimization, &solutions, &test);
		}
		else if (sectionSize >= 27 && conSize == 1)
		{
			cout << "This recursion will take extreme amount of time..." << endl;
			cout << "And there is only one section" << endl;
			cout << "Therefore, it is a better choice to make a guess..." << endl;
			randomGuess(m, orders);
			return;
		}
		else if (sectionSize >= 27 && conSize >= 1 && s == conSize - 1)
		{
			cout << "This recursion will take extreme amount of time..." << endl;
			cout << "It is highly likely that multiple sections are over limit." << endl;
			cout << "Therefore, we make a guess..." << endl;
			randomGuess(m, orders);
			return;
		}
		else if (sectionSize >= 27 && conSize >= 1)
		{
			cout << "This recursion will take extreme amount of time..." << endl;
			cout << "But there are more than one section" << endl;
			cout << "Therefore, it is a better choice to pass this section and come back later" << endl;
			continue;
		}
		else 
		{
			tankRecurse(connectedTiles[s], &mcopy, 0, borderOptimization, &solutions, &test);
		}
		bigSolutions.push_back(solutions);
	}

	// probability calculation 
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
								* double(bigSolutions[p][mineNumK].size()) / double(sectionSolutionTotalStore[p])
								* mineNumK;
						}
						else
						{
							probStore[tile] += double(timesMines) / double(solutionSets[9999])
								* double(bigSolutions[p][mineNumK].size()) / double(sectionSolutionTotalStore[p])
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
					int chosenTileCol = p.first % m->returnCol();
					int chosenTileRow = p.first / m->returnCol();
					std::cout << "AI chosed Col: " << chosenTileCol << " Row: "
						<< chosenTileRow << " with the chance of "
						<< minProbTile.second * 100 << "%"
						<< " the tile being mine" << std::endl;
					orders->push_back(simpleClick(chosenTileCol, chosenTileRow));
				}
			}
		}
		else if (minProbTile.second < randomProb) {
			int chosenTileCol = minProbTile.first % m->returnCol();
			int chosenTileRow = minProbTile.first / m->returnCol();
			std::cout << "AI chosed Col: " << chosenTileCol << " Row: " << chosenTileRow << " with the chance of "
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
					int chosenTileCol = p.first % m->returnCol();
					int chosenTileRow = p.first / m->returnCol();
					std::cout << "AI chosed Col: " << chosenTileCol << " Row: "
						<< chosenTileRow << " with the chance of "
						<< minProbTile.second * 100 << "%"
						<< " the tile being mine" << std::endl;
					orders->push_back(simpleClick(chosenTileCol, chosenTileRow));
				}
			}
		}
		else if (minProbTile.second < randomProb) {
			int chosenTileCol = minProbTile.first % m->returnCol();
			int chosenTileRow = minProbTile.first / m->returnCol();
			std::cout << "AI chosed Col: " << chosenTileCol << " Row: " << chosenTileRow << " with the chance of "
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
	if (conSize > 1)
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
	cout << endl << "tankRecurse was called " << test << " times. " << endl << endl;

}

void AI(MineSweeper* m, json* to_send)
{
	// first is for flagging
	vector<string> orders;
	
	// basic flagging
	basicFlagging(m, &orders);

	// Double clicking
	basicDoubleClicking(m, &orders);

	if (orders.size() == 0)
	{
		cout << "Guesses start..." << endl;
		tankSolver(m, &orders);
	}
	(*to_send)["orders"] = orders;
}