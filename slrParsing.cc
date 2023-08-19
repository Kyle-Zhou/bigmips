#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <stack>
#include <fstream>
#include <algorithm>

const std::string INPUT       = ".INPUT";
const std::string TRANSITIONS = ".TRANSITIONS";
const std::string REDUCTIONS  = ".REDUCTIONS";
const std::string END         = ".END";

struct Production {
  std::string LHS;
  std::vector<std::string> RHS;
};

struct Transition {
  std::string fromState;
  std::string transitionValue;
  std::string toState;
};

struct Reduction {
  std::string currentState;
  int rule;
  std::string lookAhead;
};

void printElement(std::string s) {
  std::cout << s << " ";
}

void print(std::vector<std::string> &reductionSequence, std::vector<std::string> &inputSequence) {
  // need to print reduction sequence, (.), input sequence
  for (size_t i = 0; i < reductionSequence.size(); i++) {
    std::cout << reductionSequence[i] << " ";
  }
  std::cout << ". ";
  for_each(inputSequence.rbegin(),inputSequence.rend(), printElement); // print input sequence in reverse -> since we reversed it earlier
  std::cout << std::endl;

}

int main() {
  std::istream& in = std::cin;
  std::string s;

  // define our data structures: ------------------------------------------------------------------------------------
  // reduction sequence -> vector
  // input sequence -> vector
  std::vector<std::string> reductionSequence;
  std::vector<std::string> inputSequence;

  // efficiency requirements: ---------------------------------------------------------------------------------------
  // shift -> push and pop from both vectors = O(1)
  // reduce n -> look for rule (O(n)) + remove RHS of rule from right end of reduction (pop = O(1)) 
  //                                  + add the LHS to the righty end of reduction (emplace_back) = O(n)
  // print -> iterate through length of the reduction and input sequence (vectors), print at each iteration = O(n)
  // ----------------------------------------------------------------------------------------------------------------

  std::vector<Production> productions;
  std::vector<Transition> transitions;
  std::vector<Reduction> reductions;

  std::stack<std::string> stateStack;

  // .CFG
  while(std::getline(in, s)) {
    if (s == INPUT) { 
      break; 
    } else {
      std::string word;
      std::istringstream line(s);
      line >> word; // skips .CFG
      if (word == ".CFG") continue;
      std::vector<std::string> singleProduction;
      while(line >> s) {        
        if (s != ".EMPTY") { singleProduction.push_back(s); }
      }
      Production p = {word, singleProduction};
      productions.push_back(p);
    }
  }
  //testPrint(productions);

  // .INPUT -> we want to store the entire input in the inputSequence
  while(std::getline(in, s)) {
    if (s == TRANSITIONS) { 
      break; 
    } else {
      std::istringstream line(s);
      while(line >> s) {        
        inputSequence.push_back(s);
        std::rotate(inputSequence.rbegin(), inputSequence.rbegin() + 1, inputSequence.rend());
      }
    }
  }

  // .TRANSITIONS
  while(std::getline(in, s)) {
    if (s == REDUCTIONS) { 
      break; 
    } else {
      std::string fromState, transitionValue, toState;
      std::istringstream line(s);
      line >> fromState, line >> transitionValue, line >> toState;
      Transition t = {fromState, transitionValue, toState};
      transitions.push_back(t);
    }
  }

  // .REDUCTIONS
  while(std::getline(in, s)) {
    if (s == END) { 
      break; 
    } else {
      std::string currentState, lookAhead;
      int rule;
      std::istringstream line(s);
      line >> currentState, line >> rule, line >> lookAhead;
      Reduction r = {currentState, rule, lookAhead};
      reductions.push_back(r);
    }
  }

  bool accepted = false;
  bool reductionMade = false;
  int counter = 0;
  if (transitions.size() >= 1) stateStack.push(transitions[0].fromState);
  print(reductionSequence, inputSequence);
  
  // PARSING LOOP
  while(!accepted) {
    
    bool transitionFound = false;
    // check if there is a transition that can be made on the current terminal
    for (size_t i = 0; i < transitions.size(); i++) {

      // special case: if previous iteration made a reduction, we look at the top of the reduction sequence
      if (reductionMade) {
        if (transitions[i].fromState == stateStack.top() && transitions[i].transitionValue == reductionSequence.back()) {
          stateStack.push(transitions[i].toState);
          transitionFound = true;
          reductionMade = false;
          break;
        }
      } else {
        if (transitions[i].fromState == stateStack.top() && transitions[i].transitionValue == inputSequence.back()) {
          // shift
          std::string shifted = inputSequence.back(); // pops from the back but technically its the left end b/c we reversed the vector
          reductionSequence.push_back(shifted);
          inputSequence.pop_back();

          // add to stateStack
          stateStack.push(transitions[i].toState); 

          transitionFound = true;
          counter++;
          break; // move to next iteration in while loop
        }
      }
    }

    // if no transition found -> check reductions
    if (!transitionFound) {
      for (size_t i = 0; i < reductions.size(); i++) {
        if (reductions[i].currentState == stateStack.top() && reductions[i].lookAhead == ".ACCEPT") {
          // reduce (replace top of reduction sequence with new rule)
          int n = reductions[i].rule;
          Production ruleN = productions[n];
          for (size_t i = 0; i < ruleN.RHS.size(); i++) {
            reductionSequence.pop_back();
          }
          reductionSequence.push_back(ruleN.LHS);
          accepted = true, reductionMade = true;
          transitionFound = true; // JUST SO WE CAN PRINT THE LAST LINE then we end parsing
          break;
        } else if (reductions[i].currentState == stateStack.top() && reductions[i].lookAhead == inputSequence.back()) {
          // reduce (replace top of reduction sequence with new rule)
          int n = reductions[i].rule;
          Production ruleN = productions[n];
          for (size_t i = 0; i < ruleN.RHS.size(); i++) {
            reductionSequence.pop_back();
            stateStack.pop(); // pop from state stack for # of elements in RHS of rule
          }
          reductionSequence.push_back(ruleN.LHS);
          reductionMade = true;
          break;
        }
      }
      // if no transition AND no reduction found: throw error
      if (!reductionMade) {
        std::cerr << "ERROR at " << counter + 1 << std::endl;
        return 0;
      }

    }
    if (transitionFound) print(reductionSequence, inputSequence);
  }

}