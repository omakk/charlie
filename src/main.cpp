#include "ast.h"
#include "parser.h"

#include <iostream>

using namespace charlie;

int main() {
  std::cout << "Welcome to Charlie!" << '\n';

  Parser p("examples.ch");

  std::cout << "\nParsing..." << '\n';
  auto module = p.Parse();

  std::cout << "\nPrinting AST..." << '\n';
  AstDisplayVisitor adv;

  module->Accept(adv);

  return 0;
}
