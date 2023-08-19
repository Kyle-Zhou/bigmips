#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <stack>
#include <fstream>
#include <algorithm>
#include "wlp4data.h"

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

struct Node {
  std::string symbol;
  std::vector<Node> children;
};

struct InputObject {
  std::string token;
  std::string lexeme;
};

void reduce(std::vector<std::string> &reductionSequence, Production &ruleN, Node &parseTree, std::stack<std::string> &stateStack) {
  std::string lhs = ruleN.LHS;
  if (ruleN.RHS.size() == 0) {
    lhs += " .EMPTY";
  } else {
    for (size_t i = 0; i < ruleN.RHS.size(); i++) {
      lhs += " " + ruleN.RHS[i];
    }
  }

  std::vector<Node> children;
  for (size_t i = 0; i < ruleN.RHS.size(); i++) {
    children.push_back((parseTree.children).back());
    std::rotate(children.rbegin(), children.rbegin() + 1, children.rend());
    parseTree.children.pop_back();
    reductionSequence.pop_back();
    stateStack.pop();
  }
  reductionSequence.push_back(ruleN.LHS);
  Node node = {lhs, children};
  parseTree.children.push_back(node);
}

void printNode(Node &node) {
  if (node.symbol != "") std::cout << node.symbol << std::endl;
  for (size_t i = 0; i < node.children.size(); i++) {
    printNode(node.children[i]);
  }
}

int main() {
  std::istream& in = std::cin;
  std::string s;

  // define our data structures: ------------------------------------------------------------------------------------
  // reduction sequence -> vector
  // input sequence -> vector
  std::vector<std::string> reductionSequence;
  std::vector<InputObject> inputSequence;

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
  std::istringstream wlp4cfg {WLP4_CFG};
  std::getline(wlp4cfg, s); // skip header
  while(std::getline(wlp4cfg, s)) {
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

  // .TRANSITIONS
  std::istringstream wlp4t {WLP4_TRANSITIONS};
  std::getline(wlp4t, s); // skip header
  while(std::getline(wlp4t, s)) {
    std::string fromState, transitionValue, toState;
    std::istringstream line(s);
    line >> fromState, line >> transitionValue, line >> toState;
    Transition t = {fromState, transitionValue, toState};
    transitions.push_back(t);
  }

  // .REDUCTIONS
  std::istringstream wlp4r {WLP4_REDUCTIONS};
  std::getline(wlp4r, s);
  while(std::getline(wlp4r, s)) {
    std::string currentState, lookAhead;
    int rule;
    std::istringstream line(s);
    line >> currentState, line >> rule, line >> lookAhead;
    Reduction r = {currentState, rule, lookAhead};
    reductions.push_back(r);
  }

  // INPUT ------------------------------------------------
  InputObject firstLine = {"BOF", "BOF"};
  inputSequence.push_back(firstLine);
  while(std::getline(in, s)) {
    std::istringstream line(s);
    std::string token;
    std::string lexeme;
    line >> token;
    line >> lexeme;
    InputObject io = {token, lexeme};
    inputSequence.push_back(io);
    std::rotate(inputSequence.rbegin(), inputSequence.rbegin() + 1, inputSequence.rend());
  }
  InputObject lastLine = {"EOF", "EOF"};
  inputSequence.push_back(lastLine);
  std::rotate(inputSequence.rbegin(), inputSequence.rbegin() + 1, inputSequence.rend());

  // for (size_t i = 0; i < inputSequence.size(); i++) {
  //   std::cout << inputSequence[i].token << " " << inputSequence[i].lexeme << std::endl;
  // }
  // -------------------------------------------------------

  Node parseTree;
  bool accepted = false;
  bool reductionMade = false;
  int counter = 0;
  if (transitions.size() >= 1) stateStack.push(transitions[0].fromState);
  
  // // PARSING LOOP
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
        if (transitions[i].fromState == stateStack.top() && transitions[i].transitionValue == inputSequence.back().token) {
          // shift
          std::string shifted = inputSequence.back().token; // pops from the back but technically its the left end b/c we reversed the vector
          reductionSequence.push_back(shifted);
          // push new node
          std::string nodeValue = inputSequence.back().token + " " + inputSequence.back().lexeme;
          std::vector<Node> emptyChildren;
          Node node = {nodeValue, emptyChildren};
          parseTree.children.push_back(node);
          inputSequence.pop_back();

          // add to stateStack
          stateStack.push(transitions[i].toState); 

          transitionFound = true;
          if (reductionSequence.back() != "BOF" &&  reductionSequence.back() != "EOF") counter++;
          break; // move to next iteration in while loop
        }
      }
    }

    // if no transition found -> check reductions
    if (!transitionFound) {
      for (size_t i = 0; i < reductions.size(); i++) {
        if (reductions[i].currentState == stateStack.top() && reductions[i].lookAhead == ".ACCEPT") {
          // reduce (replace top of reduction sequence with new rule)
          // create data for new node
          int n = reductions[i].rule;
          Production ruleN = productions[n];
          reduce(reductionSequence, ruleN, parseTree, stateStack);
          reductionSequence.push_back(ruleN.LHS);
          accepted = true, reductionMade = true;
          break;
        } else if (reductions[i].currentState == stateStack.top() && reductions[i].lookAhead == inputSequence.back().token) {
          // reduce -----------------------------------------------------------------------------------
          int n = reductions[i].rule;
          Production ruleN = productions[n];
          reduce(reductionSequence, ruleN, parseTree, stateStack);
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
  }
  if (accepted) printNode(parseTree);

}