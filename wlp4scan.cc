#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <fstream>

const std::string ALPHABET    = ".ALPHABET";
const std::string STATES      = ".STATES";
const std::string TRANSITIONS = ".TRANSITIONS";
const std::string INPUT       = ".INPUT";
const std::string EMPTY       = ".EMPTY";

bool isChar(std::string s) {
  return s.length() == 1;
}
bool isRange(std::string s) { // to check if it's of the form: a-z
  return s.length() == 3 && s[1] == '-';
}

// EX: {accepting, {{l: "q1"}, {f: "q2"}, {a: ""}}}
struct TableObject {
  bool accepting;
  std::map<char, std::string> transitions;
};

// EX {{stateName, tableObject}, {stateName2, tableObject2}}
std::map<std::string, TableObject> transitionTable;
std::map<char, std::string> alphabet;
std::string startingState;

std::string findNextState(std::string fromState, char c) {
  return transitionTable[fromState].transitions[c];
}

std::string outputState(std::string s, std::string t) {
  
  if (s == "id") { 
    if (t == "int") { return "INT"; } 
    else if (t == "wain") { return "WAIN"; } 
    else if (t == "if") { return "IF"; } 
    else if (t == "else") { return "ELSE"; } 
    else if (t == "while") { return "WHILE"; } 
    else if (t == "println") { return "PRINTLN"; } 
    else if (t == "return") { return "RETURN"; } 
    else if (t == "NULL") { return "NULL"; } 
    else if (t == "new") { return "NEW"; } 
    else if (t == "delete") { return "DELETE"; } 
    else { return "ID"; }
  }
  else if (s == "zero") { return "NUM"; }
  else if (s == "num") { 
    long num;
    std::stringstream ss;
    ss << t;
    ss >> num;
    if (num >= 2147483648) {
      return "ERROR";
    }
    return "NUM"; 
  }
  else if (s == "lparen") { return "LPAREN"; }
  else if (s == "rparen") { return "RPAREN"; }
  else if (s == "lbrace") { return "LBRACE"; }
  else if (s == "rbrace") { return "RBRACE"; }
  else if (s == "equals") { return "BECOMES"; }
  else if (s == "equalsequals") { return "EQ"; }
  else if (s == "NE") { return "NE"; }
  else if (s == "LT") { return "LT"; }
  else if (s == "GT") { return "GT"; }
  else if (s == "LE") { return "LE"; }
  else if (s == "GE") { return "GE"; }
  else if (s == "plus") { return "PLUS"; }
  else if (s == "minus") { return "MINUS"; }
  else if (s == "star") { return "STAR"; }
  else if (s == "slash") { return "SLASH"; }
  else if (s == "percent") { return "PCT"; }
  else if (s == "comma") { return "COMMA"; }
  else if (s == "semi") { return "SEMI"; }
  else if (s == "lsquare") { return "LBRACK"; }
  else if (s == "rsquare") { return "RBRACK"; }
  else if (s == "amp") { return "AMP"; }
  else { return s; }
}

int main() {
  std::istream& in = std::cin;
  std::string s;
  std::ifstream dfaIn("wlp4.dfa");

  std::getline(dfaIn, s); // Alphabet section (skip header)
  // Read characters or ranges separated by whitespace
  while(dfaIn >> s) {
    if (s == STATES) { 
      break; 
    } else {
      if (isChar(s)) {
        //// Variable 's[0]' is an alphabet symbol
        alphabet.insert({s[0], ""});
      } else if (isRange(s)) {
        for(char c = s[0]; c <= s[2]; ++c) {
          //// Variable 'c' is an alphabet symbol
          alphabet.insert({c, ""});
        }
      } 
    }
  }

  std::getline(dfaIn, s); // States section (skip header)
  // Read states separated by whitespace
  while(dfaIn >> s) {
    if (s == TRANSITIONS) { 
      break; 
    } else {
      static bool initial = true;
      bool accepting = false;
      if (s.back() == '!' && !isChar(s)) {
        accepting = true;
        s.pop_back();
      }
      //// Variable 's' contains the name of a state
      if (initial) {
        //// The state is initial
        initial = false;
        startingState = s;
      }
      if (accepting) {
        //// The state is accepting
        TableObject t = {true, alphabet};
        transitionTable.insert({s, t});
      } else {
        TableObject t = {false, alphabet};
        transitionTable.insert({s, t});
      }

    }
  }

  std::getline(dfaIn, s); // Transitions section (skip header)
  // Read transitions line-by-line
  while(std::getline(dfaIn, s)) {
    if (s == INPUT) { 
      // Note: Since we're reading line by line, once we encounter the
      // input header, we will already be on the line after the header
      break; 
    } else {
      std::string fromState, symbols, toState;
      std::istringstream line(s);
      line >> fromState;
      bool last;
      while(line >> s) {
        if(line.peek() == EOF) { // If we reached the last item on the line
          // Then it's the to-state
          toState = s;
        } else { // Otherwise, there is more stuff on the line
          // We expect a character or range
          if (isChar(s)) {
            symbols += s;
          } else if (isRange(s)) {
            for(char c = s[0]; c <= s[2]; ++c) {
              symbols += c;
            }
          }
        }
      }
      for ( char c : symbols ) {
        //// There is a transition from 'fromState' to 'toState' on 'c'
        transitionTable[fromState].transitions[c] = toState;
      }

    }
  }

  // Input section (already skipped header)
  // TREAT AS A SISNGLE STRING
  std::string inputString;

  while(std::getline(in, s)){

    bool comment = false;
    std::vector<std::string> stringsInLine;
    int lastIndex = 0;
    for (size_t i = 0; i < s.size(); i++) {

      if (s[i] == '/' && i+1 < s.size() && s[i+1] == '/'){
        stringsInLine.push_back(s.substr(lastIndex, i - lastIndex));
        comment = true;
        break;
      }
      if (s[i] == 32 || s[i] == 9 || s[i] == 10) {
        stringsInLine.push_back(s.substr(lastIndex, i - lastIndex));
        lastIndex = i+1;
      }
    }
    if (!comment && s.size() > 0) stringsInLine.push_back(s.substr(lastIndex, s.size() - lastIndex));

    // std::cout << "SSSS: " << inputString << std::endl;

    // for (size_t j = 0; j < stringsInLine.size(); j++) {
    //   std::cout << "SIL: " << stringsInLine[j] << std::endl;
    // }


    for (auto &inputString : stringsInLine) {
      size_t counter = 0;
      std::string state = startingState;
      std::string outputToken = "";

      while (counter < inputString.size()) {

        if (findNextState(state, inputString[counter]) == "") { // no next state exists
          // if (s[0]) is in an accepting state
          if (transitionTable[state].accepting == true) {
            std::string out = outputState(state, outputToken);
            if (out == "ERROR") {
              std::cerr << "ERROR" << std::endl;
              return 0;
            }
            std::cout << out << " " << outputToken << std::endl;
            outputToken = "";
            state = startingState;
          } else {
            std::cerr << "ERROR" << std::endl;
            return 0;
          }
        } else {
          outputToken += inputString[counter];
          state = findNextState(state, inputString[counter]);
          counter++;
        }
      }

      if (inputString.size() > 0) {
        if (transitionTable[state].accepting == true) {
          std::string out = outputState(state, outputToken);
          if (out == "ERROR") {
            std::cerr << "ERROR" << std::endl;
            return 0;
          }
          std::cout << out << " " << outputToken << std::endl;
        } else {
          std::cerr << "ERROR" << std::endl;
          return 0;
        }
      }
    }


  }

}