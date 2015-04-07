#include "compiler.hpp"
#include "typage.hpp"

#include "middlelanguage.hpp"

#include <iostream>

using namespace ast;
using namespace std;



vector<string> getArgs(int argc, char* argv[])
{
	vector<string> args;

	for(int i=0 ; i<argc ; ++i)
		args.push_back(argv[i]);

	return args;
}


int main(int argc, char* argv[])
{
	const vector<string> args = getArgs(argc, argv);
	const bool lexicalLog = find(args.begin(), args.end(), "--lexicalLog") != args.end();
	const bool syntacticLog = find(args.begin(), args.end(), "--syntacticLog") != args.end();

	ast::Node* program = ast::build(lexicalLog, syntacticLog);

	if(!program)
		return EXIT_FAILURE;

	FunctionTable * ft = createFunctionTable(program);

	try
	{
		processType(program, ft);
	}
	catch(std::string bouh)
	{
		cout << "ERROR : " << bouh << endl;
	}
	catch(const std::exception & e)
	{
		cout << "ERROR : " << e.what() << endl;
	}

	MiddleLanguage::IL il(program);
	il.process();
	il.writeToFile("out.il");
	il.optimize();
	il.writeToFile("out_optimize.il");
	il.selectInstruction();
	il.writeToFileSelection("out_select.il");
	il.optimizeSelect();
	il.writeOptimizedSelection("out_optimize_select.il");
	il.writeToFileMips("out_mips.asm");

	ast::makeDot(program, "out.dot", true);
	ast::deleteTree(program);

	delete ft;

	printf("End of the program !\n");
	return EXIT_SUCCESS;
}


