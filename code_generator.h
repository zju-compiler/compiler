#ifndef __CODE_GENERATOR_H_
#define __CODE_GENERATOR_H_
#include <fstream>
#include <iostream>
#include <cctype>
#include <cstdio>
#define FP 30
#define SP 29
#define A0 4
#define F12 44
#define V1 3
using namespace std;
extern ofstream code;
class RegManager;
class LabelManager;
extern RegManager *regManager;
extern LabelManager  *labelManager;
extern int traceEmit;
const string regTable[] = {
	"$zero",
	"$at",
	"$v0",
	"$v1",
	"$a0",
	"$a1",
	"$a2",
	"$a3",
	"$t0",
	"$t1",
	"$t2",
	"$t3",
	"$t4",
	"$t5",
	"$t6",
	"$t7",
	"$s0",
	"$s1",
	"$s2",
	"$s3",
	"$s4",
	"$s5",
	"$s6",
	"$s7",
	"$t8",
	"$t9",
	"",
	"",
	"$gp",
	"$sp",
	"$fp",
	"$ra",

	"$f0",
	"$f1",
	"$f2",
	"$f3",
	"$f4",
	"$f5",
	"$f6",
	"$f7",
	"$f8",
	"$f9",
	"$f10",
	"$f11",
	"$f12",
	"$f13",
	"$f14",
	"$f15",
	"$f16",
	"$f17",
	"$f18",
	"$f19",
	"$f20",
	"$f21",
	"$f22",
	"$f23",
	"$f24",
	"$f25",
	"$f26",
	"$f27",
	"$f28",
	"$f29",
	"$f30",
	"$f31",
};

const string floatRegTable[] = {

};

class RegManager {
	 static const int BEGIN_TMP = 8;
	 static const int END_TMP   = 23;
	 static const int BEGIN_FLOAT = 32;
	 static const int END_FLOAT = 63;
	 int reg[32];
public:
	 int getTmpReg(int isFloat = 0) {
	 	if (isFloat) {
	 		return getFloatReg();
	 	}
		for(int i = BEGIN_TMP; i <= END_TMP; i++) {
			if (reg[i] == 0) {
				reg[i] = 1;
				cout << "use reg " << regTable[i] << endl;
				return i;
			}
		}
		cout << "reg is run out" << endl;
		return -1;
	}

	int getFloatReg() {
		for(int i = BEGIN_FLOAT; i <= END_FLOAT; i++) {
			if (reg[i] == 0) {
				reg[i] = 1;
				cout << "use reg " << regTable[i] << endl;
				return i;
			}
		}
		cout << "reg is run out" << endl;
		return -1;	
	}

	void useReg(int i) {
		if (i < 0 || i > 31) return;
	 	reg[i] = 1;
	}

	void freeReg(int i) {
		
		if (i < 0 || i > 63) return;
		cout << "free " << regTable[i] << endl;
		char c = regTable[i][1];
		reg[i] = 0;
	}



	void freeAll() {
		for(int i = BEGIN_TMP; i < END_TMP; i++) {
			reg[i] = 0;
		}
	}

};

class LabelManager {
	static int loop_number;
	static int func_number;
	static int case_number;
	static int goto_number;
	static int if_number;
	static int do_number;
	static int string_label_number;
	static int real_label_number;
	static int equal_number;
public:
	
	int getLoopLabel(){return loop_number;}
	void addLoopLabel(){loop_number++;} 
	int getRepeatLabel(){return do_number;}
	void addRepeatLabel(){do_number++;} 
	int getEqualLabel(){return equal_number;}
	void addEqualLabel(){equal_number++;} 
	int getIfLabel(){return if_number;}
	void addIfLabel(){if_number++;} 
	int getgotoLabel(){return goto_number;}
	void addgotoLabel(){goto_number++;} 
	int getCaseLabel(){return case_number;}
	void addCaseLabel(){case_number++;}
	int getFuncLabel() {return func_number;}
	void addFuncLabel(string &label) {
		char labelNum[10] = {0,};
		sprintf(labelNum, "%d", func_number++);
		label = label + labelNum;	
	}
	string getStringLabel() {
		char ch[32] = {0,};
		sprintf(ch, "string%d", string_label_number++);
		return ch;
	}
	string getRealLabel() {
		char ch[32] = {0,};
		sprintf(ch, "real%d", real_label_number++);
		return ch;
	}


};



