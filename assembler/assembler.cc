#include <iostream>
#include <string>
#include <vector>
#include "scanner.h"
#include <map>

using namespace std;

const int memUpperBound = 32767; // 2^5 - 1
const int memLowerBound = -32768; // 2^5 * -1

struct customError {
  public:
    string ErrorMessage;
};

bool customAssert(bool conditionToCheck, string ErrorMessage) {
    if (conditionToCheck) {
        return true;
    } else {
      throw customError{ErrorMessage};
      return false;
    }
};


void outputBinary(int instr) {
  unsigned char c = instr >> 24;
  cout << c;
  c = instr >> 16;
  cout << c;
  c = instr >> 8;
  cout << c;
  c = instr;
  cout << c;
}

// checks that the registers are between $0 and $31
bool checkRegisters(vector<Token> registers) {
  for (size_t i = 0; i < registers.size(); i++) {
    Token curRegister = registers[i];
    if (curRegister.getKind() != Token::REG) {
      return false;
    }
    if (curRegister.toNumber() < 0 || curRegister.toNumber() > 31) {
      return false;
    }
  }
  return true;
}

// returns false if there are too many args in the instruction
bool checkExtraArgs(vector<Token> tokenLine, size_t shouldBeLast) {
  if (shouldBeLast == tokenLine.size() - 1) {
    return true;
  } else {
    return false;
  }
}

// for beq/bne, lw and sw
bool checkBoundsOnI(Token iValueToCheck) {
  if (iValueToCheck.getKind() == Token::INT) {
    if (iValueToCheck.toNumber() >= memLowerBound && iValueToCheck.toNumber() <= memUpperBound) {
      return true;
    }
  } else if (iValueToCheck.getKind() == Token::HEXINT) {
    if (iValueToCheck.toNumber() <= 65535 && iValueToCheck.toNumber() >= -65535) {
      return true;
    }
  }
  return false;
}

