#ifndef AST_HPP
#define AST_HPP


#include <cstdio>
#include <cstring>
#include <string>
#include <stdexcept>
#include <vector>


namespace ast
{
	class Node
	{
		public:

			enum Type
			{
				PROGRAM = 1,

				TYPE = 2,
				IDENTIFIER = 3,
				ARRAY_ACCESS = 4,
				BLOCK = 5,
				DECLARATION = 6,
				IDENTIFIERLIST = 7,
				DECLARATIONLIST = 8,
				INSCRUCTIONLIST = 9,
				AFFECTATION = 10,
				ALLOCATION = 11,
				RETURN = 12,

				CONST_FLOAT = 13,
				CONST_INT = 14,
				CONST_BOOL = 15,

				CONDITION_GROUP = 16,
				CONDITION_IF = 17,
				CONDITION_ELSE = 18,

				WHILE_LOOP = 19,

				FUNCTION_DECL = 20,
				FUNCTION_DECL_PARAMLIST = 21,
				FUNCTION_DECL_PARAM = 22,

				FREE_CALL = 23,
				FUNCTION_CALL = 24,
				FUNCTION_CALL_PARAMLIST = 25,

				OP_UNA_ARM_SUB = 26,
				OP_UNA_LOG_NOT = 27,

				OP_BIN_ARM_SUB = 28,
				OP_BIN_ARM_ADD = 29,
				OP_BIN_ARM_MUL = 30,
				OP_BIN_ARM_DIV = 31,

				OP_BIN_LOG_OR = 32,
				OP_BIN_LOG_AND = 33,

				OP_BIN_LOG_EQUAL = 34,
				OP_BIN_LOG_DIFF = 35,
				OP_BIN_LOG_INFEQ = 36,
				OP_BIN_LOG_INF = 37,
				OP_BIN_LOG_SUPEQ = 38,
				OP_BIN_LOG_SUP = 39
			};


		protected:

			std::vector<Node*> _children;
			Node::Type _type;
			int _line;
			int _chr;
			std::string _metadataType;
			bool _returnMark;


		public:

			Node(Type type);
			virtual ~Node();

			void addChild(Node* child);
			void deleteAllSubNode();

			Type type() const;
			std::string metadataType() const;
			void setMetadataType(const std::string& value);
			const std::vector<Node*>& children() const;
			std::vector<Node*>& children();

			const Node* child(unsigned int i) const;
			Node* child(unsigned int i);

			bool hasChild() const;

			int line() const;
			int chr() const;
			void setPos(int line, int chr);

			void mark();
			bool isMarked() const;
	};


	class IntNode : public Node
	{
		protected:

			int _value;


		public:

			IntNode(Type type, int value);

			int value() const;
	};


	class FloatNode : public Node
	{
		protected:

			float _value;


		public:

			FloatNode(Type type, float value);

			float value() const;
	};


	class StrNode : public Node
	{
		protected:

			std::string _value;


		public:

			StrNode(Type type, const std::string& value);
			~StrNode();

			std::string value() const;
	};


	// User friendly functions to build the AST


	Node* makeProgram();

	Node* makeVoidType();
	Node* makeArrayType(Node* subType);
	Node* makeType(const char* type);
	Node* makeVar(const char* identifier);
	Node* makeArrayAccess(Node* arrayIdentifier, Node* indexIdentifier);
	Node* makeBlock();
	Node* makeDeclaration(Node* type, Node* identifierList);
	Node* makeIdentifierList();
	Node* makeDeclarationList();
	Node* makeInstructionList();
	Node* makeAffectation(Node* identifier, Node* expression);
	Node* makeAllocation(Node* type, Node* identifier, Node* expression);
	Node* makeReturn();
	Node* makeReturn(Node* expression);

	Node* makeFloatConst(float constValue);
	Node* makeIntConst(int constValue);
	Node* makeBoolConst(bool constValue);

	Node* makeConditionGroup();
	Node* makeIfCondition(Node* condExpression, Node* block);
	Node* makeElseCondition(Node* block);

	Node* makeWhileLoop(Node* condExpression, Node* block);

	Node* makeFunctionDecl(Node* type, Node* identifier, Node* paramList, Node* bloc);
	Node* makeFunctionDeclParamList();
	Node* makeFunctionDeclParam(Node* type, Node* identifier);

	Node* makeFreeCall(Node* expression);
	Node* makeFunctionCall(Node* identifier, Node* paramList);
	Node* makeFunctionCallParamList();

	Node* makeOpUnaSub(Node* expression);
	Node* makeOpUnaNot(Node* expression);

	Node* makeOpBinSub(Node* left, Node* right);
	Node* makeOpBinAdd(Node* left, Node* right);
	Node* makeOpBinMul(Node* left, Node* right);
	Node* makeOpBinDiv(Node* left, Node* right);
	Node* makeOpBinOr(Node* left, Node* right);
	Node* makeOpBinAnd(Node* left, Node* right);

	Node* makeOpBinEqual(Node* left, Node* right);
	Node* makeOpBinDiff(Node* left, Node* right);
	Node* makeOpBinInfeq(Node* left, Node* right);
	Node* makeOpBinInf(Node* left, Node* right);
	Node* makeOpBinSupeq(Node* left, Node* right);
	Node* makeOpBinSup(Node* left, Node* right);

	void deleteTree(Node* root);

	void makeDot(Node* astRoot, const std::string& filename, bool showTypes = false);

	extern Node* build(bool lexicalLog, bool syntacticLog);
}


#endif

