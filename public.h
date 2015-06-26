#ifndef _PUBLIC_H_
#define _PUBLIC_H_

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <fstream>
#include <cstdlib>
#include <map>
#include <cstdio>
#define YYSTYPE TreeNode *
using namespace std;
typedef int TokenType;
/*
* following variables are defined in main.c
*/
extern int lineno;
extern FILE *source;
extern FILE *listing;
extern string currentToken;
extern ofstream ast;
extern ofstream code;
extern ofstream sym;
extern ofstream parserOut;
extern map<string,string> constStringMap;
extern map<string,string> constRealMap;
class Symtab;
class SymBucket;
static int constStringNumber;

////////////////////////////////////////////////////////
// AST data structure								  //
////////////////////////////////////////////////////////
/*
* base class for all different kind of node within a AST
*/
class TreeNode {
private:
	const string treeName;
protected:
	Symtab *env;
	vector<TreeNode *> children;
	int lineNO;
public:
	TreeNode(const string _name):treeName(_name){}
	TreeNode():treeName(""){
		lineNO = lineno;
	}
	virtual ~TreeNode() {}
	virtual void printSelf() {
		ast << " TreeNode ";
	}
	virtual vector<TreeNode *> getChildren() {
		return children;
	}
	const string& getName() {
		return treeName;
	}
	int getLineNO() {
		return lineNO;
	}
	virtual void insert(TreeNode* t) {}

	// return 0-31 : it is a reg number
	// return -1 : it is a variable at stack , the offset is at bucket->loc
	// return -2 : it is a immediate
	// return -3 : the offset is at bucket->offsetReg
	virtual SymBucket *genCode(Symtab *symtab, int *reg = NULL) {
		code << "default genCode" << endl;
	}

	virtual void updateSymtab(Symtab *symtab) {
		sym << "default updateSymtab" << endl;
	}

	virtual string typeCheck(Symtab *symtab) {
		return "treenode";
	}
	void setEnv(Symtab *e) {
		env = e;
	}
};


class StmtTreeNode : public TreeNode {
protected:
	int label;
public:
	virtual ~StmtTreeNode() {}
	void setLabel(int _label) {
		label = _label;
	}
	void printSelf() {
		ast << "StmtTreeNode";
	}
};

class IDTreeNode : public TreeNode {
public:
	virtual ~IDTreeNode() {}
	void printSelf() {
		ast << "IDTreeNode";
	}
	virtual const string getType() {}
	
};

/// expr node
class ExprTreeNode : public TreeNode {
public:
	virtual ~ExprTreeNode() {}
	void printSelf() {
		ast << "ExprTreeNode";
	}
	TreeNode * getlhs() { return NULL; }
};

/*
* base class for all kinds of type such as simple type, array type and record type
*/
class TypeTreeNode : public TreeNode {
public:
	virtual ~TypeTreeNode() {}
	void printSelf() {
		ast << "TypeTreeNode";
	}
	virtual SymBucket* genSymItem(const string typeName, Symtab *symtab){}
	virtual const string getType() {}
};

/*
* node contains a list of sub-nodes with same type
*/
class ListTreeNode : public TreeNode {
private:
	string typeName;
	vector<TreeNode *> list;
public:
	ListTreeNode( const string _name):typeName(_name) {
	}
	ListTreeNode( const string _name,  vector<TreeNode *>& _list)
	:typeName(_name),list(_list) {
	}
	//a.insert(a.end(), b.begin(), b.end());
	void append() {}
	void insert(TreeNode * newNode) {
		list.push_back(newNode);
	}
	TreeNode *get(int index) {
		return list[index];
	}
	int size() {
		return list.size();
	}
	vector<TreeNode *>& getList() {
		return list;
	}
	void printSelf() {
		ast << "a list of " << typeName<<":"<<list.size();	
	}
	vector<TreeNode *> getChildren() {
		return list;
	}

