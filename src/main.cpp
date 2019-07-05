#include "ast.h"
#include "parser.h"

#include <iostream>

using namespace charlie;

int main() {
  std::cout << "Welcome to Charlie!" << '\n';

  Parser p("examples.ch");

  std::cout << "Parsing...\n";
  auto module = p.Parse();
  if (!module) {
    std::cerr << "Parse failed!" << '\n';
    return 1;
  }
  std::cout << "Parse done\n\n";

  std::cout << "Printing AST...\n";
  AstDisplayVisitor adv;
  module->Accept(adv);
  std::cout << "Print done\n\n";

  std::cout << "Codegen from AST...\n";
  CodegenVisitor cv;
  module->Accept(cv);
  std::cout << "Codegen done\n\n";

  return 0;
}
