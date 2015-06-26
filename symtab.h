#ifndef __SYMTAB_H_
#define __SYMTAB_H_
#include <fstream>
#include <map>
#include <string>
#include <algorithm>
#include <vector>
#include "code_generator.h"
using namespace std;

class SymBucket;
class Symtab;	

typedef vector<SymBucket *> * SYMQUEUE;
typedef map<string, SYMQUEUE> SYMMAP;
extern RegManager *regManager;



class SymBucket {
	int 			lineNO;
	// the address of this type, default -1
	int 			location;

	int 			size;
	// default -1, define the reg store this symbol
	int 			regNum;
	// the reg stores this symbol's stack offset
	// int 			offsetReg;
	// the order being inserted into symtab
	int 			order;
	// does this bucket presents a type
	int 			isType;
	// whether it is copy by ref
	int 		    isRef;
	// var
	int 			isVar;
	//
	int 			intImme;

	int 			isConst;
	// type name
	const string 	type;
	// id name
	string 			name;
	// the sub level symtab for funciton or procedure
	Symtab 			*nextSymtab;
	// currnt symtab
	Symtab 			*curSymtab;

	void _print(ofstream &out) {
		if (location != -1)
			out << lineNO << " " << name << ":\t" << type << "\t at " << location ;
		else if (regNum != -1)
			out << lineNO << " " << name << ":\t" << type << "\t at reg " << regNum;
		else 
			out << lineNO << " " << name << ":\t" << type;
	}

public:
	// next bucket in the one complicated datastructure
	SymBucket 		*next;
	SymBucket 		*last;
	// deprecated
	SymBucket 		*ref;
	SymBucket(const string _name, int _lineNO, const string _type, Symtab* _curSymtab)
		:name(_name),lineNO(_lineNO), type(_type), curSymtab(_curSymtab),nextSymtab(NULL),
		location(-1), next(this), regNum(-1),last(this),ref(NULL),isType(0),isConst(0),
		isRef(0), isVar(0), intImme(0)
		{}
	SymBucket(const SymBucket* that) : type(that->type){
		lineNO 		= that->lineNO;
		location 	= that->location;
		size 		= that->size;
		regNum 		= that->regNum;
		// offsetReg 	= that->offsetReg;
		name 		= that->name;
		// nextSymtab 	= that->nextSymtab;
		// curSymtab 	= that->curSymtab;
		// next 		= that->next;
		// last 		= that->last;
		isRef 		= that->isRef;
		nextSymtab 	= NULL;
		last 		= this;
		next 		= this;
		isConst     = 0;
//		offsetReg 	= -1;
	}
/////////////////////////////////////////////////////
// set functions								   //
/////////////////////////////////////////////////////		
	void setLoc(int l) {
		location = l;
	}

	void setSize(int s) {
		size = s;
	}

	void setIsRef(int r) {
		isRef = r;
	}
	void setIsVar(int v) {
		isVar = v;
	}

	void setIsType(int t) {
		isType = t;
	}
	void setSubSymtab(Symtab *symtab) {
		nextSymtab = symtab;
	}

	void setRegNum(int rn) {
		regNum = rn;
	}

	void setIntImme(int n) {
		intImme = n;
	}
	void setIsConst(int n){
		isConst = n;
	}

	// void setType(string t) {
	// 	type = t;
	// }
	// void setOffsetReg(int reg) {
	// 	offsetReg = reg;
	// }

	void setName(string n) {
		name = n;
	}

	void setOrder(int o) {
		order = o;
	}
	void setCurSymtab(Symtab *s) {
		curSymtab = s;
	}
/////////////////////////////////////////////////////
// get functions								   //
/////////////////////////////////////////////////////	
	int getLoc() {
		return location;
	}

	int getOrder() {
		return order;
	}

	int getIsVar() {
		return isVar;
	}

	int getLineno() {
		return lineNO;
	}

	const string& getType() {
		return type;
	}

	Symtab* getNextSymtab() {
		return nextSymtab;
	}

	Symtab* getCurSymtab() {
		return curSymtab;
	}
	const string& getName() {
		return name;
	}

	int getIsRef() {
		return isRef;
	}
	int getRegNum() {
		return regNum;
	}

	int getSize() {
		return size;
	}

	// int getOffsetReg(){
	// 	return offsetReg;
	// } 

	int getIsType() {
		return isType;
	}

	int getIntImme() {
		return intImme;
	}

	int getIsConst(){
		return isConst;
	}
	void printBucket(ofstream &out) {
		_print(out);
		SymBucket *tmp = next;
		while(tmp != this) {
			out << " .. ";
			tmp->_print(out);
			tmp = tmp->next;
		}
		out << endl;
	}

