#ifndef RAM_H
#define RAM_H

#include <vector>
#include <map>

#include "addr_space.hpp"
#include "address.hpp"
#include "const.hpp"

using namespace std;

class Ram {
  public:
    Ram(ProvidesOutput inputIn) 
        : input(inputIn) {
      // Initializes the state, one per address space.
      state[CODE] = vector<vector<bool>>(RAM_SIZE, vector<bool>(WORD_SIZE));
      state[DATA] = vector<vector<bool>>(RAM_SIZE, vector<bool>(WORD_SIZE)); 
    }

    map<AddrSpace, vector<vector<bool>>> state;
    vector<bool> output;
    bool outputPending = false;

    vector<bool> get(Address adr) const;
    void set(Address adr, vector<bool> wordIn);
    string getString() const;

  private:
    const ProvidesOutput input;

    vector<bool> getLastAddress(AddrSpace addrSpace) const;
    vector<bool> getInput() const;
    void saveWord(AddrSpace addrSpace, int address, vector<bool> wordIn);
    void assignToLastAddress(AddrSpace addrSpace, vector<bool> wordIn);
    string getString(AddrSpace addrSpace) const;
};

#endif