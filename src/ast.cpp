#include "ast.hpp"

#include <sstream>

#include "functions.hpp"


#define TO_STRING(X) #X

using namespace std;


ast::Node* makeBinaryOp(ast::Node::Type type, ast::Node* left, ast::Node* right);
const char* typeToString(ast::Node::Type type);
void subMakeDot(ast::Node* astRoot, FILE* file, bool showTypes);


// Node class methods


ast::Node::Node(Type type) : _type(type), _line(-1), _chr(-1), _returnMark(false)
{

}


ast::Node::~Node()
{

}

void ast::Node::mark()
{
	_returnMark = true;
}

bool ast::Node::isMarked() const
{
	return _returnMark;
}

bool ast::Node::hasChild() const
{
	return children().size() > 0;
}

const ast::Node* ast::Node::child(unsigned int i) const
{
	if(i < children().size())
	{
		return children()[i];
	}
	return NULL;
}

ast::Node* ast::Node::child(unsigned int i)
{
	if(i < children().size())
	{
		return children()[i];
	}
	return NULL;
}

void ast::Node::addChild(Node* child)
{
	_children.push_back(child);
}


ast::Node::Type ast::Node::type() const
{
	return _type;
}


std::string ast::Node::metadataType() const
{
	return _metadataType;
}


void ast::Node::setMetadataType(const string& value)
{
	_metadataType = value;
}


const vector<ast::Node*>& ast::Node::children() const
{
	return _children;
}


vector<ast::Node*>& ast::Node::children()
{
	return _children;
}


void ast::Node::deleteAllSubNode()
{
	for(unsigned int i=0 ; i<_children.size() ; ++i)
	{
		_children[i]->deleteAllSubNode();
		delete _children[i];
	}

	_children.clear();
}

int ast::Node::line() const
{
	return _line;
}

int ast::Node::chr() const
{
	return _chr;
}

void ast::Node::setPos(int line, int chr)
{
	_line = line;
	_chr = chr;
}


// IntNode class methods


ast::IntNode::IntNode(Type type, int value) : Node(type), _value(value)
{

}


int ast::IntNode::value() const
{
	return _value;
}


// FloatNode class methods


ast::FloatNode::FloatNode(Type type, float value) : Node(type), _value(value)
{

}


float ast::FloatNode::value() const
{
	return _value;
}


// StrNode class methods


ast::StrNode::StrNode(Type type, const string& value) : Node(type), _value(value)
{

}


ast::StrNode::~StrNode()
{

}


string ast::StrNode::value() const
{
	return _value;
}


// User friendly functions to build the AST


ast::Node* ast::makeProgram()
{
	return new Node(Node::PROGRAM);
}


ast::Node* ast::makeVoidType()
{
	return new StrNode(Node::TYPE, "void");
}


ast::Node* ast::makeArrayType(Node* subType)
{
	Node* arrayNode = new StrNode(Node::TYPE, "array");
	arrayNode->addChild(subType);

	return arrayNode;
}


ast::Node* ast::makeType(const char* type)
{
	return new StrNode(Node::TYPE, type);
}


ast::Node* ast::makeVar(const char* identifier)
{
	return new StrNode(Node::IDENTIFIER, identifier);
}


ast::Node* ast::makeArrayAccess(Node* arrayIdentifier, Node* indexIdentifier)
{
	Node* arrayAccessNode = new Node(Node::ARRAY_ACCESS);
	arrayAccessNode->addChild(arrayIdentifier);
	arrayAccessNode->addChild(indexIdentifier);

	return arrayAccessNode;
}


ast::Node* ast::makeBlock()
{
	return new Node(Node::BLOCK);
}


ast::Node* ast::makeDeclaration(Node* type, Node* identifier)
{
	Node* declNode = new Node(Node::DECLARATION);
	declNode->addChild(type);
	declNode->addChild(identifier);

	return declNode;
}


ast::Node* ast::makeIdentifierList()
{
	return new Node(Node::IDENTIFIERLIST);
}


ast::Node* ast::makeDeclarationList()
{
	return new Node(Node::DECLARATIONLIST);
}


ast::Node* ast::makeInstructionList()
{
	return new Node(Node::INSCRUCTIONLIST);
}


ast::Node* ast::makeAffectation(Node* identifier, Node* expression)
{
	Node* affectNode = new Node(Node::AFFECTATION);
	affectNode->addChild(identifier);
	affectNode->addChild(expression);

	return affectNode;
}


