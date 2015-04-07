%{
	#include <string>
	#include <cstdlib>
    #include <iostream>

	#include "code.y.hpp"

	#define YY_DECL int yylex(int& line, int& chr, bool lexicalLog)

	void lexLog(const std::string& msg, bool lexicalLog);

	using namespace std;
%}


/* Ne permet pas de changer de flux a la fin d'un autre */
%option noyywrap

PROGRAM                 program
TYPE                    int|decimal
ARRAY_TYPE              array
RETURN                  return
IF                      if
THEN                    then
ELSE                    else
WHILE                   while
DO                      do
TRUE_OP                 true
FALSE_OP                false
BEGIN_OP                begin
END                     end
FUNCTION                function
PROCEDURE               procedure
IS                      is
NEW                     new
FREE                    free
OP_AFFECTATION          :=
OP_COLON                :
OP_SEMICOLON            ;
OP_COMMA                ,
OPEN_PARENTHESIS        \(
CLOSE_PARENTHESIS       \)
OPEN_SQUARE             \[
CLOSE_SQUARE            \]

IDENTIFIER              [a-z][a-zA-Z0-9]*
FLOAT_VALUE             [0-9]+\.[0-9]+
INT_VALUE               [0-9]+

OP_SUB                  \-
OP_ADD                  \+
OP_MUL                  \*
OP_DIV                  \/
OP_EQUAL                =
OP_DIFF                 <>
OP_INFEQ                <=
OP_INF                  <
OP_SUPEQ                >=
OP_SUP                  >

OP_LOGICAL_AND          &&
OP_LOGICAL_OR           \|\|
OP_LOGICAL_NEG          !

END_OF_LINE             \n


%%

{PROGRAM}               {lexLog("PROGRAM", lexicalLog); chr += yyleng; return PROGRAM;}
{TYPE}                  {lexLog("TYPE:\"" + string(yytext) + "\"", lexicalLog); chr += yyleng; yylval.strValue = strdup(yytext); return TYPE;}
{ARRAY_TYPE}            {lexLog("ARRAY_TYPE:\"" + string(yytext) + "\"", lexicalLog); chr += yyleng; return ARRAY_TYPE;}
{RETURN}                {lexLog("RETURN", lexicalLog); chr += yyleng; return RETURN;}
{IF}                    {lexLog("IF", lexicalLog); chr += yyleng; return IF;}
{THEN}                  {lexLog("THEN", lexicalLog); chr += yyleng; return THEN;}
{ELSE}                  {lexLog("ELSE", lexicalLog); chr += yyleng; return ELSE;}
{WHILE}                 {lexLog("WHILE", lexicalLog); chr += yyleng; return WHILE;}
{DO}                    {lexLog("DO", lexicalLog); chr += yyleng; return DO;}
{TRUE_OP}               {lexLog("TRUE_OP", lexicalLog); chr += yyleng; yylval.booleanValue = true; return TRUE_OP;}
{FALSE_OP}              {lexLog("FALSE_OP", lexicalLog); chr += yyleng; yylval.booleanValue = false; return FALSE_OP;}
{BEGIN_OP}              {lexLog("BEGIN_OP", lexicalLog); chr += yyleng; return BEGIN_OP;}
{END}                   {lexLog("END", lexicalLog); chr += yyleng; return END;}
{FUNCTION}              {lexLog("FUNCTION", lexicalLog); chr += yyleng; return FUNCTION;}
{PROCEDURE}             {lexLog("PROCEDURE", lexicalLog); chr += yyleng; return PROCEDURE;}
{IS}                    {lexLog("IS", lexicalLog); chr += yyleng; return IS;}
{NEW}                   {lexLog("NEW", lexicalLog); chr += yyleng; return NEW;}
{FREE}                  {lexLog("FREE", lexicalLog); chr += yyleng; return FREE;}
{OP_AFFECTATION}        {lexLog("OP_AFFECTATION", lexicalLog); chr += yyleng; return OP_AFFECTATION;}
{OP_COLON}              {lexLog("OP_COLON", lexicalLog); chr += yyleng; return OP_COLON;}
{OP_SEMICOLON}          {lexLog("OP_SEMICOLON", lexicalLog); chr += yyleng; return OP_SEMICOLON;}
{OP_COMMA}              {lexLog("OP_COMMA", lexicalLog); chr += yyleng; return OP_COMMA;}
{OPEN_PARENTHESIS}      {lexLog("OPEN_PARENTHESIS", lexicalLog); chr += yyleng; return OPEN_PARENTHESIS;}
{CLOSE_PARENTHESIS}     {lexLog("CLOSE_PARENTHESIS", lexicalLog); chr += yyleng; return CLOSE_PARENTHESIS;}
{OPEN_SQUARE}           {lexLog("OPEN_SQUARE", lexicalLog); chr += yyleng; return OPEN_SQUARE;}
{CLOSE_SQUARE}          {lexLog("CLOSE_SQUARE", lexicalLog); chr += yyleng; return CLOSE_SQUARE;}

