#include "Syntax.h"

class Syntax {
	Syntax(vector<Lexem>&& t_lex_table) {
		if (t_lex_table.empty()) {
			cout << "Lexemes table is empty!" << endl;
		}
		if (t_lex_table.at(0).GetToken() == eof_tk) {
			cout << "Opened file is empty!" << endl;
		}
	}
	Syntax::~Syntax() {}
}