ast::Node* ast::makeAllocation(Node* type, Node* identifier, Node* expression)
{
	Node* allocNode = new Node(Node::ALLOCATION);
	allocNode->addChild(type);
	allocNode->addChild(identifier);
	allocNode->addChild(expression);

	return allocNode;
}


ast::Node* ast::makeReturn()
{
	return new Node(Node::RETURN);
}


ast::Node* ast::makeReturn(Node* expression)
{
	Node* retNode = new Node(Node::RETURN);
	retNode->addChild(expression);

	return retNode;
}


ast::Node* ast::makeFloatConst(float constValue)
{
	return new FloatNode(Node::CONST_FLOAT, constValue);
}


ast::Node* ast::makeIntConst(int constValue)
{
	return new IntNode(Node::CONST_INT, constValue);
}


ast::Node* ast::makeBoolConst(bool constValue)
{
	return new IntNode(Node::CONST_BOOL, constValue);
}


ast::Node* ast::makeConditionGroup()
{
	return new Node(Node::CONDITION_GROUP);
}


ast::Node* ast::makeIfCondition(Node* condExpression, Node* block)
{
	Node* condNode = new Node(Node::CONDITION_IF);
	condNode->addChild(condExpression);
	condNode->addChild(block);

	return condNode;
}


ast::Node* ast::makeElseCondition(Node* block)
{
	Node* condNode = new Node(Node::CONDITION_ELSE);
	condNode->addChild(block);

	return condNode;
}


ast::Node* ast::makeWhileLoop(Node* condExpression, Node* block)
{
	Node* loopNode = new Node(Node::WHILE_LOOP);
	loopNode->addChild(condExpression);
	loopNode->addChild(block);

	return loopNode;
}


ast::Node* ast::makeFunctionDecl(Node* type, Node* identifier, Node* params, Node* bloc)
{
	Node* functionNode = new Node(Node::FUNCTION_DECL);
	functionNode->addChild(type);
	functionNode->addChild(identifier);
	functionNode->addChild(params);
	functionNode->addChild(bloc);

	return functionNode;
}


ast::Node* ast::makeFunctionDeclParamList()
{
	return new Node(Node::FUNCTION_DECL_PARAMLIST);
}


ast::Node* ast::makeFunctionDeclParam(Node* type, Node* identifier)
{
	Node* paramNode = new Node(Node::FUNCTION_DECL_PARAM);
	paramNode->addChild(type);
	paramNode->addChild(identifier);

	return paramNode;
}


ast::Node* ast::makeFreeCall(Node* expression)
{
	Node* declNode = new Node(Node::FREE_CALL);
	declNode->addChild(expression);

	return declNode;
}


ast::Node* ast::makeFunctionCall(Node* identifier, Node* params)
{
	Node* declNode = new Node(Node::FUNCTION_CALL);
	declNode->addChild(identifier);
	declNode->addChild(params);

	return declNode;
}


ast::Node* ast::makeFunctionCallParamList()
{
	return new Node(Node::FUNCTION_CALL_PARAMLIST);
}


ast::Node* makeBinaryOp(ast::Node::Type type, ast::Node* left, ast::Node* right)
{
	ast::Node* opNode = new ast::Node(type);
	opNode->addChild(left);
	opNode->addChild(right);

	return opNode;
}


ast::Node* ast::makeOpUnaSub(Node* expression)
{
	ast::Node* opNode = new ast::Node(Node::OP_UNA_ARM_SUB);
	opNode->addChild(expression);

	return opNode;
}


ast::Node* ast::makeOpUnaNot(Node* expression)
{
	ast::Node* opNode = new ast::Node(Node::OP_UNA_LOG_NOT);
	opNode->addChild(expression);

	return opNode;
}


ast::Node* ast::makeOpBinSub(Node* left, Node* right)
{
	return makeBinaryOp(Node::OP_BIN_ARM_SUB, left, right);
}


ast::Node* ast::makeOpBinAdd(Node* left, Node* right)
{
	return makeBinaryOp(Node::OP_BIN_ARM_ADD, left, right);
}


ast::Node* ast::makeOpBinMul(Node* left, Node* right)
{
	return makeBinaryOp(Node::OP_BIN_ARM_MUL, left, right);
}