	SymBucket *genCode(Symtab *symtab, int *reg = NULL );
	void updateSymtab(Symtab*);
	string typeCheck(Symtab *symtab);

};



//=====================================================================
/// struct node
/*
* node for routine head
*/
class RoutineHeadTreeNode : public TreeNode {
private:
	ListTreeNode * constPart;
	ListTreeNode * typePart;
	ListTreeNode * varPart;
	ListTreeNode * routinePart;
public:
	RoutineHeadTreeNode( TreeNode *_constPart,  TreeNode *_typePart, 
		 TreeNode *_varPart,  TreeNode *_routinePart)
		:constPart((ListTreeNode*)_constPart), typePart((ListTreeNode*)_typePart)
		, varPart((ListTreeNode*)_varPart),routinePart((ListTreeNode*)_routinePart)
		{
			children.push_back(constPart);
			children.push_back(typePart);
			children.push_back(varPart);
			children.push_back(routinePart);
		}
	void printSelf() {
		ast << "RoutineHead";
	}
	void updateSymtab(Symtab *);
	ListTreeNode *getConstPart() {
		return constPart;
	}
	SymBucket *genCode(Symtab *symtab, int *reg = NULL );
};
/*
* node for routine
*/
class RoutineTreeNode : public TreeNode {
private:
	RoutineHeadTreeNode *head;
	ListTreeNode *body;
public:
	RoutineTreeNode( TreeNode *_head,  TreeNode *_body)
					:head((RoutineHeadTreeNode*)_head), body((ListTreeNode *)_body) 
					{
						children.push_back(head);
						children.push_back(body);
					}
	void printSelf() {
		ast << "RoutineTreeNode";
	}					
	SymBucket *genCode(Symtab *symtab, int *reg = NULL);
	void updateSymtab(Symtab*);
	string typeCheck(Symtab *symtab) {
		return body->typeCheck(symtab);
	}

};

class ProgramHeadTreeNode : public TreeNode {
private:
	const string name;
public:
	ProgramHeadTreeNode(const string _name):name(_name){}

	const string& getName() {
		return name;
	}
	void printSelf() {
		ast << "ProgramHeadTreeNode";
	}
};

/*
* root of the AST, representing the whole program
*/
class ProgramTreeNode : public TreeNode {
private:
	const string name;
	RoutineTreeNode * routine;
public:
	ProgramTreeNode( const string _name,  TreeNode * _routine):name(_name), routine((RoutineTreeNode *)_routine)
	{
		children.push_back(routine);
	}
	void printSelf() {
		ast << "ProgramTreeNode";
	}
	SymBucket *genCode(Symtab *symtab, int *reg = NULL );
	void updateSymtab(Symtab*);
	string typeCheck(Symtab *symtab) {return routine->typeCheck(symtab);}
};

//======
//===============================================================
///type node

/*
* node for user defined type
*/
class CustomTypeTreeNode : public TypeTreeNode {
private:
	const string name;
	TypeTreeNode *type;
public:
	
	///if type is null, then it means we need to check whether this 
	///type exists
	CustomTypeTreeNode( const string _name,  TreeNode* _type=NULL) 
						: name(_name),type((TypeTreeNode*)_type)
						{}
	void printSelf() {
		ast << "CustomTypeTreeNode:" << name;
	}
	const string getType() {
		return type->getType();
	}
	void updateSymtab(Symtab *);
	SymBucket *genSymItem(const string type, Symtab *symtab);
};

/*
* node for simple type such as sys type , subrange type, enum tpye
*/
class SimpleTypeTreeNode : public TypeTreeNode {
public:
	virtual ~SimpleTypeTreeNode() {}
};

class SysTypeTreeNode : public SimpleTypeTreeNode {
private:
	const string type;
public:
	SysTypeTreeNode( const string _name):type(_name){}
	void printSelf() {
		ast << "SysTypeTreeNode";
	}
	const string getType() {
		return type;
	}
	SymBucket *genSymItem(const string type, Symtab *symtab);
};

