#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>

#include "json.hpp"
#include "minesweeper.h"

using namespace std;
using boost::asio::ip::tcp;
using json = nlohmann::json;

void update(MineSweeper* m, json info)
{
	int _col = m->returnCol(), _row = m->returnRow();
	m->setGameEnd(info["GameEnd"]);
	m->setWin(info["win"]);
	string mineState = info["mineField"];
	int length1 = mineState.length();
	for (int i = 0; i < length1; ++i)
	{
		int row = i / _col;
		int col = i % _col;
		// when numbers
		int k = mineState[i];
		if (isdigit(k))
		{
			m->setNeighborCount(col, row, k - 48);
			m->setReveal(col, row);
			if (m->returnNeighborCount(col, row) == 0)
			{
				m->setDone(col, row, true);
			}
		}
		// when flagged;
		else if (k == 'F')
		{
			if (!m->returnFlagged(col, row))
			{
				cout << "Col: " << col << " Row: " << row << " should be flagged but not flagged" << endl;
			}
		}
		// when covered;
		else if (k == 'C')
		{
			if (!m->returnCovered(col, row)) 
			{
				cout << "Col: " << col << " Row: " << row << " should be covered but not covered" << endl;
			}
		}
		// when mined;
		else if (k == 'M')
		{
			m->setReveal(col, row);
			m->setMine(col, row);
		}
	}
}

void startGamefunction(int &col, int &row, int &mineNum)
{
	try
	{
		// setup
		boost::asio::io_service io_service;
		tcp::resolver resolver(io_service);
		tcp::resolver::query query("localhost", "1234");
		tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
		tcp::socket socket(io_service);

		boost::asio::connect(socket, endpoint_iterator);

		// write
		boost::system::error_code ignored_error;

		cout << "Input level: ";
		string message;
		getline(cin, message);
		if (message != "1" && message != "2" && message != "3")
		{
			cout << "Not supported level" << endl;
			cout << "Input level:";
			getline(cin, message);
		}

		boost::asio::write(socket, boost::asio::buffer(message), ignored_error);
		cout << "Message send" << endl;

		// read
		cout << "Reading messages..." << endl;
		boost::array<char, 700> buf;
		boost::system::error_code error;
		size_t len = socket.read_some(boost::asio::buffer(buf), error);

		auto message_rec = string(buf.begin(), buf.begin()+len);
		cout << "Message read: " << message_rec << endl;

		if (error)
			throw boost::system::system_error(error);
		
		auto message_rec_json = json::parse(message_rec);
		
		col = message_rec_json["col"];
		row = message_rec_json["row"];
		mineNum = message_rec_json["mineNum"];
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
}

void firstScriptTransfer(int col, int row, nlohmann::json& info)
{
	try
	{
		// setup
		boost::asio::io_service io_service;
		tcp::resolver resolver(io_service);
		tcp::resolver::query query("localhost", "1234");
		tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
		tcp::socket socket(io_service);

		boost::asio::connect(socket, endpoint_iterator);

		// write
		boost::system::error_code ignored_error;

		json to_send;
		to_send["col"] = int(col / 2);
		to_send["row"] = int(row / 2);
		to_send["command"] = string("E");

		string message = to_send.dump();
		boost::asio::write(socket, boost::asio::buffer(message), ignored_error);
		cout << "Message send" << endl;

		// read
		cout << "Reading messages..." << endl;
		boost::array<char, 700> buf;
		boost::system::error_code error;
		size_t len = socket.read_some(boost::asio::buffer(buf), error);

		if (error)
			throw boost::system::system_error(error);
		auto message_rec = string(buf.begin(), buf.begin() + len);
		cout << "Message read: " << message_rec << endl;
		info = json::parse(message_rec);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
}

void ingameTransfer(nlohmann::json* to_send, nlohmann::json* info, MineSweeper* m)
{
	try
	{
		// setup
		boost::asio::io_service io_service;
		tcp::resolver resolver(io_service);
		tcp::resolver::query query("localhost", "1234");
		tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
		tcp::socket socket(io_service);

		boost::asio::connect(socket, endpoint_iterator);

		// write
		boost::system::error_code ignored_error;

		string message = to_send->dump();
		boost::asio::write(socket, boost::asio::buffer(message), ignored_error);
		cout << "Message send" << endl;

		// read
		cout << "Reading messages..." << endl;
		boost::array<char, 700> buf;
		boost::system::error_code error;
		size_t len = socket.read_some(boost::asio::buffer(buf), error);

		if (error)
			throw boost::system::system_error(error);
		auto message_rec = string(buf.begin(), buf.begin() + len);
		cout << "Message read: " << message_rec << endl;
		*info = json::parse(message_rec);
		update(m, *info);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
}
