#ifndef RAM_H
#define RAM_H

#include <vector>
#include <map>

#include "addr_space.hpp"

using namespace std;

class Ram {
	public:
		Ram();
		map<AddrSpace, vector<vector<bool>>> state;
		vector<bool> get(AddrSpace addrSpace, vector<bool> adr);
		void set(AddrSpace addrSpace, vector<bool> adr, vector<bool> wordIn);
		string getString();

	private:
		vector<bool> getLastAddress(AddrSpace addrSpace);
		vector<bool> getInput();
		void saveWord(AddrSpace addrSpace, int address, vector<bool> wordIn);
		void assignToLastAddress(AddrSpace addrSpace, vector<bool> wordIn);
		string getString(AddrSpace addrSpace);
};

#endif