ast::Node* ast::makeOpBinDiv(Node* left, Node* right)
{
	return makeBinaryOp(Node::OP_BIN_ARM_DIV, left, right);
}


ast::Node* ast::makeOpBinOr(Node* left, Node* right)
{
	return makeBinaryOp(Node::OP_BIN_LOG_OR, left, right);
}


ast::Node* ast::makeOpBinAnd(Node* left, Node* right)
{
	return makeBinaryOp(Node::OP_BIN_LOG_AND, left, right);
}


ast::Node* ast::makeOpBinEqual(Node* left, Node* right)
{
	return makeBinaryOp(Node::OP_BIN_LOG_EQUAL, left, right);
}


ast::Node* ast::makeOpBinDiff(Node* left, Node* right)
{
	return makeBinaryOp(Node::OP_BIN_LOG_DIFF, left, right);
}


ast::Node* ast::makeOpBinInfeq(Node* left, Node* right)
{
	return makeBinaryOp(Node::OP_BIN_LOG_INFEQ, left, right);
}


ast::Node* ast::makeOpBinInf(Node* left, Node* right)
{
	return makeBinaryOp(Node::OP_BIN_LOG_INF, left, right);
}


ast::Node* ast::makeOpBinSupeq(Node* left, Node* right)
{
	return makeBinaryOp(Node::OP_BIN_LOG_SUPEQ, left, right);
}


ast::Node* ast::makeOpBinSup(Node* left, Node* right)
{
	return makeBinaryOp(Node::OP_BIN_LOG_SUP, left, right);
}


void ast::deleteTree(Node* root)
{
	if(root != 0)
	{
		root->deleteAllSubNode();
		delete root;
	}
}


// Affichage de l'AST dans un DOT


void ast::makeDot(Node* astRoot, const string& filename, bool showTypes)
{
	FILE* file = fopen(filename.c_str(), "w");

	if(!file)
		throw runtime_error("unable to open the dot file \"" + filename + "\" in writing mode");

	const string begin = "digraph G\n{\n";
	const string end = "}";

	fwrite(begin.c_str(), begin.size(), 1, file);
	subMakeDot(astRoot, file, showTypes);
	fwrite(end.c_str(), end.size(), 1, file);

	fclose(file);
}


const char* typeToString(ast::Node::Type type)
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


void subMakeDot(ast::Node* astRoot, FILE* file, bool showTypes)
{
	ostringstream ss;

	const vector<ast::Node*>& children = astRoot->children();

	if(showTypes && astRoot->metadataType() != "")
		ss << "<br/><font color=\"blue\">type:" << unwrapType(astRoot->metadataType()) << "</font>";

	const string type = ss.str();
	ss.str("");

	switch(astRoot->type())
	{
		case ast::Node::TYPE:
		case ast::Node::IDENTIFIER:
			ss << "\t" << long(astRoot) << " [label=<node:" << typeToString(astRoot->type()) << "<br/>value:" << ((ast::StrNode*)astRoot)->value() << type << ">];" << endl;
			break;

		case ast::Node::CONST_FLOAT:
			ss << "\t" << long(astRoot) << " [label=<node:" << typeToString(astRoot->type()) << "<br/>value:" << ((ast::FloatNode*)astRoot)->value() << type << ">];" << endl;
			break;

		case ast::Node::CONST_INT:
			ss << "\t" << long(astRoot) << " [label=<node:" << typeToString(astRoot->type()) << "<br/>value:" << ((ast::IntNode*)astRoot)->value() << type << ">];" << endl;
			break;
		case ast::Node::CONST_BOOL:
			ss << "\t" << long(astRoot) << " [label=<node:" << typeToString(astRoot->type()) << "<br/>value:" << ((((ast::IntNode*)astRoot)->value() == 0) ? "false" : "true") << type << ">];" << endl;
			break;
		default:
			ss << "\t" << long(astRoot) << " [label=<node:" << typeToString(astRoot->type()) << type << ">];" << endl;
			break;
	}

	const string content = ss.str();
	fwrite(content.c_str(), content.size(), 1, file);

	for(unsigned int i=0 ; i<children.size() ; ++i)
	{
		char buff[64];
		sprintf(buff, "\t%ld -> %ld;\n", long(astRoot), long(children[i]));
		fwrite(buff, strlen(buff), 1, file);

		subMakeDot(children[i], file, showTypes);
	}
}


