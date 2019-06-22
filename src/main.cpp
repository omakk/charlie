#include "ast.h"
#include "parser.h"

#include <iostream>

using namespace charlie;

int main() {
  std::cout << "Welcome to Charlie!" << '\n';

  Parser p("examples");

  auto module = p.Parse();

  AstDisplayVisitor adv;

  module->Accept(adv);

  return 0;
}
