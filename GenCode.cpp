#ifndef GEN_CODE_CPP
#define GEN_CODE_CPP
#include <iostream>
#include "GenCode.h"
GenCode::GenCode(Tree&& t_synt_tree) {
	try {
		synt_tree = &t_synt_tree;
		code.open(synt_tree->GetValue() + ".txt", std::ios::out | std::ios::trunc);
		if (!code.is_open())
			throw;
	}
	catch (const std::exception& exp) {
		std::string what;
		std::string lel(exp.what());
		what = "<E> GenCode: Catch exception in constructor: ";

		throw std::runtime_error(what + lel);
	}
}


GenCode::~GenCode() {
	code.close();
	clearBuffer();
	Tree::FreeTree(synt_tree);
}


/**
 * @brief Generate GAS code by syntax tree of program
 * @param none
 *
 * @return  EXIT_SUCCESS - if code was generated successful
 * @return -EXIT_FAILURE - if can't generate code
 */
int GenCode::GenerateAsm() {
	try {
		if (synt_tree->GetLeftNode() == nullptr &&
			synt_tree->GetRightNode() == nullptr) {
			std::cerr << "<E> GenCode: Empty tree" << std::endl;
			return -EXIT_FAILURE;
		}

		if (synt_tree->GetLeftNode() != nullptr)
			generateDeclVars();
		if (synt_tree->GetRightNode() != nullptr) {
			generateTextPart();
		}

		generateEnd();
		return EXIT_SUCCESS;
	}
	catch (const std::exception& exp) {
		std::cerr << "<E> GenCode: Catch exception in " << __func__ << ": "
			<< exp.what();
		return -EXIT_FAILURE;
	}
}


/**
 * @brief Add GAS construction in the file with assembler code
 * @param[in] code_line - line of GAS code
 *
 * @return none
 */
void GenCode::addLine(std::string&& code_line) {
	code << code_line << std::endl;
}


/**
 * @brief Add GAS construction in the stream buffer
 * @param[in] code_line - line of GAS code
 *
 * @return none
 * @note Then use this buffer for writing in the file with assembler code
 */
void GenCode::buildLine(std::string&& code_line) {
	test_str << code_line << std::endl;
}


/**
 * @brief Generate variables declaration
 * @param none
 *
 * @return  EXIT_SUCCESS - generate was successful
 * @return -EXIT_FAILURE - can't generate GAS code
 */
int GenCode::generateDeclVars() {
	if (synt_tree->GetLeftNode()->GetValue() != "var") {
		std::cerr << "<E> GenCode: Can't find declaration of variables" << std::endl;
		return -EXIT_FAILURE;
	}

	generateInitVars(synt_tree->GetLeftNode());
	if (!test_str.str().empty()) { // if we have any initialized variables
		addLine(DATA_SECT);
		addLine(test_str.str());
		clearBuffer();
	}

	generateUninitVars(synt_tree->GetLeftNode());
	if (!test_str.str().empty()) { // if we have any uninitialized variables
		addLine(BSS_SECT);
		addLine(test_str.str());
		clearBuffer();
	}

	return EXIT_SUCCESS;
}


/**
 * @brief Generate GAS code for initialized variables
 * @param[inout] var_root - pointer to the root of subtree of variables
 *
 * @return  EXIT_SUCCESS - section of .data generated successful
 * @return -EXIT_FAILURE - can't generate .data section, can't find variables
 * @note Work for code like:
 *   var a : integer = 5;
 *       b : integer = 1;
 *       c : boolean = false;
 */
int GenCode::generateInitVars(Tree* var_root) {
	if (var_root->GetLeftNode() == nullptr) {
		std::cerr << "<E> GenCode: Can't find any variables" << std::endl;
		return -EXIT_FAILURE;
	}

	generateDataVar(var_root->GetLeftNode());
	if (var_root->GetRightNode() != nullptr)
		generateInitVars(var_root->GetRightNode());

	return EXIT_SUCCESS;
}


