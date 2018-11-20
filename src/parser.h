#pragma once

#include <memory>

#include "lexer.h"

namespace charlie {

class Parser {
public:
    Parser(const std::string& file);
    ~Parser();

    void parse();

    /*
     * FunctionDeclaration: "fn" IDENTIFIER '(' FunctionParameters* ')' FunctionReturnType+ Block
     */
    void parse_function_definition();
    /*
     * Block:= '{' Statement* '}'
     */
    void parse_block();
    /*
     * Statement: BasicStatement ';'
     */
    void parse_statement();
    /*
     * BasicStatement: "return" Expression ';'
     */
    void parse_basic_statement();
    void parse_return_statement();
    /*
     * Expression: IntegerLiteral
     */
    void parse_expression();
    void print_current_token();
private:
    std::unique_ptr<Lexer> mLexer;
}; // class Parser

} // namespace charlie
