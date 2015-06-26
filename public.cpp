#include <typeinfo>
#include <queue>
#include <algorithm>
#include <map>
#include "code_generator.h"
#include "public.h"
#include "symtab.h"
#include <cmath>

ofstream ast("result/AST");
ofstream code("result/CODE");
ofstream sym("result/SYM");
ofstream error("result/ERROR");
ofstream parserOut("result/parserOut");
LabelManager *labelManager;
map<string,string> constStringMap;
map<string,string> constRealMap;
RegManager *regManager;
int traceGenCode = TRUE;
int traceEmit = TRUE;
int isMain = TRUE;
extern Symtab* mainSymtab;

/////////////////////////////////////////////////////////////
// utils												   //
/////////////////////////////////////////////////////////////
void selectOP(SymBucket *bucket, int &reg, string &load, string &store, int &loc, int currentLevel = 0) {
	if (reg == -3 && bucket != NULL) {
		reg = -1;
		// loc = bucket->getOffsetReg();
		// 表明 loc是一个寄存器号，array和record的情形
		loc = bucket->getLoc();
		load = "load_reg";
		store = "store_reg";
		return;
	} else {
		load = "load";
		store = "store";
	}

	if (reg == -1 && bucket != NULL) {
		loc = bucket->getLoc();
		if (bucket->getIsRef()) {
			cout <<  bucket->getName() << " : copy by referance" << endl;
			load += "_ref";
			store += "_ref";
		}
		cout << "in select " << bucket->getCurSymtab() << endl;
		int level = bucket->getCurSymtab()->getLevel();
		if (level < currentLevel) {
			char ch[5] = {0,};
			sprintf(ch, "%d", currentLevel - level);
			load = load +  "-" + ch;
			store = store + "-" +  ch;
		}
	}

	cout << "out select" <<endl;
}

void traceGen(string msg) {
	if (traceGenCode)
		cout << msg << endl;
}


string intTostring(int x){
	char t[256];
    string s;
 
    sprintf(t, "%d", x);
    s = t;
    return s;
}

bool isTmpReg(int r) {
	return r >=8 && r <=15;
}

void autoFreeReg(int beFree, int* contrain) {
	if (isTmpReg(beFree)) {
		if (contrain != NULL && *contrain != beFree) {
			regManager->freeReg(beFree);
		}
	}
}

int isFloatExpr(SymBucket* b, int reg) {
	if ( (b != NULL && b->getType() == "real") || reg > 31) 
		return 1;
	else return 0;
}

void printAST(TreeNode *root) {
	queue<TreeNode *> q;
	int curLevel = 0;
	int nextLevel = 0;
	q.push(root);
	curLevel = 1;
	while(!q.empty()) {
		TreeNode *t = q.front();
		q.pop();
		curLevel--;
		ast << " ";
		t->printSelf();
		ast << " ";
	
		vector<TreeNode *> children = t->getChildren();
		for(int i = 0; i < children.size(); i++) {
			nextLevel++;
			q.push(children[i]);
		}
		if (curLevel == 0) {
			ast << endl;
			curLevel = nextLevel;
			nextLevel = 0;
		}
	}
}

int getSize(const string& type) {
	int size;
	if (type == "integer") 
		size = 4;
	else if (type == "real") 
		size = 4;
	else if (type == "char")
		size = 1;
	else if (type == "string") 
		size = 4;
	return size;
}



////////////////////////////////////////////////////////////
// utils functions										  //
////////////////////////////////////////////////////////////
void childrenGenCode(vector<TreeNode*>& children, Symtab *symtab ,int *reg) {
	cout << children.size() << endl;
	for(int i = 0; i < children.size(); i++) {
		children[i]->genCode(symtab,reg);
	}
}

void childrenUpdateSymtab(vector<TreeNode*>& children, Symtab *symtab) {
	for(int i = 0; i < children.size(); i++) 
		children[i]->updateSymtab(symtab);
}

string childrenTypeCheck(vector<TreeNode*>& children, Symtab *symtab) {
	string s;
	s = "success";
	for(int i = 0; i < children.size(); i++) {
		if((children[i]->typeCheck(symtab))=="failure") {
			s = "failure";
		}
	}
	return s;
}




////////////////////////////////////////////////////////////
// update symtab functions								  //
////////////////////////////////////////////////////////////

void ProgramTreeNode::updateSymtab(Symtab *symtab) {
	cout << "pu" << endl;
	env = symtab;
	routine->updateSymtab(symtab);	
}

void RoutineTreeNode::updateSymtab(Symtab* symtab) {
	cout << "ru" << endl;
	env = symtab;
	head->updateSymtab(symtab);
}	

void RoutineHeadTreeNode::updateSymtab(Symtab *symtab) {
	cout << "rhu" << endl;
	env = symtab;
	childrenUpdateSymtab(children, symtab);
}

void ListTreeNode::updateSymtab(Symtab *symtab) {
	env = symtab;
	cout << typeName << " lu " << endl;
	childrenUpdateSymtab(list, symtab);	
}

