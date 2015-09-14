#include "interactive_mode.hpp"

#include <string>

#include "computer.hpp"
#include "load.hpp"
#include "ram.hpp"

using namespace std;

Computer InteractiveMode::getComputer(string filename) {
  Ram ram = Ram(input);
  Load::fillRamWithFile(filename, ram);
  return Computer(0, ram, redrawScreen, sleepAndCheckForKey);
}

////////////////////

void InteractiveMode::start() {
  selectView();
  setEnvironment();
  prepareOutput();
  clearScreen();
  redrawScreen();
  userInput();
  // printer.run();
}

void InteractiveMode::selectView() {
  const char* term = std::getenv("TERM");
  if (strcmp(term, "linux") == 0) {
    selectedView = &VIEW_3D_B;
  } else if (strcmp(term, "rxvt") == 0) {
    selectedView = &VIEW_2D;
  }
}

/*
 * Initializes 'output.cpp' by sending dimensions of a 'drawing' and 
 * a 'drawScreen' callback function, that output.c will use on every 
 * screen redraw.
 */
void InteractiveMode::prepareOutput() {
  setOutput(&drawScreen, selectedView->width, selectedView->height);
}

void InteractiveMode::drawScreen() {
  buffer = Renderer::renderState(printer, ram, cpu, cursor, *selectedView);
  int i = 0;
  for (vector<string> line : buffer) {
    replaceLine(line, i++);
  }
}

//////////////////////
/// EXECUTION MODE ///
//////////////////////

/*
 * Saves the state of the ram and starts the execution of a program.
 * When execution stops, due to it reaching last address or user pressing 
 * 'esc', it loads back the saved state of the ram, and resets the cpu.
 */
void InteractiveMode::run() {
  // if (executionCounter > 0) {
  //   printer.printEmptyLine();
  // }
  savedRamState = ram.state;
  printer.run()
  // If 'esc' was pressed then it doesn't wait for keypress at the end.
  if (executionCanceled) {
    executionCanceled = false;
  } else {
    readStdin(false);
  }
  computer.ram.state = savedRamState;
  cpu.reset();
  redrawScreen();
  executionCounter++;
}

/*
 * Runs every cycle.
 */
void InteractiveMode::sleepAndCheckForKey() {
  usleep(FQ*1000);
  // Exits if ctrl-c was pressed.
  if (pleaseExit) {
    exit(0);
  }
  // Pauses execution if a key was hit, and waits for another key hit.
  if (int keyCode = Util::getKey()) {
    // If escape was pressed.
    if (keyCode == 27) {
      executionCanceled = true;
      return;
    }
    // "Press key to continue."
    keyCode = readStdin(false);
    // If esc key was pressed.
    if (keyCode == 27) {
      executionCanceled = true;
    }
  }
}

////////////////////
/// EDITING MODE ///
////////////////////

void InteractiveMode::userInput() {
  while(1) {
    char c = readStdin(true);
    if (insertChar) {
      isertCharIntoRam(c);
    } else if (shiftPressed) {
      processInputWithShift(c);
    } else {
      if (insertNumber) {
        if (insertNumberIntoRam(c)) {
          continue;
        }
      }
      switch (c) {   
        // BEGIN EXECUTION
        case 10:   // enter
          run();
          break;

        // MODES
        case 50:   // 2
          shiftPressed = true;
          break;
        case 105:  // i
          engageInsertCharMode();
          break;
        case 73:   // I
          engageInsertNumberMode();
          break;

        // VIEWS
        case 118:  // v
          switchDrawing();
          break;

        // SAVE
        case 115:  // s
          saveRamToNewFile();
          break;
        case 83:   // S
          saveRamToCurrentFile();
          break;

        // BASIC MOVEMENT
        case 107:  // k
        case 65:   // A, part of escape seqence of up arrow
          cursor.decreaseY();
          break;
        case 106:  // j
        case 66:   // B, part of escape seqence of down arrow
          cursor.increaseY();
          break;
        case 108:  // l
        case 67:   // C, part of escape seqence of rigth arrow
          cursor.increaseX();
          break;
        case 104:  // h
        case 68:   // D, part of escape seqence of left arrow
          cursor.decreaseX();
          break;
        case 116:  // t
        case 9:    // tab
          cursor.switchAddressSpace();
          break;

        // ADVANCED MOVEMENT
        case 72:   // H (home)
        case 103:  // g
          cursor.setBitIndex(0);
          break;
        case 70:   // F (end)
        case 71:   // G
          cursor.setBitIndex(WORD_SIZE-1);
          break; 
          break;
        case 111:  // o
          cursor.increaseY();
          cursor.setBitIndex(0);
          break;
        case 101:  // e
          cursor.goToEndOfWord();
          break;
        case 98:   // b
          cursor.goToBeginningOfWord();
          break;
        case 119:  // w
          cursor.goToBeginningOfNextWord();
          break;
        case 97:   // a
          cursor.setBitIndex(4);
          break;
        case 36:   // $
          cursor.setBitIndex(WORD_SIZE-1);
          break;
        case 94:   // ^
          cursor.setBitIndex(0);
          break;        
        case 122:  // z
        case 90:   // shift + tab
        case 84:   // T
          cursor.goToInstructionsAddress();
          break;

        // BASIC MANIPULATION
        case 32:   // space
          cursor.switchBit();
          break;
        case 51:   // 3, part of escape seqence of delete key
          cursor.eraseByte();
          break;        
        case 75:   // K
        case 53:   // 5, part of escape seqence of page up
          cursor.moveByteUp();
          break;
        case 74:   // J
        case 54:   // 6, part of escape seqence of page down
          cursor.moveByteDown();
          break;

        // ADVANCED MANIPULATION
        case 102:  // f
          cursor.setBit(true);
          cursor.increaseX();
          break;
        case 100:  // d
          cursor.setBit(false);
          cursor.increaseX();
          break;
        case 120:  // x
          cursor.eraseByte();
          cursor.setBitIndex(0);
          break;
      }
    }
    redrawScreen();
  }
}

