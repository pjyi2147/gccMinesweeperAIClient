#pragma once

bool compare(pair<int, int> i, pair<int, int> j);
unsigned nChoosek(unsigned n, unsigned k);

string simpleClick(int col, int row);
string doubleClick(int col, int row);
string flagClick(int col, int row);

void basicFlagging(MineSweeper* m, vector<string>* orders);
void basicDoubleClicking(MineSweeper* m, vector<string>* orders);

vector<int> getEmptyTiles(MineSweeper* m);
double getRandomGuessProb(MineSweeper* m);

vector<int> getBorderTiles(MineSweeper* m);
vector<vector<int>> getConnectedBorderTiles(MineSweeper *m);

void randomGuess(MineSweeper* m, vector<string>* orders);
void bruteRecurse(MineSweeper* m, map<int, vector<map<int, bool>>>* sectionSolutions, 
    vector<int> section, bool borderOptimization, int depthK, int* functionCalls);
void bruteSolver(MineSweeper* m, vector<string>* orders);

void AI(MineSweeper* m, nlohmann::json* to_send);