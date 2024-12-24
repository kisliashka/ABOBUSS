#ifndef LEXER_H
#define LEXER_H

#include <fstream>
#include <iostream>
#include "MyVector.h"
#include "Lexem.h"
using namespace std;
class Lexer
{
public:

	Lexer(const char* file_path);
	mylib::Vector<Lexem> ScanCode();
	~Lexer();

private:
	ifstream code;
	char cursor{ -1 };
	int	line{ 0 };
	mylib::Vector<Lexem> lex_table;

	Lexem GetLex();
	char GetChar();

	inline char	GetCurrentCurs() noexcept { return cursor; }

};

#endif // !LEXER_H