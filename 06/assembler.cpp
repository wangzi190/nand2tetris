#include <bitset>
#include <string>
#include <iostream>
#include <fstream>
#include <unordered_map>
using namespace std;

string noSpacesOrComments(string); // removes spaces and comments from program line
bool numerical(string); // checks whether every character in a string is a digit 0-9

int main (int argc, char** argv) {
  string instr;
  
  string asmFileName = argv[1]; // gets .asm file name from command line argument 1
  ifstream asmFile (asmFileName);

  string hackFileName = argv[1]; // generates corresponding .hack file
  if (hackFileName.length() > 4) {
    hackFileName = hackFileName.substr(0, hackFileName.length() - 4) + ".hack";
  }
  ofstream hackFile (hackFileName);

  if (asmFile.is_open() && hackFile.is_open()) {
    int line = 0;

    unordered_map<string, int> symbols;

    while (getline(asmFile, instr)) { // first pass (handles symbols and generates no code)
      instr = noSpacesOrComments(instr);
      if (instr.length() > 0) {
        if (instr[0] == '(' && instr[instr.length() - 1] == ')') {
          symbols[instr.substr(1, instr.length() - 2)] = line;
        } else {
          line++;
        }
      }
    }

    unordered_map<string, int> predefined = {
      {"SP", 0}, {"LCL", 1}, {"ARG", 2}, {"THIS", 3}, {"THAT", 4},
      {"R0", 0}, {"R1", 1}, {"R2", 2}, {"R3", 3}, {"R4", 4},
      {"R5", 5}, {"R6", 6}, {"R7", 7}, {"R8", 8}, {"R9", 9},
      {"R10", 10}, {"R11", 11}, {"R12", 12}, {"R13", 13}, {"R14", 14}, {"R15", 15},
      {"SCREEN", 16384}, {"KBD", 24576}
    };
    unordered_map<string, string> compBin = {
      {"0", "0101010"}, {"1", "0111111"}, {"-1", "0111010"}, {"D", "0001100"},
      {"A", "0110000"}, {"!D", "0001101"}, {"!A", "0110001"}, {"-D", "0001111"},
      {"-A", "0110011"}, {"D+1", "0011111"}, {"A+1", "0110111"}, {"D-1", "0001110"},
      {"A-1", "0110010"}, {"D+A", "0000010"}, {"D-A", "0010011"}, {"A-D", "0000111"},
      {"D&A", "0000000"}, {"D|A", "0010101"}, {"M", "1110000"}, {"!M", "1110001"},
      {"-M", "1110011"}, {"M+1", "1110111"}, {"M-1", "1110010"}, {"D+M", "1000010"},
      {"D-M", "1010011"}, {"M-D", "1000111"}, {"D&M", "1000000"}, {"D|M", "1010101"}
    };
    unordered_map<string, string> destBin = {
      {"null", "000"}, {"M", "001"}, {"D", "010"}, {"MD", "011"},
      {"A", "100"}, {"AM", "101"}, {"AD", "110"}, {"AMD", "111"}
    };
    unordered_map<string, string> jumpBin = {
      {"null", "000"}, {"JGT", "001"}, {"JEQ", "010"}, {"JGE", "011"},
      {"JLT", "100"}, {"JNE", "101"}, {"JLE", "110"},{"JMP", "111"}
    };
    
    asmFile.clear();
    asmFile.seekg(0, asmFile.beg);
    line = 0;

    int varAddr = 16;
    unordered_map<string, int> variables;

    while (getline(asmFile, instr)) { // second pass (generates code and handles variables)
      string machCode = "";
      bool valid = true;
      bool notPseudo = true;

      instr = noSpacesOrComments(instr);
      if (instr.length() > 0) { // if the line is not blank
        if (instr[0] == '@') {
          // A instruction --> bin
          string instrField = instr.substr(1, instr.length() - 1);
          if (numerical(instrField)) {
            int val = stoi(instrField);
            if (val >= 0 && val <= 32767) {
              bitset<15> bin(val); machCode = "0" + bin.to_string();
            } else {
              valid = false;
              cout << "[Line " << line << "]" << " Cannot load A register with this value." << '\n';
            }
          } else {
            unordered_map<string, int>::const_iterator get = predefined.find(instrField);
            if ( get != predefined.end() ) { // if non-numerical value is predefined
              bitset<15> bin(get->second); machCode = "0" + bin.to_string();
            } else {
              get = symbols.find(instrField);
              if ( get != symbols.end() ) { // if non-numerical value is in symbols table
                bitset<15> bin(get->second); machCode = "0" + bin.to_string();
              } else { // if non-numerical value is a preexisting variable
                get = variables.find(instrField);
                if ( get != variables.end() ) {
                  bitset<15> bin(get->second); machCode = "0" + bin.to_string();
                } else { // otherwise, add a key-value pair into the variables table
                  variables[instrField] = varAddr;
                  bitset<15> bin(varAddr); machCode = "0" + bin.to_string();
                  varAddr++;
                }
              }
            }
          }
        } else {
          if ( !(instr[0] == '(' && instr[instr.length() - 1] == ')') ) {
            // C instruction --> bin

            string dest = "null"; // initializing instruction fields
            string comp = "";
            string jump = "null";

            // identifying and splitting into fields
            int indEqSign = instr.find("=");
            int indSmColn = instr.find(";");
            if (indEqSign != -1 && indSmColn != -1) {
              dest = instr.substr(0, indEqSign);
              comp = instr.substr(indEqSign + 1, indSmColn - indEqSign - 1);
              jump = instr.substr(indSmColn + 1, instr.length() - indSmColn);
            } else if (indEqSign != -1) {
              dest = instr.substr(0, indEqSign);
              comp = instr.substr(indEqSign + 1, instr.length() - indEqSign);
            } else if (indSmColn != -1) {
              comp = instr.substr(0, indSmColn);
              jump = instr.substr(indSmColn + 1, instr.length() - indSmColn);
            } else {
              valid = false;
              cout << "[Line " << line << "]" << " C instruction missing equal sign and semicolon." << '\n';
            }

            // translate each component into binary
            unordered_map<string, string>::const_iterator getComp = compBin.find(comp);
            unordered_map<string, string>::const_iterator getDest = destBin.find(dest);
            unordered_map<string, string>::const_iterator getJump = jumpBin.find(jump);
            if ( getComp != compBin.end() && getDest != destBin.end() && getJump != jumpBin.end()) {
              machCode = "111" + getComp->second + getDest->second + getJump->second;
            } else {
              valid = false;
              cout << "[Line " << line << "]" << " C instruction contains invalid dest, comp, or jump field." << '\n';
            }
          } else {
            notPseudo = false;
          }
        }

        if (valid && notPseudo) {
          hackFile << machCode << '\n';
        }
        if (notPseudo) {
          line++;
        }
      }
    }
    asmFile.close();
    hackFile.close();
  } else {
    cout << "Unable to open file(s).";
  }
  return 0;
}

string noSpacesOrComments(string instr) {
  int i = 0;
  while (i < instr.length()) {
    if (instr[i] == ' ') {
      instr.erase(i, 1);
    } else if (instr[i] == '/' && instr[i+1] == '/') {
      instr.erase(i, instr.length() - i);
    } else {
      i++;
    }
  }
  return instr;
}

bool numerical(string str) {
  bool numerical = true;
  int i = 0;
  while (i < str.length() && numerical) {
    if (!isdigit(str[i])) {
      numerical = false;
    }
    i++;
  }
  return numerical;
}