class CodeGenerator {
private:
	static int equal;

public:
	static string intTostring(int x){
		char t[256];
	    string s;
	 
	    sprintf(t, "%d", x);
	    s = t;
	    return s;
	}
	// R-type instruments
	// if the op is "~" then src_1 = 0 and src_2 is the right hand variable
	static void emitCodeR(const string op, int dst, int src_1, int src_2, int isFloat = 0) {
		if (traceEmit) cout << "emit R : " << op << " " << dst << " " << src_1 << " " << src_2 <<endl;
		string c;
		if (op == "+" || op == "=") {
			if (!isFloat) 
				c = "add " + regTable[dst] + "," + regTable[src_1] + "," + regTable[src_2];
			else 
				c = "add.s " + regTable[dst] + "," + regTable[src_1] + "," + regTable[src_2];
		} else if (op == "-") {
			if (!isFloat) 
				c = "sub " + regTable[dst] + "," + regTable[src_1] + "," + regTable[src_2];
			else 
				c = "sub.s " + regTable[dst] + "," + regTable[src_1] + "," + regTable[src_2];
		} else if (op == "*") {
			if (!isFloat) 
				c = "mul " + regTable[dst] + "," + regTable[src_1] + "," + regTable[src_2];
			else 
				c = "mul.s " + regTable[dst] + "," + regTable[src_1] + "," + regTable[src_2];
		} else if (op == "/") {
			if (!isFloat)
				c = "div " + regTable[dst] + "," + regTable[src_1] + "," + regTable[src_2];
			else 
				c = "div.s " + regTable[dst] + "," + regTable[src_1] + "," + regTable[src_2];
		} else if (op == "%") {
			c = "rem " + regTable[dst] + "," + regTable[src_1] + "," + regTable[src_2];
		} else if (op == "&&") {
			c = "and " + regTable[dst] + "," + regTable[src_1] + "," + regTable[src_2];
		} else if (op == "||") {
			c = "or " + regTable[dst] + "," + regTable[src_1] + "," + regTable[src_2];
		} else if (op == ">=") {
			int tmp = regManager->getTmpReg();
            c = "slt " + regTable[tmp] + "," + regTable[src_1] + "," + regTable[src_2] + "\n";
            c = c + "xori " + regTable[dst] + "," + regTable[tmp] + "," + "1";
            regManager->freeReg(tmp); 
		} else if (op == ">") {
            c = "slt " + regTable[dst] + "," + regTable[src_2] + "," + regTable[src_1];
		} else if (op == "<=") {
			int tmp = regManager->getTmpReg();
            c = "slt " + regTable[tmp] + "," + regTable[src_2] + "," + regTable[src_1] + "\n";
            c = c + "xori " + regTable[dst] + "," + regTable[tmp] + "," + "1";
            regManager->freeReg(tmp);
		} else if (op == "<") {
            c = "slt " + regTable[dst] + "," + regTable[src_1] + "," + regTable[src_2] ;
		} else if (op == "==") {
			string s = "equal" + intTostring(equal);
			c = "add " + regTable[dst] + "," + regTable[0] + "," + regTable[0] + "\n";
            c = c + "beq " + regTable[src_1] + "," + regTable[src_2] + "," + s +"\n";
            c = c + "addi " + regTable[dst] + "," + regTable[dst] + "," + "-1" + "\n";
            c = c + s + ":\n";
            c = c + "addi " + regTable[dst] + "," + regTable[dst] + ",1";
            equal++;
		} else if (op == "!=") {
			string s = "equal" + intTostring(equal);
			c = "add " + regTable[dst] + "," + regTable[0] + "," + regTable[0] + "\n";
            c = c + "bne " + regTable[src_1] + "," + regTable[src_2] + "," + s +"\n";
            c = c + "addi " + regTable[dst] + "," + regTable[dst] + "," + "-1" + "\n";
            c = c + s + ":\n";
            c = c + "addi " + regTable[dst] + "," + regTable[dst] + ",1";
            equal++;
		} else if (op == "~") {
			c = c + "xori " + regTable[dst] + ", " + regTable[src_2] + ", -1" ;
		}
		code << c << endl;
	}