void ConstTreeNode::updateSymtab(Symtab *symtab) {
	traceGen("cu");
	env = symtab;
	const string type = value->getType();
	SymBucket *b = new SymBucket(name, lineNO, type, symtab);
	b->setIsConst(1);
	int size = getSize(type);
	b->setSize(size);
	b->setLoc(symtab->genLoc(size));
	symtab->insert(b);

}
// in var part
void VariableTreeNode::updateSymtab(Symtab *symtab) {
	traceGen("vu: " + name);
	env = symtab;
	vector<TreeNode*>& list = nameList->getList();
	if (name != "") {
		list.push_back(new TreeNode(name));
	}
	for(int i = 0; i < list.size(); i++) {
		SymBucket *b = typeNode->genSymItem(list[i]->getName(), symtab);
		b->setIsType(0);
		string type = b->getType();
		b->setCurSymtab(symtab);
		if ((type.find("integer") != string::npos || type.find("real") != string::npos 
			|| type.find("char")!=string::npos) && !symtab->isRegSpill()) {

			int reg = symtab->genRegNum();
			b->setLoc(symtab->genLoc(b->getSize()));
			b->setRegNum(reg);			
		} else {
			SymBucket *tmpBucket;
			// need to gencode to allocate memory in stack for these variables.

			if (type.find("record") != string::npos) {
				SymBucket *member = b->next;
				b->setLoc(symtab->genLoc(0));
				do {
					member->setLoc(env->genLoc(member->getSize()));
					member = member->last->next;
				} while(member != b);
				// 为了对齐
				symtab->genLoc(b->getSize()+b->getLoc()-symtab->getCurLoc());
			} else {
				//cout << "in vu" << b->getName() << " size is " << b->getSize() << endl;
				b->setLoc(symtab->genLoc(b->getSize()));
			}
		}
		if (getIsVar()) {
		 	b->setIsVar(1);
		}
		symtab->insert(b);
	}
}
//////// 对于传引用的方法，可以改成直接放具体的地址，而不是偏移
////////
void FunctionTreeNode::updateSymtab(Symtab *symtab) {
	cout << "fu" << endl;
	env = symtab;
	// new a bucket for function
	SymBucket *bucket = new SymBucket(name, lineNO, !isProc ? "func" : "proc", symtab);
	bucket->setSize(0);
	Symtab *subSymtab = new Symtab(name+"-subSymtab", bucket);
	subSymtab->setLevel(symtab->getLevel() + 1);
	subSymtab->setParentBucket(bucket);
	bucket->setSubSymtab(subSymtab);
	args->updateSymtab(subSymtab);
	// copy args into the function bucket
	vector<SymBucket *> v;
	subSymtab->getSymBucketList(v);
	SymBucket *tmpBucket = bucket;
	subSymtab->setCurRegNum(symtab->getCurReg());
	subSymtab->setCurLoc(0);
	for (int  i = 0; i < v.size(); i++) {
		cout << name << " : " << v[i]->getName() << endl;
		if (!v[i]->getIsRef())
			v[i]->setLoc(subSymtab->genLoc(v[i]->getSize()));
		else v[i]->setLoc(subSymtab->genLoc(4));
		v[i]->setRegNum(-1);
		if (v[i]->getIsVar()) {
			v[i]->setIsRef(1);
		}
		// here we only need the first node of arguments' type
		SymBucket *newBucket = new SymBucket(v[i]);
		tmpBucket->next = newBucket;
		tmpBucket = newBucket;
	}
	// set subsymtab for this node
	body->updateSymtab(subSymtab);
	subTab = subSymtab;
	if (!isProc) {
		cout << "it is not proc!!!!!" << endl;
		SymBucket *returnTypeBucket = returnType->genSymItem("returnType", symtab);
		bucket->last = returnTypeBucket->last;
		bucket->last->next = bucket;
		tmpBucket->next = returnTypeBucket;
		symtab->insert(bucket);
		// insert return node
		SymBucket *returnNameBucket = new SymBucket(returnTypeBucket);
		returnNameBucket->setCurSymtab(subSymtab);
		returnNameBucket->setName(name);
		// set return reg num
		returnNameBucket->setRegNum(V1);
		subSymtab->insert(returnNameBucket);
	} else {
		bucket->last = tmpBucket;		
		tmpBucket->next = bucket;
		symtab->insert(bucket);
	}
}

// in type part
void CustomTypeTreeNode::updateSymtab(Symtab *symtab) {
	env = symtab;
	SymBucket *b = type->genSymItem(name, symtab);
	b->setIsType(1);
	symtab->insert(b);
}


	

////////////////////////////////////////////////////////////
// gen code functions 									  //
////////////////////////////////////////////////////////////


SymBucket * ProgramTreeNode::genCode(Symtab *symtab, int *reg) {
	traceGen("pg");
	code << ".data" << endl;
	for(map<string,string>::iterator iter = constStringMap.begin();iter != constStringMap.end(); iter++) {
		string constStr = iter->first;
		string label    = iter->second;
		CodeGenerator::emitCodeConstStr(constStr, label);
	}
	for(map<string,string>::iterator iter = constRealMap.begin();iter != constRealMap.end(); iter++) {
		string constStr = iter->first;
		string label    = iter->second;
		CodeGenerator::emitCodeConstReal(constStr, label);
	}
	code << ".text" << endl;
	routine->genCode(env);
	
	CodeGenerator::addLabel("exit");
	return NULL;
}

SymBucket * RoutineTreeNode::genCode(Symtab *symtab, int *reg) {
	traceGen("rg");
	env = symtab;
	vector<SymBucket *> bucketList;
	symtab->getSymBucketList(bucketList);
	int totalStackSize = 0;
	for (int i = 0; i < bucketList.size(); i++) {
		SymBucket *b = bucketList[i];
		cout << b->getName() << "'s size is "  << b->getSize() << endl;
		if (b->getRegNum() == -1) {
			totalStackSize += b->getSize();
		}
	}
	if (isMain) {
		CodeGenerator::emitCodeR("+", FP,SP,0);
	}
	if (totalStackSize > 0) {
		CodeGenerator::emitCodeI("-", SP,SP,totalStackSize);
	}
	head->getConstPart()->genCode(env);
	body->genCode(env);
	if (totalStackSize > 0) 
			CodeGenerator::emitCodeI("+", SP,SP,totalStackSize);
	if (!isMain) {
		CodeGenerator::emitCodeJ("jr", 31, 0, 0, "");
	} else {
		CodeGenerator::emitCodeJ("j", 31, 0, 0, "exit");
	}
	if (isMain) {
		isMain = FALSE;
	}
	head->genCode(env);
	return NULL;
}

SymBucket * ListTreeNode::genCode(Symtab *symtab, int *reg) {
	traceGen("lg");
	childrenGenCode(list, symtab ,reg);
	return 0;
}

SymBucket * ConstTreeNode::genCode(Symtab *symtab, int *reg) {
	traceGen("cg");
	if (reg != NULL) *reg = -1;
	env = symtab;
	SymBucket *b = env->find(name);
	if (!getIsFirst()) {
		SymBucket * returnBucket = new SymBucket(b);
		returnBucket->setCurSymtab(b->getCurSymtab());
		return returnBucket;
	} else {
		setIsFirst(0);
		VariableTreeNode* lhs = new VariableTreeNode(name);
		BinaryExprTreeNode * b = new BinaryExprTreeNode("=", lhs, value);
		b->genCode(symtab);
		regManager->freeAll();
	}
}

SymBucket * VariableTreeNode::genCode(Symtab *symtab, int *reg) {
	//traceGen("vg : " + name + " env" + symtab);
	cout << "vg : " <<  name <<  " env : " << symtab << endl;
	env = symtab;
	SymBucket *b = env->find(name);
	cout << "b = " << b << endl;
	if (b != NULL) {
		if (reg != NULL) *reg = b->getRegNum();
	} else {
		cout << lineNO << ": variable " << name << " is not defined" << endl;
	}
	SymBucket * returnBucket = new SymBucket(b);
	returnBucket->setCurSymtab(b->getCurSymtab());
	return returnBucket;
}

// const var parts will be processed later
SymBucket * RoutineHeadTreeNode::genCode(Symtab *symtab, int *reg) {
	routinePart->genCode(symtab);
	return 0;
}

SymBucket * CompoundStmtTreeNode::genCode(Symtab *symtab, int *reg) {
	stmtList->genCode(symtab);
	return NULL;
}