class SubRangeTypeTreeNode : public SimpleTypeTreeNode {
private:
	IDTreeNode *upperBound;
	IDTreeNode *lowerBound;
public:
	SubRangeTypeTreeNode( TreeNode *_l, TreeNode *_u)
						:upperBound((IDTreeNode*)_u), lowerBound((IDTreeNode*)_l)
						{}
	void printSelf() {
		ast << "SubRangeTypeTreeNode";
	}
	const string getType() {
		return "subrange";
	}
	SymBucket *genSymItem(const string type, Symtab *symtab);
};

/*
* node for enum type
*/
class EnumTypeTreeNode : public SimpleTypeTreeNode {
private:
	const string name;
	ListTreeNode* elemList;
public:
	EnumTypeTreeNode(TreeNode* _elemList,const string _name="")
					:name(_name), elemList((ListTreeNode*)_elemList)
					{}
	void printSelf() {
		ast << "EnumTypeTreeNode";
	}					
	const string getType() {
		return "enum";
	}
};

/*
* node for array type
*/
class ArrayTypeTreeNode : public TypeTreeNode {
private:
	SimpleTypeTreeNode *indexType;
	TypeTreeNode *elemType;
public:
	ArrayTypeTreeNode(TreeNode *_indexType,
					 TreeNode *_elemType):indexType((SimpleTypeTreeNode*)_indexType),elemType((TypeTreeNode*)_elemType)
	{
		// children.push_back(indexType);
		// children.push_back(elemType);
	}
	void printSelf() {
		ast << "ArrayTypeTreeNode";
	}	
	const string getType() {
		return "array";
	}
	SymBucket *genSymItem(const string type, Symtab *symtab);
};

/*
* node for record type
*/
class RecordTypeTreeNode : public TypeTreeNode {
private:
	ListTreeNode * elemList;
public:
	RecordTypeTreeNode( TreeNode *_list)
						:elemList((ListTreeNode*)_list)
						{
							// children.push_back(elemList);
						}
	void printSelf() {
		ast << "RecordTypeTreeNode";
	}	
	const string getType() {
		return "record";
	}
	SymBucket *genSymItem(const string type, Symtab *symtab);

};

//==============================================================
/// id node

/**
* node for differnet kinds of  values
*/
class NumberTreeNode : public IDTreeNode {
private:
	string value;
	const string type;
public:
	NumberTreeNode(const string v, const string _type):value(v),type(_type) {}

	const string get() {
		return value;
	}

	const string getType() {
		return type;
	}

	void set(const string v) {
		value = v;
	}

	void printSelf() {
		ast << "NumberTreeNode";
	}

	int getInt() {
		if (type  == "integer") {
			return atoi(value.c_str());
		}
	}

	char getChar() {
		if (type == "char") {
			return value[0];
		}
	}
	double getReal() {
		if (type == "real") {
			return atof(value.c_str());
		}
	}

	string getString() {
		if (type == "string") {
			return value;
		}
	}
	string typeCheck(Symtab *symtab){return type;}
	SymBucket *genCode(Symtab *symtab, int *reg = NULL );
};

/*
*  node for  variables
*/
class ConstTreeNode : public IDTreeNode {
private:
	const string name;
	IDTreeNode *value;  // NumberTreeNode
	int isFirst;		// since the first time gencode is called it will 
						// actually gen some code
public:
	ConstTreeNode( const string _name,  TreeNode *_value)
				:name(_name),value((IDTreeNode*)_value), isFirst(1)
				{}
	const string& getName() {
		return name;
	}
	int getIsFirst() {
		return isFirst;
	}
	void setIsFirst(int f) {
		isFirst = f;
	}
	void printSelf() {
		ast << "ConstTreeNode";
	}
	const string getType() {
		return value->getType();
	}
	SymBucket *genCode(Symtab *symtab, int *reg = NULL );
	string typeCheck(Symtab *symtab){return "const " + getType();}
	void updateSymtab(Symtab* symtab);
};