// check the correctness of the order of instructions
bool commandCorrectness(string opcode, vector<Token> tokenLine) {

  bool result = false;

  if (opcode == ".word") { // ----------------------------------------------------------------------------------------
    if (tokenLine[0].getLexeme() == opcode) {
      if (tokenLine[1].getKind() == Token::INT) {
        if (tokenLine[1].toNumber() > (4294967296 - 1) || tokenLine[1].toNumber() < (-1 * 2147483648)) { // if number is > 2^32-1 OR < -2^31
          result = false;
        } else {
          result = true;
        }
      } else if (tokenLine[1].getKind() == Token::HEXINT) {
        if (tokenLine[1].toNumber() > 4294967295) { // if hexint is > max
          result = false;
        } else {
          result = true;
        }
      } else if (tokenLine[1].getKind() == Token::ID) {
        result = true; // but we still have to check if this is a valid label or not (if it exists) -> do it in second parse
      }
      // if no condition is met, then .word is not using int, hexint or label so result stays false
      
      // check that there aren't extra args
      if (result) {
        result = checkExtraArgs(tokenLine, 1);
      }
    }
  } else if (opcode == "add" || opcode == "sub" || opcode == "slt" || opcode == "sltu") { // -------------------------
    // ID, REG, COMMA, REG, COMMA, REG
    if (tokenLine[0].getLexeme() == opcode && tokenLine[1].getKind() == Token::REG && 
      tokenLine[2].getKind() == Token::COMMA && tokenLine[3].getKind() == Token::REG &&
      tokenLine[4].getKind() == Token::COMMA && tokenLine[5].getKind() == Token::REG) {
      result = true;
    }
    // if the formatting of instruction is correct: check that the registers are within $0 and $31
    if (result) {
      result = checkRegisters({tokenLine[1], tokenLine[3], tokenLine[5]});
    }

    // check that there aren't extra args
    if (result) {
      result = checkExtraArgs(tokenLine, 5);
    }


  } else if (opcode == "mult" || opcode == "multu" || opcode == "div" || opcode == "divu") { // -------------------------
    // ID, REG, COMMA, REG
    if (tokenLine[0].getLexeme() == opcode && tokenLine[1].getKind() == Token::REG && 
      tokenLine[2].getKind() == Token::COMMA && tokenLine[3].getKind() == Token::REG) {
      result = true;
    }
    // if the formatting of instruction is correct: check that the registers are within $0 and $31
    if (result) {
      result = checkRegisters({tokenLine[1], tokenLine[3]});
    }

    // check that there aren't extra args
    if (result) {
      result = checkExtraArgs(tokenLine, 3);
    }
  
  } else if (opcode == "mfhi" || opcode == "mflo" || opcode == "lis" ) { // ---------------------------------------------
    // ID, REG
    if (tokenLine[0].getLexeme() == opcode && tokenLine[1].getKind() == Token::REG) {
      result = true;
    }
    // if the formatting of instruction is correct: check that the registers are within $0 and $31
    if (result) {
      result = checkRegisters({tokenLine[1]});
    }

    // check that there aren't extra args
    if (result) {
      result = checkExtraArgs(tokenLine, 1);
    }

  } else if (opcode == "lw" || opcode == "sw") { // ---------------------------------------------------------------------
    // ID, REG, COMMA, INT or HEXINT, LPAREN, REG, RPAREN
     if (tokenLine[0].getLexeme() == opcode && tokenLine[1].getKind() == Token::REG && 
      tokenLine[2].getKind() == Token::COMMA && (tokenLine[3].getKind() == Token::INT || tokenLine[3].getKind() == Token::HEXINT) &&
      tokenLine[4].getKind() == Token::LPAREN && tokenLine[5].getKind() == Token::REG && tokenLine[6].getKind() == Token::RPAREN) {
      result = true;
    }
    // if the formatting of instruction is correct: check that the registers are within $0 and $31
    if (result) {
      result = checkRegisters({tokenLine[1], tokenLine[5]});
    }
    // check that there aren't extra args
    if (result) {
      result = checkExtraArgs(tokenLine, 6);
    }
    if (result) {
      if (tokenLine[3].getKind() != Token::ID) result = checkBoundsOnI(tokenLine[3]);
    }

  } else if (opcode == "beq" || opcode == "bne") { // ---------------------------------------------------------------------------
    // ID, REG, COMMA, REG, COMMA, INT or HEXINT or ID
    if (tokenLine[0].getLexeme() == opcode && tokenLine[1].getKind() == Token::REG && 
      tokenLine[2].getKind() == Token::COMMA && tokenLine[3].getKind() == Token::REG &&
      tokenLine[4].getKind() == Token::COMMA && 
      (tokenLine[5].getKind() == Token::INT || tokenLine[5].getKind() == Token::HEXINT || tokenLine[5].getKind() == Token::ID)) {
      result = true;
    }
    // if the formatting of instruction is correct: check that the registers are within $0 and $31
    if (result) {
      result = checkRegisters({tokenLine[1], tokenLine[3]});
    }
    // check that there aren't extra args
    if (result) {
      result = checkExtraArgs(tokenLine, 5);
    }
    // check that the immediate is within bounds
    if (result) {
      if (tokenLine[5].getKind() != Token::ID) result = checkBoundsOnI(tokenLine[5]);
    }

  } else if (opcode == "jr" || opcode == "jalr") { // ---------------------------------------------------------------------------
    // ID, REG
    if (tokenLine[0].getLexeme() == opcode && tokenLine[1].getKind() == Token::REG) {
      result = true;
    }
    // if the formatting of instruction is correct: check that the registers are within $0 and $31
    if (result) {
      result = checkRegisters({tokenLine[1]});
    }
    // check that there aren't extra args
    if (result) {
      result = checkExtraArgs(tokenLine, 1);
    }
  }

  return result;
}


