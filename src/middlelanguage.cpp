#include "middlelanguage.hpp"
#include "symboltable.hpp"

#include <iostream>
#include <algorithm>

using namespace ast;


#define ENTRY_POINT "main"
#define EXIT_POINT "exit"

#define IF_TYPE(var, cmp) if(var->type() == cmp)
#define TO_PTYPE(newvar, var, type) type * newvar = ((type*)var)

#define PROCESS_BINOP(var, op, frame) new ExprBinop(op, parseExpression(var->child(0), frame), parseExpression(var->child(1), frame))
#define PROCESS_UNOP(var, op, frame) new ExprUnop(op, parseExpression(var->child(0), frame));


#define ADD_TO_MAPLIST(maplist, key, value) maplist[key].push_back(value);
#define INC_MAP(map, key) if(map.count(key) <= 0) { map[key] = 0; } map[key]++

#define REMOVE_UNTIL_LABEL(current, instructions) \
	std::vector<Instruction*>::iterator start, end; \
	for(start = instructions.begin() ; start != instructions.end() && *start != current ; ++start); \
	if(start != instructions.end()) { \
		for(end = start ; end != instructions.end() && (*end)->type() != Instruction::LABEL ; end++); \
		instructions.erase(start, end); \
	}

#define INC_STATUSMAP(map, key) if(map.count(key) > 0) { map[key].first++; }

#define ADD_VECTOR(container, elements) for(unsigned int a = 0 ; a < elements.size() ; ++a) { container.push_back(elements[a]); }

#define INC_REG(index, map, reg) \
		if(map.count(reg) > 0) { \
			reg = map[reg]; \
		} else { \
			map[reg] = index; \
			reg = index; \
			index++; \
		}

namespace MiddleLanguage
{
	/** FRAME **/
	Frame::Frame() : entryLabel(""), returnLabel(""), returnRegistry(-1), stackSize(0)
	{
		regIndex = 0;
	}

	Frame::~Frame()
	{
		for(unsigned int i = 0 ; i < instructions.size() ; ++i) delete instructions[i];
	}

	void Frame::add(Instruction* i)
	{
		instructions.push_back(i);
	}

	int Frame::nextRegister()
	{
		return ++regIndex;
	}

	std::string InstructionRegistryWrite::selection()
	{
		char buffer[64];
		sprintf(buffer, "%d", storeRegister);
		if(expression->type() == Expr::CONST_INT)
		{
			return "\tli $" + std::string(buffer) + ", " + expression->selection();
		}
		else if(expression->type() == Expr::REGISTER)
		{
			return "\tmove $" + std::string(buffer) + ", " + expression->selection();
		}
		else if(expression->type() == Expr::UNOP)
		{
			ExprUnop * unop = (ExprUnop*)expression;
			if(unop->op == "-")
			{
				return "\tneg $" + std::string(buffer) + ", " + unop->expression->selection();
			}
			else if(unop->op == "!")
			{
				return "\tnot $" + std::string(buffer) + ", " +unop->expression->selection();
			}
		}
		else if(expression->type() == Expr::BINOP)
		{
			ExprBinop * binop = (ExprBinop*)expression;
			if(binop->op == "+")
			{
				return "\tadd $" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
			}
			else if(binop->op == "-")
			{
				return "\tsub $" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
			}
			else if(binop->op == "*")
			{
				return "\tmul $" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
			}
			else if(binop->op == "/")
			{
				return "\tdiv $" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
			}
			else if(binop->op == "&&")
			{
				return "\tand $" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
			}
			else if(binop->op == "||")
			{
				return "\tor $" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
			}
			else if(binop->op == "=")
			{
				return "\tseq $" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
			}
			else if(binop->op == "<>")
			{
				return "\tsne $" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
			}
			else if(binop->op == "<")
			{
				return "\tslt $" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
			}
			else if(binop->op == ">")
			{
				return "\tsgt $" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
			}
			else if(binop->op == "<=")
			{
				return "\tsle $" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
			}
			else if(binop->op == ">=")
			{
				return "\tsge $" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
			}
		}
		throw std::runtime_error("Fatal Error InstructionRegistryWrite::selection");
	}

	std::string InstructionRegistryWrite::mips()
	{
		char buffer[64];
		sprintf(buffer, "%d", storeRegister * 4);
		if(expression->type() == Expr::CONST_INT)
		{
			return "\tli $t0, " + expression->mips() + "\n\tsw $t0, " + std::string(buffer) + "($sp)";
			//return "\tli $" + std::string(buffer) + ", " + expression->selection();
		}
		else if(expression->type() == Expr::REGISTER)
		{
			// move r1, r2
			return "\tlw $t0, " + expression->mips() + "\n\tsw $t0, " + std::string(buffer) + "($sp)";
			//return "\tmove $" + std::string(buffer) + ", " + expression->selection();
		}
		else if(expression->type() == Expr::UNOP)
		{
			ExprUnop * unop = (ExprUnop*)expression;
			if(unop->op == "-")
			{
				return "\tlw $t0, " + unop->expression->mips() + "\n\tneg $t1, $t0\n\tsw $t1, " + std::string(buffer) + "($sp)";
				//return "\tneg $" + std::string(buffer) + ", " + unop->expression->mips();
			}
			else if(unop->op == "!")
			{
				return "\tlw $t0, " + unop->expression->mips() + "\n\tnot $t1, $t0\n\tsw $t1, " + std::string(buffer);
				//return "\tnot $" + std::string(buffer) + ", " +unop->expression->mips();
			}
		}
		else if(expression->type() == Expr::BINOP)
		{
			ExprBinop * binop = (ExprBinop*)expression;
			IF_TYPE(binop->expression2, Expr::CONST_INT)
			{
				if(binop->op == "+")
				{
					return "\tlw $t0, " + binop->expression1->mips() + "\n\tli $t1," + binop->expression2->mips() + "\n\tadd $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
					//return "\tadd $" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
				}
				else if(binop->op == "-")
				{
					return "\tlw $t0, " + binop->expression1->mips() + "\n\tli $t1," + binop->expression2->mips() + "\n\tsub $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
					//return "\tsub $" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
				}
				else if(binop->op == "*")
				{
					return "\tlw $t0, " + binop->expression1->mips() + "\n\tli $t1," + binop->expression2->mips() + "\n\tmul $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
					//return "\tmul $" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
				}
				else if(binop->op == "/")
				{
					return "\tlw $t0, " + binop->expression1->mips() + "\n\tli $t1," + binop->expression2->mips() + "\n\tdiv $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
					//return "\tdiv $" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
				}
				else if(binop->op == "&&")
				{
					return "\tlw $t0, " + binop->expression1->mips() + "\n\tli $t1," + binop->expression2->mips() + "\n\tand $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
					//return "\tand $" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
				}
				else if(binop->op == "||")
				{
					return "\tlw $t0, " + binop->expression1->mips() + "\n\tli $t1," + binop->expression2->mips() + "\n\tor $t2, $t0, $t1\n\tsw $2t, " + std::string(buffer) + "($sp)";
					//return "\tor $" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
				}
				else if(binop->op == "=")
				{
					return "\tlw $t0, " + binop->expression1->mips() + "\n\tli $t1," + binop->expression2->mips() + "\n\tseq $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
					//return "\tseq $" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
				}
				else if(binop->op == "<>")
				{
					return "\tlw $t0, " + binop->expression1->mips() + "\n\tli $t1," + binop->expression2->mips() + "\n\tsne $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
					//return "\tsne $" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
				}
				else if(binop->op == "<")
				{
					return "\tlw $t0, " + binop->expression1->mips() + "\n\tli $t1," + binop->expression2->mips() + "\n\tslt $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
					//return "\tslt $" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
				}
				else if(binop->op == ">")
				{
					return "\tlw $t0, " + binop->expression1->mips() + "\n\tli $t1," + binop->expression2->mips() + "\n\tsgt $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
					//return "\tsgt $" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
				}
				else if(binop->op == "<=")
				{
					return "\tlw $t0, " + binop->expression1->mips() + "\n\tli $t1," + binop->expression2->mips() + "\n\tsle $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
					//return "\tsle $" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
				}
				else if(binop->op == ">=")
				{
					return "\tlw $t0, " + binop->expression1->mips() + "\n\tli $t1," + binop->expression2->mips() + "\n\tsge $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
					//return "\tsge $" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
				}
			}
			else
			{
				if(binop->op == "+")
				{
					return "\tlw $t0, " + binop->expression1->mips() + "\n\tlw $t1, " + binop->expression2->mips() + "\n\tadd $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
					//return "\tadd $" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
				}
				else if(binop->op == "-")
				{
					return "\tlw $t0, " + binop->expression1->mips() + "\n\tlw $t1, " + binop->expression2->mips() + "\n\tsub $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
					//return "\tsub $" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
				}
				else if(binop->op == "*")
				{
					return "\tlw $t0, " + binop->expression1->mips() + "\n\tlw $t1, " + binop->expression2->mips() + "\n\tmul $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
					//return "\tmul $" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
				}
				else if(binop->op == "/")
				{
					return "\tlw $t0, " + binop->expression1->mips() + "\n\tlw $t1, " + binop->expression2->mips() + "\n\tdiv $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
					//return "\tdiv $" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
				}
				else if(binop->op == "&&")
				{
					return "\tlw $t0, " + binop->expression1->mips() + "\n\tlw $t1, " + binop->expression2->mips() + "\n\tand $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
					//return "\tand $" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
				}
				else if(binop->op == "||")
				{
					return "\tlw $t0, " + binop->expression1->mips() + "\n\tlw $t1, " + binop->expression2->mips() + "\n\tor $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
					//return "\tor $" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
				}
				else if(binop->op == "=")
				{
					return "\tlw $t0, " + binop->expression1->mips() + "\n\tlw $t1, " + binop->expression2->mips() + "\n\tseq $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
					//return "\tseq $" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
				}
				else if(binop->op == "<>")
				{
					return "\tlw $t0, " + binop->expression1->mips() + "\n\tlw $t1, " + binop->expression2->mips() + "\n\tsne $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
					//return "\tsne $" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
				}
				else if(binop->op == "<")
				{
					return "\tlw $t0, " + binop->expression1->mips() + "\n\tlw $t1, " + binop->expression2->mips() + "\n\tslt $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
					//return "\tslt $" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
				}
				else if(binop->op == ">")
				{
					return "\tlw $t0, " + binop->expression1->mips() + "\n\tlw $t1, " + binop->expression2->mips() + "\n\tsgt $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
					//return "\tsgt $" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
				}
				else if(binop->op == "<=")
				{
					return "\tlw $t0, " + binop->expression1->mips() + "\n\tlw $t1, " + binop->expression2->mips() + "\n\tsle $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
					//return "\tsle $" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
				}
				else if(binop->op == ">=")
				{
					return "\tlw $t0, " + binop->expression1->mips() + "\n\tlw $t1, " + binop->expression2->mips() + "\n\tsge $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
					//return "\tsge $" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
				}
			}
		}
		throw std::runtime_error("Fatal Error InstructionRegistryWrite::selection");
	}

