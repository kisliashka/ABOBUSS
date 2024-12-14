#ifndef SINTAX_ANALIZ_H
#define SINTAX_ANALIZ_H
#include <regex>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <vector>
#include <cstdlib>
#include <typeinfo>
#include <memory>
#include <queue>
#include <utility>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <stdexcept>
#include <sstream>
#include <array>

#include "Tree.h"
#include "Lexem.h"
#include "variable.h"

using namespace std;

class Syntaxx {
public:
    vector<Lexem> lex_table; // out table of lexemes
    map<string, Variable> id_map; // our table of identifiers
    //Tree syntax_tree;
    Tree* root;
    int Errors = 0;
    int log_count = 0;
    int rep_count = 0;
    bool var_section_started = false;
    string label_setup;

    using lex_it = std::vector<Lexem>::iterator; // alias of vector iterator
    lex_it cursor;
    lex_it label_adr;

    // Проверяем пустой ли у нас файл и инициализируем таблицу лексем
    Syntaxx(vector<Lexem> t_lex_table) : lex_table(t_lex_table), cursor(lex_table.begin()) {
        if (lex_table.empty()) {
            cout << "Lexemes table is empty!" << endl;
        }
        else if (lex_table.at(0).GetToken() == eof_tk) {
            cout << "Opened file is empty!" << endl;
        }
    }

    // Получаем по итерации следующую лексему
    lex_it getNextLex(lex_it iter) {
        ++iter;
        return iter;
    }

    // Соответствует ли лексема токену
    bool checkLexem(const lex_it& t_iter, const tokens& t_tok) {
        if (t_iter == lex_table.end()) {
            return false;
        }
        return t_iter->GetToken() == t_tok;
    }

    //Проверка существования переменной
    bool isVarExist(const string& t_var_name)
    {
        auto map_iter = id_map.find(t_var_name);
        return !(map_iter == id_map.end());
    }

    //Получить тип переменной
    string getVarType(const string& t_var_name)
    {
        auto map_iter = id_map.find(t_var_name);
        return map_iter->second.type;
    }

    //Обновление информации о типе в карте индентификаторов
    void updateVarTypes(const list<string>& t_var_list,
        const string& t_type_name, int t_array_l) {
        try {
            for (auto& el : t_var_list)
            {
                id_map.at(el).type = t_type_name;
                id_map.at(el).array_l = t_array_l;
            }
        }
        catch (const exception& exp) {
            cerr << "<E> Syntax: Catch exception in " << __func__ << ": "
                << exp.what() << endl;
        }
    }

    //Обновление информации о значении в карте индентификаторов
    void updateVarValue(const list<string>& t_var_list,
        const string& t_value) {
        try {
            for (auto& el : t_var_list)
            {
                id_map.at(el).value = t_value;
            }
        }
        catch (const exception& exp) {
            cerr << "<E> Syntax: Catch exception in " << __func__ << ": "
                << exp.what() << endl;
        }
    }

    void printIdMap() const {
        cout << "Identifier Table:" << endl;
        if (id_map.empty()) {
            cout << "  (empty)" << endl;
            return;
        }
        for (const auto& pair : id_map) {
            cout << "  " << pair.first << ": { type: " << pair.second.type
                << ", value: " << pair.second.value << ", array_l: " << pair.second.array_l << " }" << endl;
        }
    }

    bool ifVarInt(const string& t_var_name) {
        if (getVarType(t_var_name) == "integer") {
            return true;
        }
        else {
            return false;
        }
    }

    bool ifVarBool(const string& t_var_name) {
        if (getVarType(t_var_name) == "boolean") {
            return true;
        }
        else {
            return false;
        }
    }

