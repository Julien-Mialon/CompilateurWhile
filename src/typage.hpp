#ifndef TYPAGE_HPP
#define TYPAGE_HPP

#include "ast.hpp"
#include "functions.hpp"
#include "functiontable.hpp"
#include "symboltable.hpp"

FunctionTable * createFunctionTable(ast::Node* program);

void processType(ast::Node* program, FunctionTable * tableFunction);

void process(ast::Node* node, FunctionTable * tableFunction, SymbolTable * tableSymbol, ast::Node* parent);

void processDeclaration(ast::Node* node, SymbolTable * tableSymbol);
void processIdentifier(ast::Node* node, SymbolTable* tableSymbol, ast::Node::Type parentType);
void processFunctionDecl(ast::Node* node, const std::string & returnType);

/**
 * @brief isChildrenTypeEqual Check if all children of the node has the same type
 * @param node the node to check.
 * @return true if all children type are the same.
 */
bool isChildrenTypeEqual(ast::Node* node);

void checkChildrenType(ast::Node* node);

void throwError(ast::Node* node, const std::string & messageError);

std::string extractIdentifier(ast::Node* node);

std::string errorTypeToString(ast::Node::Type type);

bool hasChildrenMarked(ast::Node* node);

#endif // TYPAGE_HPP
