#include "symboltable.hpp"

#include <stdexcept>
#include <iostream>

using namespace std;

SymbolTable::SymbolTable()
{
	push();
}

void SymbolTable::push()
{
	_stack.push_front(map<string, Symbol>());
}

void SymbolTable::pop()
{
	_stack.pop_front();
}

bool SymbolTable::exist(const std::string &variableName)
{
	list<map<string, Symbol> >::const_iterator it = _stack.begin();
	for (; it != _stack.end(); ++it)
	{
		if ((*it).find(variableName) != (*it).end())
			return true;
	}

	return false;
}

SymbolTable::Symbol & SymbolTable::get(const std::string & variableName)
{
	list<map<string, Symbol> >::iterator it = _stack.begin();
	for (; it != _stack.end(); ++it)
	{
		map<string, Symbol>::iterator itFind = (*it).find(variableName);
		if (itFind != (*it).end())
			return (*it).at(variableName);
	}

	throw std::runtime_error("Invalid SymbolTable::getType call on \"" +
							 variableName + "\" : no such variable");
}

std::string SymbolTable::getType(const string & variableName)
{
	list<map<string, Symbol> >::const_iterator it = _stack.begin();
	for (; it != _stack.end(); ++it)
	{
		map<string, Symbol>::const_iterator itFind = (*it).find(variableName);
		if (itFind != (*it).end())
			return (*it).at(variableName).type;
	}

	throw std::runtime_error("Invalid SymbolTable::getType call on \"" +
							 variableName + "\" : no such variable");
}

void SymbolTable::addVariable(const std::string &variableName, const std::string &type)
{
	if (exist(variableName))
		throw std::runtime_error("Invalid SymbolTable::addVariable call : \"" + variableName + "\" already exists");

	Symbol symbol;

	symbol.type = type;
	symbol.address = -0x2A;

	_stack.front()[variableName] = symbol;
}