/*
* node for variables such as "fuck", it represents three kinds
* of variable: only a name, a name with a type, a list of name with a type
* if this node's type is NULL, we need to determine it in the analysis pass
*/
/// copy by reference if ref = 1, else copy by value
/// if this node is not within a arg-list , then ref can be ignored
class VariableTreeNode : public IDTreeNode {
private:
	const string name;
	TypeTreeNode * typeNode;
	ListTreeNode *nameList;
	int isVar;
public:
	VariableTreeNode( const string _name = "",  TreeNode* _type=NULL, int _ref = 0)
	:name(_name), typeNode((TypeTreeNode*)_type), nameList(NULL),isVar(_ref)
	 {}
	VariableTreeNode(TreeNode *_list, TreeNode* _type=NULL, int _ref = 0)
	:name(""),typeNode((TypeTreeNode*)_type),isVar(_ref),nameList((ListTreeNode*)_list)
	{}
	const string& getName() {
		return name;
	}
	const TypeTreeNode* getTreeNodeType() {
		return typeNode;
	}
	ListTreeNode *getNameList() {
		return nameList;
	}
	int getIsVar() {
		return isVar;
	}
	void printSelf() {
		ast << "VariableTreeNode:";
		if (name != "") {
			ast << name;
		} else {
			vector<TreeNode *> list = nameList->getList();
			if (list.size() > 0) {
				for (int i = 0; i < list.size(); i++) 
					ast << list[i]->getName();
			}
		}

	}	
	TypeTreeNode *getTypeNode() {
		return typeNode;
	}
	virtual void print() {cout<<"I am a VariableTreeNode!"<<endl;}
	void updateSymtab(Symtab*);
	string typeCheck(Symtab *symtab);
	SymBucket *genCode(Symtab *symtab, int *reg = NULL );
};

class ArrayElemTreeNode : public IDTreeNode {
private:
	const string name;
	//ExprTreeNode *index;
	TreeNode *index;
public:
	ArrayElemTreeNode( const string _name,  TreeNode *_index)
					: name(_name), index(_index) 
					{}
	void printSelf() {
		ast << "ArrayElemTreeNode:" << name;
	}
	string typeCheck(Symtab *symtab);
	SymBucket *genCode(Symtab *symtab, int *reg = NULL );
};

class RecordElemTreeNode : public IDTreeNode {
private:
	const string recordName;
	const string elemName;
public:
	RecordElemTreeNode( const string& _rName,  const string& _eName)
					:recordName(_rName), elemName(_eName)
					{}
	void printSelf() {
		ast << "RecordElemTreeNode:"<<recordName<<"."<<elemName;
	}		
	SymBucket *genCode(Symtab *symtab, int *reg);
	string typeCheck(Symtab *symtab);

};

//===============================================
//expr nodes
/*
* node for unary operator
*/
class UnaryExprTreeNode : public ExprTreeNode {
private:
	const string op;
	TreeNode *operand;
public:
	UnaryExprTreeNode(const string _op,  TreeNode *_operand):op(_op),operand(_operand)
	{
		children.push_back(operand);
	}
	void printSelf() {
		ast << "UnaryExprTreeNode";
	}	
	SymBucket *genCode(Symtab *symtab, int *reg = NULL );
	string typeCheck(Symtab *symtab);
};

/*
* node for binary operaotr such as '+'  '='
*/
class BinaryExprTreeNode : public ExprTreeNode {
private:
	const string op;
	TreeNode * rhs, *lhs;
public:
	BinaryExprTreeNode(const string _op,  TreeNode* l,  TreeNode* r)
						:op(_op),rhs(r),lhs(l) 
	{
		children.push_back(rhs);
		children.push_back(lhs);
	}
	void printSelf() {
		ast << "BinaryExprTreeNode";
	}
	TreeNode * getLhs() { return lhs; }
	SymBucket *genCode(Symtab *symtab, int *reg = NULL );

