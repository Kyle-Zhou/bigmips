#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>

struct Node {
  std::vector<Node*> children;
  std::string lhs; // type
  std::string rhs; // lexeme
  std::string dataType;
  std::vector<std::string> entireRule;

  Node(std::vector<Node*> children, std::vector<std::string> entireRule, std::string dataType) {
    this->children = children;
    this->dataType = dataType;
    this->entireRule = entireRule;
    // split the rule into lhs and rhs
    bool typed = false;
    this->lhs = entireRule[0];
    for (size_t i = 1; i < entireRule.size() - 1; i++) {
      if (entireRule[i+1] == ":") {

        if (entireRule[i+2] == "INT" || entireRule[i+2] == "NUM" || entireRule[i+2] == "int") {
          this->dataType = "INT";
        } else {
          this->dataType = "INT_STAR";
        }

        this->rhs += entireRule[i];
        typed = true;
        break;      
      } else {
        this->rhs += entireRule[i] + " ";
      }
    }
    if (!typed) this->rhs += entireRule[entireRule.size() - 1];
  }

  ~Node() {
    for (size_t i = 0; i < children.size(); i++) {
      delete children[i];
      children[i] = nullptr;
    }
  }
};

struct Variable {
  std::string type;
  int offset;
};

void push3() {
  std::cout << "sw $3, -4($30)" << std::endl;
  std::cout << "sub $30, $30, $4" << std::endl;
}
void pop5() {
  std::cout << "add $30, $30, $4" << std::endl;
  std::cout << "lw $5, -4($30)" << std::endl;
}
void push1() {
  std::cout << "sw $1, -4($30)" << std::endl;
  std::cout << "sub $30, $30, $4" << std::endl;
}
void pop1() {
  std::cout << "add $30, $30, $4" << std::endl;
  std::cout << "lw $1, -4($30)" << std::endl;
}
void push31() {
  std::cout << "sw $31, -4($30)" << std::endl;
  std::cout << "sub $30, $30, $4" << std::endl;
}
void pop31() {
  std::cout << "add $30, $30, $4" << std::endl;
  std::cout << "lw $31, -4($30)" << std::endl;
}

std::string createNewLabel(int &i) {
  std::string str = "Label" + std::to_string(i);
  i++;
  return str;
}