	std::string InstructionArg::selection()
	{
		char buffer[64];
		sprintf(buffer, "%d", storeRegister);
		if(expression->type() == Expr::CONST_INT)
		{
			return "\tli $a" + std::string(buffer) + ", " + expression->selection();
		}
		else if(expression->type() == Expr::REGISTER)
		{
			return "\tmove $a" + std::string(buffer) + ", " + expression->selection();
		}
		else if(expression->type() == Expr::UNOP)
		{
			ExprUnop * unop = (ExprUnop*)expression;
			if(unop->op == "-")
			{
				return "\tneg $a" + std::string(buffer) + ", " + unop->expression->selection();
			}
			else if(unop->op == "!")
			{
				return "\tnot $a" + std::string(buffer) + ", " +unop->expression->selection();
			}
		}
		else if(expression->type() == Expr::BINOP)
		{
			ExprBinop * binop = (ExprBinop*)expression;
			if(binop->op == "+")
			{
				return "\tadd $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
			}
			else if(binop->op == "-")
			{
				return "\tsub $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
			}
			else if(binop->op == "*")
			{
				return "\tmul $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
			}
			else if(binop->op == "/")
			{
				return "\tdiv $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
			}
			else if(binop->op == "&&")
			{
				return "\tand $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
			}
			else if(binop->op == "||")
			{
				return "\tor $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
			}
			else if(binop->op == "=")
			{
				return "\tseq $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
			}
			else if(binop->op == "<>")
			{
				return "\tsne $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
			}
			else if(binop->op == "<")
			{
				return "\tslt $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
			}
			else if(binop->op == ">")
			{
				return "\tsgt $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
			}
			else if(binop->op == "<=")
			{
				return "\tsle $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
			}
			else if(binop->op == ">=")
			{
				return "\tsge $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
			}
		}
		throw std::runtime_error("Fatal Error InstructionRegistryWrite::selection");
	}

