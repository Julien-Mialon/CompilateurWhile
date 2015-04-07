%code requires
{
	#include <string>
	#include <map>
	#include <cstring>
	#include <iostream>

	#include "ast.hpp"
}


%code
{
	void yyerror(ast::Node*& program, int& line, int& chr, bool lexicalLog, bool syntacticLog, const char* msg);
	int yylex(int& line, int& chr, bool lexicalLog);
	void yLog(const std::string& msg, bool syntacticLog);
	int yylex_destroy();

	using namespace ast;
	using namespace std;
}


%union
{
	bool booleanValue;
	int intValue;
	float floatValue;
	char* strValue;
	ast::Node* nodeValue;
	ast::Node* program;
}


%lex-param {int& line}
%lex-param {int& chr}
%lex-param {bool lexicalLog}
%parse-param {ast::Node*& program}
%parse-param {int& line}
%parse-param {int& chr}
%parse-param {bool lexicalLog}
%parse-param {bool syntacticLog}


%error-verbose


%destructor
{
	free($$);
} <strValue>

%destructor
{
	deleteTree($$);
} <nodeValue>


%token <strValue>       TYPE
%token                  ARRAY_TYPE
%token <strValue>       IDENTIFIER
%token <floatValue>     FLOAT_VALUE
%token <intValue>       INT_VALUE

%token                  PROGRAM
%token                  RETURN
%token                  IF
%token                  THEN
%token                  ELSE
%token                  WHILE
%token                  DO
%token <booleanValue>   TRUE_OP
%token <booleanValue>   FALSE_OP
%token                  BEGIN_OP
%token                  END
%token                  FUNCTION
%token                  PROCEDURE
%token                  IS
%token                  NEW
%token                  FREE
%token                  OP_AFFECTATION
%token                  OP_COLON
%token                  OP_SEMICOLON
%token                  OP_COMMA
%token                  OPEN_PARENTHESIS
%token                  CLOSE_PARENTHESIS
%token                  OPEN_SQUARE
%token                  CLOSE_SQUARE
%token                  OP_SUB
%token                  OP_ADD
%token                  OP_MUL
%token                  OP_DIV
%token                  OP_EQUAL
%token                  OP_DIFF
%token                  OP_INFEQ
%token                  OP_INF
%token                  OP_SUPEQ
%token                  OP_SUP
%token                  OP_LOGICAL_AND
%token                  OP_LOGICAL_OR
%token                  OP_LOGICAL_NEG
%token                  END_OF_LINE



%type   <program>       program
%type   <nodeValue>     functionsDeclList
%type   <nodeValue>     functionDecl
%type   <nodeValue>     functionParamsList
%type   <nodeValue>     functionDeclParamsList
%type   <nodeValue>     functionDeclParam
%type   <nodeValue>     statements
%type   <nodeValue>     variablesList
%type   <nodeValue>     variableDecl
%type   <nodeValue>     identifierList
%type   <nodeValue>     statementsList
%type   <nodeValue>     instruction
%type   <nodeValue>     functionCall
%type   <nodeValue>     functionCallParamsList
%type   <nodeValue>     booleanExpression
%type   <nodeValue>     arithmeticExpression
%type   <nodeValue>     lhs
%type   <nodeValue>     types


%left OP_ADD OP_SUB
%left OP_MUL OP_DIV

%left OP_LOGICAL_OR
%left OP_LOGICAL_AND
%left OP_LOGICAL_NEG


%%

%start program;

program:
		PROGRAM statements { yLog("Program 1", syntacticLog); program = makeProgram(); program->addChild($2); $$ = program; $$->setPos(line, chr); }
		| functionsDeclList PROGRAM statements { yLog("Program 2", syntacticLog); $1->addChild($3); $$ = $1; $$->setPos(line, chr); }

functionsDeclList:
		functionDecl { yLog("functionsDeclList 1", syntacticLog); program = makeProgram(); program->addChild($1); $$ = program; $$->setPos(line, chr); }
		| functionsDeclList functionDecl { yLog("functionsDeclList 2", syntacticLog); $1->addChild($2); $$ = $1; $$->setPos(line, chr); }

functionDecl:
		FUNCTION IDENTIFIER functionParamsList OP_COLON types IS statements { yLog("functionDecl 1", syntacticLog); $$ = makeFunctionDecl($5, makeVar($2), $3, $7); free($2); $$->setPos(line, chr); }
		| PROCEDURE IDENTIFIER functionParamsList IS statements { yLog("functionDecl 2", syntacticLog); $$ = makeFunctionDecl(makeVoidType(), makeVar($2), $3, $5); free($2); $$->setPos(line, chr); }

functionParamsList:
		OPEN_PARENTHESIS CLOSE_PARENTHESIS { yLog("functionParamsList 1", syntacticLog); $$ = makeFunctionDeclParamList(); $$->setPos(line, chr); }
		| OPEN_PARENTHESIS functionDeclParamsList CLOSE_PARENTHESIS { yLog("functionParamsList 2", syntacticLog); $$ = $2; $$->setPos(line, chr); }