void generateCode(Node* tree, std::map<std::string, std::map<std::string, Variable>> &symbolTable, int &dclNum, int &dclRegNum, int &label, std::map<std::string, std::vector<std::string>> &procedureTable, std::string &curProcedure) {
  if (tree->lhs == "procedures") {
    if (tree->rhs == "procedure procedures") { // procedures → procedure procedures
      generateCode(tree->children[1], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure);
      generateCode(tree->children[0], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure);
    } else if (tree->rhs == "main") {
      generateCode(tree->children[0], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure);
    }
  } else if (tree->lhs == "procedure") { // procedure → INT ID LPAREN params RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE
    curProcedure = tree->children[1]->entireRule[1] + "X";
    dclNum = 1;
    dclRegNum = 1;

    // prologue
    std::cout << curProcedure << ":" << std::endl;
    std::cout << "; PROLOGUE" << std::endl;
    std::cout << "sub $29, $30, $4" << std::endl;            

    if (tree->children[6]->entireRule[1] != ".EMPTY") { 
      generateCode(tree->children[6], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure);
    } else std::cout << std::endl; // end prologue

    generateCode(tree->children[7], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure);
    generateCode(tree->children[9], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure);

    // epilogue
    std::cout << "; EPILOGUE" << std::endl;
    for (size_t i = 0; i < symbolTable[curProcedure].size() - procedureTable[curProcedure].size(); i++) { 
      std::cout << "add $30, $30, $4" << std::endl;
    }    
    std::cout << "jr $31" << std::endl << std::endl;

  } else if (tree->lhs == "main") {

    dclNum = 1;
    dclRegNum = 1;
    curProcedure = tree->children[1]->entireRule[1]; // should be wain

    std::cout << ".import init" << std::endl;
    std::cout << ".import new" << std::endl;
    std::cout << ".import delete" << std::endl;
    std::cout << ".import print" << std::endl << std::endl;

    // prologue
    std::cout << curProcedure << ":" << std::endl; // Printing main's label
    std::cout << "; PROLOGUE" << std::endl;
    std::cout << "lis $4" << std::endl;
    std::cout << ".word 4" << std::endl;
    std::cout << "lis $11" << std::endl;
    std::cout << ".word 1" << std::endl;
    std::cout << "sub $29, $30, $4" << std::endl; // frame pointer

    generateCode(tree->children[3], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure); // dcl
    generateCode(tree->children[5], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure); // dcl
    if (tree->children[3]->children[1]->dataType != "INT_STAR") std::cout << "add $2, $0, $0" << std::endl; // when param1 is not int* -> $2 value must be 0 on init
    push31();
    std::cout << "lis $5" << std::endl << ".word init" << std::endl << "jalr $5" << std::endl;
    pop31();
    generateCode(tree->children[8], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure); // dcls
    generateCode(tree->children[9], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure); // statement
    generateCode(tree->children[11], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure); // expr
        
    // epilogue
    std::cout << "; EPILOGUE" << std::endl;
    // std::cout << "add $30, $29, $4" << std::endl;
    for (size_t i = 0; i < symbolTable[curProcedure].size(); i++) { 
      std::cout << "add $30, $30, $4" << std::endl;
    }

    std::cout << "jr $31" << std::endl << std::endl;

  } else if (tree->lhs == "dcl") { // dcl -> type ID
    std::string name = tree->children[1]->rhs;
    int offset = symbolTable[curProcedure][name].offset;

    int maxParams = procedureTable[curProcedure].size(); // replace .find
    bool wain = false;
    if (curProcedure == "wain") wain = true;

    if (dclNum <= maxParams && wain) {
      std::cout << "sw $" << dclRegNum << ", " << offset << "($29)" << std::endl;
      dclRegNum++;
      while ((dclRegNum == 4) || (dclRegNum == 10) || (dclRegNum == 11)) { 
        dclRegNum++; 
      }
    } else { 
      std::cout << "sw $3" << ", " << offset << "($29)" << std::endl; 
    }
    std::cout << "sub $30, $30, $4" << std::endl;
    dclNum++;

  } else if (tree->lhs == "dcls") {
    if (tree->rhs == "dcls dcl BECOMES NUM SEMI") { // dcls -> dcls dcl BECOMES NUM SEMI
      generateCode(tree->children[0], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure); 
      std::cout << "lis $3" << std::endl;
      std::cout << ".word " << tree->children[3]->entireRule[1] << std::endl;
      generateCode(tree->children[1], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure); 
    } else if (tree->rhs == "dcls dcl BECOMES NULL SEMI") {
      generateCode(tree->children[0], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure); 
      std::cout << "add $3, $0, $11" << std::endl;
      generateCode(tree->children[1], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure); 
    }
  } else if (tree->lhs == "statement") {
    if (tree->rhs == "lvalue BECOMES expr SEMI") { // statement -> lvalue BECOMES expr SEMI
      // generateCode(tree->children[2], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure); 
      // std::cout << "sw $3, ";
      Node* lval = tree->children[0];
      bool lvalue = true;
      while (lvalue) {
        if (lval->rhs == "ID") {
          generateCode(tree->children[2], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure); 
          std::cout << "sw $3, ";
          generateCode(tree->children[0], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure); 
          lvalue = false;
        }
        else if (lval->rhs == "STAR factor") {
          generateCode(tree->children[2], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure); 
          push3();
          generateCode(lval->children[1], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure); 
          pop5();
          std::cout << "sw $5, 0($3)" << std::endl;
          lvalue = false;
        }
        else if (lval->rhs == "LPAREN lvalue RPAREN") { 
          lval = lval->children[1];
        }
      }

      // generateCode(tree->children[0], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure); 
    } else if (tree->rhs == "PRINTLN LPAREN expr RPAREN SEMI") { // statement -> PRINTLN LPAREN expr RPAREN SEMI
      push1();
      generateCode(tree->children[2], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure); 
      std::cout << "add $1, $3, $0" << std::endl;
      push31();
      std::cout << "lis $5" << std::endl << ".word print" << std::endl << "jalr $5" << std::endl;
      pop31();
      pop1();
    } else if (tree->rhs == "IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE") {  // statement -> IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE
      std::string label1 = createNewLabel(label);
      std::string label2 = createNewLabel(label);
      generateCode(tree->children[2], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure); 
      std::cout << "beq $3, $0, " << label1 << std::endl;
      generateCode(tree->children[5], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure);
      std::cout << "beq $0, $0, " << label2 << std::endl;
      std::cout << label1 << ":" << std::endl;
      generateCode(tree->children[9], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure);
      std::cout << label2 << ":" << std::endl;

    } else if (tree->rhs == "WHILE LPAREN test RPAREN LBRACE statements RBRACE") {  // statement → WHILE LPAREN test RPAREN LBRACE statements RBRACE
      std::string label1 = createNewLabel(label);
      std::string label2 = createNewLabel(label);
      std::cout << label1 << ":" << std::endl;
      generateCode(tree->children[2], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure); 
      std::cout << "beq $3, $0, " << label2 << std::endl;
      generateCode(tree->children[5], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure); 
      std::cout << "beq $0, $0, " << label1 << std::endl;
      std::cout << label2 << ":" << std::endl;

    } else if (tree->rhs == "DELETE LBRACK RBRACK expr SEMI") {
      generateCode(tree->children[3], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure); 
      std::string labelD = createNewLabel(label);
      std::cout << "beq $3, $11, " << labelD << std::endl;
      std::cout << "add $1, $3, $0" << std::endl;
      push31();
      std::cout << "lis $5" << std::endl << ".word delete" << std::endl << "jalr $5" << std::endl;
      pop31();
      std::cout << labelD << ":" << std::endl;
    }

  } else if (tree->lhs == "statements") {
    if (tree->rhs != ".EMPTY") { // statements -> statements statement
      generateCode(tree->children[0], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure); 
      generateCode(tree->children[1], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure); 
    }

  } else if (tree->lhs == "expr") {
    if (tree->rhs == "term") {     // expr -> term
      generateCode(tree->children[0], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure); 
    } else if (tree->rhs == "expr PLUS term" || tree->rhs == "expr MINUS term") { // expr -> expr PLUS term  expr -> expr MINUS term
      generateCode(tree->children[0], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure); 
      if (tree->children[0]->dataType == "INT" && tree->children[2]->dataType == "INT_STAR") std::cout << "mult $3, $4" << std::endl << "mflo $3" << std::endl;
      push3();
      generateCode(tree->children[2], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure); 
      if (tree->children[0]->dataType == "INT_STAR" && tree->children[2]->dataType == "INT") std::cout << "mult $3, $4" << std::endl << "mflo $3" << std::endl;
      pop5();
      if (tree->children[1]->lhs == "PLUS") { 
        std::cout << "add $3, $5, $3" << std::endl;
      } else { 
        std::cout << "sub $3, $5, $3" << std::endl;
      }
      if (tree->children[0]->dataType == "INT_STAR" && tree->children[2]->dataType == "INT_STAR") std::cout << "div $3, $4" << std::endl << "mflo $3" << std::endl;
    }

  } else if (tree->lhs == "term") {
    if (tree->rhs == "factor") {    // term -> factor
      generateCode(tree->children[0], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure); 
    } else if (tree->rhs == "term STAR factor" || tree->rhs == "term SLASH factor" || tree->rhs == "term PCT factor") {
      generateCode(tree->children[0], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure); 
      push3();
      generateCode(tree->children[2], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure); 
      pop5();
      if (tree->children[1]->lhs == "STAR") { 
        std::cout << "mult $5, $3" << std::endl;
        std::cout << "mflo $3" << std::endl;
      }
      else {
        std::cout << "div $5, $3" << std::endl;
        if (tree->children[1]->lhs == "SLASH") { 
          std::cout << "mflo $3" << std::endl; 
        } else { 
          std::cout << "mfhi $3" << std::endl; 
        }
      }
    }

  } else if (tree->lhs == "factor") {
    if (tree->rhs == "NUM") {  // factor -> NUM
        std::cout << "lis $3" << std::endl;
        std::cout << ".word " << tree->children[0]->entireRule[1] << std::endl; // factor's kid is -> NUM 21 -> .word 21
    } else if (tree->rhs == "ID") { // factor -> ID

        std::string name = tree->children[0]->rhs;
        int offset = symbolTable[curProcedure][name].offset;

        std::cout << "lw $3, " << offset << "($29)" << std::endl;
    } else if (tree->rhs == "LPAREN expr RPAREN") { // factor -> LPAREN expr RPAREN
      generateCode(tree->children[1], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure); 
    } else if (tree->rhs == "ID LPAREN RPAREN") { // factor -> ID LPAREN RPAREN
      
      std::string procedureName = tree->children[0]->entireRule[1];
      if (procedureName != "wain") procedureName += "X";
      std::cout << "sw $29, -4($30)" << std::endl << "sub $30, $30, $4" << std::endl;
      std::cout << "sw $31, -4($30)" << std::endl << "sub $30, $30, $4" << std::endl;

      std::cout << "lis $5"  << std::endl;
      std::cout << ".word " << procedureName << std::endl;
      std::cout << "jalr $5" << std::endl;
      std::cout << "add $30, $30, $4" << std::endl << "lw $31, -4($30)" << std::endl;
      std::cout << "add $30, $30, $4" << std::endl << "lw $29, -4($30)" << std::endl;

    } else if (tree->rhs == "ID LPAREN arglist RPAREN") {
      std::string procedureName = tree->children[0]->entireRule[1];
      if (procedureName != "wain") procedureName += "X";

      int maxParams = procedureTable[procedureName].size();
      std::cout << "sw $29, -4($30)" << std::endl << "sub $30, $30, $4" << std::endl;
      std::cout << "sw $31, -4($30)" << std::endl << "sub $30, $30, $4" << std::endl;
      generateCode(tree->children[2], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure); 
      std::cout << "lis $5"  << std::endl;
      std::cout << ".word " << procedureName << std::endl;
      std::cout << "jalr $5" << std::endl;
      for (int i = 0; i < maxParams; i++) {
        std::cout << "add $30, $30, $4" << std::endl << "lw $31, -4($30)" << std::endl;
      }
      std::cout << "add $30, $30, $4" << std::endl << "lw $31, -4($30)" << std::endl;
      std::cout << "add $30, $30, $4" << std::endl << "lw $29, -4($30)" << std::endl;
    } else if (tree->rhs == "NULL") {
      std::cout << "add $3, $0, $11" << std::endl;
    } else if (tree->rhs == "AMP lvalue") {

      Node* lval = tree->children[1];
      bool lvalue = true;
      while (lvalue) {
        if (lval->rhs == "ID") {
          std::string name = lval->children[0]->rhs;
          int offset = symbolTable[curProcedure][name].offset;
          std::cout << "lis $3" << std::endl;
          std::cout << ".word " << offset << std::endl;
          std::cout << "add $3, $3, $29" << std::endl;
          lvalue = false;
        }
        else if (lval->rhs == "STAR factor") {
          generateCode(lval->children[1], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure); 
          lvalue = false;
        }
        else if (lval->rhs == "LPAREN lvalue RPAREN") { 
          lval = lval->children[1];
        }
      }
    } else if (tree->rhs == "STAR factor") {
      generateCode(tree->children[1], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure); 
      std::cout << "lw $3, 0($3)" << std::endl;
    } else if (tree->rhs == "NEW INT LBRACK expr RBRACK") {
      generateCode(tree->children[3], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure); 
      std::cout << "add $1, $3, $0" << std::endl;
      push31();
      std::cout << "lis $5" << std::endl << ".word new" << std::endl << "jalr $5" << std::endl;
      pop31();
      std::cout << "bne $3, $0, 1" << std::endl;
      std::cout << "add $3, $11, $0" << std::endl;
    }

  } else if (tree->lhs == "lvalue") {
    if (tree->rhs == "ID") {     // lvalue -> ID
      std::string name = tree->children[0]->rhs;
      int offset = symbolTable[curProcedure][name].offset;
      std::cout << offset << "($29)" << std::endl;
    } else if (tree->rhs == "LPAREN lvalue RPAREN") {     // lvalue -> LPAREN lvalue RPAREN
      generateCode(tree->children[1], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure); 
    } else if (tree->rhs == "STAR factor") {
      generateCode(tree->children[1], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure); 
      std::cout << "lw $3, 0($3)" << std::endl;
    }
  } else if (tree->lhs == "test") {
    bool slt = true; // slt or sltu
    if (tree->children[0]->dataType == "INT_STAR" && tree->children[2]->dataType == "INT_STAR") slt = false; // use sltu

    if (tree->rhs == "expr EQ expr") {   // test -> expr EQ expr && test -> expr NE expr
      generateCode(tree->children[0], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure); 
      push3();
      generateCode(tree->children[2], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure); 
      pop5();
      if (slt) std::cout << "slt $6, $3, $5" << std::endl << "slt $7, $5, $3" << std::endl << "add $3, $6, $7" << std::endl;
      else std::cout << "sltu $6, $3, $5" << std::endl << "sltu $7, $5, $3" << std::endl << "add $3, $6, $7" << std::endl;
      std::cout << "sub $3, $11, $3" << std::endl; 
    } else if (tree->rhs == "expr NE expr") {
      generateCode(tree->children[0], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure); 
      push3();
      generateCode(tree->children[2], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure); 
      pop5();
      if (slt) std::cout << "slt $6, $3, $5" << std::endl << "slt $7, $5, $3" << std::endl << "add $3, $6, $7" << std::endl;
      else std::cout << "sltu $6, $3, $5" << std::endl << "sltu $7, $5, $3" << std::endl << "add $3, $6, $7" << std::endl;
    } else if (tree->rhs == "expr GE expr") {  // test -> expr LT expr && test -> expr GE expr
      generateCode(tree->children[0], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure);
      push3();
      generateCode(tree->children[2], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure);
      pop5();
      if (slt) std::cout << "slt $3, $5, $3" << std::endl;
      else std::cout << "sltu $3, $5, $3" << std::endl;
      std::cout << "sub $3, $11, $3" << std::endl; 
    } else if (tree->rhs == "expr LT expr") {
      generateCode(tree->children[0], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure);
      push3();
      generateCode(tree->children[2], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure);
      pop5();
      if (slt) std::cout << "slt $3, $5, $3" << std::endl;
      else std::cout << "sltu $3, $5, $3" << std::endl;
    } else if (tree->rhs == "expr LE expr") { // test -> expr GT expr && test -> expr LE expr
      generateCode(tree->children[0], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure);
      push3();
      generateCode(tree->children[2], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure);
      pop5();
      if (slt) std::cout << "slt $3, $3, $5" << std::endl;
      else std::cout << "sltu $3, $3, $5" << std::endl;
      std::cout << "sub $3, $11, $3" << std::endl; 
    } else if (tree->rhs == "expr GT expr") {
      generateCode(tree->children[0], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure);
      push3();
      generateCode(tree->children[2], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure);
      pop5();
      if (slt) std::cout << "slt $3, $3, $5" << std::endl;
      else std::cout << "sltu $3, $3, $5" << std::endl;
    }

  } else if (tree->lhs == "arglist") {
    if (tree->rhs == "expr") {
      generateCode(tree->children[0], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure);
      push3();
    } else if (tree->rhs == "expr COMMA arglist") {
      generateCode(tree->children[2], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure);
      generateCode(tree->children[0], symbolTable, dclNum, dclRegNum, label, procedureTable, curProcedure);
      push3();
    }
  }

}

