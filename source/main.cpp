#include <iostream>

#include <lambda/parse_ast.h>
#include <lambda/typed_ast.h>

#include <optional>
#include <variant>
#include <vector>

using lambda::parse_ast;
using lambda::typed_ast;

int main() {
  parse_ast parse = parse_ast::call(
      parse_ast::lambda(
          "x",
          parse_ast::call(parse_ast::variable("x"), parse_ast::variable("x"))),
      parse_ast::variable("y"));

  std::cout << "parse: " << parse << "\n\n";

  auto const pre_eval = lambda::make_typed(parse);
  std::cout << "typed: " << pre_eval << "\n\n";

  auto const post_eval = lambda::eval(pre_eval);
  std::cout << "eval'd: " << post_eval << '\n';
}