/**
 * @brief Parse tree and generate lines only for initialized variables
 * @param[in] node - node of syntax tree with variable
 *
 * @return  EXIT_SUCCESS - if node doesn't contain error
 * @return -EXIT_FAILURE - variable doesn't have a type
 * @note Method skips any other (uninit or specific) variables
 */
int GenCode::generateDataVar(Tree* node) {
	if (node->GetRightNode() == nullptr) {
		std::cerr << "<E> GenCode: Variable doesn't have a type node" << std::endl;
		return -EXIT_FAILURE;
	}

	if (!checkType(getType(node))) // get label variable
		return EXIT_SUCCESS;

	if (checkSpec(getSpec(node))) { // get specific variable
		if (getSpec(node) == specif.at(0)) {
			// XXX: if you implement array initialization in tree, add here
			//   generation of GAS code for array

			return EXIT_SUCCESS; // because array can't be initialized
		}
		else { // const
			return EXIT_SUCCESS;
		}
	}

	if (node->GetLeftNode() != nullptr) {
		auto val = node->GetLeftNode()->GetValue();
		if (getType(node) == types.at(1)) { // if type is boolean
			if (val == "false") val = "0";
			else                val = "1";
		}

		std::string type = (getType(node) == types.at(0)) ? LONG_TYPE : BYTE_TYPE;
		id_map.emplace(node->GetValue(), type);
		generateLabel(node->GetValue(), type, val);
	}

	return EXIT_SUCCESS;
}


/**
 * @brief Generate GAS code for uninitialized variables
 * @param[in] var_root - pointer to the root of subtree of variables
 *
 * @return  EXIT_SUCCESS - section of .bss generated successful
 * @return -EXIT_FAILURE - can't generate .bss section
 * @note Work for code like:
 *   var a : integer;
 *       c : boolean;
 */
int GenCode::generateUninitVars(Tree* var_root) {
	if (var_root->GetLeftNode() == nullptr) {
		std::cerr << "<E> GenCode: Can't find any variables" << std::endl;
		return -EXIT_FAILURE;
	}

	generateBssVaar(var_root->GetLeftNode());
	if (var_root->GetRightNode() != nullptr)
		generateUninitVars(var_root->GetRightNode());

	return EXIT_SUCCESS;
}


/**
 * @brief Parse tree and generate lines only for uninitialized variables
 * @param[in] node - node of syntax tree with variable
 *
 * @return  EXIT_SUCCESS - if node doesn't contain error
 * @return -EXIT_FAILURE - variable doesn't have a type
 * @note Method skips any other (init or specific) variables
 */
int GenCode::generateBssVaar(Tree* node) {
	if (node->GetRightNode() == nullptr) {
		std::cerr << "<E> GenCode: Variable doesn't have a type node" << std::endl;
		return -EXIT_FAILURE;
	}

	if (!checkType(getType(node)))
		return EXIT_SUCCESS;

	if (node->GetLeftNode() != nullptr && !checkSpec(getSpec(node)))
		return EXIT_SUCCESS;

	std::string val;
	if (getSpec(node) == specif.at(0))
		val = getArraySize(node->GetLeftNode(), getType(node));
	else
		val = (getType(node) == types.at(0)) ? LONG_SIZE : BYTE_SIZE;

	std::string type = SPAC_TYPE;
	id_map.emplace(node->GetValue(), (getType(node) == types.at(0)) ? LONG_TYPE : BYTE_TYPE);
	generateLabel(node->GetValue(), type, val);

	return EXIT_SUCCESS;
}


/**
 * @brief Generate GAS code for code section (right node from root)
 * @param none
 *
 * @return none
 */

void GenCode::generateTextPart() {
	addLine(TEXT_SECT);
	addLine(GLOB_SECT);
	addLine(MAIN_SECT);
	addLine("pushl %ebp;");
	addLine("movl %esp, %ebp;");
	auto Save_tree = synt_tree->GetRightNode()->GetRightNode();
	while (Save_tree->GetValue() != "end.")
	{
		generateCompound(Save_tree);
		Save_tree = Save_tree->GetRightNode();
	}
	return;
}


/**
 * @brief Generate GAS code for 'begin/end' operator
 * @param none
 *
 * @return none
 */