void createSymbolTable(Node *node, std::string &curProcedure, int &dclNum, std::map<std::string, std::map<std::string, Variable>> &symbolTable, std::map<std::string, std::vector<std::string>> &procedureTable, bool &add) {
  if ((node->lhs == "procedure") || (node->lhs == "main")) {
      curProcedure = node->children[1]->entireRule[1];
      if (curProcedure != "wain") curProcedure += "X";
      dclNum = 0;
      add = true;
      // add new entry in -> symbol tables + update procedure table
      std::map<std::string, Variable> st;
      std::pair<std::string, std::map<std::string, Variable>> tempST = {curProcedure, st};
      symbolTable.insert(tempST);
      std::vector<std::string> params;
      std::pair<std::string, std::vector<std::string>> procedureItem = {curProcedure, params};
      procedureTable.insert(procedureItem);
  } else if ((node->lhs) == "dcl") { // If lhs is dcl -> need to add declaration -> symbol table
      std::string symbol = node->children[1]->entireRule[1]; // FIRST ELEM OF RHS ...? works i think... can't be 0 (tested)
      Variable tempVar;
      dclNum++;
      if (add) {
        procedureTable[curProcedure].push_back(node->children[1]->dataType);
        if (curProcedure == "wain") {
          tempVar = {node->children[1]->dataType, (dclNum-1) * -4};
        } else {
          tempVar = {node->children[1]->dataType, dclNum * 4};
        }
      } else if (curProcedure == "wain") {
        tempVar = {node->children[1]->dataType, (dclNum-1) * -4};
      } else {
        int diff = dclNum - procedureTable[curProcedure].size();
        tempVar = {node->children[1]->dataType, (diff - 1) * -4};
      }
      std::pair<std::string, Variable> newEntry = {symbol, tempVar};
      ((symbolTable.find(curProcedure))->second).insert(newEntry);
      
  } else if ((node->lhs) == "dcls") {
    add = false;
  }
  for (size_t i = 0; i < node->children.size(); i++) {
    createSymbolTable(node->children[i], curProcedure, dclNum, symbolTable, procedureTable, add);
  }
}