SymBucket * FunctionTreeNode::genCode(Symtab *symtab, int *reg) {
	traceGen("funtion gencode");
	string label = name;
	//labelManager->addFuncLabel(label);
	CodeGenerator::addLabel(label);
	body->genCode(subTab);
	return NULL;
}

// expression will only return 
//
SymBucket * BinaryExprTreeNode::genCode(Symtab *symtab, int *reg) {
	traceGen("bg : " + op);
	env = symtab;
	int regR, regL;
	int locR = 0, locL = 0;
	// for arrayElem and recordElem
	SymBucket *bucketR, *bucketL;
	bucketR = rhs->genCode(env, &regR);
	bucketL = lhs->genCode(env, &regL);

	int isFloat = isFloatExpr(bucketR, regR) && isFloatExpr(bucketL, regL);
	SymBucket *returnBucket;
	if (bucketL != NULL)
		returnBucket = new SymBucket(bucketL);
	else 
		returnBucket = NULL;
	string loadOPR, storeOPR;
	string loadOPL, storeOPL;
	selectOP(bucketR, regR, loadOPR, storeOPR, locR, symtab->getLevel());
	selectOP(bucketL, regL, loadOPL, storeOPL, locL, symtab->getLevel());
	cout << "regL : " << regL << " regR " << regR << endl;
	if (regL == -1 && regR == -1) {		
		int tmpSrc_1 = regManager->getTmpReg(isFloat);
		int tmpSrc_2 = regManager->getTmpReg(isFloat);
		int tmpDst = regManager->getTmpReg(isFloat);
		if (op == "=") {
			if (bucketR->getSize() > 4)  {
				if (bucketL->getSize() == bucketR->getSize()) {
					CodeGenerator::emitCodeB(loadOPR,storeOPL, bucketR->getSize(), locL, locR,FP,4);
				} else {
					cout << lineNO << ": error two record has different size" << endl;
					exit(-1);
				}
			} else {
				CodeGenerator::emitCodeM(bucketR->getSize(),loadOPR, locR, FP, tmpSrc_2, isFloat);			
				CodeGenerator::emitCodeM(bucketL->getSize(),storeOPL, locL, FP, tmpSrc_2, isFloat);
				if (reg != NULL) *reg = tmpSrc_2;
			}
		} else {
			CodeGenerator::emitCodeM(bucketL->getSize(),loadOPL, locL, FP, tmpSrc_1, isFloat);
			CodeGenerator::emitCodeM(bucketR->getSize(),loadOPR, locR, FP, tmpSrc_2, isFloat);
			CodeGenerator::emitCodeR(op, tmpDst, tmpSrc_1, tmpSrc_2, isFloat);
			if (reg != NULL) *reg = tmpDst;
		}
		regManager->freeReg(tmpSrc_1);
		regManager->freeReg(tmpSrc_2);
		autoFreeReg(tmpDst, reg);
	} else if (regL > 0 && regR > 0) {
		cout << "L=" << regL << " R = " << regR <<endl;
		if (op == "=") {
			CodeGenerator::emitCodeR(op, regL, regR, 0, isFloat);
			if (reg != NULL) *reg = regL;
		} else {
			int tmpReg = regManager->getTmpReg(isFloat);
			CodeGenerator::emitCodeR(op, tmpReg, regR, regL, isFloat);
			if (reg != NULL) *reg = tmpReg;
			autoFreeReg(tmpReg, reg);
		}
		regManager->freeReg(regL);
		regManager->freeReg(regR);
	} else if (regL > 0 && regR == -1) {
		int tmpReg = regManager->getTmpReg(isFloat);
		CodeGenerator::emitCodeM(bucketR->getSize(),loadOPR, locR, FP, tmpReg, isFloat);
		if (op == "=") {
			CodeGenerator::emitCodeR(op, regL, tmpReg, 0, isFloat);
			if (reg != NULL) *reg = regL;
			
		} else {
			int tmpDst = regManager->getTmpReg(isFloat);
			CodeGenerator::emitCodeR(op, tmpDst, regL, tmpReg, isFloat);
			if (reg != NULL) *reg = tmpDst;
		}
		regManager->freeReg(tmpReg);
	} else if (regL == -1 && regR > 0) {
		if (op == "=") {
			cout << "ltype = " << bucketL->getType() << endl;
			cout << "rtype = " << bucketR->getType() << endl;
			if (bucketL->getType() == "char" && bucketR->getType() == "string"){
				int tmpReg = regManager->getTmpReg();
				CodeGenerator::emitCodeM(getSize("char"),loadOPR, 0,regR, tmpReg);
				CodeGenerator::emitCodeM(getSize("char"),storeOPL, locL,FP, tmpReg);
			} else {
				CodeGenerator::emitCodeM(bucketL->getSize(),storeOPL, locL, FP, regR, isFloat);
			}
			if (reg != NULL) *reg = regR;
		} else {
			int tmpSrc = regManager->getTmpReg(isFloat);
			int tmpDst = regManager->getTmpReg(isFloat);
			CodeGenerator::emitCodeM(bucketL->getSize(),loadOPL, locL, FP, tmpSrc, isFloat);
			CodeGenerator::emitCodeR(op, tmpDst, tmpSrc, regR, isFloat);
			regManager->freeReg(tmpSrc);
			if (reg != NULL) *reg = tmpDst;
			autoFreeReg(tmpDst, reg);
		}
		regManager->freeReg(regR);
	} else if (regL == -2 || regR == -2) {
		int tmpDst = regManager->getTmpReg(isFloat);
		int tmpSrc = regManager->getTmpReg(isFloat);
		cout << "br = " << bucketR << " bl = " << bucketL << endl;
		int immeR, immeL;
		if (bucketR != NULL) 
		  	immeR = bucketR->getIntImme();
		if (bucketL != NULL) 
		 	immeL = bucketL->getIntImme();

		cout << "immeR = " << immeR << " immeL = " << immeL << endl;
		if (regL == -2 && regR == -2) {
			CodeGenerator::emitCodeI(op, tmpDst, 0, immeR);
			CodeGenerator::emitCodeI(op, tmpDst, 0, immeL);
			if (reg != NULL) *reg = tmpDst;
		} else if (regL == -2 && regR > 0) {
			CodeGenerator::emitCodeI(op, tmpDst, regR, immeL);
			if (reg != NULL) *reg = tmpDst;
		} else if (regL == -2 && regR == -1) {
			CodeGenerator::emitCodeM(bucketR->getSize(),loadOPR, locR, FP, tmpSrc, isFloat);
			CodeGenerator::emitCodeI(op, tmpDst, tmpSrc, immeL);
			if (reg != NULL) *reg = tmpDst;
		} else if (regL > 0 && regR == -2) {
			if (op == "=") {
				CodeGenerator::emitCodeI(op, regL, 0, immeR);
				if (reg != NULL) *reg = regL;
			} else {
				CodeGenerator::emitCodeI(op, tmpDst, regL, immeR);
				if (reg != NULL) *reg = tmpDst;
			}
		} else if (regL == -1 && regR == -2) {
			if (op == "=") {
				CodeGenerator::emitCodeI(op, tmpDst, 0, immeR);
				CodeGenerator::emitCodeM(bucketL->getSize(),storeOPL, locL, FP, tmpDst, isFloat);
				if (reg != NULL) *reg = tmpDst;
			} else {
				CodeGenerator::emitCodeM(bucketL->getSize(),loadOPL, locL, FP, tmpSrc, isFloat);
				CodeGenerator::emitCodeI(op, tmpDst, tmpSrc, immeR);
				if (reg != NULL) *reg = tmpDst;
			}
		}
		regManager->freeReg(tmpSrc);
		autoFreeReg(tmpDst, reg);
	}
	if (bucketL != NULL) delete bucketL;
	if (bucketR != NULL) delete bucketR;
	autoFreeReg(regL, reg);
	autoFreeReg(regR, reg);
	return returnBucket;
}