void GenCode::generateCompound(Tree* compound_tree) {
	auto Op_Tree = compound_tree;
	if (Op_Tree->GetValue() == "end.") {
		return;
	}
	if (Op_Tree->GetValue() == "end") {
		return;
	}
	addLine(Op_Tree->GetValue() + ":");
	if (Op_Tree->GetLeftNode()->GetValue() == "begin")
	{
		auto Save_tree = Op_Tree->GetLeftNode()->GetRightNode();
		while (Save_tree->GetValue() != "end")
		{
			generateCompound(Save_tree);
			Save_tree = Save_tree->GetRightNode();
		}
	}
	else
		if (Op_Tree->GetLeftNode()->GetValue() == "repeat")
		{
			generateRepeatState(Op_Tree);
			return;
		}
		else
			if (Op_Tree->GetLeftNode()->GetValue() == ":=")
			{
				generateExpressions(Op_Tree->GetLeftNode()->GetRightNode());
				if (Op_Tree->GetLeftNode()->GetLeftNode()->GetValue() == "array")
				{
					if (std::isdigit(Op_Tree->GetLeftNode()->GetLeftNode()->GetRightNode()->GetValue().at(0)))
					{
						auto Ind = std::atoi(Op_Tree->GetLeftNode()->GetLeftNode()->GetRightNode()->GetValue().c_str()) * 4;
						addLine("popl " + Op_Tree->GetLeftNode()->GetLeftNode()->GetLeftNode()->GetValue() + "+" + std::to_string(Ind) + ";");
						Last_push_type.pop_back();
						Stack_depth -= 4;
					}
					else
					{
						generateExpressions(Op_Tree->GetLeftNode()->GetLeftNode()->GetRightNode());
						addLine("popl %eax;");
						Last_push_type.pop_back();
						Stack_depth -= 4;
						addLine("movl $4,%ebx;");
						addLine("mull %ebx;");
						addLine("add " + Op_Tree->GetLeftNode()->GetLeftNode()->GetLeftNode()->GetValue() + ",%eax;");
						addLine("popl (%eax);");
						Last_push_type.pop_back();
						Stack_depth -= 4;
					}
				}
				else
				{
					addLine("popl " + Op_Tree->GetLeftNode()->GetLeftNode()->GetValue() + ";");
					Last_push_type.pop_back();
					Stack_depth -= 4;
				}
			}
			else
			{
				std::cerr << "<E> GenCode: Can't find avaliable operator" << std::endl;
				return;
			}
	addLine(Op_Tree->GetValue() + "end:");
}

void GenCode::generateRepeatState(Tree* state_tree)
{
	auto Save_tree = state_tree->GetLeftNode()->GetRightNode();
	while (Save_tree->GetValue() != "end")
	{
		generateCompound(Save_tree);
		Save_tree = Save_tree->GetRightNode();
	}
	generateExpressions(state_tree->GetLeftNode()->GetLeftNode());
	addLine("pop %ax;");
	Last_push_type.pop_back();
	Stack_depth -= 2;
	addLine("cmp %ax,$1;");
	addLine("jne " + state_tree->GetValue());
}