{IDENTIFIER}            {lexLog("IDENTIFIER:\"" + string(yytext) + "\"", lexicalLog); yylval.strValue = strdup(yytext); chr += yyleng; return IDENTIFIER;}
{FLOAT_VALUE}           {lexLog("FLOAT_VALUE:\"" + string(yytext) + "\"", lexicalLog); yylval.floatValue = ::atof(yytext); chr += yyleng; return FLOAT_VALUE;}
{INT_VALUE}             {lexLog("INT_VALUE:\"" + string(yytext) + "\"", lexicalLog); yylval.intValue = ::atoi(yytext); chr += yyleng; return INT_VALUE;}

{OP_SUB}                {lexLog("OP_SUB", lexicalLog); chr += yyleng; return OP_SUB;}
{OP_ADD}                {lexLog("OP_ADD", lexicalLog); chr += yyleng; return OP_ADD;}
{OP_MUL}                {lexLog("OP_MUL", lexicalLog); chr += yyleng; return OP_MUL;}
{OP_DIV}                {lexLog("OP_DIV", lexicalLog); chr += yyleng; return OP_DIV;}
{OP_EQUAL}              {lexLog("OP_EQUAL", lexicalLog); chr += yyleng; return OP_EQUAL;}
{OP_DIFF}               {lexLog("OP_DIFF", lexicalLog); chr += yyleng; return OP_DIFF;}
{OP_INFEQ}              {lexLog("OP_INFEQ", lexicalLog); chr += yyleng; return OP_INFEQ;}
{OP_INF}                {lexLog("OP_SUB", lexicalLog); chr += yyleng; return OP_INF;}
{OP_SUPEQ}              {lexLog("OP_SUPEQ", lexicalLog); chr += yyleng; return OP_SUPEQ;}
{OP_SUP}                {lexLog("OP_SUP", lexicalLog); chr += yyleng; return OP_SUP;}


{OP_LOGICAL_AND}        {lexLog("OP_LOGICAL_AND", lexicalLog); chr += yyleng; return OP_LOGICAL_AND;}
{OP_LOGICAL_OR}         {lexLog("OP_LOGICAL_OR", lexicalLog); chr += yyleng; return OP_LOGICAL_OR;}
{OP_LOGICAL_NEG}        {lexLog("OP_LOGICAL_NEG", lexicalLog); chr += yyleng; return OP_LOGICAL_NEG;}

{END_OF_LINE}           {lexLog("END_OF_LINE", lexicalLog); line++; chr = 1;}
[[:blank:]]             {chr += yyleng;}
.                       {std::cerr << "error at line " << line << " column " << chr << " : unknown character \"" << yytext[0] << "\"" << std::endl; return -1;}



%%


void lexLog(const string& msg, bool lexicalLog)
{
	if(lexicalLog)
		std::cout << "[" << msg << "]" << std::endl;
}