// CREATE TREE ------------------------------------------------------------------------------------------

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

std::string untokenize(std::vector<std::string> vec) {
  std::string untokenized = "";
  for (size_t i = 0; i < vec.size() - 1; i++) { //I CHANGED THIS TO A 1
    untokenized += vec[i] + " ";
  }
  untokenized += vec[vec.size() - 1];
  return untokenized;
}

int lenSubtree(Node* tree) {
  if (tree->children.empty()) {
    return 1;
  }
  int num = 1;
  for (Node* child : tree->children) {
    num += lenSubtree(child);
  }
  return num;
}

Node* createTree(std::vector<std::vector<std::string>> &inputs, std::vector<std::string> &inputStrings, int index, int numChildren) {

  std::vector<Node*> children;
  size_t i = index + 1;

  // base case
  if (numChildren == 0) return new Node({}, inputs[index], "");

  for (int c = 0; c < numChildren; c++) { // for each child, look at THEIR kids
    if (i >= inputs.size()) break;
    // std::string entireRule = untokenize(inputs[i]);
    std::string entireRule = inputStrings[i];
    int newChildren = 0;
    if (entireRule != ".EMPTY") {
      for (size_t i = 0; i < entireRule.size() - 1; i++) {
        if (entireRule[i + 1] == ':') break;
        else if (entireRule[i] == ' ') newChildren++;
      }
    }
    if (newChildren == 1) {
      std::string lhs = inputs[i][0];
      std::string firstRhs = inputs[i][1];
      if (isTerminal(lhs) || firstRhs == ".EMPTY") { // if a terminal or empty
        newChildren = 0;
      }
    }

    Node* node {createTree(inputs, inputStrings, i, newChildren)};
    children.emplace_back(node);
    int lines = lenSubtree(node);
    i += lines;
  }

  Node* node = new Node(children, inputs[index], "");
  return node;
}

