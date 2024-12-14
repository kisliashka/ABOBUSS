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
class Syntax {
public:
	std::vector<Lexem>              lex_table;   // out table of lexemes
	std::map<std::string, Variable> id_map;      // our table of identifiers
	Tree* root_tree;
	int Errors = 0;
	int log_count = 0;
	int rep_count = 0;

	std::map<std::string, int> operations;
	using lex_it = std::vector<Lexem>::iterator; // alias of vector iterator
	lex_it                          cursor;
	
	 //��������� ������ �� � ��� ���� � ������� ������
	 Syntax(vector<Lexem> t_lex_table) {
		 if (t_lex_table.empty()) {
			 cout << "Lexemes table is empty!" << endl;
		 }
		 if (t_lex_table.at(0).GetToken() == eof_tk) {
			 cout << "Opened file is empty!" << endl;
		 }
	 }

	 //�������� �� �������� ��������� �������
	 lex_it getNextLex(lex_it& iter) {
		 if (iter != lex_table.end()) {
			 iter++;
		 }
		 return iter;
	 }

	 //����������� �� N ������ �����
	 lex_it peekLex(int N, lex_it t_iter) {
		 auto iter = t_iter;
		 while (iter != lex_table.end()) {
			 if (N == 0) {
				 return iter;
			 }
			 iter++;
			 N--;
		 }
		 return iter;
	 }

	 //����������� �� ������� ������
	 bool checkLexem(const lex_it& t_iter, const tokens& t_tok) {
		 if (t_iter == lex_table.end()) {
			 return false;
		 }
		 if (t_iter->GetToken() != t_tok) {
			 return false;
		 }
		 return true;
	 }

	 //�������� ������������� ����������
	 bool isVarExist(const string& t_var_name) {
		 auto map_iter = id_map.find(t_var_name);
		 return !(map_iter == id_map.end());
	 }

	 //�������� ��� ����������
	 string getVarType(const string& t_var_name) {
		 auto map_iter = id_map.find(t_var_name);
		 return map_iter->second.type;
	 }

	 //�������� �� type(�� ��������)
	 bool CheckVarType(lex_it& t_iter, int log_count) {
		 if (log_count > 0) {
			 if (t_iter->GetToken() == constant_tk) {
				 return false;
			 }
			 if (getVarType(t_iter->GetName()) != "boolean") {
				 return false;
			 }
		 }
		 return true;
	 }

	 // ���������� ���� � ���� � ����� ����������������
	 void updateVarTypes(const list<string>& t_var_list,
		 const string& t_type_name, int t_array_l) {
		 for (auto& el : t_var_list) {
			 id_map.at(el).type = t_type_name;
			 id_map.at(el).array_l = t_array_l;
		 }
	 }

	 //���������� ��������
	 void updateVarValue(const list<string>& t_var_list, const string& t_value) {
		 for (auto& el : t_var_list) {
			 id_map.at(el).value = t_value;
		 }
	 }

	 void SetForlter(string var_name, int T_F) {
		 id_map.find(var_name)->second.iterator = T_F;
	 }

	 //���������� ����� ���������� ����������
	 void buildVarTree(const list<string>& t_var_list, Tree* t_tree, int Length) {
		 auto i = 0;
		 for (auto& el : t_var_list) {
			 auto* tmp_tree = Tree::CreateNode(el);
			 tmp_tree->AddRightNode(id_map.at(el).type);
			 if (Length != -1) {
				 tmp_tree->AddLeftNode("array");
				 tmp_tree->GetLeftNode()->AddRightNode(to_string(Length));
			 }
			 createVarTree(t_tree, tmp_tree, i++);
		 }
	 }

	 void buildVarTree(const list<string>& t_var_list, Tree* t_tree, int Length, string Value) {
		 auto i = 0;
		 for (auto& el : t_var_list) {
			 auto* tmp_tree = Tree::CreateNode(el);
			 tmp_tree->AddRightNode(id_map.at(el).type);
			 if (Length != -1) {
				 tmp_tree->AddLeftNode("array");
				 tmp_tree->GetLeftNode()->AddRightNode(to_string(Length));
			 }
			 createVarTree(t_tree, tmp_tree, i++);
		 }
	 }

	 void createVarTree(Tree* t_tree, Tree * t_donor_tree, int lvl){
		 if (lvl > 0) {
			 lvl--;
			 createVarTree(t_tree->GetRightNode(), t_donor_tree, lvl);
		 }
		 else {
			 t_tree->AddLeftTree(t_donor_tree);
			 t_tree->AddRightNode("$");
		 }
	 }
	//������ ���� �������� ������ ���� ���������(� ���������� ���),
	// ������� ����� ����������� �� ������ ����, � ����� ������ ������

