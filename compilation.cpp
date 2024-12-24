#include "Course_project.h"
#include "MyVector.h"
#include <iostream>
#include "GenCode.h"
// #include "CodeGenerator.h"
#include "MySoon.h"
#include <memory>

using namespace std;

// Реализация функции Compile
int Compile(const string& file_path) {
    Lexer lex(file_path.c_str());

    // Сканируем код
    mylib::Vector<Lexem> table = lex.ScanCode();
    if (table.empty()) {
        cerr << "<E> No lexemes processed, abort!" << endl;
        return EXIT_FAILURE;
    }

    // Выводим список лексем
    for (size_t i = 0; i < table.size(); ++i) {
        cout << table[i].GetLine() << ' ' << table[i].GetName() << ' ' << table[i].GetToken() << endl;
    }
    cout << "Number of Lexemes: " << table.size() << endl;

    // Передаем table в Syntaxx
    Syntaxx syntx(std::move(table));

    // Генерируем синтаксическое дерево
    Tree *tree = syntx.Parser();
    if (!tree) {
        cerr << "<E> Incorrect syntax tree, abort!" << endl;
        return EXIT_FAILURE;
    }

    // Если понадобится генерация кода:
    
    GenCode generator(tree);
    auto result = generator.GenerateAsm();
    if (result == EXIT_SUCCESS) {
        std::cout << "Code generation completed successfully." << std::endl;
    }
    else {
        std::cerr << "Code generation failed." << std::endl;
    }


    return EXIT_SUCCESS;
}

