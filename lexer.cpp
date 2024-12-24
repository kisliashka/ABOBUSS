#include "Lexer.h"
#include "MyVector.h"
using namespace std;

Lexer::Lexer(const char* file_path) {
    try {
        code.open(file_path);
    }
    catch (const exception& exp) {
        string what = "<E> Lexer: Catch exception in constructor:";
        cout << what << exp.what() << endl;
    }
}

mylib::Vector<Lexem> Lexer::ScanCode()
{
    try {
        if (!code) {
            cout << "<E> Can't open file" << endl;
            return lex_table;
        }

        while (!code.eof()) {
            Lexem current_lex = GetLex();
            if (current_lex.GetToken() != unknown_tk) {
                lex_table.push_back(current_lex);
            }
            if (code.eof()) break;
        }

        // Добавляем EOF токен только если последняя лексема не EOF
        if (lex_table.empty() || lex_table.back().GetToken() != eof_tk) {
            lex_table.push_back(Lexem(move("EOF"), eof_tk, line));
        }
        return lex_table;
    }
    catch (const exception& exp) {
        cout << "<E> Catch exception in Scanning Code: " << exp.what() << endl;
        return lex_table;
    }
}

Lexer::~Lexer() {
    if (code.is_open()) {
        code.close();
    }
}

Lexem Lexer::GetLex()
{
    try {
        auto ch = GetCurrentCurs();
        // Пропускаем пробельные символы
        while (!code.eof() && (ch == -1 || ch == ' ' || ch == '\r' || ch == '\n' || ch == '\t')) {
            if (ch == '\n') line++;
            ch = GetChar();
        }

        // Проверка на конец файла
        if (code.eof()) {
            return Lexem(move("EOF"), eof_tk, line);
        }

        auto isId = [](char ch) {
            return isalpha(static_cast<unsigned char>(ch)) ||
                isdigit(static_cast<unsigned char>(ch));
        };

        string lex;
        if (isdigit(ch)) { // константы(числа)
            while (isdigit(ch)) {
                lex += ch;
                ch = GetChar();
                if (code.eof()) break;
            }
            return Lexem(move(lex), constant_tk, line);
        }
        else if (isalpha(static_cast<unsigned char>(ch))) { // идентификаторы
            while (!code.eof() && isId(ch)) {
                lex += ch;
                ch = GetChar();
            }

            // Проверка ключевых слов
            if (lex == "program") return Lexem(move(lex), program_tk, line);
            else if (lex == "var") return Lexem(move(lex), var_tk, line);
            else if (lex == "begin") return Lexem(move(lex), begin_tk, line);
            else if (lex == "integer") return Lexem(move(lex), integer_tk, line);
            else if (lex == "boolean") return Lexem(move(lex), boolean_tk, line);
            else if (lex == "const") return Lexem(move(lex), const_tk, line);
            else if (lex == "end") return Lexem(move(lex), end_tk, line);
            else if (lex == "false") return Lexem(move(lex), bool_false_tk, line);
            else if (lex == "true") return Lexem(move(lex), bool_true_tk, line);
            else if (lex == "of") return Lexem(move(lex), of_tk, line);
            else if (lex == "div") return Lexem(move(lex), div_tk, line);
            else if (lex == "if") return Lexem(move(lex), if_tk, line);
            else if (lex == "else") return Lexem(move(lex), else_tk, line);
            else if (lex == "label") return Lexem(move(lex), label_tk, line);
            else if (lex == "goto") return Lexem(move(lex), goto_tk, line);
            else return Lexem(move(lex), id_tk, line);
        }
        else if (ispunct(static_cast<unsigned char>(ch))) { // другие символы
            tokens tok = unknown_tk;
            switch (ch) {
            case ',': tok = comma_tk; break;
            case '.': tok = dot_tk; break;
            case ':': tok = colon_tk; break;
            case ';': tok = semicolon_tk; break;
            case '=': tok = equals_tk; break;
            case '+': tok = plus_tk; break;
            case '-': tok = minus_tk; break;
            case '*': tok = mul_tk; break;
            case '(': tok = opb_tk; break;
            case ')': tok = cpb_tk; break;
            case '[': tok = osb_tk; break;
            case ']': tok = csb_tk; break;
            case '>': tok = bool_bigger_tk; break;
            case '<': tok = bool_less_tk; break;
            case '\'': tok = apostrophe_tk; break;
            default:
                cout << "<E> Unknown token " << ch << endl;
                tok = unknown_tk;
                break;
            }
            lex += ch;

            // Обработка составных операторов
            if (!code.eof()) {
                if (tok == colon_tk) {
                    ch = GetChar();
                    if (ch == '=') {
                        lex += ch;
                        tok = assignment_tk;
                    }
                }
                else if (tok == equals_tk) {
                    ch = GetChar();
                    if (ch == '=') {
                        lex += ch;
                        tok = bool_equals_tk;
                    }
                }
                else if (tok == bool_bigger_tk) {
                    ch = GetChar();
                    if (ch == '=') {
                        lex += ch;
                        tok = bool_bigeqv_tk;
                    }
                }
                else if (tok == bool_less_tk) {
                    ch = GetChar();
                    if (ch == '=') {
                        lex += ch;
                        tok = bool_leseqv_tk;
                    }
                }
                // '<>'
                else if (tok == bool_less_tk) {
                    ch = GetChar();
                    if (ch == '>') {
                        lex += ch;
                        tok = bool_noneqv_tk;
                    }
                }
            }

            if (tok != unknown_tk) {
                GetChar(); // Получаем следующий символ
                return Lexem(move(lex), tok, line);
            }
        }

        GetChar(); // Пропускаем неизвестный символ
        return Lexem(move(""), unknown_tk, line);
    }
    catch (const exception& exp) {
        cout << "<E> Exception in GetLex: " << exp.what() << endl;
        return Lexem(move(""), unknown_tk, line);
    }
}

char Lexer::GetChar()
{
    if (code.fail()) {
        cout << "<E> Can't read from the file" << endl;
        return EOF;
    }

    code >> noskipws >> cursor;
    return cursor;
}