	 //����� ����� � ������� �������������� ������ � ����������
	 //param[inout] t_iter - �������� ������� �������
	 void printError(errors t_err, Lexem lex) {
		 Errors = 1;
		 switch (t_err) {
		 case UNKNOWN_LEXEM: {
			 cout << "SYNTAX ERROR: Get unknown lexem!" << lex.GetName() <<
				 " on" << lex.GetLine() << " line" << endl;
			 break;
		 }
		 case MUST_BE_ID: {
			 cout << "SYNTAX ERROR: Must be indetifier instead '" << lex.GetName() <<
				 " on" << lex.GetLine() << "line" << endl;
			 break;
		 }
		 case EOF_ERR: {
			 cout << "SYNTAX ERROR: End of file!" << endl;
			 break;
		 }
		 case MUST_BE_SEMI: {
			 cout << "SYNTAX ERROR: Must be ; instead " << lex.GetName() << " on" << lex.GetLine() << " line" << endl;
			 break;
		 }
		 case MUST_BE_PROG: {
			 cout << "SYNTAX ERROR: Must be 'program' on " << lex.GetLine() << " line" << endl;
			 break;
		 }
		 case DUPL_ID_ERR: {
			 cout << "<E> Syntax: Duplicate identifier '" << lex.GetName()
				 << "' on " << lex.GetLine() << " line" << endl;
			 break;
		 }
		 case UNKNOWN_ID: {
			 cout << "<E> Syntax: Undefined variable '" << lex.GetName()
				 << "' on " << lex.GetLine() << " line" << endl;
			 break;
		 }
		 case MUST_BE_BEGIN: {
			 cout << "SYNTAX ERROR: EOF instead 'begin' on " << lex.GetLine() << " line" << endl;
		 }
		 case MUST_BE_OP: {
			 cout << "SYNTAX ERROR: Must be operator instead " << lex.GetName() << " on" << lex.GetLine() << " line" << endl;
		 }
		 default: {
			 cout << "<E> Syntax: Undefined type of error" << endl;
			 break;
		 }
		 }
	 }

	 /*int ArrayParse(lex_it& t_iter, list<string> var_list)
	 {
		 int Leng;
		 getNextLex(t_iter);
		 if (!checkLexem(t_iter, osb_tk))
		 {
			 printError(MUST_BE_OSB, *t_iter);
			 return -EXIT_FAILURE;
		 }
		 getNextLex(t_iter);
		 if (checkLexem(t_iter, constant_tk))
		 {
			 Leng = atoi(t_iter->GetName().c_str());
		 }
		 else
		 {
			 if (checkLexem(t_iter, minus_tk))
			 {
				 if (!checkLexem(peekLex(1, t_iter), constant_tk))
				 {
					 printError(MUST_BE_CONST, *t_iter);
					 return -EXIT_FAILURE;
				 }
				 getNextLex(t_iter);
				 Leng = atoi(t_iter->GetName().c_str()) * -1;
			 }
			 else
			 {
				 printError(MUST_BE_CONST, *t_iter);
				 return -EXIT_FAILURE;
			 }
		 }
		 getNextLex(t_iter);
		 if (!checkLexem(t_iter, dot_tk))
		 {
			 printError(MUST_BE_DOT, *t_iter);
			 return -EXIT_FAILURE;
		 }
		 else
		 {
			 getNextLex(t_iter);
			 if (!checkLexem(t_iter, dot_tk))
			 {
				 printError(MUST_BE_DOT, *t_iter);
				 return -EXIT_FAILURE;
			 }
		 }
		 getNextLex(t_iter);
		 if (checkLexem(t_iter, constant_tk))
		 {
			 Leng = atoi(t_iter->GetName().c_str()) - Leng + 1;
		 }
		 else
		 {
			 if (checkLexem(t_iter, minus_tk))
			 {
				 if (!checkLexem(peekLex(1, t_iter), constant_tk))
				 {
					 printError(MUST_BE_CONST, *t_iter);
					 return -EXIT_FAILURE;
				 }
				 getNextLex(t_iter);
				 Leng = (atoi(t_iter->GetName().c_str()) * -1) - Leng + 1;
			 }
			 else
			 {
				 printError(MUST_BE_CONST, *t_iter);
				 return -EXIT_FAILURE;
			 }
		 }
		 getNextLex(t_iter);
		 if (!checkLexem(t_iter, csb_tk))
		 {
			 printError(MUST_BE_CSB, *t_iter);
			 return -EXIT_FAILURE;
		 }
		 getNextLex(t_iter);
		 if (!checkLexem(t_iter, of_tk))
		 {
			 printError(MUST_BE_OF, *t_iter);
			 return -EXIT_FAILURE;
		 }
		 getNextLex(t_iter);
		 if (!checkLexem(t_iter, type_tk))
		 {
			 printError(MUST_BE_TYPE, *t_iter);
			 return -EXIT_FAILURE;
		 }
		 auto type_iter = t_iter;
		 getNextLex(t_iter);
		 if (!checkLexem(t_iter, semicolon_tk))
		 {
			 printError(MUST_BE_SEMI, *t_iter);
		 }
		 updateVarTypes(var_list, type_iter->GetName(), Leng);
		 return Leng;
	 }

	 // ������� ����������
	 list <string> vardParse(lex_it& t_iter) {
		 auto iter = getNextLex(t_iter);
		 if (!checkLexem(iter, id_tk)) {
			 printError(MUST_BE_ID, *iter); // ����� ��� ���������������
			 return list<string>();
		 }
		 if (isVarExist(iter->GetName())) {
			 printError(DUPL_ID_ERR, *iter); // ����� ��� ��� ����
		 }
		 else {
			 id_map.emplace(iter->GetName(), Variable("?", "?", 0));
		 }
		 list<string> var_list;
		 var_list.push_back(t_iter->GetName());

		 iter = getNextLex(t_iter);
		 if (checkLexem(iter, comma_tk)) { // �������� �� ������� ":"
			 var_list.splice(var_list.end(), vardParse(t_iter));
		 }
		 return var_list;
	 }
	 //������� ����� ���������� ���������� ��������������� �������
	 int vardpParse(lex_it& t_iter, Tree* t_tree) {
		 //var_list �������� ������ ���������� �� ������� ������ ����
		 auto var_list = vardParse(t_iter);
		 if (!checkLexem(t_iter, colon_tk)) { // :
			 printError(MUST_BE_COMMA, *t_iter); // :
		 }

		 getNextLex(t_iter);
		 if (checkLexem(t_iter, type_tk)) { // integer
			 auto type_iter = t_iter;
			 {
				 getNextLex(t_iter);
				 if (checkLexem(t_iter, eqv_tk)) // =
				 {
					 getNextLex(t_iter);
					 if (checkLexem(t_iter, eqv_tk)) {
						 getNextLex(t_iter);
						 if (!checkLexem(t_iter, constant_tk)) {
							 printError(MUST_BE_CONST, *t_iter);
						 }
						 auto value = t_iter->GetName();
						 getNextLex(t_iter);
						 if (!checkLexem(t_iter, semicolon_tk)) {
							 printError(MUST_BE_SEMI, *t_iter);
							 updateVarTypes(var_list, type_iter->GetName(),0);
							 buildVarTree(var_list, t_tree, -1, value);
						 }
					 }
				 }
				 else {
					 if (checkLexem(t_iter, array_tk)) {
						 int Leng = ArrayParse(t_iter, var_list);
					 }
				 }
			 }
		 }
	 }*/