SymBucket * WhileStmtTreeNode::genCode(Symtab *symtab, int *reg) {
	SymBucket *bucketR, *bucketL;
	int regL, regR;
	int locL, LocR;
	string loadOPR, storeOPR;
	string loadOPL, storeOPL;
	int x;
	x=labelManager->getLoopLabel();
	char ch[16] = {0,};
	sprintf(ch,"%d",x);
	string loop = "loop";
	loop = loop + ch;
	string breakn = "break";
	breakn = breakn + ch;
	labelManager->addLoopLabel();
	CodeGenerator::addLabel(loop);
	bucketL = condition->genCode(symtab, &regL);
	selectOP(bucketL,regL,loadOPL,storeOPL,locL, symtab->getLevel());
	if(regL == -1){
		int tmp = regManager->getTmpReg();
		CodeGenerator::emitCodeM(bucketL->getSize(),loadOPL, locL, FP, tmp);
		CodeGenerator::emitCodeJ("beq",tmp,0,0,breakn);
		regManager->freeReg(tmp);
	}
	else {
		CodeGenerator::emitCodeJ("beq",regL,0,0,breakn);
	}
	body->genCode(symtab, &regR);
	CodeGenerator::emitCodeJ("j",0,0,0,loop);
	CodeGenerator::addLabel(breakn);
	if (bucketL != NULL) delete bucketL;
	return NULL;
}

SymBucket * ForStmtTreeNode::genCode(Symtab *symtab, int *reg){
	SymBucket *bucketR, *bucketL;
	int regL, regR ,regB;
	int locL, locR;
	string loadOPR, storeOPR;
	string loadOPL, storeOPL;
	int x;
	x=labelManager->getLoopLabel();
	char ch[16] = {0,};
	sprintf(ch,"%d",x);
	string loop = "loop";
	loop = loop + ch;
	string breakn = "break";
	breakn = breakn + ch;
	labelManager->addLoopLabel();
	bucketL = assignExpr->genCode(symtab, &regL);
	regManager->freeReg(regL);
	// get the id
	SymBucket* idBucket = ((BinaryExprTreeNode*)assignExpr)->getLhs()->genCode(symtab);
	// begin loop
	CodeGenerator::addLabel(loop);
	int regID = regManager->getTmpReg();
	// load id
	CodeGenerator::emitCodeM(idBucket->getSize(),"load", idBucket->getLoc(), FP, regID);
	bucketR = dirExpr->genCode(symtab,&regR);
	selectOP(bucketR,regR,loadOPR,storeOPR,locR, symtab->getLevel());
	int comp = regManager->getTmpReg();
	if(regR == -1){
		int tmp = regManager->getTmpReg();
		CodeGenerator::emitCodeM(bucketR->getSize(),loadOPR, locR, FP, tmp);
		CodeGenerator::emitCodeR("<=",comp,regID,tmp);
		CodeGenerator::emitCodeJ("beq",comp,0,0,breakn);
		regManager->freeReg(tmp);
	}
	else if(regR == -2){
		int imm = bucketR->getIntImme();
		CodeGenerator::emitCodeI("<=",comp,regID,imm);
		CodeGenerator::emitCodeJ("beq",comp,0,0,breakn);
	}
	else {
		CodeGenerator::emitCodeR("<=",comp,regID,regR);
		CodeGenerator::emitCodeJ("beq",comp,0,0,breakn);
	}
	body->genCode(symtab, &regB);
	
	int tmp = regManager->getTmpReg();
	
	CodeGenerator::emitCodeM(idBucket->getSize(),"load", idBucket->getLoc(), FP, tmp);
	if(direction == "to") {
		CodeGenerator::emitCodeI("+",tmp,tmp,1);
	}
	else {
		CodeGenerator::emitCodeI("-",tmp,tmp,1);
	}
	
	CodeGenerator::emitCodeM(idBucket->getSize(),"store", idBucket->getLoc(), FP, tmp);
	regManager->freeReg(tmp);
	CodeGenerator::emitCodeJ("j",0,0,0,loop);
	CodeGenerator::addLabel(breakn);
	regManager->freeReg(regR);
	regManager->freeReg(regB);
	regManager->freeReg(regID);
	if (bucketL != NULL) delete bucketL;
	if (bucketR != NULL) delete bucketR;
	return NULL;
}

SymBucket * SwitchStmtTreeNode::genCode(Symtab *symtab, int *reg){
	cout << "sg" << endl;
	SymBucket *bucketE, *bucketLD;
	int regE;	
	string loadOPE, storeOPE;
	int locE;
	cout << expr << endl;
	bucketE = expr->genCode(symtab,&regE);
	selectOP(bucketE,regE,loadOPE,storeOPE,locE, symtab->getLevel());
	if(regE == -1){
		int tmp = regManager->getTmpReg();
		if (bucketE != NULL && bucketE->getType() == "string") {
			CodeGenerator::emitCodeM(4,loadOPE, locE, FP, tmp);
			CodeGenerator::emitCodeM(getSize("char"),"load", 0, tmp, tmp);
		} else {
			CodeGenerator::emitCodeM(bucketE->getSize(),loadOPE, locE, FP, tmp);
		}
		bucketLD = caseExprList->genCode(symtab,&tmp);
		regManager->freeReg(tmp);
	}
	else if (regE > 0) {
		if (bucketE != NULL && bucketE->getType() == "string") {
			CodeGenerator::emitCodeM(getSize("char"),"load", 0, regE, regE);
		}
		bucketLD = caseExprList->genCode(symtab,&regE);
	}
	regManager->freeReg(regE);
	if (bucketE != NULL) delete bucketE;
	return NULL;
}