	std::string InstructionArg::mips()
	{
		char buffer[64];
		sprintf(buffer, "%d", storeRegister);
		if(storeRegister < 4)
		{
			if(expression->type() == Expr::CONST_INT)
			{
				return "\tli $a" + std::string(buffer) + ", " + expression->mips();
			}
			else if(expression->type() == Expr::REGISTER)
			{
				return "\tlw $t0, " + expression->mips() + "\n\tmove $a" + std::string(buffer) + ", $t0";
				//return "\tmove $a" + std::string(buffer) + ", " + expression->mips();
			}
			else if(expression->type() == Expr::UNOP)
			{
				ExprUnop * unop = (ExprUnop*)expression;
				if(unop->op == "-")
				{
					return "\tneg $a" + std::string(buffer) + ", " + unop->expression->mips();
				}
				else if(unop->op == "!")
				{
					return "\tnot $a" + std::string(buffer) + ", " +unop->expression->mips();
				}
			}
			else if(expression->type() == Expr::BINOP)
			{
				ExprBinop * binop = (ExprBinop*)expression;
				IF_TYPE(binop->expression2, Expr::CONST_INT)
				{
					if(binop->op == "+")
					{
						return "\tlw $t0, " + binop->expression1->mips() + "\n\tadd $a" + std::string(buffer) + ", $t0, " + binop->expression2->mips();
						//return "\tadd $a" + std::string(buffer) + ", " + binop->expression1->mips() + ", " + binop->expression2->mips();
					}
					else if(binop->op == "-")
					{
						return "\tlw $t0, " + binop->expression1->mips() + "\n\tsub $a" + std::string(buffer) + ", $t0, " + binop->expression2->mips();
						//return "\tsub $a" + std::string(buffer) + ", " + binop->expression1->mips() + ", " + binop->expression2->mips();
					}
					else if(binop->op == "*")
					{
						return "\tlw $t0, " + binop->expression1->mips() + "\n\tmul $a" + std::string(buffer) + ", $t0, " + binop->expression2->mips();
						//return "\tmul $a" + std::string(buffer) + ", " + binop->expression1->mips() + ", " + binop->expression2->mips();
					}
					else if(binop->op == "/")
					{
						return "\tlw $t0, " + binop->expression1->mips() + "\n\tdiv $a" + std::string(buffer) + ", $t0, " + binop->expression2->mips();
						//return "\tdiv $a" + std::string(buffer) + ", " + binop->expression1->mips() + ", " + binop->expression2->mips();
					}
					else if(binop->op == "&&")
					{
						return "\tlw $t0, " + binop->expression1->mips() + "\n\tand $a" + std::string(buffer) + ", $t0, " + binop->expression2->mips();
						//return "\tand $a" + std::string(buffer) + ", " + binop->expression1->mips() + ", " + binop->expression2->mips();
					}
					else if(binop->op == "||")
					{
						return "\tlw $t0, " + binop->expression1->mips() + "\n\tor $a" + std::string(buffer) + ", $t0, " + binop->expression2->mips();
						//return "\tor $a" + std::string(buffer) + ", " + binop->expression1->mips() + ", " + binop->expression2->mips();
					}
					else if(binop->op == "=")
					{
						return "\tlw $t0, " + binop->expression1->mips() + "\n\tseq $a" + std::string(buffer) + ", $t0, " + binop->expression2->mips();
						//return "\tseq $a" + std::string(buffer) + ", " + binop->expression1->mips() + ", " + binop->expression2->mips();
					}
					else if(binop->op == "<>")
					{
						return "\tlw $t0, " + binop->expression1->mips() + "\n\tsne $a" + std::string(buffer) + ", $t0, " + binop->expression2->mips();
						//return "\tsne $a" + std::string(buffer) + ", " + binop->expression1->mips() + ", " + binop->expression2->mips();
					}
					else if(binop->op == "<")
					{
						return "\tlw $t0, " + binop->expression1->mips() + "\n\tslt $a" + std::string(buffer) + ", $t0, " + binop->expression2->mips();
						//return "\tslt $a" + std::string(buffer) + ", " + binop->expression1->mips() + ", " + binop->expression2->mips();
					}
					else if(binop->op == ">")
					{
						return "\tlw $t0, " + binop->expression1->mips() + "\n\tsgt $a" + std::string(buffer) + ", $t0, " + binop->expression2->mips();
						//return "\tsgt $a" + std::string(buffer) + ", " + binop->expression1->mips() + ", " + binop->expression2->mips();
					}
					else if(binop->op == "<=")
					{
						return "\tlw $t0, " + binop->expression1->mips() + "\n\tsle $a" + std::string(buffer) + ", $t0, " + binop->expression2->mips();
						//return "\tsle $a" + std::string(buffer) + ", " + binop->expression1->mips() + ", " + binop->expression2->mips();
					}
					else if(binop->op == ">=")
					{
						return "\tlw $t0, " + binop->expression1->mips() + "\n\tsge $a" + std::string(buffer) + ", $t0, " + binop->expression2->mips();
						//return "\tsge $a" + std::string(buffer) + ", " + binop->expression1->mips() + ", " + binop->expression2->mips();
					}
				}
				else
				{
					if(binop->op == "+")
					{
						return "\tlw $t0, " + binop->expression2->mips() + "\n\tlw $t1, " + binop->expression1->mips() + "\n\tadd $a" + std::string(buffer) + ", $t1, $t0";
						//return "\tadd $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
					}
					else if(binop->op == "-")
					{
						return "\tlw $t0, " + binop->expression2->mips() + "\n\tlw $t1, " + binop->expression1->mips() + "\n\tsub $a" + std::string(buffer) + ", " + binop->expression1->mips() + ", $t0";
						//return "\tsub $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
					}
					else if(binop->op == "*")
					{
						return "\tlw $t0, " + binop->expression2->mips() + "\n\tlw $t1, " + binop->expression1->mips() + "\n\tmul $a" + std::string(buffer) + ", " + binop->expression1->mips() + ", $t0";
						//return "\tmul $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
					}
					else if(binop->op == "/")
					{
						return "\tlw $t0, " + binop->expression2->mips() + "\n\tlw $t1, " + binop->expression1->mips() + "\n\tdiv $a" + std::string(buffer) + ", " + binop->expression1->mips() + ", $t0";
						//return "\tdiv $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
					}
					else if(binop->op == "&&")
					{
						return "\tlw $t0, " + binop->expression2->mips() + "\n\tlw $t1, " + binop->expression1->mips() + "\n\tand $a" + std::string(buffer) + ", " + binop->expression1->mips() + ", $t0";
						//return "\tand $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
					}
					else if(binop->op == "||")
					{
						return "\tlw $t0, " + binop->expression2->mips() + "\n\tlw $t1, " + binop->expression1->mips() + "\n\tor $a" + std::string(buffer) + ", " + binop->expression1->mips() + ", $t0";
						//return "\tor $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
					}
					else if(binop->op == "=")
					{
						return "\tlw $t0, " + binop->expression2->mips() + "\n\tlw $t1, " + binop->expression1->mips() + "\n\tseq $a" + std::string(buffer) + ", " + binop->expression1->mips() + ", $t0";
						//return "\tseq $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
					}
					else if(binop->op == "<>")
					{
						return "\tlw $t0, " + binop->expression2->mips() + "\n\tlw $t1, " + binop->expression1->mips() + "\n\tsne $a" + std::string(buffer) + ", " + binop->expression1->mips() + ", $t0";
						//return "\tsne $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
					}
					else if(binop->op == "<")
					{
						return "\tlw $t0, " + binop->expression2->mips() + "\n\tlw $t1, " + binop->expression1->mips() + "\n\tslt $a" + std::string(buffer) + ", " + binop->expression1->mips() + ", $t0";
						//return "\tslt $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
					}
					else if(binop->op == ">")
					{
						return "\tlw $t0, " + binop->expression2->mips() + "\n\tlw $t1, " + binop->expression1->mips() + "\n\tsgt $a" + std::string(buffer) + ", " + binop->expression1->mips() + ", $t0";
						//return "\tsgt $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
					}
					else if(binop->op == "<=")
					{
						return "\tlw $t0, " + binop->expression2->mips() + "\n\tlw $t1, " + binop->expression1->mips() + "\n\tsle $a" + std::string(buffer) + ", " + binop->expression1->mips() + ", $t0";
						//return "\tsle $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
					}
					else if(binop->op == ">=")
					{
						return "\tlw $t0, " + binop->expression2->mips() + "\n\tlw $t1, " + binop->expression1->mips() + "\n\tsge $a" + std::string(buffer) + ", " + binop->expression1->mips() + ", $t0";
						//return "\tsge $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
					}
				}
			}
			throw std::runtime_error("Fatal Error InstructionRegistryWrite::selection");
		}
		else
		{
			if(expression->type() == Expr::CONST_INT)
			{
				return "\tli $t0, " + expression->mips() + "\n\tsw $t0, " + std::string(buffer) + "($sp)";
				//return "\tli $a" + std::string(buffer) + ", " + expression->selection();
			}
			else if(expression->type() == Expr::REGISTER)
			{
				return "\tlw $t0, " + expression->mips() + "\n\tmove $t1, $t0\n\tsw $t1, " + std::string(buffer) + "($sp)";
				//return "\tmove $a" + std::string(buffer) + ", " + expression->selection();
			}
			else if(expression->type() == Expr::UNOP)
			{
				ExprUnop * unop = (ExprUnop*)expression;
				if(unop->op == "-")
				{
					return "\tlw $t0, " + unop->expression->mips() + "\n\tneg $t1, $t0\n\tsw $t1, " + std::string(buffer) + "($sp)";
					//return "\tneg $a" + std::string(buffer) + ", " + unop->expression->selection();
				}
				else if(unop->op == "!")
				{
					return "\tlw $t0, " + unop->expression->mips() + "\n\tnot $t1, $t0\n\tsw $t1, " + std::string(buffer) + "($sp)";
					//return "\tnot $a" + std::string(buffer) + ", " +unop->expression->selection();
				}
			}
			else if(expression->type() == Expr::BINOP)
			{
				ExprBinop * binop = (ExprBinop*)expression;
				IF_TYPE(binop->expression2, Expr::CONST_INT)
				{
					if(binop->op == "+")
					{
						return "\tlw $t0, " + binop->expression1->mips() + "\n\tli $t1," + binop->expression2->mips() + "\n\tadd $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
						//return "\tadd $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
					}
					else if(binop->op == "-")
					{
						return "\tlw $t0, " + binop->expression1->mips() + "\n\tli $t1," + binop->expression2->mips() + "\n\tsub $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
						//return "\tsub $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
					}
					else if(binop->op == "*")
					{
						return "\tlw $t0, " + binop->expression1->mips() + "\n\tli $t1," + binop->expression2->mips() + "\n\tmul $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
						//return "\tmul $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
					}
					else if(binop->op == "/")
					{
						return "\tlw $t0, " + binop->expression1->mips() + "\n\tli $t1," + binop->expression2->mips() + "\n\tdiv $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
						//return "\tdiv $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
					}
					else if(binop->op == "&&")
					{
						return "\tlw $t0, " + binop->expression1->mips() + "\n\tli $t1," + binop->expression2->mips() + "\n\tand $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
						//return "\tand $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
					}
					else if(binop->op == "||")
					{
						return "\tlw $t0, " + binop->expression1->mips() + "\n\tli $t1," + binop->expression2->mips() + "\n\tor $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
						//return "\tor $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
					}
					else if(binop->op == "=")
					{
						return "\tlw $t0, " + binop->expression1->mips() + "\n\tli $t1," + binop->expression2->mips() + "\n\tseq $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
						//return "\tseq $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
					}
					else if(binop->op == "<>")
					{
						return "\tlw $t0, " + binop->expression1->mips() + "\n\tli $t1," + binop->expression2->mips() + "\n\tsne $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
						//return "\tsne $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
					}
					else if(binop->op == "<")
					{
						return "\tlw $t0, " + binop->expression1->mips() + "\n\tli $t1," + binop->expression2->mips() + "\n\tslt $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
						//return "\tslt $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
					}
					else if(binop->op == ">")
					{
						return "\tlw $t0, " + binop->expression1->mips() + "\n\tli $t1," + binop->expression2->mips() + "\n\tsgt $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
						//return "\tsgt $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
					}
					else if(binop->op == "<=")
					{
						return "\tlw $t0, " + binop->expression1->mips() + "\n\tli $t1," + binop->expression2->mips() + "\n\tsle $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
						//return "\tsle $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
					}
					else if(binop->op == ">=")
					{
						return "\tlw $t0, " + binop->expression1->mips() + "\n\tli $t1," + binop->expression2->mips() + "\n\tsge $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
						//return "\tsge $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
					}
				}
				else
				{
					if(binop->op == "+")
					{
						return "\tlw $t0, " + binop->expression1->mips() + "\n\tlw $t1, " + binop->expression2->mips() + "\n\tadd $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
						//return "\tadd $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
					}
					else if(binop->op == "-")
					{
						return "\tlw $t0, " + binop->expression1->mips() + "\n\tlw $t1, " + binop->expression2->mips() + "\n\tsub $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
						//return "\tsub $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
					}
					else if(binop->op == "*")
					{
						return "\tlw $t0, " + binop->expression1->mips() + "\n\tlw $t1, " + binop->expression2->mips() + "\n\tmul $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
						//return "\tmul $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
					}
					else if(binop->op == "/")
					{
						return "\tlw $t0, " + binop->expression1->mips() + "\n\tlw $t1, " + binop->expression2->mips() + "\n\tdiv $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
						//return "\tdiv $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
					}
					else if(binop->op == "&&")
					{
						return "\tlw $t0, " + binop->expression1->mips() + "\n\tlw $t1, " + binop->expression2->mips() + "\n\tand $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
						//return "\tand $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
					}
					else if(binop->op == "||")
					{
						return "\tlw $t0, " + binop->expression1->mips() + "\n\tlw $t1, " + binop->expression2->mips() + "\n\tor $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
						//return "\tor $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
					}
					else if(binop->op == "=")
					{
						return "\tlw $t0, " + binop->expression1->mips() + "\n\tlw $t1, " + binop->expression2->mips() + "\n\tseq $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
						//return "\tseq $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
					}
					else if(binop->op == "<>")
					{
						return "\tlw $t0, " + binop->expression1->mips() + "\n\tlw $t1, " + binop->expression2->mips() + "\n\tsne $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
						//return "\tsne $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
					}
					else if(binop->op == "<")
					{
						return "\tlw $t0, " + binop->expression1->mips() + "\n\tlw $t1, " + binop->expression2->mips() + "\n\tslt $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
						//return "\tslt $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
					}
					else if(binop->op == ">")
					{
						return "\tlw $t0, " + binop->expression1->mips() + "\n\tlw $t1, " + binop->expression2->mips() + "\n\tsgt $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
						//return "\tsgt $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
					}
					else if(binop->op == "<=")
					{
						return "\tlw $t0, " + binop->expression1->mips() + "\n\tlw $t1, " + binop->expression2->mips() + "\n\tsle $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
						//return "\tsle $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
					}
					else if(binop->op == ">=")
					{
						return "\tlw $t0, " + binop->expression1->mips() + "\n\tlw $t1, " + binop->expression2->mips() + "\n\tsge $t2, $t0, $t1\n\tsw $t2, " + std::string(buffer) + "($sp)";
						//return "\tsge $a" + std::string(buffer) + ", " + binop->expression1->selection() + ", " + binop->expression2->selection();
					}
				}
			}
			throw std::runtime_error("Fatal Error InstructionRegistryWrite::selection");
		}
	}

	/** IL **/
	IL::IL(ast::Node* program)
	{
		m_labelIndex = 0;
		m_program = program;
	}

	std::string IL::newLabel()
	{
		m_labelIndex++;
		char buffer[64];
		sprintf(buffer, "%d", m_labelIndex);
		std::string res = "label_";
		res += buffer;
		return res;
	}

	void IL::writeToFile(const char * filename)
	{
		FILE * out = fopen(filename, "w+");
		if(out != NULL)
		{
			//ecriture des instructions du programme
			InstructionGoto g(ENTRY_POINT);
			fprintf(out, "%s\n", g.code().c_str());

			//ecriture des frames
			for(unsigned int i = 0 ; i < m_ilProgram->frames.size() ; ++i)
			{
				Frame * f = m_ilProgram->frames[i];

				fprintf(out, "frame(%d) : { entry : %s; return : %s; args :", f->frameIndex, f->entryLabel.c_str(), f->returnLabel.c_str());
				if(f->argsRegistry.size() > 0)
				{
					for(unsigned int j = 0 ; j < f->argsRegistry.size() -1 ; ++j)
					{
						fprintf(out, " $%d,", f->argsRegistry[j]);
					}
					fprintf(out, " $%d", f->argsRegistry[f->argsRegistry.size()-1]);
				}
				else
				{
					fprintf(out, " no");
				}
				fprintf(out, "; ");
				if(f->returnRegistry > 0) // not a procedure
				{
					fprintf(out, "return : $%d; ", f->returnRegistry);
				}
				fprintf(out, "stack : %d}\n\n", f->stackSize);

				// ecriture instructions de la frame
				for(unsigned int i = 0 ; i < f->instructions.size() ; ++i)
				{
					fprintf(out, "%s\n", f->instructions[i]->code().c_str());
				}

				fprintf(out, "\n\n");
			}

			for(unsigned int i = 0 ; i < m_ilProgram->main->instructions.size() ; ++i)
			{
				fprintf(out, "%s\n", m_ilProgram->main->instructions[i]->code().c_str());
			}

			fclose(out);
		}
	}