    int programParse(lex_it& t_iter, Tree* parent) {
        // PROGRAMPARSING
        if (checkLexem(t_iter, program_tk)) { // ВАРИАНТ ПАРСИНГА С УЧЕТОМ PROGRAM
            auto iter = getNextLex(t_iter);
            if (!checkLexem(iter, id_tk)) { // проверка на название программы
                printError(MUST_BE_ID, *iter);
                Errors++;
                return -EXIT_FAILURE;
            }
            Tree* NAMEPROG = Tree::CreateNode(iter->GetName());
            parent->AddRightTree(NAMEPROG);
            iter++;

            // Создание узла BLOCK и добавление его как левый сын Program
            Tree* BLOCK = Tree::CreateNode("BLOCK");
            parent->AddLeftTree(BLOCK);

            // Обработка ';' после названия программы
            if (!checkLexem(iter, semicolon_tk)) {
                printError(MUST_BE_SEMI, *iter);
                Errors++;
                return -EXIT_FAILURE;
            }

            iter++;

            // VARPARSING: Обработка объявлений переменных
            while (!checkLexem(iter, begin_tk)) { // Пока не встретится BEGIN
                // ОБРАБОТКА ПЕРЕМЕННЫХ
                while (checkLexem(iter, var_tk)) {
                    iter++; // Переходим после 'var'

                    // Сбор имен идентификаторов
                    list<string> var_list;
                    while (checkLexem(iter, id_tk)) {
                        string varName = iter->GetName();

                        // Проверка на повторное объявление
                        if (isVarExist(varName)) {
                            cout << "Re-declared identifier '" << varName << "'" << endl;
                            Errors++;
                            return -EXIT_FAILURE;
                        }
                        else {
                            // Временная инициализация переменной с неизвестным типом
                            id_map.emplace(varName, Variable("?", "?", 0));
                            var_list.push_back(varName);
                        }

                        iter++;

                        if (checkLexem(iter, comma_tk)) {
                            iter++; // Пропускаем запятую и продолжаем
                            if (!checkLexem(iter, id_tk)) {
                                printError(MUST_BE_ID, *iter);
                                Errors++;
                                return -EXIT_FAILURE;
                            }
                        }
                        else {
                            break; // Нет запятой, заканчиваем сбор переменных
                        }
                    }

                    // После списка переменных должен идти ':'
                    if (!checkLexem(iter, colon_tk)) {
                        cout << "Must be ':' after variable names." << endl;
                        Errors++;
                        return -EXIT_FAILURE;
                    }
                    iter++; // Переходим после ':'

                    // Проверка типа переменной
                    if (!checkLexem(iter, integer_tk) && !checkLexem(iter, boolean_tk)) {
                        cout << "Unknown type." << endl;
                        Errors++;
                        return -EXIT_FAILURE;
                    }
                    string type_iter = iter->GetName();
                    iter++; // Переходим после типа

                    // Проверка на ';' после объявления переменных
                    if (!checkLexem(iter, semicolon_tk)) {
                        cout << "Must be ';' after variable declaration." << endl;
                        Errors++;
                        return -EXIT_FAILURE;
                    }
                    iter++; // Переходим после ';'

                    // Обновление типов переменных в карте идентификаторов
                    for (const string& varName : var_list) {
                        id_map.at(varName).type = type_iter;
                    }

                    // Создание отдельного VarDecl для каждой переменной
                    for (const string& varName : var_list) {
                        // Создание узла VarDecl
                        Tree* currentDecl = Tree::CreateNode("VarDecl");

                        // Добавление VarDecl как сиблинг под BLOCK
                        if (BLOCK->GetLeftNode() == nullptr) {
                            BLOCK->AddLeftTree(currentDecl);
                        }
                        else {
                            Tree* sibling = BLOCK->GetLeftNode();
                            while (sibling->GetRightNode() != nullptr) {
                                sibling = sibling->GetRightNode();
                            }
                            sibling->AddRightTree(currentDecl);
                        }

                        // Добавление имени переменной как левый узел VarDecl
                        Tree* varNode = Tree::CreateNode(varName);
                        currentDecl->AddLeftTree(varNode);

                        // Добавление типа как правый узел VarDecl
                        Tree* typeNode = Tree::CreateNode(type_iter);
                        varNode->AddRightTree(typeNode);
                    }
                }

                // ОБРАБОТКА LABELS
                if (checkLexem(iter, label_tk)) {
                    iter++;
                    // ОБРАБОТКА ИНДЕНТИФИКАТОРОВ
                    if (!checkLexem(iter, id_tk)) { // проверка на идентификатор
                        cout << "Must be identifier." << endl;
                        Errors++;
                        return -EXIT_FAILURE;
                    }

                    list<string> label_list;

                    // Сбор меток
                    while (checkLexem(iter, id_tk)) {
                        string labelName = iter->GetName();

                        // Проверка на повторное объявление
                        if (isVarExist(labelName)) {
                            cout << "Re-declared identifier '" << labelName << "'" << endl;
                            Errors++;
                            return -EXIT_FAILURE;
                        }
                        else {
                            id_map.emplace(labelName, Variable("label", "?", 0));
                            label_list.push_back(labelName);
                        }

                        iter++;

                        if (checkLexem(iter, comma_tk)) {
                            iter++; // Пропускаем запятую и продолжаем
                            if (!checkLexem(iter, id_tk)) {
                                printError(MUST_BE_ID, *iter);
                                Errors++;
                                return -EXIT_FAILURE;
                            }
                        }
                        else {
                            break; // Нет запятой, заканчиваем сбор меток
                        }
                    }

                    // Проверка на ';' после объявлений меток
                    if (!checkLexem(iter, semicolon_tk)) {
                        cout << "Must be ';' after label declarations." << endl;
                        Errors++;
                        return -EXIT_FAILURE;
                    }
                    iter++; // Переходим после ';'

                    // Создание отдельного LabelDecl для каждой метки
                    for (const string& labelName : label_list) {
                        // Создание узла LabelDecl
                        Tree* currentLabelDecl = Tree::CreateNode("LabelDecl");

                        // Добавление LabelDecl как сиблинг под BLOCK
                        if (BLOCK->GetLeftNode() == nullptr) {
                            BLOCK->AddLeftTree(currentLabelDecl);
                        }
                        else {
                            Tree* sibling = BLOCK->GetLeftNode();
                            while (sibling->GetRightNode() != nullptr) {
                                sibling = sibling->GetRightNode();
                            }
                            sibling->AddRightTree(currentLabelDecl);
                        }

                        // Добавление имени метки как левый узел LabelDecl
                        Tree* labelNode = Tree::CreateNode(labelName);
                        currentLabelDecl->AddLeftTree(labelNode);
                    }
                }
                // ОБРАБОТКА BEGIN
                else if (!checkLexem(iter, begin_tk)) {
                    cout << "Expected 'begin'." << endl;
                    Errors++;
                    return -EXIT_FAILURE;
                }
            }
            iter++;

            //Создание дерева COMPOUND
            Tree* COMPOUND = Tree::CreateNode("BEGIN");
            BLOCK->AddRightTree(COMPOUND);
            // ОБРАБАТЫВАЕТСЯ ВСЕ, ЧТО НАХОДИТСЯ ПОД BEGIN
            while (!checkLexem(iter, end_tk)) {

                //label
                // ПРОВЕРКА НА ИНДЕТИФИКАТОРЫ
                if (checkLexem(iter, id_tk)) {
                    if (isVarExist(iter->GetName())) {
                    
                    //если переменная является меткой, то будут следующие правила;
                        if (getVarType(iter->GetName()) == "label") {
                            label_adr = iter;
                            label_setup = iter->GetName();
                            iter++;
                            if (!checkLexem(iter, colon_tk)) {
                                cout << "Must be ':' after label. " << endl;
                                Errors++;
                                return -EXIT_FAILURE;
                            }
                        }
                        //если переменная является boolean, то будут следующие правила
                        else if (getVarType(iter->GetName()) == "boolean") {

                            Tree* VARNAME = Tree::CreateNode(iter->GetName());
                            COMPOUND->AddLeftTree(VARNAME);

                            iter++;
                            if (!checkLexem(iter, assignment_tk)) {
                                cout << "Must be ':='." << endl;
                                Errors++;
                                return -EXIT_FAILURE;
                            }

                            Tree* ASSIGN = Tree::CreateNode(iter->GetName());
                            VARNAME->AddLeftTree(ASSIGN);

                            iter++;
                            if (!checkLexem(iter, bool_true_tk)) {
                                if (!checkLexem(iter, bool_false_tk)) {
                                    if (checkLexem(iter, constant_tk)) {
                                        cout << "Couldn't convert boolean to integer." << endl;
                                        Errors++;
                                        return -EXIT_FAILURE;
                                    }
                                    
                                    else if (getVarType(iter->GetName()) == "integer") {
                                        cout << "Couldn't convert boolean to integer." << endl;
                                        Errors++;
                                        return -EXIT_FAILURE;
                                    }
                                    else if (getVarType(iter->GetName()) == "label") {
                                        cout << "Expected for var name." << endl;
                                        Errors++;
                                        return -EXIT_FAILURE;
                                    }
                                    else if (isVarExist(iter->GetName())) {
                                        if (getVarType(iter->GetName()) == "boolean") {
                                            Tree* VARNAME = Tree::CreateNode(iter->GetName());
                                            ASSIGN->AddLeftTree(VARNAME);
                                        }
                                    else {
                                        cout << "Expected for expression." << endl;
                                        Errors++;
                                        return -EXIT_FAILURE;
                                    }
                                    }
                                    else {
                                        cout << "Unknown indentifire '" << iter->GetName() << "'." << endl;
                                        Errors++;
                                        return -EXIT_FAILURE;
                                    }
                                }
                            }
                            if (checkLexem(iter, bool_true_tk)) {
                                Tree* VARNAME = Tree::CreateNode(iter->GetName());
                                ASSIGN->AddLeftTree(VARNAME);
                            }
                            if (checkLexem(iter, bool_false_tk)) {
                                Tree* VARNAME = Tree::CreateNode(iter->GetName());
                                ASSIGN->AddLeftTree(VARNAME);
                            }
                        }
                        //если переменная является integer, то будут следующие правила
                        //Логика :=
                        else if (getVarType(iter->GetName()) == "integer") {

                            Tree* VARNAME = Tree::CreateNode(iter->GetName());
                            COMPOUND->AddLeftTree(VARNAME);

                            iter++;
                            if (!checkLexem(iter, assignment_tk)) {
                                cout << "Must be ':='." << endl;
                                Errors++;
                                return -EXIT_FAILURE;
                            }

                            Tree* ASSIGN = Tree::CreateNode(iter->GetName());
                            VARNAME->AddLeftTree(ASSIGN);

                            iter++;
                            if (checkLexem(iter, constant_tk)) {

                                Tree* VARNAME = Tree::CreateNode(iter->GetName());
                                ASSIGN->AddLeftTree(VARNAME);

                            }
                            else if (!checkLexem(iter, constant_tk)) {
                                if (checkLexem(iter, id_tk)) {
                                if (!isVarExist(iter->GetName())) {
                                    cout << "Unknown indentifire '" << iter->GetName() << "'." << endl;
                                    Errors++;
                                    return -EXIT_FAILURE;
                                }
                                else {
                                    if (getVarType(iter->GetName()) == "integer") {

                                        Tree* VARNAME = Tree::CreateNode(iter->GetName());
                                        ASSIGN->AddLeftTree(VARNAME);

                                    }
                                }
                            }
                                if (checkLexem(iter, bool_false_tk)) {
                                    cout << "Couldn't convert integer to boolean." << endl;
                                    Errors++;
                                    return -EXIT_FAILURE;
                                }
                                else if (checkLexem(iter, bool_true_tk)) {
                                    cout << "Couldn't convert integer to boolean." << endl;
                                    Errors++;
                                    return -EXIT_FAILURE;
                                }
                                else if (isVarExist(iter->GetName())) {
                                    if (getVarType(iter->GetName()) == "label") {
                                        cout << "Expected for var name." << endl;
                                        Errors++;
                                        return -EXIT_FAILURE;
                                    }
                                    else if (getVarType(iter->GetName()) == "boolean") {
                                        cout << "Couldn't convert integer to boolean." << endl;
                                        Errors++;
                                        return -EXIT_FAILURE;
                                    }
                                }
                                else {
                                    if (checkLexem(iter, id_tk)) {
                                        cout << "Unknown indentifier. '" << iter->GetName() << "'" << endl;
                                        Errors++;
                                        return -EXIT_FAILURE;
                                    }
                                    else {
                                        cout << "Expected for expression." << endl;
                                    }
                                }
                                
                            }
                            else {
                                //Когда сразу узнали об ошибке
                                if (checkLexem(iter, id_tk)) {
                                    cout << "Unknown indentifier. '" << iter->GetName() << "'" << endl;
                                    Errors++;
                                    return -EXIT_FAILURE;
                                }
                                else {
                                    cout << "Expected for expression." << endl;
                                    Errors++;
                                    return -EXIT_FAILURE;
                                }
                            }
                        }
                    }
                    
                    else {
                        cout << "Unknown indentifier. '" << iter->GetName() <<"'" << endl;
                        Errors++;
                        return -EXIT_FAILURE;
                    }
                    
                    iter++;
                    //если переменная является integer, то будут следующие правила;
                    //if (type_var == "boolean") {
                        //iter++;
                    //}
                }

                //ПРОВЕРКА НА GOTO
                if (checkLexem(iter, goto_tk)) {
                    iter++;
                    if (!checkLexem(iter, id_tk)) {
                        printError(MUST_BE_ID, *iter);
                        Errors++;
                        return -EXIT_FAILURE;
                    }
                    if (isVarExist(iter->GetName())) {
                        if (getVarType(iter->GetName()) == "label") {
                            if (label_setup == iter->GetName()) {
                                iter = label_adr;
                            }
                            else {
                                cout << "Label '" << iter->GetName() << "' doesn't setup " << endl;
                                Errors++;
                                return -EXIT_FAILURE;
                            }
                        }
                        else {
                            cout << "Indentifire '" << iter->GetName() << "' is not a label. " << endl;
                            Errors++;
                            return -EXIT_FAILURE;
                        }
                    }
                    
                }

                // В случае, если нет ни того, ни другого
                else if (!checkLexem(iter, end_tk)) {
                    cout << "Expected operator." << endl;
                    Errors++;
                    return -EXIT_FAILURE;
                }
                else if (!checkLexem(iter, end_tk)) {
                    iter++;
                }
            }
            iter++;
            if (!checkLexem(iter, dot_tk)) { // проверка на точку после end
                // printError(MUST_BE_OP, *iter);
                cout << "Expected '.' after 'end'." << endl;
                Errors++;
                return -EXIT_FAILURE;
            }
            iter++;
            if (!checkLexem(iter, eof_tk)) {
                cout << "Unexpected symbols after '.'." << endl;
                Errors++;
                return -EXIT_FAILURE;
            }
            Tree* END = Tree::CreateNode("END");
            COMPOUND->AddRightTree(END);
        }

        // ВАРИАНТ ПРОВЕРКИ БЕЗ PROGRAM
        else {
            while (!checkLexem(t_iter, begin_tk)) {
                // ПАРСИНГ INTEGER И BOOLEAN
                if (checkLexem(t_iter, id_tk)) {
                    //string type_var = getVarType(t)
                    t_iter++;
                    if (!checkLexem(t_iter, colon_tk)) {
                        cout << "Expected ':'." << endl;
                        Errors++;
                        return -EXIT_FAILURE;
                    }
                    t_iter++;
                    if (!checkLexem(t_iter, type_tk)) {
                        printError(MUST_BE_TYPE, *t_iter);
                        Errors++;
                        return -EXIT_FAILURE;
                    }
                    t_iter++;
                    if (!checkLexem(t_iter, semicolon_tk)) {
                        printError(MUST_BE_SEMI, *t_iter);
                        Errors++;
                        return -EXIT_FAILURE;
                    }
                }
                t_iter++;
            }
            // ОБРАБАТЫВАЕТСЯ ВСЕ, ЧТО НАХОДИТСЯ ПОД BEGIN
            while (!checkLexem(t_iter, end_tk)) {
                t_iter++;
            }
            t_iter++;
            if (!checkLexem(t_iter, dot_tk)) { // проверка на точку после end
                printError(MUST_BE_OP, *t_iter);
                Errors++;
                return -EXIT_FAILURE;
            }
            
        }
        return EXIT_SUCCESS;
    }