// -----------------------------------------------------------------------------------------------

void printTree(Node *node) {
  if (node->rhs != "" || node->lhs != "") {
    std::cout << node->lhs << " " << node->rhs;
    std::cout << std::endl;
  }
  for (size_t i = 0; i < node->children.size(); i++) {
    printTree(node->children[i]);
  }
}

void printSymbolTables(const std::map<std::string, std::map<std::string, Variable>>& symbol_tables) {
  for (const auto& outer_pair : symbol_tables) {
    const std::string& outer_key = outer_pair.first;
    const std::map<std::string, Variable>& inner_map = outer_pair.second;

    std::cout << "Procedure: " << outer_key << std::endl;

    for (const auto& inner_pair : inner_map) {
      const std::string& inner_key = inner_pair.first;
      const Variable& variable = inner_pair.second;

      std::cout << "   Variable Name: " << inner_key << std::endl;
      std::cout << "        Type: " << variable.type << std::endl;
      std::cout << "        Offset: " << variable.offset << std::endl;
    }
  }
}

int main() {

  // READ INPUT AND STORE IN INPUT VECTOR ---------------------------------------
  std::string s;
  std::istream& in = std::cin;
  std::vector<std::vector<std::string>> inputs; // (rules)
  std::vector<std::string> inputStrings; // (rules)
  
  while(std::getline(in, s)) {
    inputStrings.push_back(s);
    inputs.push_back(tokenize(s));
  }

  // CREATE TREE ----------------------------------------------------------------
  Node* tree = createTree(inputs, inputStrings, 0, 3);

  // CREATE SYMBOL TABLE ---------------------------------------------------------
  std::string curProcedure = "";
  int dclNum = 0;
  int dclRegNum = 0;
  bool add = false;
  std::map<std::string, std::map<std::string, Variable>> symbol_tables; // { procedure: { variable name: {type, offset}, ... }, ... }
  std::map<std::string, std::vector<std::string>> procedure_tables;

  createSymbolTable(tree, curProcedure, dclNum, symbol_tables, procedure_tables, add);

  // GENERATE CODE----------------------------------------------------------------
  int label = 0;
  generateCode(tree->children[1], symbol_tables, dclNum, dclRegNum, label, procedure_tables, curProcedure); // children[1] -> start at: procedures main

  delete tree;
}