functionDeclParamsList:
		functionDeclParam { yLog("functionDeclParamsList 1", syntacticLog); $$ = makeFunctionDeclParamList(); $$->addChild($1); $$->setPos(line, chr); }
		| functionDeclParamsList OP_COMMA functionDeclParam { yLog("functionDeclParamsList 2", syntacticLog); $1->addChild($3); $$ = $1; $$->setPos(line, chr); }

functionDeclParam:
		IDENTIFIER OP_COLON types { yLog("functionDeclParam 1", syntacticLog); $$ = makeFunctionDeclParam($3, makeVar($1)); free($1); $$->setPos(line, chr); }

statements:
		IF booleanExpression THEN statements ELSE statements { yLog("statements 2", syntacticLog); $$ = makeConditionGroup(); $$->addChild(makeIfCondition($2, $4)); $$->addChild(makeElseCondition($6)); $$->setPos(line, chr); }
		| WHILE booleanExpression DO statements { yLog("statements 3", syntacticLog); $$ = makeWhileLoop($2, $4); $$->setPos(line, chr); }
		| BEGIN_OP variablesList statementsList END { yLog("statements 4", syntacticLog); $$ = makeBlock(); $$->addChild($2); $$->addChild($3); $$->setPos(line, chr); }
		| BEGIN_OP statementsList END { yLog("statements 5", syntacticLog); $$ = $2; $$->setPos(line, chr); }
		| instruction { yLog("statements 1", syntacticLog); $$ = $1; $$->setPos(line, chr); }

variablesList:
		variableDecl { yLog("variablesList 1", syntacticLog); $$ = makeDeclarationList(); $$->addChild($1); $$->setPos(line, chr); }
		| variablesList variableDecl { yLog("variablesList 2", syntacticLog); $1->addChild($2); $$ = $1; $$->setPos(line, chr); }

variableDecl:
		types identifierList OP_SEMICOLON { yLog("variableDecl 1", syntacticLog); $$ = makeDeclaration($1, $2); $$->setPos(line, chr); }

identifierList:
		IDENTIFIER { yLog("identifierList 1", syntacticLog); $$ = makeIdentifierList(); $$->addChild(makeVar($1)); free($1); $$->setPos(line, chr); }
		| identifierList OP_COMMA IDENTIFIER { yLog("identifierList 2", syntacticLog); $1->addChild(makeVar($3)); $$ = $1; free($3); $$->setPos(line, chr); }

statementsList:
		statements { yLog("statementsList 1", syntacticLog); $$ = makeInstructionList(); $$->addChild($1); $$->setPos(line, chr); }
		| statementsList OP_SEMICOLON statements { yLog("statementsList 2", syntacticLog); $1->addChild($3); $$ = $1; $$->setPos(line, chr); }

instruction:
		RETURN { yLog("instruction 1", syntacticLog); $$ = makeReturn(); $$->setPos(line, chr); }
		| RETURN arithmeticExpression { yLog("instruction 2", syntacticLog); $$ = makeReturn($2); $$->setPos(line, chr); }
		| lhs OP_AFFECTATION arithmeticExpression { yLog("instruction 3", syntacticLog); $$ = makeAffectation($1, $3); $$->setPos(line, chr); }
		| lhs OP_AFFECTATION NEW types OPEN_SQUARE arithmeticExpression CLOSE_SQUARE { yLog("instruction 4", syntacticLog); $$ = makeAllocation($4, $1, $6); $$->setPos(line, chr); }
		| FREE OPEN_PARENTHESIS arithmeticExpression CLOSE_PARENTHESIS { yLog("instruction 5", syntacticLog); $$ = makeFreeCall($3); $$->setPos(line, chr); }
		| functionCall { yLog("instruction 6", syntacticLog); $$ = $1; $$->setPos(line, chr); }
		| lhs OP_AFFECTATION functionCall { yLog("instruction 7", syntacticLog); $$ = makeAffectation($1, $3); $$->setPos(line, chr); }

functionCall:
		IDENTIFIER OPEN_PARENTHESIS functionCallParamsList CLOSE_PARENTHESIS { yLog("functionCall 1", syntacticLog); $$ = makeFunctionCall(makeVar($1), $3); free($1); $$->setPos(line, chr); }
		| IDENTIFIER OPEN_PARENTHESIS CLOSE_PARENTHESIS { yLog("functionCall 2", syntacticLog); $$ = makeFunctionCall(makeVar($1), makeFunctionDeclParamList()); free($1); $$->setPos(line, chr); }

functionCallParamsList:
		arithmeticExpression { yLog("functionCallParamsList 1", syntacticLog); $$ = makeFunctionCallParamList(); $$->addChild($1); $$->setPos(line, chr); }
		| functionCallParamsList OP_COMMA arithmeticExpression { yLog("functionCallParamsList 2", syntacticLog); $1->addChild($3); $$ = $1; $$->setPos(line, chr); }