    Tree* Parser() {
        root = Tree::CreateNode("Program");
        lex_it it = cursor;
        while (it != lex_table.end()) {
            programParse(it, root);
            break;
        }
        //cout << "Current programParse lex: " << it->GetName() << endl;
        if (Errors == 0) {  // Исправлено, чтобы проверить на сравнение
            cout << endl;
            printIdMap(); // Выводим содержимое id_map
            cout << endl;
            cout << "Syntax tree: " << endl;
           //root->PrintTreeIndented(2);
            root->PrintASCII();
            Tree::FreeTree(root); // освобождаем память
            cout << endl;
            //cout << getVarType("c1");
            cout << "PROGRAMM WAS SUCCESSFULLY PARSEeD!" << endl;
            return root;
        }
        else if (Errors >= 1) {
            cout << "PROGRAMM WAS PARSED WITH ERRORS!" << endl;
        }

    }

    void printError(errors t_err, Lexem lex) {
        switch (t_err) {
        case UNKNOWN_LEXEM:
            cout << "SYNTAX ERROR: Get unknown lexem: " << lex.GetName()
                << " on " << lex.GetLine() << " line" << endl;
            break;
        case MUST_BE_ID:
            cout << "SYNTAX ERROR: Must be identifier instead '" << lex.GetName()
                << "' on " << lex.GetLine() << " line" << endl;
            break;
        case EOF_ERR:
            cout << "SYNTAX ERROR: End of file!" << endl;
            break;
        case MUST_BE_SEMI:
            cout << "SYNTAX ERROR: Must be ';' instead " << lex.GetName()
                << " on " << lex.GetLine() << " line" << endl;
            break;
        case MUST_BE_PROG:
            cout << "SYNTAX ERROR: Must be 'program' on " << lex.GetLine() << " line instead of '" << lex.GetName() << "'" << endl;
            break;
        case DUPL_ID_ERR:
            cout << "<E> Syntax: Duplicate identifier '" << lex.GetName()
                << "' on " << lex.GetLine() << " line" << endl;
            break;
        case UNKNOWN_ID:
            cout << "<E> Syntax: Undefined variable '" << lex.GetName()
                << "' on " << lex.GetLine() << " line" << endl;
            break;
        case MUST_BE_BEGIN:
            cout << "SYNTAX ERROR: Waited for 'begin' on " << lex.GetLine() << " line" << endl;
            break;
        case MUST_BE_OP:
            cout << "SYNTAX ERROR: Must be operator instead " << lex.GetName()
                << " on " << lex.GetLine() << " line" << endl;
            break;
        default:
            cout << "<E> Syntax: Undefined type of error" << endl;
            break;
        }
    }

    ~Syntaxx() {}
};

#endif // SINTAX_ANALIZ_H
