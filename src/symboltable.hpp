#ifndef SYMBOLTABLE_HPP
#define SYMBOLTABLE_HPP

#include <string>
#include <list>
#include <map>

class SymbolTable
{
public:
	struct Symbol
	{
		std::string type;
		int address;
		int ilRegistry;
	};

	SymbolTable();

	void push();
	void pop();

	bool exist(const std::string & variableName);

	void addVariable(const std::string & variableName, const std::string & type);
	std::string getType(const std::string & variableName);

	Symbol & get(const std::string & variableName);

private:
	std::list<std::map<std::string, Symbol> > _stack;
};

#endif // SYMBOLTABLE_HPP