	void IL::writeOptimizedSelection(const char * filename)
	{
		FILE * out = fopen(filename, "w+");
		if(out != NULL)
		{
			//ecriture des instructions du programme
			InstructionGoto g(ENTRY_POINT);
			fprintf(out, "%s\n", g.selection().c_str());

			//ecriture des frames
			for(unsigned int i = 0 ; i < m_ilProgram->frames.size() ; ++i)
			{
				Frame * f = m_ilProgram->frames[i];

				fprintf(out, "frame(%d) : { entry : %s; return : %s; args :", f->frameIndex, f->entryLabel.c_str(), f->returnLabel.c_str());
				if(f->argsRegistry.size() > 0)
				{
					for(unsigned int j = 0 ; j < f->argsRegistry.size() -1 ; ++j)
					{
						fprintf(out, " $%d,", f->argsRegistry[j]);
					}
					fprintf(out, " $%d", f->argsRegistry[f->argsRegistry.size()-1]);
				}
				else
				{
					fprintf(out, " no");
				}
				fprintf(out, "; ");
				if(f->returnRegistry > 0) // not a procedure
				{
					fprintf(out, "return : $%d; ", f->returnRegistry);
				}
				fprintf(out, "stack : %d}\n\n", f->stackSize);

				// ecriture instructions de la frame
				for(unsigned int i = 0 ; i < f->instructions.size() ; ++i)
				{
					IF_TYPE(f->instructions[i], Instruction::CALL_FUNCTION)
					{
						TO_PTYPE(call, f->instructions[i], InstructionCallFunction);
						fprintf(out, "%s\n", call->selectionOptimize().c_str());
					}
					else IF_TYPE(f->instructions[i], Instruction::CALL_REGISTRY_WRITE)
					{
						TO_PTYPE(call, f->instructions[i], InstructionCallRegistryWrite);
						fprintf(out, "%s\n", call->selectionOptimize().c_str());
					}
					else
					{
						fprintf(out, "%s\n", f->instructions[i]->selection().c_str());
					}
				}

				fprintf(out, "\n\n");
			}

			for(unsigned int i = 0 ; i < m_ilProgram->main->instructions.size() ; ++i)
			{
				IF_TYPE(m_ilProgram->main->instructions[i], Instruction::CALL_FUNCTION)
				{
					TO_PTYPE(call, m_ilProgram->main->instructions[i], InstructionCallFunction);
					fprintf(out, "%s\n", call->selectionOptimize().c_str());
				}
				else IF_TYPE(m_ilProgram->main->instructions[i], Instruction::CALL_REGISTRY_WRITE)
				{
					TO_PTYPE(call, m_ilProgram->main->instructions[i], InstructionCallRegistryWrite);
					fprintf(out, "%s\n", call->selectionOptimize().c_str());
				}
				else
				{
					fprintf(out, "%s\n", m_ilProgram->main->instructions[i]->selection().c_str());
				}

				//fprintf(out, "%s\n", m_ilProgram->main->instructions[i]->selection().c_str());
			}

			fclose(out);
		}
	}

	void IL::writeToFileSelection(const char * filename)
	{
		FILE * out = fopen(filename, "w+");
		if(out != NULL)
		{
			//ecriture des instructions du programme
			InstructionGoto g(ENTRY_POINT);
			fprintf(out, "%s\n", g.selection().c_str());

			//ecriture des frames
			for(unsigned int i = 0 ; i < m_ilProgram->frames.size() ; ++i)
			{
				Frame * f = m_ilProgram->frames[i];

				fprintf(out, "frame(%d) : { entry : %s; return : %s; args :", f->frameIndex, f->entryLabel.c_str(), f->returnLabel.c_str());
				if(f->argsRegistry.size() > 0)
				{
					for(unsigned int j = 0 ; j < f->argsRegistry.size() -1 ; ++j)
					{
						fprintf(out, " $%d,", f->argsRegistry[j]);
					}
					fprintf(out, " $%d", f->argsRegistry[f->argsRegistry.size()-1]);
				}
				else
				{
					fprintf(out, " no");
				}
				fprintf(out, "; ");
				if(f->returnRegistry > 0) // not a procedure
				{
					fprintf(out, "return : $%d; ", f->returnRegistry);
				}
				fprintf(out, "stack : %d}\n\n", f->stackSize);

				// ecriture instructions de la frame
				for(unsigned int i = 0 ; i < f->instructions.size() ; ++i)
				{
					fprintf(out, "%s\n", f->instructions[i]->selection().c_str());
				}

				fprintf(out, "\n\n");
			}

			for(unsigned int i = 0 ; i < m_ilProgram->main->instructions.size() ; ++i)
			{
				fprintf(out, "%s\n", m_ilProgram->main->instructions[i]->selection().c_str());
			}

			fclose(out);
		}
	}

	void IL::writeToFileMips(const char * filename)
	{
		FILE * out = fopen(filename, "w+");
		if(out != NULL)
		{
			//ecriture des instructions du programme
			InstructionGoto g(ENTRY_POINT);
			fprintf(out, "%s\n", g.selection().c_str());

			//ecriture des frames
			for(unsigned int j = 0 ; j < m_ilProgram->frames.size() ; ++j)
			{
				Frame * f = m_ilProgram->frames[j];

				fprintf(out, "%s\n", f->instructions[0]->mips().c_str());

				for(unsigned int i = 0 ; i < f->argsRegistry.size() ; ++i)
				{
					fprintf(out, "\t\tsw $a%d, %d($sp)\n", i, f->argsRegistry[i]*4);
				}

				// ecriture instructions de la frame
				for(unsigned int i = 1 ; i < f->instructions.size() - 1; ++i)
				{
					fprintf(out, "%s\n", f->instructions[i]->mips().c_str());
				}

				if(f->returnRegistry >= 0)
				{
					fprintf(out, "\t\tlw $v0, %d($sp)\n", f->returnRegistry * 4);
				}

				fprintf(out, "\tjr $ra\n");

				fprintf(out, "\n\n");
			}

			for(unsigned int i = 0 ; i < m_ilProgram->main->instructions.size() ; ++i)
			{
				fprintf(out, "%s\n", m_ilProgram->main->instructions[i]->mips().c_str());
			}

			fprintf(out, "\tli $v0, 10\n\tsyscall\n");

			fclose(out);
		}
	}

	Program* IL::process()
	{
		// récupération des noeuds fonctions
		std::vector<Node*> functions;

		for(unsigned int i = 0 ; i < m_program->children().size() ; ++i)
		{
			if(m_program->child(i)->type() == Node::FUNCTION_DECL)
			{
				functions.push_back(m_program->child(i));
			}
		}

		m_ilProgram = new Program();

		// création des frames
		for(unsigned int i = 0 ; i < functions.size() ; ++i)
		{
			Node * n = functions[i];
			Frame * f = new Frame();
			f->functionDecl = n;
			f->frameIndex = i+1;
			f->entryLabel = newLabel();
			f->returnLabel = newLabel();

			m_functionsMap.insert(std::make_pair(functionName(n), f));
			m_ilProgram->frames.push_back(f);
		}

		for(unsigned int i = 0 ; i < m_ilProgram->frames.size() ; ++i)
		{
			Frame * f = m_ilProgram->frames[i];
			Node * n = f->functionDecl;

			f->symbols = new SymbolTable();
			std::vector<std::string> params = extractParameters(n);

			for(unsigned int i = 0 ; i < params.size() ; ++i)
			{
				f->symbols->addVariable(params[i], "");
				f->symbols->get(params[i]).ilRegistry = f->nextRegister();

				f->argsRegistry.push_back(f->symbols->get(params[i]).ilRegistry);
			}

			//handle return if its a function
			if(n->child(0)->metadataType() != VOID_TYPE)
			{
				f->returnRegistry = f->nextRegister();
			}

			// parse instruction in frame
			Node * instructions = n->child(3);
			parseInstructions(instructions, f);

		}

		for(unsigned int i = 0 ; i < m_ilProgram->frames.size() ; ++i)
		{
			Frame * f = m_ilProgram->frames[i];
			f->instructions.insert(f->instructions.begin(), new InstructionNop());
			f->instructions.insert(f->instructions.begin(), new InstructionLabel(f->entryLabel));
			f->instructions.push_back(new InstructionLabel(f->returnLabel));
			f->instructions.push_back(new InstructionNop());
		}


		// gestion des instructions du programme
		m_ilProgram->main = new Frame();
		m_ilProgram->main->entryLabel = ENTRY_POINT;
		m_ilProgram->main->returnLabel = EXIT_POINT;
		m_ilProgram->main->symbols = new SymbolTable();

		parseInstructions(m_program->child(m_program->children().size() - 1), m_ilProgram->main);

		//m_ilProgram->instructions = main->instructions;
		m_ilProgram->main->instructions.insert(m_ilProgram->main->instructions.begin(), new InstructionLabel(ENTRY_POINT));
		m_ilProgram->main->instructions.push_back(new InstructionLabel(EXIT_POINT));

		return m_ilProgram;
	}

	int IL::countParameters(ast::Node * functionDecl)
	{
		//function FUNCTION_DECL_PARAM_LIST child + count its children
		for(unsigned int i = 0 ; i < functionDecl->children().size() ; ++i)
		{
			if(functionDecl->child(i)->type() == Node::FUNCTION_DECL_PARAMLIST)
			{
				return functionDecl->child(i)->children().size();
			}
		}
		return 0;
	}

	std::vector<std::string> IL::extractParameters(ast::Node * functionDecl)
	{
		std::vector<std::string> params;
		for(unsigned int i = 0 ; i < functionDecl->children().size() ; ++i)
		{
			if(functionDecl->child(i)->type() == Node::FUNCTION_DECL_PARAMLIST)
			{
				Node * paramsList = functionDecl->child(i);
				for(unsigned int i = 0 ; i < paramsList->children().size() ; ++i)
				{
					std::string v = ((StrNode*)paramsList->child(i)->child(1))->value();
					params.push_back(v);
				}
				return params;
			}
		}
		return params;
	}

