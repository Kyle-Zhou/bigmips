#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <map> 

struct Procedure {
	std::vector<std::string> signature; // procedure signature = parameters
	std::map<std::string, std::string> symbolTable;
};

std::map<std::string, Procedure> tables;

class Tree {
  public:
    std::string rule; // e.g. expr expr PLUS term
    std::vector <Tree*> children;
    std::string annotation;
    
    Tree (std::string rule, std::vector<Tree*> children, std::string annotation) {
      this->rule = rule; 
      this->children = children; 
      this->annotation = annotation;
    }

    void printchildren() {
      for (Tree* child : children) {
        std::cout << child->rule << ", ";
      }
    };

    void annotate(const Tree *t) {
      for ( Tree* child : t->children ){
        // visit the children
        //if (t->children.)

        annotate(child);
      }

    }

};

bool isTerminal(std::string s) {
  if (s == "ID" || s == "NUM" || s == "LPAREN" || s == "RPAREN" || s == "LBRACE" || 
      s == "RBRACE" || s == "RETURN" || s == "IF" || s == "ELSE" || s == "WHILE" || 
      s == "PRINTLN" || s == "WAIN" || s == "BECOMES" || s == "INT" || s == "EQ" || 
      s == "NE" || s == "LT" || s == "GT" || s == "LE" || s == "GE" ||
      s == "PLUS" || s == "MINUS" || s == "STAR" || s == "SLASH" || s == "PCT" || 
      s == "COMMA" || s == "SEMI" || s == "NEW" || s == "DELETE" || s == "LBRACK" || s == "RBRACK" || 
      s == "AMP" || s == "NULL" || s == "EOF" || s == "BOF") {
    return true;
  }
  return false;     
}

std::vector<std::string> tokenize(std::string inputString) {
  std::stringstream ss(inputString); 
  // create a vector that we can traverse for each input string
  std::vector<std::string> stringToVector {};
  while (ss >> inputString) {
    stringToVector.push_back(inputString);
  }
  return stringToVector; 
}

Tree* reconstructTree() {
  // read in input
  std::istream& in = std::cin;
  std::string s;
  std::getline(in, s); // s is the line of strings 

  std::vector<Tree*> children {};
  std::vector<std::string> lineInput = tokenize(s); 

  for (size_t i = 1; i < lineInput.size(); i++) { // loop through line, ignoring the first term (LHS of rule):

    if (isTerminal(lineInput[i])) {
      // look at next line (need some kind of line counter)
      // append the terminal (BOF, INT...) to some structure...
      std::string next;
      getline(in, next);

      std::vector<Tree*> emptyChildren;
      Tree* terminalNode = new Tree("", emptyChildren, "");
      std::vector<Tree*> onlyChild {terminalNode};
      Tree* child = new Tree(next, onlyChild, "");
      children.push_back(child);

    } else if (lineInput[i] == ".EMPTY") {
      continue;

    } else {
      children.push_back(reconstructTree()); // recurse on the next line
    }
  }

  Tree* parent = new Tree(s, children, "");
  return parent; 

}

void printTree(Tree &node) {
  if (node.rule != "") {
    std::cout << node.rule;
    if (node.annotation != "") std::cout << " " << node.annotation;
    std::cout << std::endl;
  }
  for (size_t i = 0; i < node.children.size(); i++) {
    printTree(*node.children[i]);
  }
}



int main() {
  Tree* newTree = reconstructTree();
  printTree(*newTree);

  // traverse through the tree and check for semantic errors:
  
  // std::cout << std::endl;
  // newTree->printchildren();
  // std::cout << std::endl;
  // newTree->doSomething(newTree);
  // std::cout << std::endl;
}
