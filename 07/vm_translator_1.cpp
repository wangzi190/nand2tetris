#include <iostream>
#include <fstream>
#include <unordered_map>
using namespace std;

string clean(string); // removes comments and unnecessary spaces from program line
string getIneqAsm(string, string); // returns the asm for eq, lt, or gt

string incrSP = "@SP\nM=M+1\n";
string decrSP = "@SP\nM=M-1\n";
string dRegAtSP = "@SP\nA=M\nM=D\n";

string bin1 = decrSP + "A=M\nD=M\n" + decrSP + "A=M\nA=M\n";
string bin2 = dRegAtSP + incrSP;
string un1 = decrSP + "A=M\n";
string un2 = dRegAtSP + incrSP;
unordered_map<string, string> arithAndLog = {
  {"add", bin1+"D=D+A\n"+bin2}, {"sub", bin1+"D=A-D\n"+bin2}, {"neg", un1+"D=-M\n"+un2},
  {"and", bin1+"D=D&A\n"+bin2}, {"or", bin1+"D=D|A\n"+bin2}, {"not", un1+"D=!M\n"+un2}
};
unordered_map<string, string> ineq = { // inequalities
  {"eq", "D;JEQ\n"}, {"gt", "D;JGT\n"}, {"lt", "D;JLT\n"}
};

int main(int argc, char** argv) {
  string vmFileName = argv[1];
  ifstream vmFile (vmFileName);
  if (vmFileName.length() > 3) {
    vmFileName = vmFileName.substr(0, vmFileName.length() - 3);
  }
  ofstream asmFile (vmFileName + ".asm");

  int ineqOpCtr = 0;
  string ineqOpStr = to_string(ineqOpCtr);
  
  if (vmFile.is_open() && asmFile.is_open()) {
    string instr;
    while (getline(vmFile, instr)) {
      instr = clean(instr);
      if (instr.length() > 0) {
        asmFile << "// " + instr + "\n";
        int indSpace = instr.find(' ', 0);
        string arg1 = instr;
        if (indSpace == -1) {
          unordered_map<string, string>::const_iterator get = arithAndLog.find(instr);
          if (get != arithAndLog.end()) {
            asmFile << get->second;
          } else {
            get = ineq.find(instr);
            if (get != ineq.end()) {
              string toCout = getIneqAsm(instr, ineqOpStr);
              asmFile << toCout;
              ineqOpCtr++;
              ineqOpStr = to_string(ineqOpCtr);
            }
          }
        } else {
          arg1 = instr.substr(0, indSpace);
          int indOtherSpace = instr.find(' ', indSpace + 1);
          string arg2 = instr.substr(indSpace + 1, indOtherSpace - indSpace - 1);
          indSpace = instr.find(' ', indSpace + 1);
          string arg3 = instr.substr(indOtherSpace + 1, instr.length() - indOtherSpace);
          // ^ assuming there are 3 arguments if # of args > 1
          
          bool isLocal = arg2.compare("local") == 0;
          bool isArg = arg2.compare("argument") == 0;
          bool isThis = arg2.compare("this") == 0;
          bool isThat = arg2.compare("that") == 0;
          bool isTemp = arg2.compare("temp") == 0;
          bool isPointer = arg2.compare("pointer") == 0;
          bool isStatic = arg2.compare("static") == 0;
          bool alsoValidArg2 = isLocal || isArg || isThis || isThat || isTemp || isPointer || isStatic;
          
          if (arg1.compare("push") == 0) {
            if (arg2.compare("constant") == 0) {
              asmFile << "@" + arg3 + "\nD=A\n" + dRegAtSP + incrSP;
            } else if (alsoValidArg2) {
              if (isLocal) {
                asmFile << "@LCL\nD=M\n";
              } else if (isArg) {
                asmFile << "@ARG\nD=M\n";
              } else if (isThis) {
                asmFile << "@THIS\nD=M\n";
              } else if (isThat) {
                asmFile << "@THAT\nD=M\n";
              } else if (isTemp) {
                asmFile << "@5\nD=A\n";
              } else if (isPointer) {
                asmFile << "@3\nD=A\n";
              } else { // isStatic
                asmFile << "@" + vmFileName + "." + arg3 + "\nD=M\n";
              }
              asmFile << "@" + arg3 + "\nD=D+A\nA=D\nD=M\n" + dRegAtSP + incrSP;
            }
          } else if (arg1.compare("pop") == 0) {
            if (alsoValidArg2) {
              asmFile << decrSP + "A=M\nD=M\n";
              if (isLocal) {
                asmFile << "@LCL\nA=M\n";
              } else if (isArg) {
                asmFile << "@ARG\nA=M\n";
              } else if (isThis) {
                asmFile << "@THIS\nA=M\n";
              } else if (isThat) {
                asmFile << "@THAT\nA=M\n";
              } else if (isTemp) {
                asmFile << "@5\n";
              } else if (isPointer) {
                asmFile << "@3\n";
              } else { // isStatic
                asmFile << "@" + vmFileName + "." + arg3 + "\nA=M\n";
              }
              for (int i = 0; i < stoi(arg3); i++) {
                asmFile << "A=A+1\n";
              }
              asmFile << "M=D\n";
            }
          }
        }
      }
    }
    vmFile.close();
    asmFile.close();
  } else {
    cout << "Unable to open file(s).";
  }
  return 0;
}

string getIneqAsm(string instr, string ineqOpStr) {
  string ineq1 = bin1 + "D=A-D\n" + "@CONT_" + ineqOpStr + "\n0;JMP\n(TRUE_" + ineqOpStr + ")\n@1\nD=-A\n" + dRegAtSP + "@FIN_" + ineqOpStr + "\n0;JMP\n(CONT_" + ineqOpStr + ")\n@TRUE_" + ineqOpStr + "\n";
  string ineq2 = "@0\nD=A\n" + dRegAtSP + "(FIN_" + ineqOpStr + ")\n" + incrSP;
  return ineq1 + ineq.at(instr) + ineq2;
}

string clean(string instr) {
  int i = 0;
  while (i < instr.length()) {
    if (instr[i] == ' ' && instr[i+1] == ' ') {
      instr.erase(i, 1);
    } else if (instr[i] == '/' && instr[i+1] == '/') {
      instr.erase(i, instr.length() - i);
    } else {
      i++;
    }
  }
  return instr;
}