	static void emitCodeI(const string op, int dst, int src, int imm)
	{
		if (traceEmit) cout << "emit I op = " << op << " dst = " << dst << " src = " << src <<endl;
		string c;
		char ch[16] = {0,};
		sprintf(ch,"%d",imm);
		if (op == "+" || op == "=") {
			c = "addi " + regTable[dst] + "," + regTable[src] + "," + ch;
		} else if (op == "-") {
			c = "addi " + regTable[dst] + "," + regTable[src] + "," + "-" + ch;
		} else if (op == "*") {
			int tmp = regManager->getTmpReg();
			c = "addi " + regTable[tmp] + "," + regTable[0] + "," + ch + "\n";
			c = c + "mul " + regTable[dst] + "," + regTable[src] + "," + regTable[tmp];
			regManager->freeReg(tmp);
		} else if (op == "/") {
			int tmp = regManager->getTmpReg();
			c = "addi " + regTable[tmp] + "," + regTable[0] + "," + ch + "\n";
			c = c + "div " + regTable[dst] + "," + regTable[src] + "," + regTable[tmp];
			regManager->freeReg(tmp);
		} else if (op == "%") {
			int tmp = regManager->getTmpReg();
			c = "addi " + regTable[tmp] + "," + regTable[0] + "," + ch + "\n";
			c = c + "rem " + regTable[dst] + "," + regTable[src] + "," + regTable[tmp];
			regManager->freeReg(tmp);
		} else if (op == "&&") {
			c = "andi " + regTable[dst] + "," + regTable[src] + "," + ch;
		} else if (op == "||") {
			c = "ori " + regTable[dst] + "," + regTable[src] + "," + ch;
		} else if (op == ">=") {
			int tmp = regManager->getTmpReg();
            c = "slti " + regTable[tmp] + "," + regTable[src] + "," + ch + "\n";
            c = c + "xori " + regTable[dst] + "," + regTable[tmp] + "," + "1";
            regManager->freeReg(tmp); 
		} else if (op == ">") {
			int tmp2 = regManager->getTmpReg();
			c = "addi " + regTable[tmp2] + "," + regTable[0] + "," + ch + "\n";
            c = c + "slt " + regTable[dst] + "," + regTable[tmp2] + "," + regTable[src]; 
            regManager->freeReg(tmp2);
		} else if (op == "<=") {
			int tmp = regManager->getTmpReg();
			int tmp2 = regManager->getTmpReg();
			c = "addi " + regTable[tmp2] + "," + regTable[0] + "," + ch + "\n";
            c = c + "slt " + regTable[tmp] + "," + regTable[tmp2] + "," + regTable[src] + "\n";
            c = c + "xori " + regTable[dst] + "," + regTable[tmp] + "," + "1";
            regManager->freeReg(tmp);
            regManager->freeReg(tmp2);
		} else if (op == "<") {
			int tmp = regManager->getTmpReg();
            c = "slti " + regTable[dst] + "," + regTable[src] + "," + ch ;
            regManager->freeReg(tmp); 
		} else if (op == "==") {
			int tmp = regManager->getTmpReg();
			string s = "equal" + intTostring(equal);
			c = "add " + regTable[dst] + "," + regTable[0] + "," + regTable[0] + "\n";
			c = c + "addi " + regTable[tmp] + "," + regTable[0] + "," + ch + "\n";
            c = c + "beq " + regTable[src] + "," + regTable[tmp] + "," + s +"\n";
           c = c + "addi " + regTable[dst] + "," + regTable[dst] + "," + "-1" + "\n";
            c = c + s + ":\n";
            c = c + "addi " + regTable[dst] + "," + regTable[dst] + ",1";
            equal++;
            regManager->freeReg(tmp); 
		} else if (op == "!=") {
			int tmp = regManager->getTmpReg();
			string s = "equal" + intTostring(equal);
			c = "add " + regTable[dst] + "," + regTable[0] + "," + regTable[0] + "\n";
			c = c + "addi " + regTable[tmp] + "," + regTable[0] + "," + ch + "\n";
            c = c + "bne " + regTable[src] + "," + regTable[tmp] + "," + s +"\n";
           c = c + "addi " + regTable[dst] + "," + regTable[dst] + "," + "-1" + "\n";
            c = c + s + ":\n";
            c = c + "addi " + regTable[dst] + "," + regTable[dst] + ",1";
            equal++;
            regManager->freeReg(tmp); 
		}
		code << c << endl;
	}

