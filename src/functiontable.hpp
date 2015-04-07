#ifndef FUNCTION_TABLE_HPP
#define FUNCTION_TABLE_HPP

#include <map>
#include <list>
#include <string>

#include "ast.hpp"
#include "functions.hpp"

class FunctionTable
{
public:
	struct Function
	{
		std::string returnType;
		std::vector<std::string> argTypes;
	};

	FunctionTable() {}

	void addFunction(const ast::Node * functionNode);

	bool exist(const std::string & functionName);
	bool isProcedure(const std::string & functionName);
	std::string returnType(const std::string & functionName);

	bool isCallValid(const ast::Node * callNode);

private:
	std::map<std::string, Function> _map;
};



#endif