void GenCode::generateExpressions(Tree* expr_tree)
{
	if (expr_tree == nullptr)
		return;
	generateExpressions(expr_tree->GetLeftNode());
	generateExpressions(expr_tree->GetRightNode());
	auto Node_value = expr_tree->GetValue();
	if (std::isdigit(Node_value.at(0)))
	{
		addLine("pushl $" + Node_value + ";");
		Last_push_type.emplace_back(".long");
		Stack_depth += 4;
	}
	if (std::isalpha(Node_value.at(0)))
	{
		if (Node_value == "div")
		{
			addLine("popl %ebx;");
			Last_push_type.pop_back();
			Stack_depth -= 4;
			addLine("popl %eax;");
			Last_push_type.pop_back();
			Stack_depth -= 4;
			addLine("xor %edx,%edx;");
			addLine("divl %ebx;");
			addLine("pushl %eax;");
			Last_push_type.emplace_back(".long");
			Stack_depth += 4;
			return;
		}
		else
			if (Node_value == "or")
			{
				if (Last_push_type.back() == ".long")
				{
					addLine("popl %eax;");
					Last_push_type.pop_back();
					Stack_depth -= 4;
					addLine("popl %ebx;");
					Last_push_type.pop_back();
					Stack_depth -= 4;
					addLine("or %ebx,%eax;");
					addLine("pushl %eax;");
					Last_push_type.emplace_back(".long");
					Stack_depth += 4;
				}
				else
				{
					Last_push_type.pop_back();
					addLine("pop %ax;");
					Stack_depth -= 2;
					Last_push_type.pop_back();
					addLine("pop %bx;");
					Stack_depth -= 2;
					addLine("or %ax,%bx;");
					addLine("push %ax;");
					Last_push_type.emplace_back(".byte");
					Stack_depth += 2;
				}
				return;
			}
			else
				if (Node_value == "and")
				{
					if (Last_push_type.back() == ".long")
					{
						Last_push_type.pop_back();
						addLine("popl %eax;");
						Stack_depth -= 4;
						Last_push_type.pop_back();
						addLine("popl %ebx;");
						Stack_depth -= 4;
						addLine("and %ebx,%eax;");
						addLine("pushl %eax;");
						Last_push_type.emplace_back(".long");
						Stack_depth += 4;
					}
					else
					{
						Last_push_type.pop_back();
						addLine("pop %ax;");
						Stack_depth -= 2;
						Last_push_type.pop_back();
						addLine("pop %bx;");
						Stack_depth -= 2;
						addLine("and %ax,%bx;");
						addLine("push %ax;");
						Last_push_type.emplace_back(".byte");
						Stack_depth += 2;
					}
					return;
				}
				else
					if (Node_value == "xor")
					{
						if (Last_push_type.back() == ".long")
						{
							Last_push_type.pop_back();
							addLine("popl %eax;");
							Stack_depth -= 4;
							Last_push_type.pop_back();
							addLine("popl %ebx;");
							Stack_depth -= 4;
							addLine("xor %ebx,%eax;");
							addLine("pushl %eax;");
							Last_push_type.emplace_back(".long");
							Stack_depth += 4;
						}
						else
						{
							Last_push_type.pop_back();
							addLine("pop %ax;");
							Stack_depth -= 2;
							Last_push_type.pop_back();
							addLine("pop %bx;");
							Stack_depth -= 2;
							addLine("xor %ax,%bx;");
							addLine("push %ax;");
							Last_push_type.emplace_back(".byte");
							Stack_depth += 2;
						}
						return;
					}
					else
						if (Node_value == "array")
						{
							if (std::isdigit(expr_tree->GetRightNode()->GetValue().at(0)))
							{
								auto Ind = std::atoi(expr_tree->GetRightNode()->GetValue().c_str()) * 4;
								addLine("pushl " + expr_tree->GetLeftNode()->GetValue() + "+" + std::to_string(Ind) + ";");
								Last_push_type.emplace_back(".long");
								Stack_depth += 4;
							}
							else
							{
								generateExpressions(expr_tree->GetRightNode());
								addLine("popl %eax;");
								Last_push_type.pop_back();
								Stack_depth -= 4;
								addLine("movl $4,%ebx;");
								addLine("mull %ebx;");
								addLine("add " + expr_tree->GetLeftNode()->GetValue() + ",%eax;");
								addLine("pushl (%eax);");
								Last_push_type.emplace_back(".long");
								Stack_depth += 4;
							}
							return;
						}
						else
							if (GetType(Node_value) == ".long ")
							{
								addLine("pushl " + Node_value + ";");
								Last_push_type.emplace_back(".long");
								Stack_depth += 4;
								return;
							}
							else
							{
								addLine("push " + Node_value + ";");
								Last_push_type.emplace_back(".byte");
								Stack_depth += 2;
								return;
							}
	}
	if (Node_value == "+")
	{
		addLine("popl %eax;");
		Last_push_type.pop_back();
		Stack_depth -= 4;
		addLine("addl %eax,-" + std::to_string(Stack_depth) + "(%ebp);");
		return;
	}
	if (Node_value == "-")
	{
		addLine("popl %eax;");
		Last_push_type.pop_back();
		Stack_depth -= 4;
		addLine("subl %eax,-" + std::to_string(Stack_depth) + "(%ebp);");
		return;
	}
	if (Node_value == "*")
	{
		addLine("popl %ebx;");
		Last_push_type.pop_back();
		Stack_depth -= 4;
		addLine("popl %eax;");
		Last_push_type.pop_back();
		Stack_depth -= 4;
		addLine("mull %ebx;");
		addLine("pushl %eax;");
		Last_push_type.emplace_back(".long");
		Stack_depth += 4;
		return;
	}
	if (Node_value == ">")
	{
		addLine("popl %eax;");
		Last_push_type.pop_back();
		Stack_depth -= 4;
		addLine("popl %ebx;");
		Last_push_type.pop_back();
		Stack_depth -= 4;
		addLine("cmpl %eax,%ebx;");
		addLine("jle M" + std::to_string(M) + ";");
		addLine("push $1;");
		M++;
		addLine("jmp M" + std::to_string(M) + ";");
		M--;
		addLine("M" + std::to_string(M) + ": push $0;");
		M++;
		Last_push_type.emplace_back(".byte");
		Stack_depth += 2;
		addLine("M" + std::to_string(M) + ":");
		M++;
		return;
	}
	if (Node_value == "<")
	{
		addLine("popl %eax;");
		Last_push_type.pop_back();
		Stack_depth -= 4;
		addLine("popl %ebx;");
		Last_push_type.pop_back();
		Stack_depth -= 4;
		addLine("cmpl %eax,%ebx;");
		addLine("jge M" + std::to_string(M) + ";");
		addLine("push $1;");
		M++;
		addLine("jmp M" + std::to_string(M) + ";");
		M--;
		addLine("M" + std::to_string(M) + ": push $0;");
		M++;
		Last_push_type.emplace_back(".byte");
		Stack_depth += 2;
		addLine("M" + std::to_string(M) + ":");
		M++;
		return;
	}
	if (Node_value == "=")
	{
		addLine("popl %eax;");
		Last_push_type.pop_back();
		Stack_depth -= 4;
		addLine("popl %ebx;");
		Last_push_type.pop_back();
		Stack_depth -= 4;
		addLine("cmpl %eax,%ebx;");
		addLine("jne M" + std::to_string(M) + ";");
		addLine("push $1;");
		M++;
		addLine("jmp M" + std::to_string(M) + ";");
		M--;
		addLine("M" + std::to_string(M) + ": push $0;");
		M++;
		Last_push_type.emplace_back(".byte");
		Stack_depth += 2;
		addLine("M" + std::to_string(M) + ":");
		M++;
		return;
	}
	if (Node_value == "<>")
	{
		addLine("popl %eax;");
		Last_push_type.pop_back();
		Stack_depth -= 4;
		addLine("popl %ebx;");
		Last_push_type.pop_back();
		Stack_depth -= 4;
		addLine("cmpl %eax,%ebx;");
		addLine("je M" + std::to_string(M) + ";");
		addLine("push $1;");
		M++;
		addLine("jmp M" + std::to_string(M) + ";");
		M--;
		addLine("M" + std::to_string(M) + ": push $0;");
		M++;
		Last_push_type.emplace_back(".byte");
		Stack_depth += 2;
		addLine("M" + std::to_string(M) + ":");
		M++;
		return;
	}
	if (Node_value == ">=")
	{
		addLine("popl %eax;");
		Last_push_type.pop_back();
		Stack_depth -= 4;
		addLine("popl %ebx;");
		Last_push_type.pop_back();
		Stack_depth -= 4;
		addLine("cmpl %eax,%ebx;");
		addLine("jl M" + std::to_string(M) + ";");
		addLine("push $1;");
		M++;
		addLine("jmp " + std::to_string(M) + ";");
		M--;
		addLine("M" + std::to_string(M) + ": push $0;");
		M++;
		Last_push_type.emplace_back(".byte");
		Stack_depth += 2;
		addLine("M" + std::to_string(M) + ":");
		M++;
		return;
	}
	if (Node_value == "<=")
	{
		addLine("popl %eax;");
		Last_push_type.pop_back();
		Stack_depth -= 4;
		addLine("popl %ebx;");
		Last_push_type.pop_back();
		Stack_depth -= 4;
		addLine("cmpl %eax,%ebx;");
		addLine("jg M" + std::to_string(M) + ";");
		addLine("push $1;");
		M++;
		addLine("jmp " + std::to_string(M) + ";");
		M--;
		addLine("M" + std::to_string(M) + ": push $0;");
		M++;
		Last_push_type.emplace_back(".byte");
		Stack_depth += 2;
		addLine("M" + std::to_string(M) + ":");
		M++;
		return;
	}

}