SymBucket * CaseExprTreeNode::genCode(Symtab *symtab, int *reg){

	SymBucket *bucketID;
	int regID,regE;
	int locID;
	string loadOPID, storeOPID;
	string s;
	int x;
	
	regE = *reg;

	bucketID = label->genCode(symtab,&regID);
	
	selectOP(bucketID,regID,loadOPID,storeOPID,locID, symtab->getLevel());
	if(regID == -1) {
		int tmp = regManager->getTmpReg();
		CodeGenerator::emitCodeM(bucketID ->getSize(),loadOPID, locID, FP, tmp);
		x = labelManager->getCaseLabel();
		s = "nextcase" + intTostring(x);
		CodeGenerator::emitCodeJ("bne",tmp,regE,0,s); 
		labelManager->addCaseLabel();
		regManager->freeReg(tmp);
	}
	else if (regID > 0) {
		x = labelManager->getCaseLabel();
		s = "nextcase" + intTostring(x);
		// 假定上面的switch的就是char
		if (bucketID->getType() == "string") {
			cout << "switch string !!!!!!" << endl;
			CodeGenerator::emitCodeM(getSize("char"), "load", 0, regID, regID);
		}
		CodeGenerator::emitCodeJ("bne",regID,regE,0,s); 
		labelManager->addCaseLabel();
	}
	else if (regID == -2) {
		x = labelManager->getCaseLabel();
		s = "nextcase" + intTostring(x);
		CodeGenerator::emitCodeJ("bne", regE, 32, bucketID->getIntImme(),s);
		labelManager->addCaseLabel();
	}
	regManager->freeReg(regID);
	stmt->genCode(symtab);
	CodeGenerator::addLabel(s);
	if (bucketID != NULL ) delete bucketID;
}

SymBucket * IfStmtTreeNode::genCode(Symtab *symtab, int *reg){
	SymBucket *bucketC;
	int regC,regB,regE;
	int locC;
	
	string loadOPC,storeOPC;
	bucketC = condition->genCode(symtab,&regC);
	selectOP(bucketC,regC,loadOPC,storeOPC,locC, symtab->getLevel());
	string s;
	s = "endif" + intTostring(labelManager->getIfLabel());
	string t;
	t = "endelse" + intTostring(labelManager->getIfLabel());
	if(regC == -1) {
		int tmp = regManager->getTmpReg();
		CodeGenerator::emitCodeM(bucketC ->getSize(),loadOPC, locC, FP, tmp);
		CodeGenerator::emitCodeJ("beq",tmp,0,0,s); 
		regManager->freeReg(tmp);
	}
	else if(regC > 0) {
		CodeGenerator::emitCodeJ("beq",regC,0,0,s); 
	} else {
		int tmp = regManager->getTmpReg();
		CodeGenerator::emitCodeI("+", tmp, 0, bucketC->getIntImme());
		CodeGenerator::emitCodeJ("beq",tmp,0,0,s); 
		regManager->freeReg(tmp);
	}
	body->genCode(symtab,&regB);
	labelManager->addIfLabel();
	if(elsePart!=NULL){
		CodeGenerator::emitCodeJ("j",0,0,0,t); 
		CodeGenerator::addLabel(s);
		elsePart->genCode(symtab,&regE);
		CodeGenerator::addLabel(t);
	}
	else{
		CodeGenerator::addLabel(s);
	}
	regManager->freeReg(regC);
	return NULL;
}

SymBucket * RepeatStmtTreeNode::genCode(Symtab *symtab, int *reg){
	SymBucket *bucketB,*bucketC;
	int regB,regC;
	int locC;
	string loadOPC,storeOPC;
	string s;
	s = "do" + intTostring(labelManager->getRepeatLabel());
	labelManager->addRepeatLabel();
	CodeGenerator::addLabel(s);
	bucketB = body->genCode(symtab,&regB);
	bucketC = condition->genCode(symtab,&regC);
	selectOP(bucketC,regC,loadOPC,storeOPC,locC, symtab->getLevel());
	if(regC == -1){
		int tmp = regManager->getTmpReg();
		CodeGenerator::emitCodeM(bucketC->getSize(),loadOPC, locC, FP, tmp);
		CodeGenerator::emitCodeJ("beq",tmp,0,0,s); 
		regManager->freeReg(tmp);
	}
	else if(regC > 0){
		CodeGenerator::emitCodeJ("beq",regC,0,0,s); 
	}
	else{
		int tmp = regManager->getTmpReg();
		CodeGenerator::emitCodeI("+", tmp, 0, bucketC->getIntImme());
		CodeGenerator::emitCodeJ("beq",tmp,0,0,s); 
		regManager->freeReg(tmp);
	}
	regManager->freeReg(regC);
	return NULL;
}

SymBucket * ArrayElemTreeNode::genCode(Symtab *symtab, int *reg) {
	env = symtab;
	SymBucket *type = env->find(name);
	SymBucket *elemType = type->next->last->next;
	SymBucket *returnBucket = new SymBucket("arrayElem", lineNO,  elemType->getType(), symtab );
	int elemSize = elemType->getSize();
	int exprReg;
	// the stack offset of the index
	int indexLoc; 
	string loadOP, storeOP;
	SymBucket *indexBucket = index->genCode(symtab, &exprReg);
	selectOP(indexBucket, exprReg, loadOP, storeOP, indexLoc, symtab->getLevel());
	if (exprReg == -2) {
		int offset = 0;
		offset = type->getLoc() + elemSize * (indexBucket->getIntImme());
		returnBucket->setLoc(offset);
		if (reg != NULL) *reg = -1;
	} else if (exprReg == -1) { // stack
		int tmpSrc_1 = regManager->getTmpReg();
		int tmpSrc_2 = regManager->getTmpReg();
		int tmpDst   = regManager->getTmpReg();
		CodeGenerator::emitCodeM(indexBucket->getSize(), "load", indexLoc, FP,tmpSrc_1);
		CodeGenerator::emitCodeI("+", tmpSrc_2, 0, elemSize);
		CodeGenerator::emitCodeR("*", tmpDst, tmpSrc_1, tmpSrc_2);
		CodeGenerator::emitCodeI("+", tmpDst, tmpDst, type->getLoc());
		if (reg != NULL) *reg = -3;
		//returnBucket->setOffsetReg(tmpDst);
		returnBucket->setLoc(tmpDst);
		regManager->freeReg(tmpSrc_1);
		regManager->freeReg(tmpSrc_2);
	} else if (exprReg > 0) {	// reg
		int tmpDst = regManager->getTmpReg();
		int tmpSrc = regManager->getTmpReg();
		CodeGenerator::emitCodeI("+", tmpSrc, 0, elemSize);
		CodeGenerator::emitCodeR("*", tmpDst, tmpSrc, exprReg);
		CodeGenerator::emitCodeI("+", tmpDst, tmpDst, type->getLoc());
		if (reg != NULL) *reg = -3;
		//returnBucket->setOffsetReg(tmpDst);
		returnBucket->setLoc(tmpDst);
		regManager->freeReg(tmpSrc);
	}
	returnBucket->setSize(elemSize);
	autoFreeReg(exprReg, reg);
	return returnBucket;
}




