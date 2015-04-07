#include "typage.hpp"

#include "ast.hpp"

#include <iostream>

using namespace ast;
using namespace std;

FunctionTable * createFunctionTable(ast::Node* program)
{
	std::vector<Node*> children = program->children();

	FunctionTable * table = new FunctionTable();

	for(unsigned int i = 0 ; i < children.size() - 1 ; ++i)
	{
		table->addFunction(children[i]);
	}

	return table;
}

void processType(ast::Node* program, FunctionTable *tableFunction)
{
	SymbolTable * tableSymbol = new SymbolTable();

	for(unsigned int i = 0 ; i < program->children().size() ; ++i)
	{
		process(program->child(i), tableFunction, tableSymbol, program);
	}

	delete tableSymbol;
}

void process(ast::Node* node, FunctionTable * tableFunction, SymbolTable * tableSymbol, Node* parent)
{
	Node::Type parentType = parent->type();
	if(node->type() == Node::BLOCK || node->type() == Node::FUNCTION_DECL)
	{
		tableSymbol->push();
	}

	for(unsigned int i = 0 ; i < node->children().size() ; ++i)
	{
		process(node->children()[i], tableFunction, tableSymbol, node);
	}

	switch(node->type())
	{
		case Node::TYPE:
			node->setMetadataType(typeToString(reinterpret_cast<ast::StrNode*>(node)));
			break;
		case Node::CONST_FLOAT:
			node->setMetadataType(FLOAT_TYPE);
			break;
		case Node::CONST_INT:
			node->setMetadataType(INT_TYPE);
			break;
		case Node::CONST_BOOL:
		case Node::OP_UNA_LOG_NOT:
		case Node::OP_BIN_LOG_OR:
		case Node::OP_BIN_LOG_AND:
			node->setMetadataType(BOOLEAN_TYPE);
			break;

		case Node::ARRAY_ACCESS:
			if(!isArrayType(tableSymbol->getType(extractIdentifier(node->child(0)))))
			{
				throwError(node, std::string("Unable to realize array access in variable of type ") + tableSymbol->getType(extractIdentifier(node->child(0))));
			}
			node->setMetadataType(getSubArrayType(tableSymbol->getType(extractIdentifier(node->child(0)))));
			break;
		case Node::DECLARATION:
			processDeclaration(node, tableSymbol);
			break;
		case Node::ALLOCATION:
			if(node->child(0)->metadataType() != getSubArrayType(node->child(1)->metadataType()))
			{
				throwError(node, std::string("Unable to allocate type ") + node->child(0)->metadataType() + std::string(" in variable of type ") + tableSymbol->getType(extractIdentifier(node->child(1))));
			}
			else if(node->child(2)->metadataType() != INT_TYPE)
			{
				throwError(node, std::string("can not allocate a number of object of type ") + node->child(2)->metadataType());
			}
			break;
		case Node::RETURN:
			if(node->hasChild())
			{
				node->setMetadataType(node->child(0)->metadataType());
			}
			else
			{
				node->setMetadataType(VOID_TYPE);
			}
			parent->mark();
			break;

		case Node::IDENTIFIER:
			processIdentifier(node, tableSymbol, parentType);
			break;

		case Node::FREE_CALL:
			if(!isArrayType(tableSymbol->getType(extractIdentifier(node->child(0)))))
			{
				throwError(node, "Unable to realize free operation on non array type.");
			}
			break;

		case Node::FUNCTION_DECL_PARAM:
			tableSymbol->addVariable(
					extractIdentifier(node->child(1)),
					node->child(0)->metadataType()
			);
			break;
		case Node::FUNCTION_DECL:
			processFunctionDecl(node, tableFunction->returnType(extractIdentifier(node->child(1))));
			break;

		case Node::FUNCTION_CALL:
			if(!tableFunction->isCallValid(node))
			{
				throwError(node, std::string("Unable to call function ") + extractIdentifier(node->child(0)));
			}
			node->setMetadataType(tableFunction->returnType(extractIdentifier(node->child(0))));
			break;

		case Node::OP_BIN_ARM_SUB:
		case Node::OP_BIN_ARM_ADD:
		case Node::OP_BIN_ARM_MUL:
		case Node::OP_BIN_ARM_DIV:
		case Node::AFFECTATION:
			checkChildrenType(node);
		case Node::OP_UNA_ARM_SUB:
			node->setMetadataType(node->child(0)->metadataType());
			break;

		case Node::OP_BIN_LOG_EQUAL:
		case Node::OP_BIN_LOG_DIFF:
		case Node::OP_BIN_LOG_INFEQ:
		case Node::OP_BIN_LOG_INF:
		case Node::OP_BIN_LOG_SUPEQ:
		case Node::OP_BIN_LOG_SUP:
			checkChildrenType(node);
			node->setMetadataType(BOOLEAN_TYPE);
			break;

		default:
			break;
	}

	switch(node->type())
	{
		case Node::CONDITION_GROUP:
			if(node->child(0)->isMarked() && node->child(1)->isMarked())
				node->mark();
			break;
		case Node::FUNCTION_DECL:
			if(hasChildrenMarked(node))
				node->mark();
			if(!node->isMarked() && tableFunction->returnType(extractIdentifier(node->child(1))) != VOID_TYPE)
				throwError(node, std::string("not all code path return value type #") + node->metadataType() + "#");
			break;
		default:
			if(hasChildrenMarked(node))
				node->mark();
			break;
	}

	if(node->type() == Node::BLOCK || node->type() == Node::FUNCTION_DECL)
	{
		tableSymbol->pop();
	}
}