	SymBucket* deepCopyBucket() {
		SymBucket *head = new SymBucket(this);
		SymBucket *tmpBucket = head;
		SymBucket *member = next;

		if (member == this) {
			head->next = head;
			head->last = head;
			return head;
		}
		
		do {
			SymBucket *newBucket = new SymBucket(member);
			SymBucket *subTmpBucket = newBucket;
			if (member->last != member) {
				SymBucket * subMem = member->next;
				do {
					SymBucket* subNewBucket = new SymBucket(subMem);
					subTmpBucket->next = subNewBucket;
					subTmpBucket = subNewBucket;
					subMem = subMem->next;
				} while(subMem != member->last->next);
				newBucket->last = subTmpBucket;
				tmpBucket->next = newBucket;
				tmpBucket = newBucket->last;
				member = member->last->next;
			} else {
				newBucket->last = newBucket;
				tmpBucket->next = newBucket;
				tmpBucket = newBucket;
				member = member->next;
			}
		} while (member != this);
		head->last = tmpBucket;
		head->last->next = head;
		return head;
	}

	~SymBucket() {

	}
};

bool cmp(SymBucket *x, SymBucket *y);


class Symtab {
	const int BEGIN_REG_NUM;
	const int END_REG_NUM;
	const int BEGIN_PARA_REG_NUM;
	const int END_PARA_REG_NUM;
	int level;
	// symtabName
	const string symtabName;
	// the parent symtab
	SymBucket *pBucket;
	// inner hash map for symtab
	SYMMAP symMap;
	// current offset in the stack
	int curLoc;
	// default reg num is -1
	int curRegNum;
	// curOrder
	int curOrder;
	// cur $ax reg , deprecated
	int curParaReg;
public:
	Symtab(const string _name, SymBucket *_pBucket = NULL)
		:symtabName(_name), pBucket(_pBucket),curLoc(0),curRegNum(-1)
		,BEGIN_REG_NUM(16), END_REG_NUM(23), BEGIN_PARA_REG_NUM(4),END_PARA_REG_NUM(7),curOrder(0)
		,curParaReg(-1),level(0) {}

	void insert(SymBucket* b) {
		SYMMAP::iterator iter;
		b->setOrder(curOrder++);
		string name = b->getName();
		iter = symMap.find(name);
		if (iter != symMap.end()) {
			symMap[name]->push_back(b);
		} else {
			SYMQUEUE q = new vector<SymBucket*>();
			q->push_back(b);
			symMap[name] = q;
		}
	}

	SymBucket *find(string name) {
		SYMMAP::iterator iter;
		iter = symMap.find(name);
		if (iter != symMap.end()) {
			SYMQUEUE q = symMap[name];
			for(int i = q->size()-1; i >= 0; i--) {
				if ((*q)[i]->getName() == name) 
					return (*q)[i];
			}
		}
		if (pBucket != NULL) {
			return pBucket->getCurSymtab()->find(name);
		}
//		cout << "symbol " << name << " is not defined !!!" << endl;
	
		return NULL;
	}

	void deleteSymBucket(string name) {

	}
/////////////////////////////////////////////////////
// get functions								   //
/////////////////////////////////////////////////////
	const string getSymtabName() {
		return symtabName;
	}

	const SymBucket * getParBucket() {
		return pBucket;
	}

	int getCurLoc() {
		return curLoc;
	}

	int  genLoc(int size) {
		int l = curLoc;
		curLoc += size;
		return l;
	}
	// if return num is -1, then this symbol has to be stored in the stack
	// otherwise, return a num 
	int genRegNum() {
		return -1;
		if (curRegNum == END_REG_NUM) return -1;
		if (curRegNum == -1) {
			curRegNum = BEGIN_REG_NUM;
		} else {
			curRegNum++;
		}
		regManager->useReg(curRegNum);
		return curRegNum;
	}

	int genParaRegNum() {
		if (curParaReg == END_PARA_REG_NUM) return -1;
		if (curParaReg == -1) curParaReg = BEGIN_PARA_REG_NUM;
		else curParaReg++;
		regManager->useReg(curParaReg);
		return curParaReg;
	}

	int getCurReg() {
		return curRegNum;
	}
	string getName() {
		return symtabName;
	}
	void getSymBucketList(vector<SymBucket*>& v) {
		for (SYMMAP::iterator iter = symMap.begin(); iter != symMap.end(); iter++) {
			SYMQUEUE q = iter->second;
			for (int i = 0; i < q->size(); i++) {
				if (!(*q)[i]->getIsType())
					v.push_back((*q)[i]);
			}
		}
		sort(v.begin(), v.end(), cmp);
	}

	int getLevel() {
		return level;
	}
/////////////////////////////////////////////////////
// set functions								   //
/////////////////////////////////////////////////////	

	void setParentBucket(SymBucket *b) {
		pBucket = b;
	}

	void setCurLoc(int loc) {
		curLoc = loc;
	}

	int isRegSpill() {
		return curRegNum == END_REG_NUM;
	}

	void resetRegNum() {
		curRegNum = -1;
	}

	void setCurRegNum(int r)  {
		curRegNum = r;
	}

	void setLevel(int l) {
		level = l;
	}


	void printSymtab(ofstream& out);


	virtual ~Symtab() {}

};

#endif