	string typeCheck(Symtab *symtab);
};

/*
* node for function call
*/
class CallExprTreeNode : public ExprTreeNode {
private:
	const string name;
	vector<TreeNode *> args;
	int isProc;
public:
	CallExprTreeNode( const string _name,  vector<TreeNode *> _args , int _proc = 0)
						:name(_name), args(_args), isProc(_proc){}
	CallExprTreeNode( const string _name, int _proc = 0)
						:name(_name), isProc(_proc){}
	void printSelf() {
		ast << "BinaryExprTreeNode";
	}
	SymBucket *genCode(Symtab *symtab, int *reg = NULL );
	void genSysFunc(Symtab *symtab, string name);
	string typeCheck(Symtab *symtab);
};

class CaseExprTreeNode : public ExprTreeNode {
private:
	IDTreeNode *label;
	StmtTreeNode *stmt;
public:
	CaseExprTreeNode( TreeNode* _label,  TreeNode *_stmt)
					: label((IDTreeNode*)_label), stmt((StmtTreeNode*)_stmt){}
	void printSelf() {
		ast << "CaseExprTreeNode";
	}
	SymBucket *genCode(Symtab *symtab, int *reg);
	string typeCheck(Symtab *symtab);
};


//======================================================
///function and procedure node
/*
* node for function definition
*/
class FunctionTreeNode : public TreeNode {
private:
	const string name;
	ListTreeNode* args;
	SimpleTypeTreeNode* returnType;
	ListTreeNode* body;
	Symtab* subTab;
	int isProc;
public:
	FunctionTreeNode( const string _name,  TreeNode* _args, 
		 TreeNode* _returnType,  TreeNode* _body, int _isProc = 0)
		:name(_name), args((ListTreeNode*)_args)
		, body((ListTreeNode*)_body)
		, isProc(_isProc)
		{
			if (_returnType != NULL)
				returnType = (SimpleTypeTreeNode*)_returnType;
			children.push_back(args);
			children.push_back(body);
		}
	void printSelf() {
		ast << "FunctionTreeNode";
	}	

	void setSubSymtab(Symtab *s) {
		subTab = s;
	}

	void updateSymtab(Symtab* symtab);	
	SymBucket *genCode(Symtab *symtab, int *reg = NULL );
};

/*
* node for procedure definition
*/
class ProcedureTreeNode :  public TreeNode {
private:
	const string name;
	ListTreeNode * args;
	ListTreeNode * body;
public:
	ProcedureTreeNode( const string _name,  TreeNode *  _args, 
		 TreeNode *  _body)
		:name(_name), args((ListTreeNode*)_args), body((ListTreeNode*)_body)
		{
			children.push_back(args);
			children.push_back(body);
		}
	void printSelf() {
		ast << "ProcedureTreeNode";
	}		
	void updateSymtab(Symtab* symtab);
};



//======================================================
///stmt node


class CompoundStmtTreeNode : public StmtTreeNode {
private:
	ListTreeNode *stmtList;
public:
	CompoundStmtTreeNode( TreeNode *list)
						:stmtList((ListTreeNode*)list)
	{
		children.push_back(stmtList);
	}
	void printSelf() {
		ast << "CompoundStmtTreeNode";
	}	

	SymBucket *genCode(Symtab *symtab, int *reg = NULL );
	string typeCheck(Symtab *symtab);

};