	std::string IL::functionName(ast::Node * functionDecl)
	{
		return ((StrNode*)functionDecl->child(1))->value();
	}

	void IL::parseInstructions(Node * instructions, Frame * f)
	{
		IF_TYPE(instructions, Node::BLOCK)
		{
			f->symbols->push();
			//two child => declarations + instructions;

			parseDeclarations(instructions->child(0), f);
			Node * instructionsList = instructions->child(1);
			for(unsigned int i = 0 ; i < instructionsList->children().size() ; ++i)
			{
				parseInstructions(instructionsList->child(i), f);
			}

			f->symbols->pop();
		}
		else IF_TYPE(instructions, Node::INSCRUCTIONLIST)
		{
			for(unsigned int i = 0 ; i < instructions->children().size() ; ++i)
			{
				parseInstructions(instructions->child(i), f);
			}
		}
		else IF_TYPE(instructions, Node::AFFECTATION)
		{
			Node * id = instructions->child(0);
			Node * expr = instructions->child(1);

			IF_TYPE(id, Node::IDENTIFIER)
			{
				int reg = f->symbols->get(strValue(id)).ilRegistry;
				IF_TYPE(expr, Node::FUNCTION_CALL)
				{
					//function
					std::string functionName = strValue(expr->child(0));
					Node * params = expr->child(1);

					Frame * callFunc = m_functionsMap[functionName];

					InstructionCallRegistryWrite * instr = new InstructionCallRegistryWrite(reg, callFunc);
					for(unsigned int i = 0 ; i < params->children().size() ; ++i)
					{
						Expr * e = parseExpression(params->child(i), f);
						instr->addParameter(e);
					}
					f->add(instr);
				}
				else
				{
					Expr * e = parseExpression(expr, f);
					f->add(new InstructionRegistryWrite(reg, e));
				}
			}
			else
			{
				throw std::runtime_error("Unable to process array affectation");
			}
		}
		else IF_TYPE(instructions, Node::CONDITION_GROUP)
		{
			Node * condition = instructions->child(0)->child(0);
			Node * ifInstr = instructions->child(0)->child(1);

			std::string trueLabel = newLabel();
			std::string falseLabel = newLabel();
			std::string exitLabel = newLabel();

			Expr * e = parseExpression(condition, f);
			f->add(new InstructionJump(trueLabel, falseLabel, e));

			// if () {
			f->add(new InstructionLabel(trueLabel));
			parseInstructions(ifInstr, f);
			f->add(new InstructionGoto(exitLabel));
			// }
			// else {
			f->add(new InstructionLabel(falseLabel));
			if(instructions->children().size() > 1)
			{
				parseInstructions(instructions->child(1)->child(0), f);
			}
			f->add(new InstructionGoto(exitLabel));
			// }
			f->add(new InstructionLabel(exitLabel));
			f->add(new InstructionNop());

		}
		else IF_TYPE(instructions, Node::FUNCTION_CALL)
		{
			// procedure
			std::string functionName = strValue(instructions->child(0));
			Node * params = instructions->child(1);

			Frame * callFunc = m_functionsMap[functionName];

			InstructionCallFunction * instr = new InstructionCallFunction(callFunc);
			for(unsigned int i = 0 ; i < params->children().size() ; ++i)
			{
				Expr * e = parseExpression(params->child(i), f);
				instr->addParameter(e);
			}
			f->add(instr);
		}
		else IF_TYPE(instructions, Node::RETURN)
		{
			if(instructions->hasChild())
			{
				Node * expr = instructions->child(0);
				Expr * e = parseExpression(expr, f);
				f->add(new InstructionRegistryWrite(f->returnRegistry, e));
			}
			f->add(new InstructionGoto(f->returnLabel));
			f->add(new InstructionLabel(newLabel()));
			f->add(new InstructionNop());
		}
		else IF_TYPE(instructions, Node::WHILE_LOOP)
		{
			Node * cond = instructions->child(0);
			Node * instructionsList = instructions->child(1);

			std::string testLabel = newLabel();
			std::string instLabel = newLabel();
			std::string exitLabel = newLabel();


			f->add(new InstructionLabel(testLabel));
			Expr * e = parseExpression(cond, f);
			f->add(new InstructionJump(instLabel, exitLabel, e));
			// while() {
			f->add(new InstructionLabel(instLabel));
			parseInstructions(instructionsList, f);
			f->add(new InstructionGoto(testLabel));
			// }
			f->add(new InstructionLabel(exitLabel));
			f->add(new InstructionNop());
		}
	}

	void IL::parseDeclarations(Node * declarations, Frame * f)
	{
		for(unsigned int i = 0 ; i < declarations->children().size() ; ++i)
		{
			Node * decl = declarations->child(i)->child(1);
			for(unsigned int j = 0 ; j < decl->children().size() ; ++j)
			{
				std::string name = strValue(decl->child(j));
				f->symbols->addVariable(name, "");
				f->symbols->get(name).ilRegistry = f->nextRegister();
			}
		}
	}

	std::string IL::strValue(ast::Node* node)
	{
		return ((StrNode*)node)->value();
	}

	int IL::intValue(ast::Node* node)
	{
		return ((IntNode*)node)->value();
	}

	Expr * IL::parseExpression(ast::Node * expr, Frame * f)
	{
		// variable
		IF_TYPE(expr, Node::IDENTIFIER)
		{
			std::string name = strValue(expr);
			int reg = f->symbols->get(name).ilRegistry;
			return new ExprRegister(reg);
		}
		// constante
		else IF_TYPE(expr, Node::CONST_BOOL)
		{
			int val = intValue(expr);
			return new ExprConstInt(val);
		}
		else IF_TYPE(expr, Node::CONST_FLOAT)
		{
			throw std::runtime_error("Unable to process float");
		}
		else IF_TYPE(expr, Node::CONST_INT)
		{
			int val = intValue(expr);
			return new ExprConstInt(val);
		}
		// operateur arithmétique
		else IF_TYPE(expr, Node::OP_BIN_ARM_ADD)
		{
			return PROCESS_BINOP(expr, "+", f);
		}
		else IF_TYPE(expr, Node::OP_BIN_ARM_DIV)
		{
			return PROCESS_BINOP(expr, "/", f);
		}
		else IF_TYPE(expr, Node::OP_BIN_ARM_MUL)
		{
			return PROCESS_BINOP(expr, "*", f);
		}
		else IF_TYPE(expr, Node::OP_BIN_ARM_SUB)
		{
			return PROCESS_BINOP(expr, "-", f);
		}
		else IF_TYPE(expr, Node::OP_UNA_ARM_SUB)
		{
			return PROCESS_UNOP(expr, "-", f);
		}
		// opérateur logique
		else IF_TYPE(expr, Node::OP_BIN_LOG_AND)
		{
			return PROCESS_BINOP(expr, "&&", f);
		}
		else IF_TYPE(expr, Node::OP_BIN_LOG_OR)
		{
			return PROCESS_BINOP(expr, "||", f);
		}
		else IF_TYPE(expr, Node::OP_UNA_LOG_NOT)
		{
			return PROCESS_UNOP(expr, "!", f);
		}
		// opérateur de comparaisons
		else IF_TYPE(expr, Node::OP_BIN_LOG_DIFF)
		{
			return PROCESS_BINOP(expr, "<>", f);
		}
		else IF_TYPE(expr, Node::OP_BIN_LOG_EQUAL)
		{
			return PROCESS_BINOP(expr, "=", f);
		}
		else IF_TYPE(expr, Node::OP_BIN_LOG_INF)
		{
			return PROCESS_BINOP(expr, "<", f);
		}
		else IF_TYPE(expr, Node::OP_BIN_LOG_INFEQ)
		{
			return PROCESS_BINOP(expr, "<=", f);
		}
		else IF_TYPE(expr, Node::OP_BIN_LOG_SUP)
		{
			return PROCESS_BINOP(expr, ">", f);
		}
		else IF_TYPE(expr, Node::OP_BIN_LOG_SUPEQ)
		{
			return PROCESS_BINOP(expr, ">=", f);
		}
		return NULL;
	}

	void IL::optimize()
	{
		for(unsigned int i = 0 ; i < m_ilProgram->frames.size() ; ++i)
		{
			Frame * f = m_ilProgram->frames[i];
			optimizeFrame(f);
		}

		/*
		Frame * main = new Frame();
		main->entryLabel = ENTRY_POINT;
		main->returnLabel = EXIT_POINT;
		main->instructions = m_ilProgram->instructions;
		*/
		optimizeFrame(m_ilProgram->main);
		//m_ilProgram->instructions = main->instructions;
	}

	void IL::optimizeFrame(Frame * frame)
	{
		simplifyLabel(frame);
		removeDeadCode(frame);
		checkGotoUtility(frame);

		std::map<std::string, std::vector<InstructionGoto*> > labelToGoto;
		std::map<std::string, std::vector<InstructionJump*> > labelToJump;
		std::vector<InstructionLabel*> labelToInstruction;
		std::map<std::string, int> labelUse;

		labelUse[frame->entryLabel] = 1;
		labelUse[frame->returnLabel] = 1;

		for(unsigned int i = 0 ; i < frame->instructions.size() ; ++i)
		{
			Instruction * instr = frame->instructions[i];
			IF_TYPE(instr, Instruction::JUMP)
			{
				TO_PTYPE(jump, instr, InstructionJump);
				ADD_TO_MAPLIST(labelToJump, jump->labelTrue, jump);
				ADD_TO_MAPLIST(labelToJump, jump->labelFalse, jump);
				INC_MAP(labelUse, jump->labelTrue);
				INC_MAP(labelUse, jump->labelFalse);
			}
			else IF_TYPE(instr, Instruction::GOTO)
			{
				TO_PTYPE(go, instr, InstructionGoto);
				ADD_TO_MAPLIST(labelToGoto, go->labelName, go);
				INC_MAP(labelUse, go->labelName);
			}
			else IF_TYPE(instr, Instruction::LABEL)
			{
				TO_PTYPE(label, instr, InstructionLabel);
				labelToInstruction.push_back(label);
			}
		}

		//remove unused label
		for(unsigned int i = 0 ; i < labelToInstruction.size() ; ++i)
		{
			InstructionLabel * label = labelToInstruction[i];
			if(labelUse.count(label->labelName) <= 0)
			{
				//on peut supprimer jusqu'au prochain label
				REMOVE_UNTIL_LABEL(label, frame->instructions);
			}
		}

		simplifyLabel(frame);
		removeDeadCode(frame);
		checkGotoUtility(frame);
	}

