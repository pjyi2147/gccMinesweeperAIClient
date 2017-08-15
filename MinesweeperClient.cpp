// MinesweeperAI.cpp : Defines the entry point for the console application.
//

#include <iostream>

#include "minesweeper.h"
#include "client.h"
#include "json.hpp"
#include "AI.h"

#include <boost/thread/thread.hpp>

using namespace std;
using json = nlohmann::json;

void GameLoop(json* info, json* to_send, MineSweeper *m)
{
	m->printMineField();
	AI(m, to_send);
	boost::this_thread::sleep(boost::posix_time::milliseconds(100));
	ingameTransfer(to_send, info, m);
}

int main()
{
	cout << "Minesweeper AI" << endl << endl;
	int col, row, mineNum;
	startGamefunction(col, row, mineNum);
	auto minefield = new MineSweeper(col, row, mineNum);

	json info, to_send;
	firstScriptTransfer(col, row, info);
	update(minefield, info);
	
	while (!info["GameEnd"])
	{
		GameLoop(&info, &to_send, minefield);
	}

	minefield->printMineField();
	if (info["win"])
	{
		cout << "won!" << endl;
	}
	else
	{
		cout << "lost..." << endl;
	}
	cin.ignore();
	cin.get();
	return 0;
}