// SymBucket * GotoStmtTreeNode::genCode(Symtab *symtab, int *reg){

// }

SymBucket * RecordElemTreeNode::genCode(Symtab *symtab, int *reg) {
	env = symtab;
	SymBucket *returnBucket = NULL;
	SymBucket *bucket = env->find(recordName);
	SymBucket *member = bucket->next;
	do {
		if (member->getName() == elemName) {
			returnBucket = new SymBucket("recordElem", lineNO, member->getType(),symtab);
			returnBucket->setLoc(member->getLoc());
			returnBucket->setSize(member->getSize());
			break;
		}
		member = member->last->next;
	} while (member != bucket);
	if (reg != NULL) *reg = -1;
	if (member == bucket) cout << lineNO << " : undefined record member :" << elemName << " in "  << recordName << endl;
	return returnBucket;
}

SymBucket * UnaryExprTreeNode::genCode(Symtab *symtab, int *reg) {
	env = symtab;
	
	int operandReg;
	string loadOP, storeOP;
	int operandLoc;
	SymBucket *operandBucket = operand->genCode(symtab, &operandReg);
	SymBucket *returnBucket = new SymBucket("unaryExpr", lineNO, operandBucket->getType(), symtab);
	selectOP(operandBucket, operandReg, loadOP, storeOP, operandLoc, symtab->getLevel());
	cout << "in unaryExpr reg = " << operandReg  << " with op = " << op << endl;
	int tmpDst;
	tmpDst = regManager->getTmpReg();
	if (operandReg > 0) {
		CodeGenerator::emitCodeR(op, tmpDst, 0, operandReg);
		if (reg != NULL) *reg = tmpDst;
	} else if (operandReg == -1) {
		CodeGenerator::emitCodeM(operandBucket->getSize(), loadOP, operandBucket->getLoc(),FP,tmpDst);
		CodeGenerator::emitCodeR(op,tmpDst,0,tmpDst);
		if (reg != NULL) *reg = tmpDst;
	} else if (operandReg == -2) {
		int imme = operandBucket->getIntImme();
		cout << "in unaryExpr imme = " << imme << endl;
		if (op == "-") 
			imme = -imme;
		else if (op == "~") 
			imme = ~imme;
		CodeGenerator::emitCodeI("=", tmpDst, 0, imme);
		if (reg != NULL) *reg = tmpDst;
	}
	if (operandBucket != NULL) delete operandBucket;
	autoFreeReg(operandReg, reg);
	return returnBucket;
}