	void IL::removeDeadCode(Frame * frame)
	{
		for(unsigned int i = 0 ; i < frame->instructions.size() ; ++i)
		{
			Instruction * instr = frame->instructions[i];
			if(instr->type() == Instruction::GOTO || instr->type() == Instruction::JUMP)
			{
				if(i+1 < frame->instructions.size())
				{
					Instruction * next = frame->instructions[i+1];
					if(next->type() != Instruction::LABEL)
					{
						REMOVE_UNTIL_LABEL(next, frame->instructions);
					}
				}
			}
			else IF_TYPE(instr, Instruction::NOP)
			{
				bool remove = false;
				if(i > 0 && frame->instructions[i-1]->type() != Instruction::LABEL)
				{
					remove = true;
				}
				else if(i+1 < frame->instructions.size() && frame->instructions[i+1]->type() != Instruction::LABEL)
				{
					remove = true;
				}

				if(remove)
				{
					frame->instructions.erase(frame->instructions.begin() + i);
					i--;
				}
			}
		}
	}

	void IL::checkGotoUtility(Frame * frame)
	{
		for(unsigned int i = 0 ; i < frame->instructions.size() - 1 ; ++i)
		{
			IF_TYPE(frame->instructions[i], Instruction::GOTO)
			{
				IF_TYPE(frame->instructions[i+1], Instruction::LABEL)
				{
					TO_PTYPE(go, frame->instructions[i], InstructionGoto);
					TO_PTYPE(label, frame->instructions[i+1], InstructionLabel);

					if(go->labelName == label->labelName)
					{
						frame->instructions.erase(frame->instructions.begin() + i);
					}
				}
			}
		}
	}

	void IL::simplifyLabel(Frame * frame)
	{
		std::map<std::string, std::vector<InstructionGoto*> > labelToGoto;
		std::map<std::string, std::vector<InstructionJump*> > labelToJump;
		std::vector<InstructionLabel*> labelToInstruction;

		for(unsigned int i = 0 ; i < frame->instructions.size() ; ++i)
		{
			Instruction * instr = frame->instructions[i];
			IF_TYPE(instr, Instruction::JUMP)
			{
				TO_PTYPE(jump, instr, InstructionJump);
				ADD_TO_MAPLIST(labelToJump, jump->labelTrue, jump);
				ADD_TO_MAPLIST(labelToJump, jump->labelFalse, jump);
			}
			else IF_TYPE(instr, Instruction::GOTO)
			{
				TO_PTYPE(go, instr, InstructionGoto);
				ADD_TO_MAPLIST(labelToGoto, go->labelName, go);
			}
			else IF_TYPE(instr, Instruction::LABEL)
			{
				TO_PTYPE(label, instr, InstructionLabel);
				labelToInstruction.push_back(label);
			}
		}

		for(unsigned int i = 0 ; i < labelToInstruction.size() ; ++i)
		{
			std::vector<Instruction*>::iterator begin, end;
			TO_PTYPE(currentLabel, labelToInstruction[i], InstructionLabel);

			if(currentLabel->labelName != frame->entryLabel && currentLabel->labelName != frame->returnLabel)
			{
				if(isUselessCode(frame, currentLabel, begin, end))
				{
					std::string oldLabel = currentLabel->labelName;
					std::string replaceLabel;

					IF_TYPE((*end), Instruction::GOTO)
					{
						TO_PTYPE(go, *end, InstructionGoto);
						replaceLabel = go->labelName;
						frame->instructions.erase(begin, end+1);
					}
					else IF_TYPE((*end), Instruction::LABEL)
					{
						TO_PTYPE(label, *end, InstructionLabel);
						replaceLabel = label->labelName;
						frame->instructions.erase(begin, end);
					}


					for(std::vector<InstructionGoto*>::iterator it = labelToGoto[oldLabel].begin() ; it != labelToGoto[oldLabel].end() ; ++it)
					{
						if((*it)->labelName == oldLabel)
						{
							(*it)->labelName = replaceLabel;
						}
						labelToGoto[replaceLabel].push_back(*it);
					}
					for(std::vector<InstructionJump*>::iterator it = labelToJump[oldLabel].begin() ; it != labelToJump[oldLabel].end() ; ++it)
					{
						if((*it)->labelTrue == oldLabel)
						{
							(*it)->labelTrue = replaceLabel;
						}
						if((*it)->labelFalse == oldLabel)
						{
							(*it)->labelFalse = replaceLabel;
						}
						labelToJump[replaceLabel].push_back(*it);
					}
					labelToGoto[oldLabel] = std::vector<InstructionGoto*>();
					labelToJump[oldLabel] = std::vector<InstructionJump*>();
				}
			}
		}

		//remove two label ex :
		// label1:
		// label2:
		// become label1: (to keep entry point safe)
		// warning : have to check to not remove exit point either
		for(unsigned int i = 1 ; i < frame->instructions.size() - 1 ; ++i)
		{
			IF_TYPE(frame->instructions[i], Instruction::LABEL)
			{
				IF_TYPE(frame->instructions[i+1], Instruction::LABEL)
				{
					TO_PTYPE(keep, frame->instructions[i], InstructionLabel);
					TO_PTYPE(remove, frame->instructions[i+1], InstructionLabel);

					std::string oldLabel = remove->labelName;
					std::string replaceLabel = keep->labelName;

					if(oldLabel != frame->returnLabel)
					{
						for(std::vector<InstructionGoto*>::iterator it = labelToGoto[oldLabel].begin() ; it != labelToGoto[oldLabel].end() ; ++it)
						{
							if((*it)->labelName == oldLabel)
							{
								(*it)->labelName = replaceLabel;
							}
							labelToGoto[replaceLabel].push_back(*it);
						}
						for(std::vector<InstructionJump*>::iterator it = labelToJump[oldLabel].begin() ; it != labelToJump[oldLabel].end() ; ++it)
						{
							if((*it)->labelTrue == oldLabel)
							{
								(*it)->labelTrue = replaceLabel;
							}
							if((*it)->labelFalse == oldLabel)
							{
								(*it)->labelFalse = replaceLabel;
							}
							labelToJump[replaceLabel].push_back(*it);
						}
						labelToGoto[oldLabel] = std::vector<InstructionGoto*>();
						labelToJump[oldLabel] = std::vector<InstructionJump*>();

						frame->instructions.erase(frame->instructions.begin() + i + 1);
						i--;
					}
				}
			}
		}
	}

	bool IL::isUselessCode(Frame * frame, Instruction *instr, std::vector<Instruction*>::iterator & start, std::vector<Instruction*>::iterator & end)
	{
		std::vector<Instruction*>::iterator current;
		for(current = frame->instructions.begin() ; current != frame->instructions.end() && *current != instr ; ++current);

		if(current != frame->instructions.end())
		{
			start = current;
			current++;
			for( ; current != frame->instructions.end() && (*current)->type() == Instruction::NOP ; current++);
			if(current != frame->instructions.end())
			{
				IF_TYPE((*current), Instruction::GOTO)
				{
					end = current;
					return true;
				}
				else IF_TYPE((*current), Instruction::LABEL)
				{
					end = current;
					return true;
				}
			}
			else
			{
				// std::cout << "\t\tReach end of frame" << std::endl;
			}
		}
		else
		{
			// std::cout << "\t\tInstruction not found" << std::endl;
		}
		return false;
	}




	/** Developpement des expressions **/
	void IL::selectInstruction()
	{
		for(unsigned int i = 0 ; i < m_ilProgram->frames.size() ; ++i)
		{
			selectInstruction(m_ilProgram->frames[i]);
		}

		/*
		Frame * main = new Frame();
		main->instructions = m_ilProgram->instructions;
		*/
		selectInstruction(m_ilProgram->main);

		//m_ilProgram->instructions = main->instructions;
	}

	void IL::selectInstruction(Frame * frame)
	{
		for(unsigned int i = 0 ; i < frame->instructions.size() ; ++i)
		{
			Instruction * instr = frame->instructions[i];

			// std::cout << "select Instruction " << instr << " of type " << instr->type() << std::endl;

			IF_TYPE(instr, Instruction::CALL_FUNCTION)
			{
				TO_PTYPE(functionCall, instr, InstructionCallFunction);

				std::vector<Expr*> expressions = functionCall->parameters;

				for(unsigned int j = 0 ; j < expressions.size() ; ++j)
				{
					std::pair<Expr*, std::vector<Instruction*> > n = compileExpression(expressions[j], frame);
					// compilation de l'expression

					// ajout des instructions au dessus de la courante
					for(unsigned int k = 0 ; k < n.second.size() ; ++k, ++i)
					{
						frame->instructions.insert(frame->instructions.begin() + i, n.second[k]);
					}

					// changement de l'expression actuel
					expressions[j] = n.first;
					functionCall->parameters = expressions;
				}
			}
			else IF_TYPE(instr, Instruction::CALL_REGISTRY_WRITE)
			{
				TO_PTYPE(registryWrite, instr, InstructionCallRegistryWrite);

				std::vector<Expr*> expressions = registryWrite->parameters;

				for(unsigned int j = 0 ; j < expressions.size() ; ++j)
				{
					std::pair<Expr*, std::vector<Instruction*> > n = compileExpression(expressions[j], frame);
					// compilation de l'expression

					// ajout des instructions au dessus de la courante
					for(unsigned int k = 0 ; k < n.second.size() ; ++k, ++i)
					{
						frame->instructions.insert(frame->instructions.begin() + i, n.second[k]);
					}

					// changement de l'expression actuel
					expressions[j] = n.first;
					registryWrite->parameters = expressions;
				}
			}
			else IF_TYPE(instr, Instruction::REGISTRY_WRITE)
			{
				TO_PTYPE(registryWrite, instr, InstructionRegistryWrite);

				Expr * e = registryWrite->expression;
				std::pair<Expr*, std::vector<Instruction*> > n = compileExpression(e, frame);
				// compilation de l'expression

				// ajout des instructions au dessus de la courante
				for(unsigned int k = 0 ; k < n.second.size() ; ++k, ++i)
				{
					frame->instructions.insert(frame->instructions.begin() + i, n.second[k]);
				}

				// changement de l'expression actuel
				registryWrite->expression = n.first;
			}
			else IF_TYPE(instr, Instruction::JUMP)
			{
				TO_PTYPE(jump, instr, InstructionJump);

				Expr * e = jump->expression;
				std::pair<Expr*, std::vector<Instruction*> > n = compileExpression(e, frame);
				// compilation de l'expression

				// ajout des instructions au dessus de la courante
				for(unsigned int k = 0 ; k < n.second.size() ; ++k, ++i)
				{
					frame->instructions.insert(frame->instructions.begin() + i, n.second[k]);
				}

				// changement de l'expression actuel
				jump->expression = n.first;
			}
		}
	}

