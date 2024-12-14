#include "Course_project.h"
#include <vector>
#include <iostream>
#include "MySoon.h"
using namespace std;
int Compile(const string& file_path) {


	Lexer lex(file_path.c_str());
	auto table = lex.ScanCode(); //table of lexemes
	cout << endl;
	cout << "Количество обработанных лексем: " << table.size() << endl;
	cout << endl;
	cout << "Считанный данные: " << endl;
	cout << endl;
	for (int i = 0; i < table.size(); ++i) {
		cout << table[i].GetLine() << ' ' << table[i].GetName() << ' ' << table[i].GetToken() << endl;
	}

	Syntaxx syntx(std::move(table));
	auto tree = syntx.Parser();
	/*if (tree == nullptr) {
		std::cerr << "<E> Incorrect syntax tree, abort!" << std::endl;
		return -EXIT_FAILURE;
	}*/

	return EXIT_SUCCESS;
	

	return EXIT_SUCCESS;
}
