#ifndef SINTAX_ANALIZ_H
#define SINTAX_ANALIZ_H
#include <fstream>
#include <iostream>
//#include <list>
#include <map>
#include <string>
//#include "MyVector.h"

#include "MyMap.h"
#include "Tree.h"
#include "Lexem.h"
#include "variable.h"
#include "MyList.h"

using namespace std;

class Syntaxx {
public:

    mylib::Vector<Lexem> lex_table; // out table of lexemes
    std::map<std::string, Variable> id_map; // our table of identifiers
    Tree* root;
    Tree* root_gen;
    int Errors = 0;
    int log_count = 0;
    int rep_count = 0;
    bool var_section_started = false;
    std::string label_setup;

    using lex_it = mylib::Vector<Lexem>::Iterator; // alias of vector iterator
    lex_it cursor;
    lex_it label_adr;

    // Конструктор принимает Vector по rvalue-ссылке и перемещает его
    Syntaxx(mylib::Vector<Lexem>&& t_lex_table)
        : lex_table(std::move(t_lex_table)), cursor(lex_table.begin())
    {
        if (lex_table.empty()) {
            std::cout << "Lexemes table is empty!" << std::endl;
        }
        else if (lex_table.at(0).GetToken() == eof_tk) {
            std::cout << "Opened file is empty!" << std::endl;
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
            parent->AddLeftTree(NAMEPROG);
            iter++;

            // Создание узла BLOCK и добавление его как левый сын Program
            Tree* BLOCK = Tree::CreateNode("BLOCK");
            parent->AddRightTree(BLOCK);

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
                    MyList<string> var_list;
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

                    MyList<string> label_list;

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
                            //повторное использование метки
                            if (label_setup == iter->GetName()) {
                                cout << "Repeated label '" << iter->GetName() << "' initialization." << endl;
                                Errors++;
                                return -EXIT_FAILURE;
                            }
                            label_setup = iter->GetName();

                            //РАБОТА С ДЕРЕВОМ
                            Tree* currentLabel = Tree::CreateNode(iter->GetName());

                            if (COMPOUND->GetLeftNode() == nullptr) {
                                COMPOUND->AddLeftTree(currentLabel);
                            }
                            else {
                                Tree* sibling = COMPOUND->GetLeftNode();
                                while (sibling->GetRightNode() != nullptr) {
                                    sibling = sibling->GetRightNode();
                                }
                                sibling->AddRightTree(currentLabel);
                            }

                            //обработка ошибки
                            iter++;
                            if (!checkLexem(iter, colon_tk)) {
                                cout << "Must be ':' after label. " << endl;
                                Errors++;
                                return -EXIT_FAILURE;
                            }
                        }
                        //ЕСЛИ ПЕРЕМЕННАЯ BOOLEAN, то будут следующие правила
                        else if (getVarType(iter->GetName()) == "boolean") {

                            //Добавление в дерево имени переменной
                            Tree* currentBool = Tree::CreateNode(iter->GetName());

                            if (COMPOUND->GetLeftNode() == nullptr) {
                                COMPOUND->AddLeftTree(currentBool);
                            }
                            else {
                                Tree* sibling = COMPOUND->GetLeftNode();
                                while (sibling->GetRightNode() != nullptr) {
                                    sibling = sibling->GetRightNode();
                                }
                                sibling->AddRightTree(currentBool);
                            }

                            //проверяем на ошибку
                            iter++;
                            if (!checkLexem(iter, assignment_tk)) {
                                cout << "Must be ':='." << endl;
                                Errors++;
                                return -EXIT_FAILURE;
                            }

                            //Добавление в дерево :=
                            Tree* ASSIGN = Tree::CreateNode(iter->GetName());
                            currentBool->AddLeftTree(ASSIGN);

                            iter++;
                            if (!checkLexem(iter, bool_true_tk)) {
                                if (!checkLexem(iter, bool_false_tk)) {
                                    if (checkLexem(iter, constant_tk)) {
                                        cout << "Couldn't convert boolean to integer." << endl;
                                        Errors++;
                                        return -EXIT_FAILURE;
                                    }

                                    if (isVarExist(iter->GetName())) {
                                        if (getVarType(iter->GetName()) == "integer") {
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

                                                //ПРОВЕРЯЕМ ЕСТЬ ЛИ ОПЕРАТОРЫ СРАВНЕНИЯ
                                                iter++;
                                                // <
                                                if (checkLexem(iter, bool_less_tk)) {
                                                    //Если все успешно, то записываем в дерево
                                                    Tree* LESS = Tree::CreateNode(iter->GetName());
                                                    VARNAME->AddLeftTree(LESS);

                                                    iter++;
                                                    if (checkLexem(iter, id_tk)) {
                                                        if (isVarExist(iter->GetName())) {
                                                            if (getVarType(iter->GetName()) == "integer") {
                                                                cout << "Operation '<' can't be used to boolean and integer types." << endl;
                                                                Errors++;
                                                                return -EXIT_FAILURE;
                                                            }
                                                            else if (getVarType(iter->GetName()) == "label") {
                                                                cout << "Expected for variable name." << endl;
                                                                Errors++;
                                                                return -EXIT_FAILURE;
                                                            }

                                                            //Если все успешно, то записываем в дерево
                                                            Tree* VARNAME = Tree::CreateNode(iter->GetName());
                                                            LESS->AddLeftTree(VARNAME);

                                                        }
                                                        else {
                                                            cout << "Unknown indentifire '" << iter->GetName() << "' ." << endl;
                                                            Errors++;
                                                            return -EXIT_FAILURE;
                                                        }
                                                    }
                                                    else {
                                                        cout << "Unknown indentifire '" << iter->GetName() << "' ." << endl;
                                                        Errors++;
                                                        return -EXIT_FAILURE;
                                                    }
                                                }
                                                // >
                                                else if (checkLexem(iter, bool_bigger_tk)) {
                                                    //Если все успешно,то записываем в дерево
                                                    Tree* BIGGER = Tree::CreateNode(iter->GetName());
                                                    VARNAME->AddLeftTree(BIGGER);

                                                    iter++;
                                                    if (checkLexem(iter, id_tk)) {
                                                        if (isVarExist(iter->GetName())) {
                                                            if (getVarType(iter->GetName()) == "integer") {
                                                                cout << "Operation '>' can't be used to boolean and integer types." << endl;
                                                                Errors++;
                                                                return -EXIT_FAILURE;
                                                            }
                                                            else if (getVarType(iter->GetName()) == "label") {
                                                                cout << "Expected for variable name." << endl;
                                                                Errors++;
                                                                return -EXIT_FAILURE;
                                                            }
                                                            //Если все успешно, то записываем в дерево
                                                            Tree* VARNAME = Tree::CreateNode(iter->GetName());
                                                            BIGGER->AddLeftTree(VARNAME);
                                                        }
                                                        else {
                                                            cout << "Unknown indentifire '" << iter->GetName() << "' ." << endl;
                                                            Errors++;
                                                            return -EXIT_FAILURE;
                                                        }
                                                    }
                                                    else {
                                                        cout << "Unknown indentifire '" << iter->GetName() << "' ." << endl;
                                                        Errors++;
                                                        return -EXIT_FAILURE;
                                                    }
                                                }
                                                // =
                                                else if (checkLexem(iter, equals_tk)) {
                                                    //Если все успешно,то записываем в дерево
                                                    Tree* EQUALS = Tree::CreateNode(iter->GetName());
                                                    VARNAME->AddLeftTree(EQUALS);

                                                    iter++;
                                                    if (checkLexem(iter, id_tk)) {
                                                        if (isVarExist(iter->GetName())) {
                                                            if (getVarType(iter->GetName()) == "integer") {
                                                                cout << "Operation '=' can't be used to boolean and integer types." << endl;
                                                                Errors++;
                                                                return -EXIT_FAILURE;
                                                            }
                                                            else if (getVarType(iter->GetName()) == "label") {
                                                                cout << "Expected for variable name." << endl;
                                                                Errors++;
                                                                return -EXIT_FAILURE;
                                                            }
                                                            //Если все успешно, то записываем в дерево
                                                            Tree* VARNAME = Tree::CreateNode(iter->GetName());
                                                            EQUALS->AddLeftTree(VARNAME);
                                                        }
                                                        else {
                                                            cout << "Unknown indentifire '" << iter->GetName() << "' ." << endl;
                                                            Errors++;
                                                            return -EXIT_FAILURE;
                                                        }
                                                    }
                                                    else {
                                                        cout << "Unknown indentifire '" << iter->GetName() << "' ." << endl;
                                                        Errors++;
                                                        return -EXIT_FAILURE;
                                                    }
                                                }
                                                // <=
                                                else if (checkLexem(iter, bool_leseqv_tk)) {
                                                    //Если все успешно,то записываем в дерево
                                                    Tree* LESEQV = Tree::CreateNode(iter->GetName());
                                                    VARNAME->AddLeftTree(LESEQV);

                                                    iter++;
                                                    if (checkLexem(iter, id_tk)) {
                                                        if (isVarExist(iter->GetName())) {
                                                            if (getVarType(iter->GetName()) == "integer") {
                                                                cout << "Operation '<=' can't be used to boolean and integer types." << endl;
                                                                Errors++;
                                                                return -EXIT_FAILURE;
                                                            }
                                                            else if (getVarType(iter->GetName()) == "label") {
                                                                cout << "Expected for variable name." << endl;
                                                                Errors++;
                                                                return -EXIT_FAILURE;
                                                            }
                                                            //Если все успешно, то записываем в дерево
                                                            Tree* VARNAME = Tree::CreateNode(iter->GetName());
                                                            LESEQV->AddLeftTree(VARNAME);
                                                        }
                                                        else {
                                                            cout << "Unknown indentifire '" << iter->GetName() << "' ." << endl;
                                                            Errors++;
                                                            return -EXIT_FAILURE;
                                                        }
                                                    }
                                                    else {
                                                        cout << "Unknown indentifire '" << iter->GetName() << "' ." << endl;
                                                        Errors++;
                                                        return -EXIT_FAILURE;
                                                    }
                                                }
                                                // >=
                                                else if (checkLexem(iter, bool_bigeqv_tk)) {
                                                    //Если все успешно,то записываем в дерево
                                                    Tree* BIGEQV = Tree::CreateNode(iter->GetName());
                                                    VARNAME->AddLeftTree(BIGEQV);

                                                    iter++;
                                                    if (checkLexem(iter, id_tk)) {
                                                        if (isVarExist(iter->GetName())) {
                                                            if (getVarType(iter->GetName()) == "integer") {
                                                                cout << "Operation '>=' can't be used to boolean and integer types." << endl;
                                                                Errors++;
                                                                return -EXIT_FAILURE;
                                                            }
                                                            else if (getVarType(iter->GetName()) == "label") {
                                                                cout << "Expected for variable name." << endl;
                                                                Errors++;
                                                                return -EXIT_FAILURE;
                                                            }
                                                            //Если все успешно, то записываем в дерево
                                                            Tree* VARNAME = Tree::CreateNode(iter->GetName());
                                                            BIGEQV->AddLeftTree(VARNAME);
                                                        }
                                                        else {
                                                            cout << "Unknown indentifire '" << iter->GetName() << "' ." << endl;
                                                            Errors++;
                                                            return -EXIT_FAILURE;
                                                        }
                                                    }
                                                    else {
                                                        cout << "Unknown indentifire '" << iter->GetName() << "' ." << endl;
                                                        Errors++;
                                                        return -EXIT_FAILURE;
                                                    }
                                                }
                                                // <>
                                                else if (checkLexem(iter, bool_noneqv_tk)) {
                                                    //Если все успешно,то записываем в дерево
                                                    Tree* NONEQV = Tree::CreateNode(iter->GetName());
                                                    VARNAME->AddLeftTree(NONEQV);

                                                    iter++;
                                                    if (checkLexem(iter, id_tk)) {
                                                        if (isVarExist(iter->GetName())) {
                                                            if (getVarType(iter->GetName()) == "integer") {
                                                                cout << "Operation '<>' can't be used to boolean and integer types." << endl;
                                                                Errors++;
                                                                return -EXIT_FAILURE;
                                                            }
                                                            else if (getVarType(iter->GetName()) == "label") {
                                                                cout << "Expected for variable name." << endl;
                                                                Errors++;
                                                                return -EXIT_FAILURE;
                                                            }
                                                            //Если все успешно, то записываем в дерево
                                                            Tree* VARNAME = Tree::CreateNode(iter->GetName());
                                                            NONEQV->AddLeftTree(VARNAME);
                                                        }
                                                        else {
                                                            cout << "Unknown indentifire '" << iter->GetName() << "' ." << endl;
                                                            Errors++;
                                                            return -EXIT_FAILURE;
                                                        }
                                                    }
                                                    else {
                                                        cout << "Unknown indentifire '" << iter->GetName() << "' ." << endl;
                                                        Errors++;
                                                        return -EXIT_FAILURE;
                                                    }
                                                    //iter--;
                                                }
                                            }
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
                            iter--;
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
                            //COMPOUND->AddLeftTree(VARNAME);
                            if (COMPOUND->GetLeftNode() == nullptr) {
                                COMPOUND->AddLeftTree(VARNAME);
                            }
                            else {
                                Tree* sibling = COMPOUND->GetLeftNode();
                                while (sibling->GetRightNode() != nullptr) {
                                    sibling = sibling->GetRightNode();
                                }
                                sibling->AddRightTree(VARNAME);
                            }

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

                                iter++;
                                //СМОТРИМ ЕСТЬ ЛИ ВПЕРЕДИ АРИФМЕТИЧЕСКИЕ ОПЕРАЦИИ

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
                                    cout << "Unknown indentifier '" << iter->GetName() << "' ." << endl;
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
                        cout << "Unknown indentifier. '" << iter->GetName() << "'" << endl;
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
                else if (checkLexem(iter, goto_tk)) {

                    //РАБОТА С ДЕРЕВОМ
                    Tree* GoTo = Tree::CreateNode(iter->GetName());

                    if (COMPOUND->GetLeftNode() == nullptr) {
                        COMPOUND->AddLeftTree(GoTo);
                    }
                    else {
                        Tree* sibling = COMPOUND->GetLeftNode();
                        while (sibling->GetRightNode() != nullptr) {
                            sibling = sibling->GetRightNode();
                        }
                        sibling->AddRightTree(GoTo);
                    }

                    //ищем следующую лексему
                    iter++;

                    if (!checkLexem(iter, id_tk)) {
                        printError(MUST_BE_ID, *iter);
                        Errors++;
                        return -EXIT_FAILURE;
                    }
                    if (isVarExist(iter->GetName())) {
                        if (getVarType(iter->GetName()) == "label") {
                            if (label_setup == iter->GetName()) {
                                lex_it tek_adr = label_adr;
                                //Записываем метку под goto
                                Tree* GoLabel = Tree::CreateNode(iter->GetName());
                                GoTo->AddLeftTree(GoLabel);
                                //ищем следующую лексему
                                iter++;
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
        if (Errors == 0) {
            cout << endl;
            printIdMap();
            cout << endl;
            cout << "Syntax tree: " << endl;
            root->PrintASCII();
            cout << endl;
            cout << "PROGRAMM WAS SUCCESSFULLY PARSEeD!" << endl;
            return root; // Уникальный указатель все ещё владеет Tree
        }
        else if (Errors >= 1) {
            cout << "PROGRAMM WAS PARSED WITH ERRORS!" << endl;
            return nullptr;
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

    ~Syntaxx() {
        
    }
};

#endif // SINTAX_ANALIZ_H