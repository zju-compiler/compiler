#include "public.h"
#include "utils.h"
#include "scanner.h"
#include "parser.hpp"
#include "symtab.h"
#include "code_generator.h"
#include <iostream>
#include <fstream>
#include <stdio.h>
#define MAX_OPTION 10
using namespace std;

extern int traceScan;
extern int traceParse;
extern TreeNode *root;
Symtab *mainSymtab;
extern ofstream sym;
extern RegManager *regManager;
extern LabelManager *labelManager;
extern map<string,string> constStringMap;
extern map<string,string> constRealMap;

int LabelManager::loop_number = 0;
int LabelManager::func_number = 0;
int LabelManager::case_number = 0;
int LabelManager::if_number = 0;
int LabelManager::do_number = 0;
int LabelManager::string_label_number = 0;
int LabelManager::real_label_number = 0;
int LabelManager::equal_number = 0;
int CodeGenerator::equal = 0;

int main(int argc, char *argv[]) {
	if (argc <= 1) {
		cout << "no input file" << endl;
		return 1;
	}

  if ((source = fopen(argv[1], "r")) == NULL) {
  	cout << "error can not open file " << argv[1] << endl;
  	return 1;
  }
  else{
    cout << "test file: " << argv[1] << endl;
  }

	listing = fopen("result/lex_token", "w+");
	if (listing == NULL) {
		cout << "error can not open file listing" << endl;
		return 1;
	}

  traceScan = TRUE;
  yyparse();
  cout << "======================yyparse end=========================" << endl;
  if(root == NULL)
  	cout << "Error:root is null!!!" << endl; 

  printAST(root);
  cout << "======================print AST end=========================" << endl;
  mainSymtab = new Symtab("mainSymtab");
  regManager = new RegManager();
  labelManager = new LabelManager();

  root->updateSymtab(mainSymtab);
  cout << "======================updateSymtab end=========================" << endl;
  string result = root->typeCheck(mainSymtab);
  cout << "=========================typeCheck end=========================" << endl;
  if(result == "failure"){
  	exit(1);
  }
  mainSymtab->printSymtab(sym);
  cout << "===========================begin gencode=======================" << endl;
  root->genCode(mainSymtab);
  // CodeGenerator::emitCodeM(4, "load_ref", 12, 30, 17);
  // CodeGenerator::emitCodeR("%",11,9,10);
  // CodeGenerator::emitCodeR("<=",11,9,10);
  // CodeGenerator::emitCodeR("==",11,9,10);
  // CodeGenerator::emitCodeI("%",11,9,10);
  // CodeGenerator::emitCodeI("<=",11,9,10);
  // CodeGenerator::emitCodeI("==",11,9,10);
  delete regManager;
  delete labelManager;
  delete root;
  return 0;
}

















