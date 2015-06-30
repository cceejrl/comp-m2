#ifndef RENDERER_H
#define RENDERER_H

#include <map>
#include <string>

#include "cpu.hpp"
#include "printer.hpp"
#include "ram.hpp"

using namespace std;

class Renderer {
	public:
		static string renderState(Printer printerIn, Ram ramIn, Cpu cpuIn);

	private:
		Renderer (Printer printerIn, Ram ramIn, Cpu cpuIn);
		Printer printer;
		Ram ram;
		Cpu cpu;
		// Counts occurances of a char in a drawing.
		map<char, int> switchIndex;
		string insertActualValues(string lineIn);
		char getLightbulb(char cIn);
		bool pcIsPointingToAddress(int adr);
		bool instructionIsPointingToAddress(int adr);
		bool instructionHasId(int id);
		char getFormattedOutput(int i);
		char getInstructionRamBit(int i);
		char getDataRamBit(int i);
		bool machineNotActive();
};

#endif