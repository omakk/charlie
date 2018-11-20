#include <iostream>
#include "lexer.h"
#include "parser.h"

using namespace charlie;

int main()
{
    std::cout << "Welcome to Charlie!" << '\n';


    Parser p("examples");

    p.parse();

    return 0;
}