bool hasChildrenMarked(ast::Node* node)
{
	for(unsigned int i = 0 ; i < node->children().size() ; ++i)
	{
		if(node->child(i)->isMarked())
			return true;
	}
	return false;
}

void processDeclaration(ast::Node* node, SymbolTable * tableSymbol)
{
	std::string type = node->child(0)->metadataType();

	ast::Node* idList = node->child(1);
	for(unsigned int i = 0 ; i < idList->children().size() ; ++i)
	{
		idList->child(i)->setMetadataType(type);
		tableSymbol->addVariable(extractIdentifier(idList->child(i)), type);
	}
}

void processIdentifier(ast::Node* node, SymbolTable* tableSymbol, ast::Node::Type parentType)
{
	// en fonction du parent
	/* Si parent = [FUNCTION_DECL_PARAM | IDENTIFIER_LIST] => declaration
	 * Si parent = [FUNCTION_CALL] => appel de fonction => vérification dans table des fonctions plus tard
	 * Sinon vérification de l'existence
	 */
	if(parentType != Node::IDENTIFIERLIST && parentType != Node::FUNCTION_CALL && parentType != Node::FUNCTION_DECL_PARAM && parentType != Node::FUNCTION_DECL)
	{
		if(!tableSymbol->exist(extractIdentifier(node)))
		{
			throwError(node, std::string("Variable ") + extractIdentifier(node) + " does not exists.");
		}

		node->setMetadataType(tableSymbol->getType(extractIdentifier(node)));
	}
}

void processFunctionDecl(ast::Node* node, const std::string & returnType)
{
	if(node->type() == Node::RETURN && node->metadataType() != returnType)
	{
		throwError(node, std::string("return with value of type ") + node->metadataType() + " expected value " + returnType);
	}
	else
	{
		for(unsigned int i = 0 ; i < node->children().size() ; ++i)
		{
			processFunctionDecl(node->child(i), returnType);
		}
	}
}

std::string extractIdentifier(ast::Node* node)
{
	return reinterpret_cast<StrNode*>(node)->value();
}

void throwError(ast::Node* node, const std::string & messageError)
{
	char buff[64];
	sprintf(buff, "At %d:%d : ", node->line(), node->chr());
	throw buff + std::string("(type: ") + errorTypeToString(node->type()) + ") : " + messageError;
}


void checkChildrenType(ast::Node* node)
{
	if(!isChildrenTypeEqual(node))
	{
		throwError(node, "Both part of expression are not the same type.");
	}
}

bool isChildrenTypeEqual(ast::Node* node)
{
	if(node->children().size() > 0)
	{
		std::string t = node->child(0)->metadataType();
		for(unsigned int i = 1 ; i < node->children().size() ; ++i)
		{
			if(t != node->child(i)->metadataType())
			{
				return false;
			}
		}
		return true;
	}
	return false;
}

