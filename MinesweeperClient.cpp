#include <iostream>
#include <boost/thread/thread.hpp>

#include "json.hpp"
#include "minesweeper.h"
#include "AI.h"
#include "client.h"

using namespace std;
using json = nlohmann::json;

void GameLoop(json* info, json* to_send, MineSweeper *m)
{
	while (!(*info)["GameEnd"])
	{
		m->printMineField();
		AI(m, to_send);
		boost::this_thread::sleep(boost::posix_time::milliseconds(40));
		ingameTransfer(to_send, info, m);
	}
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

	GameLoop(&info, &to_send, minefield);
	
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