booleanExpression:
		TRUE_OP { yLog("booleanExpression 1", syntacticLog); $$ = makeBoolConst($1); $$->setPos(line, chr); }
		| FALSE_OP { yLog("booleanExpression 2", syntacticLog); $$ = makeBoolConst($1); $$->setPos(line, chr); }
		| arithmeticExpression OP_EQUAL arithmeticExpression { yLog("booleanExpression 3", syntacticLog); $$ = makeOpBinEqual($1, $3); $$->setPos(line, chr); }
		| arithmeticExpression OP_DIFF arithmeticExpression { yLog("booleanExpression 4", syntacticLog); $$ = makeOpBinDiff($1, $3); $$->setPos(line, chr); }
		| arithmeticExpression OP_INFEQ arithmeticExpression { yLog("booleanExpression 5", syntacticLog); $$ = makeOpBinInfeq($1, $3); $$->setPos(line, chr); }
		| arithmeticExpression OP_INF arithmeticExpression { yLog("booleanExpression 6", syntacticLog); $$ = makeOpBinInf($1, $3); $$->setPos(line, chr); }
		| arithmeticExpression OP_SUPEQ arithmeticExpression { yLog("booleanExpression 7", syntacticLog); $$ = makeOpBinSupeq($1, $3); $$->setPos(line, chr); }
		| arithmeticExpression OP_SUP arithmeticExpression { yLog("booleanExpression 8", syntacticLog); $$ = makeOpBinSup($1, $3); $$->setPos(line, chr); }
		| OP_LOGICAL_NEG booleanExpression { yLog("booleanExpression 9", syntacticLog); $$ = makeOpUnaNot($2); $$->setPos(line, chr); }
		| booleanExpression OP_LOGICAL_AND booleanExpression { yLog("booleanExpression 10", syntacticLog); $$ = makeOpBinAnd($1, $3); $$->setPos(line, chr); }
		| booleanExpression OP_LOGICAL_OR booleanExpression { yLog("booleanExpression 11", syntacticLog); $$ = makeOpBinOr($1, $3); $$->setPos(line, chr); }
		| OPEN_PARENTHESIS booleanExpression CLOSE_PARENTHESIS { yLog("booleanExpression 12", syntacticLog); $$ = $2; $$->setPos(line, chr); }


arithmeticExpression:
		INT_VALUE { yLog("arithmeticExpression 1", syntacticLog); $$ = makeIntConst($1); $$->setPos(line, chr); }
		| FLOAT_VALUE { yLog("arithmeticExpression 2", syntacticLog); $$ = makeFloatConst($1); $$->setPos(line, chr); }
		| lhs { yLog("arithmeticExpression 3", syntacticLog); $$ = $1; $$->setPos(line, chr); }
		| OP_SUB arithmeticExpression { yLog("arithmeticExpression 4", syntacticLog); $$ = makeOpUnaSub($2); $$->setPos(line, chr); }
		| arithmeticExpression OP_ADD arithmeticExpression { yLog("arithmeticExpression 5", syntacticLog); $$ = makeOpBinAdd($1, $3); $$->setPos(line, chr); }
		| arithmeticExpression OP_SUB arithmeticExpression { yLog("arithmeticExpression 6", syntacticLog); $$ = makeOpBinSub($1, $3); $$->setPos(line, chr); }
		| arithmeticExpression OP_MUL arithmeticExpression { yLog("arithmeticExpression 7", syntacticLog); $$ = makeOpBinMul($1, $3); $$->setPos(line, chr); }
		| arithmeticExpression OP_DIV arithmeticExpression { yLog("arithmeticExpression 8", syntacticLog); $$ = makeOpBinDiv($1, $3); $$->setPos(line, chr); }
		| OPEN_PARENTHESIS arithmeticExpression CLOSE_PARENTHESIS { yLog("arithmeticExpression 9", syntacticLog); $$ = $2; $$->setPos(line, chr); }

lhs:
		IDENTIFIER { std::string log = "lhs 1 "; log += $1; yLog(log, syntacticLog); $$ = makeVar($1); free($1); $$->setPos(line, chr); }
		| IDENTIFIER OPEN_SQUARE arithmeticExpression CLOSE_SQUARE { std::string log = "lhs 2 "; log += $1; yLog(log, syntacticLog); $$ = makeArrayAccess(makeVar($1), $3); free($1); $$->setPos(line, chr); }

types:
		TYPE { yLog("types 1", syntacticLog); $$ = makeType($1); free($1); $$->setPos(line, chr); }
		| ARRAY_TYPE OPEN_PARENTHESIS types CLOSE_PARENTHESIS { yLog("types 2", syntacticLog); $$ = makeArrayType($3); $$->setPos(line, chr); }

%%


void yLog(const std::string& msg, bool syntacticLog)
{
	if(syntacticLog)
		std::cout << "{" << msg << "}" << std::endl;
}


void yyerror(Node*& program, int& line, int& chr, bool lexicalLog, bool syntacticLog, const char* msg)
{
	std::cerr << "At line " << line << " column " << chr << " : " << msg << std::endl;
}


namespace ast
{
	ast::Node* build(bool lexicalLog, bool syntacticLog)
	{
		Node* program = 0;
		int line = 1;
		int chr = 1;

		if(yyparse(program, line, chr, lexicalLog, syntacticLog) > 0)
		{
			yylex_destroy();
			return 0;
		}

		yylex_destroy();
		return program;
	}
}


