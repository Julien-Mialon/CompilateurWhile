#include "functiontable.hpp"

#include <iostream>

using namespace std;
using namespace ast;

void FunctionTable::addFunction(const Node * functionNode)
{
	const std::vector<Node*> & children = functionNode->children();

	StrNode * typeNode = (StrNode*)children[0];
	StrNode * idNode = (StrNode*)children[1];
	Node * args = children[2];

	Function f;

	std::string name = idNode->value();
	f.returnType = typeToString(typeNode);

	//cout << "==> Add function " << name << " return type #" << f.returnType << "#" << endl;

	const std::vector<Node*> & childrenOfArgs = args->children();
	f.argTypes.resize(childrenOfArgs.size());

	for (unsigned int i = 0; i < childrenOfArgs.size(); ++i)
		f.argTypes[i] = typeToString((StrNode*)childrenOfArgs[i]->children()[0]);

	_map[name] = f;
}

bool FunctionTable::exist(const std::string & functionName)
{
	return _map.count(functionName) == 1;
}

bool FunctionTable::isProcedure(const std::string & functionName)
{
	if (!exist(functionName))
		throw std::runtime_error("Invalid isProcedure call : \"" + functionName + "\" is not a known function");

	return _map[functionName].returnType == "void";
}

std::string FunctionTable::returnType(const std::string & functionName)
{
	if (!exist(functionName))
		throw std::runtime_error("Invalid isProcedure call : \"" + functionName + "\" is not a known function");

	return _map[functionName].returnType;
}

bool FunctionTable::isCallValid(const Node * callNode)
{
	const std::vector<Node*> & children = callNode->children();

	StrNode * idNode = (StrNode*)children[0];
	std::string functionName = idNode->value();

	if (!exist(functionName))
	{
		cerr << "invalid function call : function \"" << functionName << "\" does not exist" << endl;
		return false;
	}

	Node * args = children[1];

	if (args->children().size() != _map[functionName].argTypes.size())
	{
		cerr << "Invalid function call : function \"" << functionName << "\" needs "
			 << _map[functionName].argTypes.size() << " arguments, "
			 << args->children().size() << " given" << endl;

		return false;
	}

	const std::vector<Node*> & argsChildren = args->children();

	for (unsigned int i = 0; i < argsChildren.size(); ++i)
	{
		Node * arg = argsChildren[i];

		if (arg->metadataType() != _map[functionName].argTypes[i])
		{
			cerr << "Invalid function call : function \"" << functionName << "\", parameter "
				 << i << " expects a \"" << _map[functionName].argTypes[i] << "\", \""
				 << arg->metadataType() << " given" << endl;
			return false;
		}
	}

	return true;
}