int main() {
  string line;
  vector<vector<Token>> instructions {};
  map<string, int> symbolTable {}; // {{string: label, int: address}}
  int errorLine = 0;
  int addressCounter = 0;
  try {

    // FIRST PARSE ===============================================================================================
    while (getline(cin, line)) {
      errorLine += 1;
      vector<Token> tokenLine;
      try {
        tokenLine = scan(line);
      } catch (ScanningFailure &f) {
        cerr << "ERROR " << f.what() << endl;
        return 1;
      }

      if (tokenLine.size() <= 0) { // skip rest of loop if line empty
        continue;
      }

      Token firstToken = tokenLine[0];
      
      if (firstToken.getKind() == Token::WORD) { // if first token is a WORD ----------------------------
        customAssert(commandCorrectness(".word", tokenLine), "word formatting error. Line: " + to_string(errorLine));
        instructions.push_back(tokenLine);
        addressCounter += 1;

      } else if (firstToken.getKind() == Token::ID) { // if first token is an ID ----------------------------
        string opCode = firstToken.getLexeme();
        
        if (opCode == "add" || opCode == "sub" || opCode == "slt" || opCode == "sltu" || 
            opCode == "mult" || opCode == "multu" || opCode == "div" || opCode == "divu" || 
            opCode == "mfhi" || opCode == "mflo" || opCode == "lis" ||
            opCode == "lw" || opCode == "sw" ||
            opCode == "beq" || opCode == "bne" ||
            opCode == "jr" || opCode == "jalr"){
          customAssert(commandCorrectness(opCode, tokenLine), "formatting error: Line " + to_string(errorLine));
          instructions.push_back(tokenLine);
          addressCounter += 1;
        // } else if (opCode == "beq") {
        //   customAssert(commandCorrectness("beq", tokenLine), "beq formatting error: Line " + to_string(errorLine));
        //   instructions.push_back(tokenLine);
        //   addressCounter+=1;
        } else {
          throw customError{"invalid instruction"};
        }


      } else if (firstToken.getKind() == Token::LABEL) { // if line starts with a LABEL ------------------------
        
        // loop thru this line -> check if there's an instruction at the end
        int labelCount = 0;
        bool instructionPresent = false;
        
        for (size_t i = 0; i < tokenLine.size(); i++) {

          if (tokenLine[i].getKind() != Token::LABEL) { // if there is an instruction

            instructionPresent = true;
            
          } else if (tokenLine[i].getKind() == Token::LABEL) { // this ignores labels in .word and beq/bne
            string label = tokenLine[i].getLexeme();
            label.pop_back();
            customAssert(symbolTable.count(label) == 0, "label has already been defined: " + label);
            labelCount+=1;
            symbolTable.insert({label, addressCounter});
          }
        }
        if (instructionPresent) { // line with NO instruction at end
          for (int i = 0; i < labelCount; i++) {
            tokenLine.erase(tokenLine.begin());
          }
          customAssert(commandCorrectness(tokenLine[0].getLexeme(), tokenLine), "instruction after label formatting error: Line " + to_string(errorLine));
          instructions.push_back(tokenLine);
          addressCounter+=1;
        }

      } else {
        throw customError{"undefined behaviour"};
      }

    }

    // SECOND PARSE ===============================================================================================

    for (size_t i = 0; i < instructions.size(); i++) {
      vector<Token> tokenLine = (instructions[i]);

      if (tokenLine[0].getKind() == Token::WORD) { // -------------------------- WORD --------------------------

        int num = 0;
        if (tokenLine[1].getKind() == Token::INT || tokenLine[1].getKind() == Token::HEXINT) {
          num = tokenLine[1].toNumber();

        // if it is a label -> set value to the address of that label
        } else if (tokenLine[1].getKind() == Token::ID) {
          customAssert(symbolTable.count(tokenLine[1].getLexeme()), "label used hasn't been defined: " + tokenLine[1].getLexeme());
          num = symbolTable[tokenLine[1].getLexeme()] * 4; // use address of label by searching through map with key
        }
        outputBinary(num);


      } else if (tokenLine[0].getKind() == Token::ID) { // ID (add, sub, beq...) ---------------------------------------

        if (tokenLine[0].getLexeme() == "add") { // ---------------------------- add/sub/slt/sltu -----------------------------
          int64_t d = tokenLine[1].toNumber(), s = tokenLine[3].toNumber(), t = tokenLine[5].toNumber(); //separated by commas
          int instr = (0 << 26) | (s << 21) | (t << 16) | (d << 11) | 32;
          outputBinary(instr);
        } else if (tokenLine[0].getLexeme() == "sub") {
          int64_t d = tokenLine[1].toNumber(), s = tokenLine[3].toNumber(), t = tokenLine[5].toNumber();
          int instr = (0 << 26) | (s << 21) | (t << 16) | (d << 11) | 34;
          outputBinary(instr);
        } else if (tokenLine[0].getLexeme() == "slt") {
          int64_t d = tokenLine[1].toNumber(), s = tokenLine[3].toNumber(), t = tokenLine[5].toNumber();
          int instr = (0 << 26) | (s << 21) | (t << 16) | (d << 11) | 42;
          outputBinary(instr);
        } else if (tokenLine[0].getLexeme() == "sltu") {
          int64_t d = tokenLine[1].toNumber(), s = tokenLine[3].toNumber(), t = tokenLine[5].toNumber();
          int instr = (0 << 26) | (s << 21) | (t << 16) | (d << 11) | 43;
          outputBinary(instr);

        } else if (tokenLine[0].getLexeme() == "mult") { // ----------------------- mult, multu, div, divu ---------------------------
          int64_t s = tokenLine[1].toNumber(), t = tokenLine[3].toNumber();
          int instr = (0 << 26) | (s << 21) | (t << 16) | 24;
          outputBinary(instr);
        } else if (tokenLine[0].getLexeme() == "multu") {
          int64_t s = tokenLine[1].toNumber(), t = tokenLine[3].toNumber();
          int instr = (0 << 26) | (s << 21) | (t << 16) | 25;
          outputBinary(instr);
        } else if (tokenLine[0].getLexeme() == "div") {
          int64_t s = tokenLine[1].toNumber(), t = tokenLine[3].toNumber();
          int instr = (0 << 26) | (s << 21) | (t << 16) | 26;
          outputBinary(instr);
        } else if (tokenLine[0].getLexeme() == "divu") {
          int64_t s = tokenLine[1].toNumber(), t = tokenLine[3].toNumber();
          int instr = (0 << 26) | (s << 21) | (t << 16) | 27;
          outputBinary(instr);

        } else if (tokenLine[0].getLexeme() == "mfhi") { // -------------------------- mfhi, mflo, lis -----------------------------
          int64_t d = tokenLine[1].toNumber();
          int instr = (0 << 16) | (d << 11) | 16;
          outputBinary(instr);
        } else if (tokenLine[0].getLexeme() == "mflo") {
          int64_t d = tokenLine[1].toNumber();
          int instr = (0 << 16) | (d << 11) | 18;
          outputBinary(instr);
        } else if (tokenLine[0].getLexeme() == "lis") {
          int64_t d = tokenLine[1].toNumber();
          int instr = (0 << 16) | (d << 11) | 20;
          outputBinary(instr);


        } else if (tokenLine[0].getLexeme() == "lw") { // ------------------------------ lw, sw --------------------------------
          int64_t t = tokenLine[1].toNumber(), i = tokenLine[3].toNumber(), s = tokenLine[5].toNumber();
          int instr = (35 << 26) | (s << 21) | (t << 16) | (i & 0xFFFF);
          outputBinary(instr);
        } else if (tokenLine[0].getLexeme() == "sw") {
          int64_t t = tokenLine[1].toNumber(), i = tokenLine[3].toNumber(), s = tokenLine[5].toNumber();
          int instr = (43 << 26) | (s << 21) | (t << 16) | (i & 0xFFFF);
          outputBinary(instr);
        

        } else if (tokenLine[0].getLexeme() == "beq" || tokenLine[0].getLexeme() == "bne") { // ----------------------- beq, bne ---------------------------
          // assert the next 3 exist -> are registers
          int64_t s = tokenLine[1].toNumber();
          int64_t t = tokenLine[3].toNumber(); //separated by commas
          int64_t ii = tokenLine[5].toNumber(); 

          // if i is a label: set i to the value of lines to go forward/back to get to that label
          if (tokenLine[5].getKind() == Token::ID){
            // search for label in map
            customAssert(symbolTable.count(tokenLine[5].getLexeme()), "label used hasn't been defined: " + tokenLine[5].getLexeme());
            ii = symbolTable[tokenLine[5].getLexeme()] - (i+1); // use address of label by searching through map with key
            // assert that ii is not out of range
            customAssert(ii <= memUpperBound && ii >= memLowerBound, "label exceeded min or max range: " + tokenLine[5].getLexeme());
          }

          if (tokenLine[0].getLexeme() == "beq") {
            int instr = (4 << 26) | (s << 21) | (t << 16) | (ii & 0xFFFF);
            outputBinary(instr);
          } else if (tokenLine[0].getLexeme() == "bne"){
            int instr = (5 << 26) | (s << 21) | (t << 16) | (ii & 0xFFFF);
            outputBinary(instr);
          }

        } else if (tokenLine[0].getLexeme() == "jr") { // ------------------------------ jr, jalr --------------------------------
          int64_t s = tokenLine[1].toNumber();
          int instr = (0 << 26) | (s << 21) | (0 << 16) | (0 << 11) | 8;
          outputBinary(instr);
        } else if (tokenLine[0].getLexeme() == "jalr") {
          int64_t s = tokenLine[1].toNumber();
          int instr = (0 << 26) | (s << 21) | (0 << 16) | (0 << 11) | 9;
          outputBinary(instr);
        }

      }

    }

    // ===========================================================================================================



  } catch (customError e) {
    // cerr << f.what() << endl;
    cerr << "ERROR " << e.ErrorMessage << endl;
    return 1;
  }
  // You can add your own catch clause(s) for other kinds of errors.
  // Throwing exceptions and catching them is the recommended way to
  // handle errors and terminate the program cleanly in C++. Do not
  // use the std::exit function, which may leak memory.
  
  // for (auto const &pair: symbolTable) {
  //     std::cout << "{" << pair.first << ": " << pair.second << "}\n";
  // }

  return 0;
}

