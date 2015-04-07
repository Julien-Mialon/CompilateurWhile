#ifndef FUNCTIONS_HEADER
#define FUNCTIONS_HEADER

#define ARRAY_DELIMITER '#'

#define FLOAT_TYPE		"decimal"
#define INT_TYPE		"int"
#define BOOLEAN_TYPE	"boolean"
#define VOID_TYPE		"void"
#define ARRAY_TYPE		"array"

#include <string>
#include "ast.hpp"

std::string typeToString(ast::StrNode *typeNode);

bool isArrayType(const std::string & type);

std::string getSubArrayType(const std::string & type);

std::string unwrapType(const std::string & type);

#endif