class IfStmtTreeNode : public StmtTreeNode {
private:
	ExprTreeNode *condition;
	CompoundStmtTreeNode *body;
	StmtTreeNode *elsePart;
public:
	IfStmtTreeNode( TreeNode *e,  TreeNode *c)
				:condition((ExprTreeNode*)e), body((CompoundStmtTreeNode*)c)
	{
		children.push_back(condition);
		children.push_back(body);
	}
	IfStmtTreeNode( TreeNode *e,  TreeNode *c,TreeNode *s)
				:condition((ExprTreeNode*)e), body((CompoundStmtTreeNode*)c),elsePart((StmtTreeNode*)s){
		children.push_back(condition);
		children.push_back(body);
		if (elsePart != NULL)
			children.push_back(elsePart);		
	}	
	void printSelf() {
		ast << "IfStmtTreeNode";
	}							
	SymBucket * genCode(Symtab *symtab, int *reg);
	string typeCheck(Symtab *symtab);
};

class RepeatStmtTreeNode : public StmtTreeNode {
private:
	CompoundStmtTreeNode *body;
	ExprTreeNode *condition;
public:
	RepeatStmtTreeNode( TreeNode *_body, TreeNode *_cond) 
					: body((CompoundStmtTreeNode*)_body), condition((ExprTreeNode*)_cond)
	{
		children.push_back(condition);
		children.push_back(body);
	}
	void printSelf() {
		ast << "RepeatStmtTreeNode";
	}
	string typeCheck(Symtab *symtab);
	SymBucket * genCode(Symtab *symtab, int *reg);
};

class WhileStmtTreeNode : public StmtTreeNode {
private:
	StmtTreeNode *body;
	ExprTreeNode *condition;
public:
	WhileStmtTreeNode( TreeNode* _body,  TreeNode *_cond)
					: body((StmtTreeNode*)_body),condition((ExprTreeNode*)_cond)
	{
		children.push_back(condition);
		children.push_back(body);
	}
	void printSelf() {
		ast << "WhileStmtTreeNode";
	}
	string typeCheck(Symtab *symtab);
	SymBucket *genCode(Symtab *symtab, int *reg = NULL );

};

/*
* switch stmt
*/
class SwitchStmtTreeNode : public StmtTreeNode {
private:
	TreeNode *expr;
	ListTreeNode *caseExprList;
public:
	SwitchStmtTreeNode( TreeNode *_expr,  TreeNode *_list)
				:expr(_expr), caseExprList((ListTreeNode*)_list)
	{
		children.push_back(expr);
		children.push_back(caseExprList);
	}
	void printSelf() {
		ast << "SwitchStmtTreeNode";
	}
	SymBucket * genCode(Symtab *symtab, int *reg);
	string typeCheck(Symtab *symtab);
};

class ForStmtTreeNode : public StmtTreeNode {
private:
	ExprTreeNode *assignExpr;
	const string direction; //to or downto
	TreeNode *dirExpr; // may be a variable
	StmtTreeNode *body;
public:
	ForStmtTreeNode( TreeNode * _aExpr, const string _dire, 
		 TreeNode *_dExpr,  TreeNode* _body)
		: assignExpr((ExprTreeNode*)_aExpr), direction(_dire), dirExpr((ExprTreeNode*)_dExpr), body((StmtTreeNode*)_body)
		{
			children.push_back(assignExpr);
			children.push_back(dirExpr);
			children.push_back(body);
		}
	void printSelf() {
		ast << "ForStmtTreeNode";
	}		
	SymBucket * genCode(Symtab *symtab, int *reg);
	string typeCheck(Symtab *symtab);
};


class GotoStmtTreeNode : public StmtTreeNode {
private:
	const string label;
public:
	GotoStmtTreeNode(const string _label):label(_label){}
	void printSelf() {
		ast << "GotoStmtTreeNode";
	}
	// SymBucket * genCode(Symtab *symtab, int *reg);
	// string typeCheck(Symtab *symtab);
};
/*
* if treceScan = TRUE, every token along with lineno will be 
* print in listing file
*/
extern int traceScan;

void printAST(TreeNode *root);
#endif

