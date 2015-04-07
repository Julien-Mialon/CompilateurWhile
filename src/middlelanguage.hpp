#ifndef MIDDLELANGUAGE_HPP
#define MIDDLELANGUAGE_HPP

#include <cstring>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstdio>

#include "ast.hpp"

#include "functiontable.hpp"
#include "symboltable.hpp"

namespace MiddleLanguage
{

	class Expr
	{
		public:
			enum Type
			{
				CONST_INT,
				REGISTER,
				UNOP,
				BINOP,
				ARG
			};

		public:
			Expr(Type _type) : m_type(_type) {}

			virtual ~Expr() {}
			virtual std::string code() = 0;
			virtual std::string selection() = 0;
			virtual std::string mips() = 0;

			Type type() { return m_type; }

		protected:
			Type m_type;
	};

	class Instruction
	{
		public:
			enum Type
			{
				LABEL,
				REGISTRY_WRITE,
				JUMP,
				GOTO,
				NOP,
				CALL_REGISTRY_WRITE,
				CALL_FUNCTION,
				ARG
			};

		public:
			Instruction(Type _type) : m_type(_type) {}

			virtual ~Instruction() {}
			virtual std::string code() = 0;
			virtual std::string selection() = 0;
			virtual std::string mips() = 0;

			Type type() { return m_type; }

		protected:
			Type m_type;
	};

	class Frame
	{
		public:
			ast::Node * functionDecl;
			SymbolTable * symbols;
			int frameIndex;

			std::string entryLabel;
			std::string returnLabel;

			std::vector<int> argsRegistry;
			int returnRegistry;

			int stackSize;

			std::vector<Instruction*> instructions;

			Frame();
			~Frame();
			void add(Instruction* i);
			int nextRegister();

		protected:
			int regIndex;
	};

	class Program
	{
		public:
			std::vector<Frame*> frames;
			Frame * main;
	};

	class InstructionLabel : public Instruction
	{
		public:
			std::string labelName;

			InstructionLabel(const std::string & labelName) : Instruction(Instruction::LABEL), labelName(labelName) {}
			~InstructionLabel() {}

			std::string code() { return labelName + ": "; }
			std::string selection() { return labelName + ": "; }
			std::string mips() { return labelName + ": "; }
	};

	class InstructionRegistryWrite : public Instruction
	{
		public:
			int storeRegister;
			Expr * expression;

			InstructionRegistryWrite(int storeRegister, Expr * expression) : Instruction(Instruction::REGISTRY_WRITE), storeRegister(storeRegister), expression(expression) {}
			~InstructionRegistryWrite() { delete expression; }

			std::string code() {
				char buffer[64];
				sprintf(buffer, "%d", storeRegister);
				return "\t$" + std::string(buffer) + " := " + expression->code();
			}
			std::string selection();
			std::string mips();
	};

	class InstructionJump : public Instruction
	{
		public:
			std::string labelTrue;
			std::string labelFalse;
			Expr* expression;

			InstructionJump(const std::string & labelTrue, const std::string & labelFalse, Expr* expression) : Instruction(Instruction::JUMP), labelTrue(labelTrue), labelFalse(labelFalse), expression(expression) {}
			~InstructionJump() { delete expression; }

			std::string code() { return "\tjump (" + expression->code() + ") " + labelTrue + ", " + labelFalse; }
			std::string selection() { return "\tbgtz " + expression->code() + ", " + labelTrue + "\n\tj " + labelFalse; }
			std::string mips() { return "\tlw $t0, " + expression->mips() + "\n\tbgtz $t0, " + labelTrue + "\n\tj " + labelFalse; }
	};

	class InstructionGoto : public Instruction
	{
		public:
			std::string labelName;

			InstructionGoto(const std::string & labelName) : Instruction(Instruction::GOTO), labelName(labelName) {}
			~InstructionGoto() {}

			std::string code() { return "\tgoto " + labelName; }
			std::string selection() { return "\tj " + labelName; }
			std::string mips() { return "\tj " + labelName; }
	};

	class InstructionCallFunction : public Instruction
	{
		public:
			Frame* frame;
			std::vector<Expr*> parameters;

			InstructionCallFunction(Frame* frame) : Instruction(Instruction::CALL_FUNCTION), frame(frame) {}
			void addParameter(Expr* expr) { parameters.push_back(expr); }
			~InstructionCallFunction() {for(unsigned int i = 0 ; i < parameters.size() ; ++i) delete parameters[i]; }

			std::string code() {
				char buffer[64];
				sprintf(buffer, "%d", frame->frameIndex);
				std::string c = "call " + std::string(buffer) + " (";
				for(unsigned int i = 0 ; i < parameters.size() ; ++i)
				{
					c += parameters[i]->code() + ", ";
				}
				if(parameters.size() > 0)
					c = c.substr(0, c.length() - 2);
				c = "\t" + c;
				c += ")";
				return c;
			}

			std::string selectionOptimize() {
				std::string c = "";
				//jal frame label
				c += "\tjal " + frame->entryLabel;
				return c;
			}

