%{
#include "public.h"
#include "scanner.h"
#include "parser.hpp"
#include "utils.h"
#include <cstdio>
#include <iostream>
#include <string>
using namespace std;
int lineno;
FILE *source;
FILE *listing;
std::string currentToken;
int traceScan = TRUE;
string curLine;
string space(" ");
%}

digit [0-9]
number {digit}+
integer [0-9]{digit}*
real [1-9]{digit}*[.]{digit}+
string \'[^']*\'
letter [a-zA-Z]
char \'[.]\'
id ({letter}|_)+({letter}|_|{digit})*
newline \n
whitespace [ \t]+
name ({letter}|_)+({letter}|_|{digit})+

%%

"and" { return AND;}
"array" { return ARRAY;}
":=" { return ASSIGN;}
"begin" { return BEGINP;}
"case" { return CASE;}
{char} { return CHAR;}
":" { return COLON;}
"," { return COMMA;}
"const" { return CONST;}
"/" { return DIV;}
"do" { return DO;}
"." { return DOT;}
".." { return DOTDOT;}
"downto" { return DOWNTO;}
"or" { return OR;}
"else" { return ELSE;}
"end" { return END;}
"=" { return EQUAL;}
"for" { return FOR;}
"function" { return FUNCTION;}
">=" { return GE;}
"goto" { return GOTO;}
">" { return GT;}
"if" { return IF;}
{integer} {
	TreeNode *t = new TreeNode(yytext);
	yylval = t;
	return INTEGER;
}
"[" { return LB;}
"<=" { return LE;}
"(" { return LP;}
"<" { return LT;}
"-" { return MINUS;}
"%" { return MOD;}
"*" { return MUL;}
"~" { return NOT;}
"of" { return OF;}
"+" { return PLUS;}
"program" { return PROGRAM;}	
"procedure" {return PROCEDURE;}
"]" { return RB;}
"read" { 	
	TreeNode *t = new TreeNode(yytext);
	yylval = t;
	return READ;}
{real} { return REAL;}
"record" { return RECORD;}
"repeat" { return REPEAT;}
")" { return RP;}
";" { return SEMI;}
{string} { return STRING;}
"then" { return THEN;}
"to" { return TO;}
"type" { return TYPE;}
"!=" { return UNEQUAL;}
"until" { return UNTIL;}
"var" { return VAR;}
"while" { return WHILE;}
{newline} {lineno++;
curLine = space;}
{whitespace} {}
"integer"	{return SYS_TYPE;} 
"real"		{return SYS_TYPE;}
"string"	{return SYS_TYPE;}
"char"   	{return SYS_TYPE;}

{id} {
	TreeNode *t = new TreeNode(yytext);
	yylval = t;
	return ID;
}

%%

int getToken(void) {
	static int firstIn = TRUE;
	int curToken;
	
	if (firstIn) {
		firstIn = FALSE;
		yyin = source;
		yyout = listing;
		lineno++;
	}
	curToken = yylex();
	currentToken = yytext;
	curLine = curLine + yytext;
	if (traceScan) {
		fprintf(listing, "\t%d %s\n", lineno, yytext);
		fflush(listing);
	}	
	return curToken;
}

int yywrap()
{
	return 1;
}