void CallExprTreeNode::genSysFunc(Symtab *symtab, string name) {
	if (args.size() > 1 || args.size() == 0) {
		cout << lineNO << " writeln's argument num is incorrect " << endl;
	}
	int argReg;
	SymBucket * argBucket = args[0]->genCode(symtab, &argReg);
	string type = argBucket->getType();
	string loadOP, storeOP;
	int loc;
	selectOP(argBucket,argReg, loadOP, storeOP, loc, symtab->getLevel());
	if (type == "string") {
		if (argReg > 0) {
			CodeGenerator::emitCodeR("+", A0, argReg, 0);
		} else if (argReg == -1) {
			CodeGenerator::emitCodeM(getSize("string"), loadOP, loc, FP, A0);
		}
		CodeGenerator::emitSysCall("printString");
	} else if (type == "integer") {
		if (argReg > 0) {
			CodeGenerator::emitCodeR("+", A0, argReg, 0);
		} else if (argReg == -1) {
			CodeGenerator::emitCodeM(getSize("integer"), loadOP, loc, FP, A0);
		} else if (argReg == -2) {
			int imme = argBucket->getIntImme();
			CodeGenerator::emitCodeI("+", A0, 0, imme);
		}
		CodeGenerator::emitSysCall("printInteger");
	} else if (type == "real" ){
		if (argReg > 0) {
			CodeGenerator::emitCodeR("+",F12 , argReg, 0, 1);
		} else if (argReg == -1) {
			CodeGenerator::emitCodeM(getSize("real"), loadOP, loc, FP, F12, 1);
		}
		CodeGenerator::emitSysCall("printReal");
	} else if (type == "char") {
		if (argReg == -1) {
			CodeGenerator::emitCodeM(getSize("char"), loadOP, loc, FP, A0);
		}
		CodeGenerator::emitSysCall("printChar");
	}
	regManager->freeReg(argReg);
	if (argBucket != NULL) delete argBucket;
}
// we do not support passing a var type argment of one function 
// to another function's var type argument
SymBucket * CallExprTreeNode::genCode(Symtab *symtab, int *reg) {
	cout << "in call " << name <<endl;
	if (name == "writeln" || name == "readln") {
		genSysFunc(symtab, name);
		return NULL;
	} 
	env = symtab;
	cout << name << endl;
	SymBucket *funcBucket = env->find(name);
	cout << funcBucket << endl;
	SymBucket *returnBucket;
	if (funcBucket->getType() == "func")
		returnBucket = new SymBucket("call_" + name, lineNO, funcBucket->last->getType() , symtab);
	else 
		returnBucket = new SymBucket("call_" + name, lineNO, "null" , symtab);
	vector<SymBucket *> argsTypeList;
	SymBucket *member = funcBucket->next;
	do {
		argsTypeList.push_back(funcBucket->getNextSymtab()->find(member->getName()));
		member = member->last->next;
	} while (member != funcBucket);
	// save the environment, such as save ra, v0, fp, and set access link
	CodeGenerator::emitCodeI("-", SP,SP,16);
	CodeGenerator::emitCodeM(4, "store", -16, SP, 31);
	CodeGenerator::emitCodeM(4, "store", -12,  SP, V1);
	CodeGenerator::emitCodeM(4, "store", -8,  SP, FP);
	CodeGenerator::emitGetAccessLink(funcBucket->getNextSymtab()->getLevel()-symtab->getLevel());

	// pass arguments
	int tmpDst;
	int tmp;
	for (int i = 0; i < args.size(); i++) {
		tmpDst = regManager->getTmpReg();
		SymBucket *argType = argsTypeList[i];
		int argReg, argLoc;
		string loadOP, storeOP;
		SymBucket *argBucket = args[i]->genCode(symtab, &argReg);
		int isElem = argReg == -3;
		selectOP(argBucket, argReg, loadOP, storeOP, argLoc, symtab->getLevel());
		if (argReg == -1) {
			if (argType->getIsRef()) {
				if (isElem) {
					CodeGenerator::emitCodeR("+", tmpDst, FP, argLoc);   // addr = old fp + offset, argLoc现在存放的就是放地址的寄存器号
				} else if (argBucket->getIsRef()) {
					CodeGenerator::emitCodeM(4, "store", argLoc, FP, tmpDst);
				} else {
					CodeGenerator::emitCodeI("+", tmpDst, FP, argLoc);   // addr = old fp + offset
				}
				CodeGenerator::emitCodeM(4, "store", argType->getLoc(), SP, tmpDst);
			} else {
				CodeGenerator::emitCodeM(argBucket->getSize(), loadOP, argLoc, FP, tmpDst);
				CodeGenerator::emitCodeM(argType->getSize(), "store", argType->getLoc(), SP, tmpDst);
			}
		} else if (argReg == -2) {
			int imme = argBucket->getIntImme();
			CodeGenerator::emitCodeI("+", tmpDst, 0, imme);
			CodeGenerator::emitCodeM(argType->getSize(), "store", argType->getLoc(), SP, tmpDst);
		} else if (argReg > 0) { 
			if (argType->getIsRef()) {  // depreted
				cout << "immediate expression result can not be copy by reference" << endl;
				// CodeGenerator::emitCodeM(4, "store", argType->getLoc(), FP, -argReg);
			} else {
				CodeGenerator::emitCodeM(argType->getSize(), "store", argType->getLoc(), SP, argReg);
			}
		}
		regManager->freeReg(tmpDst);
		if (argBucket != NULL) delete argBucket;
	}
	CodeGenerator::emitCodeI("+", FP, SP, 0);
	CodeGenerator::emitCodeJ("jal", 0,0,0,name);
	if (!isProc) {
		tmp = regManager->getTmpReg();
		CodeGenerator::emitCodeR("+",tmp, V1, 0);
		if (reg != NULL) *reg = tmp;
	} else {
		if (reg != NULL) *reg = -4;
	}
	CodeGenerator::emitCodeM(4, "load", -16, SP, 31); 	// ra
	CodeGenerator::emitCodeM(4, "load", -12, SP, 2);		// v0	
	CodeGenerator::emitCodeM(4, "load", -8, SP, FP);		
	CodeGenerator::emitCodeI("+", SP,SP,16);
	cout << "out call " <<endl;
	return returnBucket;
}
SymBucket *NumberTreeNode::genCode(Symtab *symtab, int *reg) {
	cout<<"ng"<<endl;
	SymBucket *returnBucket = new SymBucket("NumberTreeNode", lineNO, type, symtab);
	returnBucket->setSize(getSize(type));
	if (type == "string") {
		int tmp = regManager->getTmpReg();
		CodeGenerator::emitCodeLA(constStringMap[value], tmp);
		if (reg != NULL) *reg = tmp;
	} else if (type == "integer") {
		returnBucket->setSize(getSize(type));
		returnBucket->setIntImme(getInt());
		if (reg!=NULL) *reg = -2;
	} else if (type == "real") {
		int tmp = regManager->getTmpReg();
		CodeGenerator::emitCodeLA(constRealMap[value], tmp);
		int tmpDst = regManager->getTmpReg(1);
		CodeGenerator::emitCodeM(getSize("real"), "load", 0, tmp, tmpDst, 1);
		if (reg != NULL) *reg = tmpDst;
		regManager->freeReg(tmp);
	}
	return returnBucket;
}
////////////////////////////////////////////////////////////
// type gen symtab bucket functions						  //
////////////////////////////////////////////////////////////

// they only set size , the caller will bother setting loc

// type name is the l-side of type-definition, and type is the r-side
SymBucket* SysTypeTreeNode::genSymItem(const string typeName, Symtab *symtab) {
	traceGen("type name :"  + typeName);
	cout << " env : " << symtab << endl;
	SymBucket *b = new SymBucket(typeName, lineNO, type, symtab);
	int size = getSize(type);
	b->setSize(size);
	return b;
}



// array need to consider about index type of sub range ,
SymBucket *ArrayTypeTreeNode::genSymItem(const string typeName, Symtab *symtab) {
	traceGen("array node");
	SymBucket *array = new SymBucket(typeName, lineNO, "array", symtab);
	SymBucket *indexBucket = indexType->genSymItem("index", symtab);
	SymBucket *typeBucket = elemType->genSymItem("array", symtab);
	int len;
	if (indexBucket->getType() == "subrange") {
		int u, l;
		l = atoi(indexBucket->next->getType().c_str());
		u = atoi(indexBucket->last->getType().c_str());
		len = u - l;
	}
	else {
		len = pow(2,indexBucket->getSize()*8);
	}
	int size = len * typeBucket->getSize();
	array->setSize(size);
	array->next = indexBucket;
	indexBucket->last->next = typeBucket;
	typeBucket->last->next = array;
	array->last = typeBucket->last;
	return array;
}


SymBucket *SubRangeTypeTreeNode::genSymItem(const string typeName, Symtab *symtab) {
	char cu[16] = {0,};
	char cl[16] = {0,};
	int u = ((NumberTreeNode *)upperBound)->getInt();
	int l = ((NumberTreeNode *)lowerBound)->getInt();
	sprintf(cu,"%d",u);
	sprintf(cl,"%d",l);
	SymBucket *head  = new SymBucket(typeName, lineNO, "subrange", symtab);
	SymBucket *begin = new SymBucket("subrange", lineNO, cl, symtab);
	SymBucket *end 	 = new SymBucket("subrange", lineNO, cu, symtab);
	head->setSize(getSize("integer"));
	begin->setSize(getSize("integer"));
	end->setSize(getSize("integer"));
	head->next = begin;
	begin->next = end;
	end->next = head;
	head->last = end;
	return head;
}

SymBucket *RecordTypeTreeNode::genSymItem(const string typeName, Symtab *symtab) {
	SymBucket *rb = new SymBucket(typeName, lineNO, "record",symtab);
	rb->setSize(0);
	rb->setLoc(symtab->genLoc(0));
	// 每个都是variableTreeNode
	vector<TreeNode*>& list = elemList->getList();
	for (int i = 0; i < list.size(); i++) {
		VariableTreeNode* v = (VariableTreeNode *)list[i];
		
		vector<TreeNode*>& nameList = v->getNameList()->getList();
		if (v->getName() != "") 
			nameList.push_back(new TreeNode(v->getName()));
		for (int j = 0; j < nameList.size();j++) {
			SymBucket *vb = v->getTypeNode()->genSymItem(nameList[j]->getName(), symtab);
			rb->setSize(rb->getSize() + vb->getSize());
			if (rb->next == rb) {
				rb->next = vb;
				vb->last->next = rb;
				rb->last = vb->last;
			} else {
				SymBucket *tmp = rb->next;
				rb->next = vb;
				vb->last->next = tmp;
			}
		}
	}
	int size = rb->getSize();
	int t = size / 4;
	if (t * 4 < size) {
		size = t * 4 + 4;
		rb->setSize(size);
	}

	return rb;
}

