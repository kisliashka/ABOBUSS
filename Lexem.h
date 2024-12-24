#ifndef LEXEM_H
#define LEXEM_H

#include <string>
using namespace std;

enum  tokens
{
    unknown_tk = -1,    // we get unknown token
    program_tk = 0,     // 'program'
    var_tk,             // 'var'
    begin_tk,           // 'begin'
    end_tk,             // 'end'
    type_tk,            // 'type'
    id_tk = 8,          // any [aA-zZ][0-9]
    constant_tk = 9,    // 0-9
    array_tk,           // 'array'
    const_tk,           // 'const'
    while_tk,           // 'while'
    bool_false_tk,      // 'false'
    bool_true_tk,       // 'true'
    of_tk,              // 'of'
    label_tk,           // 'label'
    goto_tk,            // 'goto'
    if_tk,              // 'if'
    else_tk,            // 'else'
    dot_tk,             // '.'
    comma_tk,           // ','
    apostrophe_tk,      // '''
    colon_tk,           // ':'
    semicolon_tk,       // ';'
    equals_tk,          // '='
    assignment_tk,      // ':='
    plus_tk,            // '+'
    minus_tk,           // '-'
    mul_tk,             // '*'
    div_tk,             // 'div'
    opb_tk,             // '('
    cpb_tk,             // ')'
    osb_tk,             // '['
    csb_tk,             // ']'
    eqv_tk,             // эквивалентность
    bool_equals_tk,     // '=='
    bool_bigger_tk,     // '>'
    bool_less_tk,       // '<'
    bool_bigeqv_tk,     // '>='
    bool_leseqv_tk,     // '<='
    comp_tk,            // 'comparison signs'
    eof_tk,             // 'end of file'
    boolean_tk,         // boolean
    integer_tk,         // integer
    repeat_tk,
    until_tk,
    and_tk,
    bool_noneqv_tk      // '<>'
};
enum errors {
    UNKNOWN_LEXEM,          // Unknown lexeme
    EOF_ERR,                // End of file error
    MUST_BE_ID,             // Error: Must be identifier
    MUST_BE_SEMI,           // Error: Must be ';'
    MUST_BE_PROG,           // Error: Must be 'program'
    MUST_BE_COMMA,          // Error: Must be ','
    MUST_BE_CONST,          // Error: Must be constant
    MUST_BE_COMP,           // Error: Must be comparison sign
    MUST_BE_TYPE,           // Error: Must be type of identifier
    MUST_BE_DOT,            // Error: Must be '.'
    MUST_BE_DOTS_ARR,       // Error: Must be '..'
    MUST_BE_ASS,            // Error: Must be ':='
    MUST_BE_BRACKET,        // Error: Must be '('
    MUST_BE_BRACKET_END,    // Error: Must be ')'
    MUST_BE_ARRBRACKET,     // Error: Must be '['
    MUST_BE_ARRBRACKET_END, // Error: Must be ']'
    MUST_BE_DO,             // Error: Must be 'do'
    MUST_BE_TO,             // Error: Must be 'to'
    MUST_BE_THEN,           // Error: Must be 'then'
    MUST_BE_OF,             // Error: Must be 'of'
    DUPL_ID_ERR,            // Error: Duplicate declaration on identifier
    UNKNOWN_ID,             // Error: Undefined identifier
    INCORRECT_TYPE,         // Error: Identifier is not an array
    INCORRECT_RANGE,        // Error: Out of array range
    MUST_BE_TYPE_OR_ARRAY,
    UNAS_EXP,
    MUST_BE_CPB,
    UNACC_TYPE,
    UNACC_EXP,
    MUST_BE_STATE,
    MUST_BE_OSB,
    MUST_BE_ESB,
    MUST_BE_CSB,
    MUST_BE_UNTIL,
    VAR_ITER,
    NO_INDEX_OF_ARRAY,
    MUST_BE_BEGIN,
    MUST_BE_OP,
    MUST_BE_END

    // TODO: Add other types of error
};

class Lexem {
public:
    Lexem() : token(0), line(0) {}

    Lexem(const std::string& name_, int token_, int line_)
        : name(name_), token(token_), line(line_) {}

    // Геттеры
    const std::string& GetName() const { return name; }
    int GetToken() const noexcept { return token; }
    int GetLine() const noexcept { return line; }

private:
    std::string name;
    int token;
    int line;
};

#endif