	static void emitCodeJ(const string op, int reg1, int reg2, int imm, const string label){
		string c;
		if(op == "beq" || op == "bne"){
			if(reg2 > 31){
				char ch[16] = {0,};
				sprintf(ch,"%d",imm);
				int tmp = regManager->getTmpReg();
				c = "addi " + regTable[tmp] + "," + regTable[0] + "," + ch + "\n";
				c = c + op + " " + regTable[reg1] + "," + regTable[tmp] + "," + label;
				regManager->freeReg(tmp); 
			}
			else {
				c = op + " " + regTable[reg1] + "," + regTable[reg2] + "," + label;
			}
		}
		else if(op == "j") {
			c = op + " " + label;
		} else if (op == "jr") {
			c = op + " " + regTable[reg1];
		} else if (op == "jal") {
			c = op + " " + label;
		}
		code << c << endl;

	}

	static void addLabel(const string label){
		code << label << ":" << endl;
	}



	static void findFP(string op, string& localOP, int& tmpAC) {
		if (op.find("-") != string::npos) {
			tmpAC = regManager->getTmpReg();
			int pos = op.find('-');
			int level = atoi(op.substr(pos+1, op.length()).c_str());
			CodeGenerator::emitCodeM(4, "load", -4, FP, tmpAC);			//应为fp在ac上面，所有ac所在的地址高于当前fp的地址
			for (int i = 1; i < level; i++) {
				CodeGenerator::emitCodeM(4, "load", 0, tmpAC, tmpAC);
			}
			CodeGenerator::emitCodeI("+", tmpAC, tmpAC, -4);
			localOP = op.substr(0,pos);
		}
	}
	// lw sw instruments
	// reg is the src or dst reg
	// load_reg , store_reg means that offset现在保存了一个寄存器号，这个寄存器中有偏移量
	static void emitCodeM(int size, const string op, int offset, int regAddr, int reg, int isFloat = 0) {
		if (traceEmit) cout << "emit M " << "op = " << op << " size = " << size << " offset = " << offset <<  endl;
		if (op.find("reg") == string::npos)
			offset = -offset;
		string c;
		char loadInstr[][4] = {"", "lb", "lh", "","lw"};
		char storeInstr[][4] = {"", "sb", "sh", "","sw"};
		string ls = "l.s";
		string ss = "s.s";
		char ch[16] = {0,};
		string instr;
		string localOP = op;
		//int tmpAC = regAddr;
		// find correct FP
		findFP(op, localOP, regAddr);
		//regAddr = tmpAC;
		if (localOP == "load" || localOP == "load_reg" || localOP == "load_ref") {
			if (!isFloat)
				instr = loadInstr[size];
			else instr = ls;
		} else {
			if (!isFloat)
				instr = storeInstr[size];
			else 
				instr = ss;
		}
		if (localOP == "load" || localOP == "store") {
			sprintf(ch,"%d",offset);
			c = instr +  " "  + regTable[reg] + ", " + ch + "(" + regTable[regAddr]+")";
			code << c << endl;
		} else if (localOP == "load_reg" || localOP =="store_reg") {
			cout << offset << " " << regAddr << endl;
			ch[0] = '\0';
			sprintf(ch, "0");
			string c1 = "add " + regTable[offset] + ", " + regTable[offset] + ", " + regTable[regAddr];
			c = instr + " " + regTable[reg] + ", " + ch + "(" + regTable[offset] + ")";
			code << c1 << endl << c << endl;
		} else if (localOP == "load_ref" || localOP == "store_ref") {
			int tmp = regManager->getTmpReg();
			CodeGenerator::emitCodeM(4, "load", offset, regAddr, tmp);
			ch[0] = '\0';
			sprintf(ch, "0");
			c = instr + " " + regTable[reg] + ", " + ch + "(" + regTable[tmp] + ")";
			regManager->freeReg(tmp);
			code << c << endl;
		}
		//if (regAddr != tmpAC) regManager->freeReg(tmpAC);
	}