#define TO_STRING(X) #X

std::string errorTypeToString(ast::Node::Type type)
{
	switch(type)
	{
		case ast::Node::PROGRAM: return TO_STRING(PROGRAM);
		case ast::Node::TYPE: return TO_STRING(TYPE);
		case ast::Node::IDENTIFIER: return TO_STRING(IDENTIFIER);
		case ast::Node::ARRAY_ACCESS: return TO_STRING(ARRAY_ACCESS);
		case ast::Node::BLOCK: return TO_STRING(BLOCK);
		case ast::Node::DECLARATION: return TO_STRING(DECLARATION);
		case ast::Node::IDENTIFIERLIST: return TO_STRING(IDENTIFIERLIST);
		case ast::Node::DECLARATIONLIST: return TO_STRING(DECLARATIONLIST);
		case ast::Node::INSCRUCTIONLIST: return TO_STRING(INSCRUCTIONLIST);
		case ast::Node::AFFECTATION: return TO_STRING(AFFECTATION);
		case ast::Node::ALLOCATION: return TO_STRING(ALLOCATION);
		case ast::Node::RETURN: return TO_STRING(RETURN);
		case ast::Node::CONST_FLOAT: return TO_STRING(CONST_FLOAT);
		case ast::Node::CONST_INT: return TO_STRING(CONST_INT);
		case ast::Node::CONST_BOOL: return TO_STRING(CONST_BOOL);
		case ast::Node::CONDITION_GROUP: return TO_STRING(CONDITION_GROUP);
		case ast::Node::CONDITION_IF: return TO_STRING(CONDITION_IF);
		case ast::Node::CONDITION_ELSE: return TO_STRING(CONDITION_ELSE);
		case ast::Node::WHILE_LOOP: return TO_STRING(WHILE_LOOP);
		case ast::Node::FUNCTION_DECL: return TO_STRING(FUNCTION_DECL);
		case ast::Node::FUNCTION_DECL_PARAMLIST: return TO_STRING(FUNCTION_DECL_PARAMLIST);
		case ast::Node::FUNCTION_DECL_PARAM: return TO_STRING(FUNCTION_DECL_PARAM);
		case ast::Node::FREE_CALL: return TO_STRING(FREE_CALL);
		case ast::Node::FUNCTION_CALL: return TO_STRING(FUNCTION_CALL);
		case ast::Node::FUNCTION_CALL_PARAMLIST: return TO_STRING(FUNCTION_CALL_PARAMLIST);
		case ast::Node::OP_UNA_ARM_SUB: return TO_STRING(OP_UNA_ARM_SUB);
		case ast::Node::OP_UNA_LOG_NOT: return TO_STRING(OP_UNA_LOG_NOT);
		case ast::Node::OP_BIN_ARM_SUB: return TO_STRING(OP_BIN_ARM_SUB);
		case ast::Node::OP_BIN_ARM_ADD: return TO_STRING(OP_BIN_ARM_ADD);
		case ast::Node::OP_BIN_ARM_MUL: return TO_STRING(OP_BIN_ARM_MUL);
		case ast::Node::OP_BIN_ARM_DIV: return TO_STRING(OP_BIN_ARM_DIV);
		case ast::Node::OP_BIN_LOG_OR: return TO_STRING(OP_BIN_LOG_OR);
		case ast::Node::OP_BIN_LOG_AND: return TO_STRING(OP_BIN_LOG_AND);
		case ast::Node::OP_BIN_LOG_EQUAL: return TO_STRING(OP_BIN_LOG_EQUAL);
		case ast::Node::OP_BIN_LOG_DIFF: return TO_STRING(OP_BIN_LOG_DIFF);
		case ast::Node::OP_BIN_LOG_INFEQ: return TO_STRING(OP_BIN_LOG_INFEQ);
		case ast::Node::OP_BIN_LOG_INF: return TO_STRING(OP_BIN_LOG_INF);
		case ast::Node::OP_BIN_LOG_SUPEQ: return TO_STRING(OP_BIN_LOG_SUPEQ);
		case ast::Node::OP_BIN_LOG_SUP: return TO_STRING(OP_BIN_LOG_SUP);
		default: return "unknown_type";
	}
}

#undef TO_STRING
