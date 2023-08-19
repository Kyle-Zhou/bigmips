#include <iostream>
#include <sstream>
#include <string>

#include <map>
#include <vector>


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

// Locations in the program that you should modify to store the
// DFA information have been marked with four-slash comments:
//// (Four-slash comment)

int main() {
  std::istream& in = std::cin;
  std::string s;

  std::getline(in, s); // Alphabet section (skip header)
  // Read characters or ranges separated by whitespace
  while(in >> s) {
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

  std::getline(in, s); // States section (skip header)
  // Read states separated by whitespace
  while(in >> s) {
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

  std::getline(in, s); // Transitions section (skip header)
  // Read transitions line-by-line
  while(std::getline(in, s)) {
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

      // print map:
      // std::cout << std::endl;
      // std::cout << fromState + ", accepting: " << transitionTable[fromState].accepting << std::endl;

      // for (auto const& x : transitionTable[fromState].transitions) {
      //   std::cout << fromState << " " << x.first << " " << x.second << std::endl;
      // }
      // std::cout << std::endl;

    }
  }

  // Input section (already skipped header)
  // TREAT AS A SISNGLE STRING
  std::string state = startingState;
  std::string outputToken = "";
  std::string inputString;
  size_t counter = 0;

  while(std::getline(in, s)){
    inputString = s;
  }

  while (counter < inputString.size()) {
    if (findNextState(state, inputString[counter]) == "") { // no next state exists
      // if (s[0]) is in an accepting state
      if (transitionTable[state].accepting == true) {
        std::cout << outputToken << std::endl;
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
  if (transitionTable[state].accepting == true) {
    std::cout << outputToken << std::endl;
  } else {
    std::cerr << "ERROR" << std::endl;
  }


}