	std::pair<Expr*, std::vector<Instruction*> > IL::compileExpression(Expr * e, Frame * f)
	{
		Expr * res = NULL;
		std::vector<Instruction*> instructions;

		// std::cout << "Expression : " << e << std::endl;
		// std::cout << "Type : " << e->type() << std::endl;

		IF_TYPE(e, Expr::BINOP)
		{
			TO_PTYPE(expression, e, ExprBinop);

			//// std::cout << "Binop compilation : " << expression->expression1 << " ; " << expression->expression2 << std::endl;

			std::pair<Expr*, std::vector<Instruction*> > r1 = compileExpression(expression->expression1, f);
			std::pair<Expr*, std::vector<Instruction*> > r2 = compileExpression(expression->expression2, f);

			ADD_VECTOR(instructions, r1.second);
			ADD_VECTOR(instructions, r2.second);
			expression->expression1 = r1.first;
			expression->expression2 = r2.first;
			InstructionRegistryWrite * operation = new InstructionRegistryWrite(f->nextRegister(), expression);

			res = new ExprRegister(operation->storeRegister);
			instructions.push_back(operation);
		}
		else IF_TYPE(e, Expr::CONST_INT)
		{
			int reg = f->nextRegister();
			instructions.push_back(new InstructionRegistryWrite(reg, e));
			res = new ExprRegister(reg);
		}
		else IF_TYPE(e, Expr::REGISTER)
		{
			res = e;
		}
		else IF_TYPE(e, Expr::UNOP)
		{
			TO_PTYPE(expression, e, ExprUnop);

			// std::cout << "Unop compilation : " << expression->expression << std::endl;

			std::pair<Expr*, std::vector<Instruction*> > result = compileExpression(expression->expression, f);
			expression->expression = result.first;
			instructions = result.second;
			res = expression;
		}

		return std::make_pair(res, instructions);
	}

	void IL::optimizeSelect()
	{
		for(unsigned int i  = 0 ; i < m_ilProgram->frames.size() ; ++i)
		{
			// std::cout << "==============> Working on frame " << i << std::endl;
			developFunctionCall(m_ilProgram->frames[i]);
			optimizeSelect(m_ilProgram->frames[i]);
			renumRegister(m_ilProgram->frames[i]);
		}

		// std::cout << "============> Working on frame main" << std::endl;
		developFunctionCall(m_ilProgram->main);
		optimizeSelect(m_ilProgram->main);
		renumRegister(m_ilProgram->main);
	}

	void IL::developFunctionCall(Frame * f)
	{
		for(unsigned int i = 0 ; i < f->instructions.size() ; ++i)
		{
			std::vector<Expr*> params;
			IF_TYPE(f->instructions[i], Instruction::CALL_FUNCTION)
			{
				TO_PTYPE(call, f->instructions[i], InstructionCallFunction);
				params = call->parameters;
			}
			else IF_TYPE(f->instructions[i], Instruction::CALL_REGISTRY_WRITE)
			{
				TO_PTYPE(call, f->instructions[i], InstructionCallRegistryWrite);
				params = call->parameters;
			}

			for(unsigned int j = 0 ; j < params.size() ; ++j, ++i)
			{
				InstructionArg * arg = new InstructionArg(j, params[j]);
				f->instructions.insert(f->instructions.begin() + i, arg);
			}
		}
	}