			std::string selection() {
				//move all parameters
				std::string c = "";
				for(unsigned int i = 0 ; i < parameters.size() ; ++i)
				{
					char buffer[64];
					sprintf(buffer, "%d", i);
					c += "\tmove $a" + std::string(buffer) + ", " + parameters[i]->selection() + "\n";
				}
				//jal frame label
				c += "\tjal " + frame->entryLabel;
				return c;
			}

			std::string mips() {
				std::string c = "";
				//jal frame label
				char buffer[64];
				int size = (int)frame->argsRegistry.size();
				sprintf(buffer, "%d", (size - 4) * 4);
				//jal frame label

				c += "\t\tsub $sp, $sp, 8\n";
				c += "\t\tsw $fp, 0($sp)\n";
				c += "\t\tsw $ra, 4($sp)\n";
				c += "\t\tmove $fp, $sp\n";
				/*
				c += "\t\tsub $sp, $sp " + std::string(buffer) + "\n";
				for(unsigned int i = 0 ; i < frame->argsRegistry.size() ; ++i)
				{
					c += "\t\tsw "
				}
				*/
				sprintf(buffer, "%d", (frame->stackSize * 4));
				c += "\t\tsub $sp, $sp, " + std::string(buffer) + "\n";


				c += "\tjal " + frame->entryLabel + "\n";

				c += "\t\tmove $sp, $fp\n";
				c += "\t\tlw $fp, 0($sp)\n";
				c += "\t\tlw $ra, 4($sp)\n";
				c += "\t\tadd $sp, $sp, 8";
				return c;
			}
	};

	class InstructionArg : public Instruction
	{
		public:
			int storeRegister;
			Expr * expression;

			InstructionArg(int storeRegister, Expr * expression) : Instruction(Instruction::ARG), storeRegister(storeRegister), expression(expression) {}
			~InstructionArg() { delete expression; }

			std::string code() {
				char buffer[64];
				sprintf(buffer, "%d", storeRegister);
				return "\t$a" + std::string(buffer) + " := " + expression->code();
			}
			std::string selection();
			std::string mips();
	};

	class InstructionCallRegistryWrite : public Instruction
	{
		public:
			Frame* frame;
			std::vector<Expr*> parameters;
			int storeRegister;

			InstructionCallRegistryWrite(int storeRegister, Frame* frame) : Instruction(Instruction::CALL_REGISTRY_WRITE), frame(frame), storeRegister(storeRegister) {}
			void addParameter(Expr* expr) { parameters.push_back(expr); }
			~InstructionCallRegistryWrite() {for(unsigned int i = 0 ; i < parameters.size() ; ++i) delete parameters[i]; }

			std::string code() {
				char buffer[64], buffer2[64];
				sprintf(buffer, "%d", storeRegister);
				sprintf(buffer2, "%d", frame->frameIndex);
				std::string c = std::string("$") + buffer + " := call " + buffer2;
				c += " (";
				for(unsigned int i = 0 ; i < parameters.size() ; ++i)
				{
					c += parameters[i]->code() + ", ";
				}
				c = c.substr(0, c.length() - 2);
				c += ")";
				c = "\t" + c;
				return c;
			}
			std::string selectionOptimize() {
				//jal frame label
				char buffer2[64];
				std::string c = "";
				sprintf(buffer2, "%d", storeRegister);

				c += "\tjal " + frame->entryLabel + "\n";
				c += "\tmove $" + std::string(buffer2) + ", $v0";
				return c;
			}

			std::string selection() {
				//move all parameters
				std::string c = "";
				for(unsigned int i = 0 ; i < parameters.size() ; ++i)
				{
					char buffer[64];
					sprintf(buffer, "%d", i);
					c += "\tmove $a" + std::string(buffer) + ", " + parameters[i]->selection() + "\n";
				}
				//jal frame label

				char buffer2[64];
				sprintf(buffer2, "%d", storeRegister);

				c += "\tjal " + frame->entryLabel + "\n";
				c += "\tmove $" + std::string(buffer2) + ", $v0";
				return c;
			}
			std::string mips() {
				std::string c = "";
				//jal frame label
				char buffer[64];
				int size = (int)frame->argsRegistry.size();
				sprintf(buffer, "%d", (size - 4) * 4);
				//jal frame label

				c += "\t\tsub $sp, $sp, 8\n";
				c += "\t\tsw $fp, 0($sp)\n";
				c += "\t\tsw $ra, 4($sp)\n";
				c += "\t\tmove $fp, $sp\n";
				/*
				c += "\t\tsub $sp, $sp " + std::string(buffer) + "\n";
				for(unsigned int i = 0 ; i < frame->argsRegistry.size() ; ++i)
				{
					c += "\t\tsw "
				}
				*/
				sprintf(buffer, "%d", (frame->stackSize * 4));
				c += "\t\tsub $sp, $sp, " + std::string(buffer) + "\n";


				c += "\tjal " + frame->entryLabel + "\n";

				c += "\t\tmove $sp, $fp\n";
				c += "\t\tlw $fp, 0($sp)\n";
				c += "\t\tlw $ra, 4($sp)\n";
				c += "\t\tadd $sp, $sp, 8\n";

				char buffer2[64];
				sprintf(buffer2, "%d", storeRegister * 4);
				c += "\tsw $v0, " + std::string(buffer2) + "($sp)";
				return c;
			}
	};

