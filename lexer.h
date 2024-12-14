#ifndef LEXER_H
#define LEXER_H

#include <fstream>
#include <iostream>
#include <vector>
#include "Lexem.h"
using namespace std;
class Lexer
{
public:

	Lexer(const char* file_path);
	vector<Lexem> ScanCode();
	~Lexer();

private:
	ifstream code;
	char cursor{ -1 };
	int	line{ 0 };
	vector<Lexem> lex_table;

	Lexem GetLex();
	char GetChar();

	inline char	GetCurrentCurs() { return cursor; }

};

#endif // !LEXER_H