	void IL::optimizeSelect(Frame * f)
	{
		// possible pour instruction dest, src1, src2 =>
		// add / sub / div / mul
		// or / and
		// seq / sne / sle / sge / slt / sgt

		// possibilité de swap src1 & src2 pour : add, mul, or, and, seq, sne
		// possibilité de swap src1 & src2 avec instructions : sle / sge & slt / sgt

		std::map<int, std::pair<int, Instruction*> > instructions;
		for(unsigned int i = 0 ; i < f->instructions.size() ; ++i)
		{
			IF_TYPE(f->instructions[i], Instruction::REGISTRY_WRITE)
			{
				TO_PTYPE(registryWrite, f->instructions[i], InstructionRegistryWrite);

				if(instructions.count(registryWrite->storeRegister) > 0)
				{
					instructions.erase(instructions.find(registryWrite->storeRegister));
				}

				IF_TYPE(registryWrite->expression, Expr::CONST_INT)
				{
					TO_PTYPE(expr, registryWrite->expression, ExprConstInt);
					instructions.insert(std::make_pair(registryWrite->storeRegister, std::make_pair(expr->number, (Instruction*)registryWrite)));
				}
				else IF_TYPE(registryWrite->expression, Expr::REGISTER)
				{
					TO_PTYPE(expr, registryWrite->expression, ExprRegister);
					if(expr->reg == registryWrite->storeRegister)
					{
						//remove instruction
						f->instructions.erase(f->instructions.begin() + i);
						i--;
					}
					else if(instructions.count(expr->reg) > 0)
					{
						std::map<int, std::pair<int, Instruction*> >::iterator it = instructions.find(expr->reg);
						std::pair<int, Instruction*> i = (*it).second;

						delete registryWrite->expression;
						registryWrite->expression = new ExprConstInt(i.first);
					}
				}
				else IF_TYPE(registryWrite->expression, Expr::BINOP)
				{
					TO_PTYPE(expr, registryWrite->expression, ExprBinop);

					Expr * e1 = expr->expression1;
					Expr * e2 = expr->expression2;
					bool ok = false;

					IF_TYPE(e2, Expr::REGISTER)
					{
						TO_PTYPE(reg, e2, ExprRegister);
						if(instructions.count(reg->reg) > 0)
						{
							std::map<int, std::pair<int, Instruction*> >::iterator it = instructions.find(reg->reg);
							std::pair<int, Instruction*> i = (*it).second;

							delete expr->expression2;
							expr->expression2 = new ExprConstInt(i.first);

							ok = true;
						}
					}

					// possible pour instruction dest, src1, src2 =>
					// add / sub / div / mul
					// or / and
					// seq / sne / sle / sge / slt / sgt

					// possibilité de swap src1 & src2 pour : add, mul, or, and, seq, sne
					// possibilité de swap src1 & src2 avec instructions : sle / sge & slt / sgt

					if(!ok)
					{
						IF_TYPE(e1, Expr::REGISTER)
						{
							TO_PTYPE(reg, e1, ExprRegister);
							if(instructions.count(reg->reg) > 0)
							{
								std::pair<int, Instruction*> i = (*(instructions.find(reg->reg))).second;

								// std::cout << "OP = " << expr->op << std::endl;
								if(expr->op == "+" || expr->op == "*" || expr->op == "||" || expr->op == "&&" || expr->op == "=" || expr->op == "<>")
								{
									delete expr->expression1;
									expr->expression1 = expr->expression2;
									expr->expression2 = new ExprConstInt(i.first);
								}
								else if(expr->op == "<" || expr->op == ">" || expr->op == "<=" || expr->op == ">=")
								{
									delete expr->expression1;
									expr->expression1 = expr->expression2;
									expr->expression2 = new ExprConstInt(i.first);
									expr->op = (expr->op == "<") ? ">" : (expr->op == ">") ? "<" : (expr->op == "<=") ? ">=" : "<=";
								}
							}
						}
					}
				}
			}
			else IF_TYPE(f->instructions[i], Instruction::ARG)
			{
				TO_PTYPE(registryWrite, f->instructions[i], InstructionArg);

				if(instructions.count(registryWrite->storeRegister) > 0)
				{
					instructions.erase(instructions.find(registryWrite->storeRegister));
				}

				IF_TYPE(registryWrite->expression, Expr::CONST_INT)
				{
					TO_PTYPE(expr, registryWrite->expression, ExprConstInt);
					instructions.insert(std::make_pair(registryWrite->storeRegister, std::make_pair(expr->number, (Instruction*)registryWrite)));
				}
				else IF_TYPE(registryWrite->expression, Expr::REGISTER)
				{
					TO_PTYPE(expr, registryWrite->expression, ExprRegister);
					if(expr->reg == registryWrite->storeRegister)
					{
						//remove instruction
						f->instructions.erase(f->instructions.begin() + i);
						i--;
					}
					else if(instructions.count(expr->reg) > 0)
					{
						std::map<int, std::pair<int, Instruction*> >::iterator it = instructions.find(expr->reg);
						std::pair<int, Instruction*> i = (*it).second;

						delete registryWrite->expression;
						registryWrite->expression = new ExprConstInt(i.first);
					}
				}
				else IF_TYPE(registryWrite->expression, Expr::BINOP)
				{
					TO_PTYPE(expr, registryWrite->expression, ExprBinop);

					Expr * e1 = expr->expression1;
					Expr * e2 = expr->expression2;
					bool ok = false;

					IF_TYPE(e2, Expr::REGISTER)
					{
						TO_PTYPE(reg, e2, ExprRegister);
						if(instructions.count(reg->reg) > 0)
						{
							std::map<int, std::pair<int, Instruction*> >::iterator it = instructions.find(reg->reg);
							std::pair<int, Instruction*> i = (*it).second;

							delete expr->expression2;
							expr->expression2 = new ExprConstInt(i.first);

							ok = true;
						}
					}

					// possible pour instruction dest, src1, src2 =>
					// add / sub / div / mul
					// or / and
					// seq / sne / sle / sge / slt / sgt

					// possibilité de swap src1 & src2 pour : add, mul, or, and, seq, sne
					// possibilité de swap src1 & src2 avec instructions : sle / sge & slt / sgt

					if(!ok)
					{
						IF_TYPE(e1, Expr::REGISTER)
						{
							TO_PTYPE(reg, e1, ExprRegister);
							if(instructions.count(reg->reg) > 0)
							{
								std::pair<int, Instruction*> i = (*(instructions.find(reg->reg))).second;

								// std::cout << "OP = " << expr->op << std::endl;
								if(expr->op == "+" || expr->op == "*" || expr->op == "||" || expr->op == "&&" || expr->op == "=" || expr->op == "<>")
								{
									delete expr->expression1;
									expr->expression1 = expr->expression2;
									expr->expression2 = new ExprConstInt(i.first);
								}
								else if(expr->op == "<" || expr->op == ">" || expr->op == "<=" || expr->op == ">=")
								{
									delete expr->expression1;
									expr->expression1 = expr->expression2;
									expr->expression2 = new ExprConstInt(i.first);
									expr->op = (expr->op == "<") ? ">" : (expr->op == ">") ? "<" : (expr->op == "<=") ? ">=" : "<=";
								}
							}
						}
					}
				}
			}
		}

		instructions.clear();
		// reg < nbOccurence, ligne >
		std::map<int, std::pair<int, int> > uselessRegister;

		for(unsigned int i = 0 ; i < f->instructions.size() ; ++i)
		{
			IF_TYPE(f->instructions[i], Instruction::REGISTRY_WRITE)
			{
				TO_PTYPE(registryWrite, f->instructions[i], InstructionRegistryWrite);

				uselessRegister.insert(std::make_pair(registryWrite->storeRegister, std::make_pair(0, i)));

				checkRegisterUseInInstruction(uselessRegister, registryWrite->expression);
			}
			else IF_TYPE(f->instructions[i], Instruction::ARG)
			{
				TO_PTYPE(registryWrite, f->instructions[i], InstructionArg);

				//uselessRegister.insert(std::make_pair(registryWrite->storeRegister, std::make_pair(0, i)));

				checkRegisterUseInInstruction(uselessRegister, registryWrite->expression);
			}
			else IF_TYPE(f->instructions[i], Instruction::CALL_FUNCTION)
			{
				TO_PTYPE(functionCall, f->instructions[i], InstructionCallFunction);
				for(unsigned int j = 0 ; j < functionCall->parameters.size() ; ++j)
				{
					checkRegisterUseInInstruction(uselessRegister, functionCall->parameters[j]);
				}
			}
			else IF_TYPE(f->instructions[i], Instruction::CALL_REGISTRY_WRITE)
			{
				TO_PTYPE(callWrite, f->instructions[i], InstructionCallRegistryWrite);
				uselessRegister.insert(std::make_pair(callWrite->storeRegister, std::make_pair(0, i)));

				for(unsigned int j = 0 ; j < callWrite->parameters.size() ; ++j)
				{
					checkRegisterUseInInstruction(uselessRegister, callWrite->parameters[j]);
				}
			}
			else IF_TYPE(f->instructions[i], Instruction::JUMP)
			{
				TO_PTYPE(jump, f->instructions[i], InstructionJump);
				checkRegisterUseInInstruction(uselessRegister, jump->expression);
			}
		}
		INC_STATUSMAP(uselessRegister, f->returnRegistry);

		int lineOffset = 0;

		std::vector<int> rearrange;

		for(unsigned int i = 0 ; i < f->instructions.size() ; ++i)
		{
			// std::cout << i << " : " << f->instructions[i]->selection() << std::endl;
		}

		for(std::map<int, std::pair<int, int> >::iterator it = uselessRegister.begin() ; it != uselessRegister.end() ; ++it)
		{
			// std::cout << "deleting line " << it->second.second << " <=> " << it->second.first << std::endl;
			if(it->second.first == 0)
			{
				// std::cout << "delete " << f->instructions[it->second.second]->selection() << std::endl;
				rearrange.push_back(it->second.second);
			}
		}

		std::sort(rearrange.begin(), rearrange.end());
		for(unsigned int i = 0 ; i < rearrange.size() ; ++i)
		{
			int line = rearrange[i] - lineOffset;
			f->instructions.erase(f->instructions.begin() + line);
			lineOffset++;
		}



		// simplification des expression du type
		// binop temp, src1, src2
		// move dest, temp
		// => binop dest, src1, src2
		for(unsigned int i = 0 ; i < f->instructions.size() - 1 ; ++i)
		{
			Instruction * current = f->instructions[i];
			Instruction * next = f->instructions[i+1];

			IF_TYPE(current, Instruction::REGISTRY_WRITE)
			{
				IF_TYPE(next, Instruction::REGISTRY_WRITE)
				{
					TO_PTYPE(c, current, InstructionRegistryWrite);
					TO_PTYPE(n, next, InstructionRegistryWrite);

					IF_TYPE(c->expression, Expr::BINOP)
					{
						IF_TYPE(n->expression, Expr::REGISTER)
						{
							TO_PTYPE(e, n->expression, ExprRegister);
							if(c->storeRegister == e->reg)
							{
								c->storeRegister = n->storeRegister;
								f->instructions.erase(f->instructions.begin()+i+1);
							}
						}
					}
				}
				else IF_TYPE(next, Instruction::ARG)
				{
					TO_PTYPE(c, current, InstructionRegistryWrite);
					TO_PTYPE(n, next, InstructionArg);

					IF_TYPE(c->expression, Expr::BINOP)
					{
						IF_TYPE(n->expression, Expr::REGISTER)
						{
							TO_PTYPE(e, n->expression, ExprRegister);
							if(c->storeRegister == e->reg)
							{
								n->expression = c->expression;
								c->expression = NULL;
								//c->storeRegister = n->storeRegister;
								f->instructions.erase(f->instructions.begin()+i);
							}
						}
					}
				}
			}
		}
	}

	void IL::checkRegisterUseInInstruction(std::map<int, std::pair<int, int> > & useMap, Expr * e)
	{
		IF_TYPE(e, Expr::REGISTER)
		{
			TO_PTYPE(expr, e, ExprRegister);

			INC_STATUSMAP(useMap, expr->reg);
		}
		else IF_TYPE(e, Expr::BINOP)
		{
			TO_PTYPE(expr, e, ExprBinop);

			Expr * e1 = expr->expression1;
			Expr * e2 = expr->expression2;

			IF_TYPE(e1, Expr::REGISTER)
			{
				TO_PTYPE(reg, e1, ExprRegister);
				INC_STATUSMAP(useMap, reg->reg);
			}
			IF_TYPE(e2, Expr::REGISTER)
			{
				TO_PTYPE(reg, e2, ExprRegister);
				INC_STATUSMAP(useMap, reg->reg);
			}
		}
		else IF_TYPE(e, Expr::UNOP)
		{
			TO_PTYPE(expr, e, ExprUnop);

			IF_TYPE(expr->expression, Expr::REGISTER)
			{
				TO_PTYPE(reg, expr->expression, ExprRegister);
				INC_STATUSMAP(useMap, reg->reg);
			}
		}
	}

	void IL::renumExpr(Expr * e, int & regIndex, std::map<int, int> &regAssoc)
	{
		IF_TYPE(e, Expr::BINOP)
		{
			TO_PTYPE(expr, e, ExprBinop);
			renumExpr(expr->expression1, regIndex, regAssoc);
			renumExpr(expr->expression2, regIndex, regAssoc);
		}
		else IF_TYPE(e, Expr::REGISTER)
		{
			TO_PTYPE(expr, e, ExprRegister);
			INC_REG(regIndex, regAssoc, expr->reg);
		}
		else IF_TYPE(e, Expr::UNOP)
		{
			TO_PTYPE(expr, e, ExprUnop);
			renumExpr(expr->expression, regIndex, regAssoc);
		}
	}

	void IL::renumRegister(Frame * f)
	{
		// map old => new
		// liste des registres temporaires de 0 à n sauf les args qui ne doivent pas etre changé
		std::map<int, int> registers;
		int regIndex = 0;
		for(unsigned int i = 0 ; i < f->argsRegistry.size() ; ++i)
		{
			registers[f->argsRegistry[i]] = regIndex;
			f->argsRegistry[i] = regIndex;
			regIndex++;
		}
		if(f->returnRegistry >= 0)
		{
			registers[f->returnRegistry] = regIndex;
			f->returnRegistry = regIndex;
			regIndex++;
		}


		for(unsigned int i = 0 ; i < f->instructions.size() ; ++i)
		{
			IF_TYPE(f->instructions[i], Instruction::ARG)
			{
				TO_PTYPE(arg, f->instructions[i], InstructionArg);
				/*
				int reg = arg->storeRegister;
				INC_REG(regIndex, registers, reg);
				arg->storeRegister = reg;
				*/
				renumExpr(arg->expression, regIndex, registers);
			}
			/*
			else IF_TYPE(f->instructions[i], Instruction::CALL_FUNCTION)
			{
				TO_PTYPE(call, f->instructions[i], InstructionCallFunction);
			}
			*/
			else IF_TYPE(f->instructions[i], Instruction::CALL_REGISTRY_WRITE)
			{
				TO_PTYPE(call, f->instructions[i], InstructionCallRegistryWrite);
				int reg = call->storeRegister;
				INC_REG(regIndex, registers, reg);
				call->storeRegister = reg;
			}
			else IF_TYPE(f->instructions[i], Instruction::JUMP)
			{
				TO_PTYPE(jump, f->instructions[i], InstructionJump);
				renumExpr(jump->expression, regIndex, registers);
			}
			else IF_TYPE(f->instructions[i], Instruction::REGISTRY_WRITE)
			{
				TO_PTYPE(regWrite, f->instructions[i], InstructionRegistryWrite);
				int reg = regWrite->storeRegister;
				INC_REG(regIndex, registers, reg);
				regWrite->storeRegister = reg;
				renumExpr(regWrite->expression, regIndex, registers);
			}
		}

		f->stackSize = regIndex;
	}

}