/**
 * @brief Generate line with label (variable) information
 * @param[in] name - name of the label
 * @param[in] type - type of the label
 * @param[in] val  - value of the label
 *
 * @return none
 * @note For line of 'a : integer = 12' will be generate 'a: .long 12'
 */
void GenCode::generateLabel(const std::string& name, const std::string& type,
	const std::string& val) {
	std::string decl = name + ": " + type + val;
	buildLine(std::move(decl));
}


/**
 * @brief Generate 'ret' operator and add empty line in the end of file
 * @param none
 *
 * @return none
 */
void GenCode::generateEnd() {
	addLine("popl %ebp;");
	addLine(RET_SECT);
	addLine("");
}


/**
 * @brief Get type of variable from syntax tree
 * @param[in] node - node with variable
 *
 * @return "" - if doesn't have node with type
 * @return type of variable
 * @note       root
 *             / \
 *          var  begin
 *          / \     \
 * node -> a   $    ...
 *          \  ...
 *        <type>
 */
std::string GenCode::getType(Tree* node) {
	if (node->GetRightNode() == nullptr)
		return "";
	else
		return node->GetRightNode()->GetValue();
}


/**
 * @brief Get specific field of variable from syntax tree
 * @param[in] node - node with variable
 *
 * @return "" - if doesn't have node with specific (parameters)
 * @return parameter of variable
 * @note       root
 *             / \
 *          var  begin
 *          / \     \
 * node -> a   $    ...
 *        / \  ...
 *  <spec>  ...
 */