// make use of last to point to previous defined symbuckets in current symtab
SymBucket *CustomTypeTreeNode::genSymItem(const string typeName, Symtab *symtab) {
	// the predefined custom type in symtab
	SymBucket *typeBucket = symtab->find(name);
	SymBucket* b = typeBucket->deepCopyBucket();
	b->setCurSymtab(symtab);
	b->setName(typeName);
	// cout << " xxxxx " << b->getSize() << endl;
	return b;
}

////////////////////////////////////////////////////////////
// type check functions for each node					  //
////////////////////////////////////////////////////////////

string ListTreeNode::typeCheck(Symtab *symtab){
	return childrenTypeCheck(list, symtab);
}

string VariableTreeNode::typeCheck(Symtab *symtab){
	SymBucket *var = symtab->find(name);
	if(var->getIsConst()){
		return "const-" + var->getType();
	}
	else{
		return var->getType();
	}
	
}

string ArrayElemTreeNode::typeCheck(Symtab *symtab){
	SymBucket * b = symtab->find(name);
	if (b == NULL || b->getType()!= "array") {
		if (b == NULL) 
			cout << lineNO << " : name is not defined."  << endl;
		else 
			cout << lineNO << " : name is not a array type."  << endl;
		return "failure";
	} else {
		return b->next->last->next->getType();
	}
}

string CompoundStmtTreeNode::typeCheck(Symtab *symtab){
	if((stmtList->typeCheck(symtab))!="failure")
		return "success";
	return "failure";
}

string IfStmtTreeNode::typeCheck(Symtab *symtab){
	string s1 = condition->typeCheck(symtab);
	string s2 = body->typeCheck(symtab);
	string s3 = elsePart->typeCheck(symtab);
	if(s1 != "failure" && s2 != "failure" && (elsePart == NULL? 1 : (s3!="failure"))){
		return "success";
	}
	 return "failure";
}

string RepeatStmtTreeNode::typeCheck(Symtab *symtab){
	string s1 = condition->typeCheck(symtab);
	string s2 = body->typeCheck(symtab);
	if(s1!="failure" && s2!="failure"){
		return "success";
	}
	return "failure";
}

string WhileStmtTreeNode::typeCheck(Symtab *symtab){
	string s1 = condition->typeCheck(symtab);
	string s2 = body->typeCheck(symtab);
	if(s1!="failure" && s2!="failure"){
		return "success";
	}
	return "failure";
}

string SwitchStmtTreeNode::typeCheck(Symtab *symtab){

	string s1 = expr->typeCheck(symtab);
	string s2 = caseExprList->typeCheck(symtab);
	if(s1!="failure" && s2!="failure"){
		return "success";
	}

	return "failure";
}

string ForStmtTreeNode::typeCheck(Symtab *symtab){
	string s1 = assignExpr->typeCheck(symtab);
	string s2 = dirExpr->typeCheck(symtab);
	string s3 = body->typeCheck(symtab);
	if(s1 != "failure" && s2 != "failure" && s3!="failure"){
		return "success";
	}
	return "failure";
}

string UnaryExprTreeNode::typeCheck(Symtab *symtab){
	string type = operand->typeCheck(symtab);
	if(type!="integer" && type!="const-integer"){
		cout << lineNO << ": " << "The right value must be an integer." << endl;
		return "failure";
	}
	return type;
}

string BinaryExprTreeNode::typeCheck(Symtab *symtab) {
	string ltype = lhs->typeCheck(symtab);
	string rtype = rhs->typeCheck(symtab);
	if(ltype == "integer" && (rtype == "integer" || rtype == "const-integer"))
		return "integer";
	else if(ltype == "real" && (rtype == "real" || rtype == "const-real"))
		return "real";
	else if(ltype == "char"){
		if(rtype == "char" || rtype == "string" || rtype == "const-string" || rtype == "const-char")
			return "char";
	}
	else if(ltype == "string"){
		if(rtype == "char" || rtype == "string" || rtype == "const-string" || rtype == "const-char")
			return "string";
	}
	else if(ltype == "const-char" || ltype == "const-integer" || ltype == "const-string" ||ltype == "const-real" ){
		cout << lineNO << ": " << "The left value must be a variable!" << endl;
		return "failure";
	}
	else if(ltype == rtype || rtype == "const-"+ltype ){
		return ltype;
	}
	cout << lineNO << ": " << "Can not transform the type " << rtype << " to the type " << ltype << "." << endl;
	return "failure";
}



string CaseExprTreeNode::typeCheck(Symtab *symtab){
	if((stmt->typeCheck(symtab))!="failure"){
		return "success";
	}
	return "failure";
}

string CallExprTreeNode::typeCheck(Symtab *symtab){
	if(symtab->find(name)==NULL && name != "writeln" && name != "readln"){
		cout << lineNO << ": " << "Can not find the function " << name << "." << endl;
		return "failure";
	}

	SymBucket *func = symtab->find(name);
	int i = 0;
	if (func != NULL) {
		SymBucket * member = func->next;
		do {
			string argType = args[i]->typeCheck(symtab);
			if (member->getIsRef() && argType.find("const") != string::npos) {
				cout << lineNO << " : const value can not be passed to var type argument" << endl;
				return "failure";
			}
			string memberType = member->getType();
			if (argType.find(memberType) == string::npos)  {
				cout << lineNO << " : argument " << i << " type dismatch " << endl;
				return "failure";
			}
			i++;
			member = member->last->next;
		} while (member != func->last && member != func);
		if (i < args.size()) {
			cout << lineNO << " : argument number is not match " << endl;
			return  "failure";
		}
	}
	if (func!=NULL)
		return func->last->getType();
	else return "success";
}

string RecordElemTreeNode::typeCheck(Symtab *symtab){
	SymBucket *bucket = symtab->find(recordName);
	if(bucket==NULL){
		cout << getLineNO() << ": " << "Can not find the record " << recordName << "." << endl; 
	}
	SymBucket *member = bucket->next;
	do {
		if (member->getName() == elemName) {
			return member->getType();
		}
		member = member->last->next;
	} while (member != bucket);
	cout << getLineNO() << ": " << "Can not find the element " << elemName << " in the record " << recordName << "." << endl; 
	return "failure";
}
