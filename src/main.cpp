#include "ast.h"
#include "lexer.h"
#include "parser.h"
#include <iostream>

using namespace charlie;

int main() {
  std::cout << "Welcome to Charlie!" << '\n';

  Parser p("examples");

  auto module = p.parse();

  AstDisplayVisitor adv;

  module->accept(adv);

  return 0;
}