	class InstructionNop : public Instruction
	{
		public:
			InstructionNop() : Instruction(Instruction::NOP) {}
			~InstructionNop() {}

			std::string code() { return "\tnop"; }
			std::string selection() { return "\tnop"; }
			std::string mips() { return "\tnop"; }
	};

	class ExprConstInt : public Expr
	{
		public:
			int number;

			ExprConstInt(int number) : Expr(Expr::CONST_INT), number(number) {}
			~ExprConstInt() {}
			std::string code() {
				char buffer[64];
				sprintf(buffer, "%d", number);
				return std::string(buffer);
			}
			std::string selection() {
				char buffer[64];
				sprintf(buffer, "%d", number);
				return std::string(buffer);
			}
			std::string mips() {
				char buffer[64];
				sprintf(buffer, "%d", number);
				return std::string(buffer);
			}
	};

	class ExprRegister : public Expr
	{
		public:
			int reg;

			ExprRegister(int reg) : Expr(Expr::REGISTER), reg(reg) {}
			~ExprRegister() {}
			std::string code() {
				char buffer[64];
				sprintf(buffer, "$%d", reg);
				return std::string(buffer);
			}
			std::string selection() {
				char buffer[64];
				sprintf(buffer, "$%d", reg);
				return std::string(buffer);
			}
			std::string mips() {
				char buffer[64];
				sprintf(buffer, "%d($sp)", reg * 4);
				return std::string(buffer);
			}
	};

	class ExprArg : public Expr
	{
		public:
			int reg;

			ExprArg(int reg) : Expr(Expr::ARG), reg(reg) {}
			~ExprArg() {}
			std::string code() {
				char buffer[64];
				sprintf(buffer, "$a%d", reg);
				return std::string(buffer);
			}
			std::string selection() {
				char buffer[64];
				sprintf(buffer, "$a%d", reg);
				return std::string(buffer);
			}
	};

	class ExprUnop : public Expr
	{
		public:
			Expr* expression;
			std::string op;

			ExprUnop(const std::string & op, Expr* expression) : Expr(Expr::UNOP), expression(expression), op(op) {}
			~ExprUnop() { delete expression; }
			std::string code() {
				return op + " " + expression->code();
			}
			std::string selection() {
				throw std::runtime_error("ExprUnop::selection");
			}
			std::string mips() {
				throw std::runtime_error("ExprUnop::selection");
			}
	};

	class ExprBinop : public Expr
	{
		public:
			Expr* expression1;
			Expr* expression2;
			std::string op;

			ExprBinop(const std::string & op, Expr* expression1, Expr* expression2) : Expr(Expr::BINOP), expression1(expression1), expression2(expression2), op(op) {}
			~ExprBinop() { delete expression1; delete expression2; }
			std::string code() {
				return expression1->code() + " " + op + " " + expression2->code();
			}
			std::string selection() {
				throw std::runtime_error("ExprBinop::selection");
			}
			std::string mips() {
				throw std::runtime_error("ExprBinop::selection");
			}
	};


	class IL
	{
		public:
			IL(ast::Node* program);

			Program* process();

			void writeToFile(const char * filename);
			void writeToFileSelection(const char * filename);

			void writeOptimizedSelection(const char * filename);
			void writeToFileMips(const char * filename);

			void optimize();

			void selectInstruction();

			void optimizeSelect();


			std::string newLabel();

		protected:
			int countParameters(ast::Node * functionDecl);
			std::string functionName(ast::Node * functionDecl);
			std::vector<std::string> extractParameters(ast::Node * functionDecl);

			void parseInstructions(ast::Node * instructions, Frame * f);
			void parseDeclarations(ast::Node * declarations, Frame * f);
			std::string strValue(ast::Node* node);
			int intValue(ast::Node* node);
			Expr * parseExpression(ast::Node * expr, Frame * f);

			void optimizeFrame(Frame * frame);
			void removeDeadCode(Frame * frame);
			void simplifyLabel(Frame * frame);
			void checkGotoUtility(Frame * frame);

			bool isUselessCode(Frame * frame, Instruction *instr, std::vector<Instruction*>::iterator & start, std::vector<Instruction*>::iterator & end);

			void selectInstruction(Frame * frame);
			std::pair<Expr*, std::vector<Instruction*> > compileExpression(Expr * e, Frame *f);

			void optimizeSelect(Frame * f);
			void checkRegisterUseInInstruction(std::map<int, std::pair<int, int> > & useMap, Expr * e);
			void developFunctionCall(Frame * f);

			void renumRegister(Frame * f);
			void renumExpr(Expr * e, int & regIndex, std::map<int, int> & regAssoc);

		protected:
			ast::Node* m_program;

			Program * m_ilProgram;

			int m_labelIndex;

			std::map<std::string, Frame*> m_functionsMap;
	};
}

#endif // MIDDLELANGUAGE_HPP