std::string GenCode::getSpec(Tree* node) {
	if (node->GetLeftNode() == nullptr)
		return "";
	else
		return node->GetLeftNode()->GetValue();
}


/**
 * @brief Get size of array
 * @param[in] spec_node - node of array field of variable
 * @param[in] type      - type of variable
 *
 * @return calculated size of array
 */
std::string GenCode::getArraySize(Tree* spec_node, std::string type) {
	int max = std::stoi(spec_node->GetRightNode()->GetValue());
	int type_size = (type == "integer") ? 4 : 1;
	return std::to_string(max * type_size);
}


/**
 * @brief Compare type of variable with know types
 * @param[in] type - type of variable
 *
 * @return true  - if type of variable is matched with known types
 * @return false - if doesn't match
 */
bool GenCode::checkType(const std::string& type) {
	auto res = std::find_if(types.begin(), types.end(), [&](const std::string& t) {
		return (t == type);
		});

	return res != types.end();
}

std::string GenCode::GetType(std::string Var)
{
	auto map_iter = id_map.find(Var);
	return map_iter->second;
}

/**
 * @brief Compare specific field of variable with known specific types
 * @param[in] spec - specific field of variable
 *
 * @return true  - if specific field of variable is matched
 * @return false - if doesn't match
 */
bool GenCode::checkSpec(const std::string& spec) {
	auto res = std::find_if(specif.begin(), specif.end(), [&](const std::string& t) {
		return (t == spec);
		});

	return res != specif.end();
}


/**
 * @brief Clear inner string stream buffer
 * @param none
 *
 * @return none
 */
void GenCode::clearBuffer() {
	test_str.str("");
	test_str.clear();
}


#endif GEN_CODE_CPP