void InteractiveMode::isertCharIntoRam(char c) {
  insertChar = false;
  if (c == 27) {  // Esc
    return;
  }
  cursor.setWord(Util::getBoolByte(c));
  cursor.increaseY();
}

void InteractiveMode::processInputWithShift(char c) {
  shiftPressed = false;
  if (c == 65) {
    cursor.moveByteUp();
  } else if (c == 66) {
    cursor.moveByteDown();
  }
}

// Returns whether the loop should continue.
bool InteractiveMode::insertNumberIntoRam(char c) {
  if (c < 48 || c > 57) {
    digits = vector<int>();
    insertNumber = false;
    return false;
  }
  digits.insert(digits.begin(), c - 48);
  int numbersValue = 0;
  int i = 0;
  for (int digit : digits) {
    numbersValue += digit * pow(10, i++);
  }
  cursor.setWord(Util::getBoolByte(numbersValue));
  redrawScreen();
  return true;
}

void InteractiveMode::engageInsertCharMode() {
  if (cursor.getAddressSpace() == DATA) {
    insertChar = true;
  }
}

void InteractiveMode::engageInsertNumberMode() {
  if (cursor.getAddressSpace() == DATA) {
    insertNumber = true;
  }
}

void InteractiveMode::switchDrawing() {
  if (*selectedView == VIEW_3D) {
    selectedView = &VIEW_3D_B;
  } else if (*selectedView == VIEW_3D_B) {
    selectedView = &VIEW_2D;
  } else {
    selectedView = &VIEW_3D;
  }
  prepareOutput();
  clearScreen();
  redrawScreen();
}

////////////
/// SAVE ///
////////////

void InteractiveMode::saveRamToNewFile() {
  string fileName = getFreeFileName();
  saveRamToFile(fileName);
  loadedFilename = fileName;
}

void InteractiveMode::saveRamToCurrentFile() {
  string fileName;
  if (loadedFilename == "") {
    fileName = getFreeFileName();
  } else {
    fileName = loadedFilename;
  }
  saveRamToFile(fileName);
}

string InteractiveMode::getFreeFileName() {
  int i = 0;
  while (Util::fileExists(SAVE_FILE_NAME + to_string(++i)));
  return SAVE_FILE_NAME + to_string(i);
}

void InteractiveMode::saveRamToFile(string fileName) {
  ofstream fileStream(fileName);
  fileStream << ram.getString();
  fileStream.close();
}

//////////////////
/// KEY READER ///
//////////////////

char InteractiveMode::readStdin(bool drawCursor) {
  char c = 0;
  errno = 0;
  ssize_t num = read(0, &c, 1);
  if (num == -1 && errno == EINTR) {
    // Exits if ctrl-c was pressed.
    if (pleaseExit) {
      exit(0);
    }
    redrawScreen();
    return readStdin(drawCursor);
  }
  return c;
}

