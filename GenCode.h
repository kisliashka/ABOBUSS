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

class Variable {
public:
	explicit Variable(std::string t_type, std::string t_value, int t_array_l) : type(t_type),
		value(t_value), array_l(t_array_l) {};
	std::string type;
	std::string value = "";
	int			array_l;
	int iterator = 0;
};

class Tree {
public:
	Tree() {
		left = nullptr;
		right = nullptr;
		parent = nullptr;
		value = "";
		alloc = true;
	}

	explicit Tree(const std::string& val) {
		left = nullptr;
		right = nullptr;
		parent = nullptr;
		value = val;
		alloc = true;
	}


	void AddLeftNode(const std::string& val) {
		this->left = CreateNode(this, val);
	}

	void AddRightNode(const std::string& val) {
		this->right = CreateNode(this, val);
	}


	Tree* GetLeftNode() {
		return this->left;
	}


	void AddLeftTree(Tree* tree) {
		tree->parent = this;
		this->left = tree;
	}


	void AddRightTree(Tree* tree) {
		tree->parent = this;
		this->right = tree;
	}


	void ChangeValue(const std::string& val) {
		value = val;
	}


	Tree* GetRightNode() {
		return this->right;
	}


	Tree* GetParentNode() {
		return this->parent;
	}


	std::string GetValue() {
		return this->value;
	}

	void PrintTree()
	{
		if (this == nullptr)
		{
			std::cout << ".";
			return;
		}
		std::cout << this->value;
		this->left->PrintTree();
		this->right->PrintTree();
	}

	/**
	 * @brief Create like a root node of syntax tree
	 * @param[in] node_name - name of the node (value)
	 *
	 * @return node of tree
	 */
	static Tree* CreateNode(const std::string& val) {
		auto* node = new Tree(val);
		return node;
	}


	/**
	 * @brief Create node of syntax tree
	 * @param[in] t_tree    - parent node
	 * @param[in] node_name - name of the created node (value)
	 *
	 * @return node of tree
	 */
	static Tree* CreateNode(Tree* parent_tree, const std::string& val) {
		auto* node = new Tree(val);
		node->parent = std::addressof(*parent_tree);
		return node;
	}



	void FreeLeftNode() {
		FreeTree(this->left);
	}


	void FreeRightNode() {
		FreeTree(this->right);
	}


	static void FreeTree(Tree*& t_tree) {
		try {
			if (t_tree->left != nullptr) FreeTree(t_tree->left);
			if (t_tree->right != nullptr) FreeTree(t_tree->right);
			delete t_tree;
			t_tree = nullptr;
		}
		catch (const std::exception& exp) {
			std::cerr << "<E> Tree: Catch exception in " << __func__ << ": "
				<< exp.what() << std::endl;
		}
	}


	virtual ~Tree() {
		FreeTree();
	} // = default;
private:
	Tree* left;
	Tree* right;
	Tree* parent;
	std::string value;
	bool alloc{ false };

	void FreeTree() {
		if (this->left != nullptr) this->left->FreeTree();
		if (this->right != nullptr) this->right->FreeTree();
		parent = nullptr;
		value = "";
		alloc = false;
	}
};


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
