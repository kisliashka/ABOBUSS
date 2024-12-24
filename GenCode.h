#ifndef GENCODE_H
#define GENCODE_H

#include <iostream>
#include <fstream>
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

class GenCode {
public:
	GenCode(Tree&& t_synt_tree);

	int GenerateAsm();

	virtual ~GenCode();
private:
	Tree* synt_tree;
	std::ofstream code;
	std::ostringstream test_str;
	std::map<std::string, std::string> id_map;
	int Stack_depth = 0;
	std::vector<std::string> Last_push_type;
	int M = 0;


	const std::array<std::string, 2> types = { "integer", "boolean" };
	const std::array<std::string, 2> specif = { "array", "const" };

	static constexpr const char* DATA_SECT = ".data";
	static constexpr const char* BSS_SECT = ".bss";

	static constexpr const char* TEXT_SECT = ".text";
	static constexpr const char* GLOB_SECT = ".global main";
	static constexpr const char* MAIN_SECT = "main:";
	static constexpr const char* RET_SECT = "ret";

	static constexpr const char* BYTE_TYPE = ".byte ";
	static constexpr const char* LONG_TYPE = ".long ";
	static constexpr const char* SPAC_TYPE = ".space ";

	static constexpr const char* LONG_SIZE = "4";
	static constexpr const char* BYTE_SIZE = "1";

	void addLine(std::string&& code_line);
	void buildLine(std::string&& code_line);

	int generateDeclVars();
	int generateInitVars(Tree* var_root);
	int generateDataVar(Tree* node);

	int generateUninitVars(Tree* var_root);
	int generateBssVaar(Tree* node);

	void generateTextPart();
	void generateCompound(Tree* compound_tree);
	void generateExpressions(Tree* expr_tree);
	void generateRepeatState(Tree* state_tree);

	void generateLabel(const std::string& name, const std::string& type,
		const std::string& val);
	void generateEnd();

	std::string getType(Tree* node);
	std::string getSpec(Tree* node);
	std::string getArraySize(Tree* spec_node, std::string type);

	bool checkType(const std::string& type);
	bool checkSpec(const std::string& spec);
	void clearBuffer();

	std::string GetType(std::string Var);
};

#endif GENCODE_H
