#pragma once

bool compare(pair<int, int> i, pair<int, int> j);
unsigned nChoosek(unsigned n, unsigned k);

void AI(MineSweeper* m, nlohmann::json* to_send);

void basicFlagging(MineSweeper* m, vector<string>* orders);
void basicDoubleClicking(MineSweeper* m, vector<string>* orders);

void tankSolver(MineSweeper* m, vector<string>* orders);
void tankRecurse(vector<int> section, MineSweeper* m, bool borderOptimization, map<int, map<int, bool>>* solutions, int*);
void randomGuess(MineSweeper* m, vector<string>* orders);

double getRandomGuessProb(MineSweeper* m);

vector<int> getBorderTiles(MineSweeper* m);
vector<int> getEmptyTiles(MineSweeper* m);
vector<vector<int>> getConnectedBorderTiles(MineSweeper *m);

string doubleClick(int col, int row);
string flagClick(int col, int row);
string simpleClick(int col, int row);