#include "functions.hpp"
#include "ast.hpp"

#include <iostream>

using namespace ast;
using namespace std;

std::string typeToString(ast::StrNode *typeNode)
{
	std::string finalType;
	int arrayCount = 0;

	StrNode * it = typeNode;

	while (it != 0)
	{
		if (it->value() == ARRAY_TYPE)
		{
			++arrayCount;

			if (it->children().size() == 0)
				it = 0;
			else
			{
				if (it->children().size() > 1)
					cerr << "Warning : a type node has 2 children..." << endl;
				it = (StrNode*) it->children()[0];
			}

		}
		else if (it->value() == VOID_TYPE)
		{
			finalType = VOID_TYPE;
			break;
		}
		else if (it->value() == INT_TYPE)
		{
			finalType = INT_TYPE;
			break;
		}
		else if (it->value() == FLOAT_TYPE)
		{
			finalType = FLOAT_TYPE;
			break;
		}
		else
			throw std::runtime_error("typeToString : unknown type read : \"" + it->value() + "\"");
	}

	for (int i = 0; i < arrayCount; ++i)
		finalType += ARRAY_DELIMITER;

	return finalType;
}

bool isArrayType(const std::string & type)
{
	return (type.length() > 0 && type[type.length()-1] == ARRAY_DELIMITER);
}

std::string getSubArrayType(const std::string & type)
{
	if(isArrayType(type))
	{
		return type.substr(0, type.length() - 1);
	}
	throw std::runtime_error("getSubArrayType : not an array type : \"" + type + "\"");
}


string unwrapType(const string &type)
{
	bool found = false;
	int arrayCount = 0;
	int indexOfFirstArray = type.size();

	for (unsigned int i = 0; i < type.size(); ++i)
	{
		if (type[i] == ARRAY_DELIMITER)
		{
			++arrayCount;

			if (!found)
			{
				found = true;
				indexOfFirstArray = i;
			}
		}
	}

	string ret;

	for (int i = 0; i < arrayCount; ++i)
		ret += "array(";

	ret += type.substr(0, indexOfFirstArray);

	for (int i = 0; i < arrayCount; ++i)
		ret += ")";

	return ret;
}