	 Tree syntaxis;

	 int programParse(lex_it& t_iter) {
		 if (!checkLexem(t_iter, program_tk)) {
			 printError(MUST_BE_PROG, *t_iter);
			 return -EXIT_FAILURE;
		 }

		 auto iter = getNextLex(t_iter);
		 if (!checkLexem(iter, id_tk)) {
			 if (iter->GetToken() == eof_tk) {
				 printError(EOF_ERR, *iter);
				 return -EXIT_FAILURE;
			 }
			 else {
				 printError(MUST_BE_ID, *iter);
				 return -EXIT_FAILURE;
			 }
		 }
		 auto root_name = iter->GetName(); // ��������� �������� ���������

		 iter = getNextLex(t_iter);
		 if (!checkLexem(iter, semicolon_tk)) { // must be ';'
			 if (iter->GetToken() == eof_tk) {
				 printError(EOF_ERR, *iter);
				 return -EXIT_FAILURE;
			 }
			 else {
				 printError(MUST_BE_SEMI, *iter); // must be ';'
				 return -EXIT_FAILURE;
			 }
		 }

		 // ������ ���� � �������, �� ����� ������ ������� ������
		 syntaxis.CreateNode(root_name); // ������ ������

		 return EXIT_SUCCESS;
	 }

	 int beginParse(lex_it& t_iter) {
		 auto iter = getNextLex(t_iter);
		 if (!checkLexem(t_iter, begin_tk)) {
			 printError(MUST_BE_BEGIN, *t_iter);
			 return -EXIT_FAILURE;
		 }
		 auto node_name = iter->GetName();
		 // ������ ���� � �������, �� ����� ������ ������� ������
		 //root_tree = Tree::CreateNode(root_name); // ������ ������
		 syntaxis.AddRightNode(node_name); // ������ ������

		 return EXIT_SUCCESS;
	 }

	 int endParse(lex_it& t_iter) {
		 auto iter = getNextLex(t_iter);
		 if (!checkLexem(t_iter, end_tk)) {
			 printError(MUST_BE_OP, *t_iter);
			 return -EXIT_FAILURE;
		 }
		 auto node_name = iter->GetName();
		 // ������ ���� � �������, �� ����� ������ ������� ������
		 //root_tree = Tree::CreateNode(root_name); // ������ ������
		 syntaxis.AddRightNode(node_name); // ������ ������

		 return EXIT_SUCCESS;
	 }

	 void Parser(vector <Lexem> lex_table) {
		 cout << "��� �������� " << lex_table.size() << " ������" << endl;
		 auto& it = cursor;
		 programParse(it);
	 }

	 Tree ParseCode()
	 {
		 cout << "��� �������� " << lex_table.size() << " ������" << endl;
		 auto& it = cursor;
		 programParse(it);
		 beginParse(it);
		 endParse(it);
		 cout << endl;
		 cout << setfill('*') << setw(50);
		 cout << "\r\n";
		 return *root_tree;
	 }

	 ~Syntax(){}
};



#endif SINTAX_ANALIZ_h