	static void emitCodeB(const string loadOP,const string storeOP, int size, int dstOffset, int srcOffset, int addrReg ,int copysize) {
		int loopTime = size / copysize;
		
		string s = "copy";
		labelManager->addLoopLabel();
		int loopNum = labelManager->getLoopLabel();
		char ch[8] = {0,};
		sprintf(ch, "%d", loopNum);
		s += ch;
		int loop = regManager->getTmpReg();
		int tmp  = regManager->getTmpReg();
		string localLoadOP = loadOP, localStoreOP = storeOP;
		//int tmpAddr = regManager->getTmpReg();
		int addrRegSrc = addrReg;
		int addrRegDst = addrReg;
		findFP(loadOP, localLoadOP, addrRegSrc);
		findFP(storeOP, localStoreOP, addrRegDst);
		if (addrRegSrc == addrReg) {
			addrRegSrc = regManager->getTmpReg();
			CodeGenerator::emitCodeR("+", addrRegSrc, addrReg, 0);
		}
		if (addrRegDst == addrReg) {
			addrRegDst = regManager->getTmpReg();
			CodeGenerator::emitCodeR("+", addrRegDst, addrReg, 0);
		}
		if (loadOP.find("reg") != string::npos) {
			CodeGenerator::emitCodeR("+", addrRegSrc, addrRegSrc, srcOffset);
		} else {
			CodeGenerator::emitCodeI("+", addrRegSrc, addrRegSrc, -srcOffset);
		}
		if (storeOP.find("reg") != string::npos) {
			CodeGenerator::emitCodeR("+", addrRegDst, addrRegDst, dstOffset);
		} else {
			CodeGenerator::emitCodeI("+", addrRegDst, addrRegDst, -dstOffset);
		}
		CodeGenerator::emitCodeR("+",loop,0,0);
		CodeGenerator::addLabel(s);
		CodeGenerator::emitCodeM(copysize, "load",0, addrRegSrc, tmp);
		CodeGenerator::emitCodeM(copysize, "store",0, addrRegDst, tmp);
		CodeGenerator::emitCodeI("+", addrRegSrc, addrRegSrc, -copysize);
		CodeGenerator::emitCodeI("+", addrRegDst, addrRegDst, -copysize);
		CodeGenerator::emitCodeI("+",loop,loop,1);
		int tmp2 = regManager->getTmpReg();
		CodeGenerator::emitCodeI("<",tmp2,loop,loopTime);
		CodeGenerator::emitCodeJ("bne",tmp2,0,0,s);
		regManager->freeReg(addrRegSrc);
		regManager->freeReg(addrRegDst);
		regManager->freeReg(tmp);
		regManager->freeReg(tmp2);
		regManager->freeReg(loop);
		cout << "yyyy" << endl;
	}

	static void emitCodeConstStr(string constStr, string label) {
		code << label << ": .asciiz " << "\"" << constStr.substr(1,constStr.length()-2) << "\"" << endl;
	}

	static void emitCodeConstReal(string constReal, string label) {
		code << label << ": .float " << " " << constReal << endl;
	}

	static void emitCodeLA(string label, int regDst) {
		code << "la " << regTable[regDst] << " " << label << endl;
	}
	static void emitSysCall(string type) {
		if (type == "printString") {
			code << "addi $v0, $zero, 4" << endl;
		} else if (type == "printInteger") {
			code << "addi $v0, $zero, 1" << endl;
		} else if (type == "printReal") {
			code << "addi $v0, $zero, 2" << endl;
		} else if (type == "printChar") {
			code << "addi $v0, $zero, 11" << endl;
		}
		code << "syscall" << endl;
	}

	static void emitGetAccessLink(int levelDiff) {
		cout << "========== levelDiff = " <<  levelDiff << endl;
		int tmpAC = regManager->getTmpReg();
		if (levelDiff == 1) {
			CodeGenerator::emitCodeI("+", tmpAC, FP, 4);
		} else if (levelDiff <= 0) {
			CodeGenerator::emitCodeM(4, "load", -4, FP, tmpAC);	
			for (int i = 0; i < -levelDiff; i++) {
				CodeGenerator::emitCodeM(4, "load", 0, tmpAC, tmpAC);
			}
		} else if (levelDiff > 0) {
			cout << "error : can not call inner function !!" << endl;
		}
		CodeGenerator::emitCodeM(4, "store", -4,  SP, tmpAC);
		regManager->freeReg(tmpAC);
	}
};	

	

#endif