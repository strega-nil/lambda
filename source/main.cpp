#include <lambda/parse_ast.h>
#include <lambda/typed_ast.h>

#include <ublib/failure.h>

#include <fstream>
#include <iostream>
#include <optional>
#include <variant>
#include <vector>

#include <sstream>

int main(int argc [[maybe_unused]], char** argv [[maybe_unused]]) {
#if 0
  if (argc != 2) {
    auto const program_name =
      (argc > 0)
      ? argv[0]
      : "[program]";
    ublib::failwith("Usage: ", program_name, " [filename]");
  }
#endif

  //auto file = std::fstream(argv[1]);
  auto program = "";
  auto file = std::stringstream(program, std::ios_base::in);

  auto parse = [&] {
    try {
      return lambda::parse_from(file);
    } catch (lambda::parse_error const& e) {
      ublib::failwith(e);
    }
  }();

  std::cout << "parse: " << parse << "\n\n";

  auto const pre_eval = lambda::make_typed(parse);
  std::cout << "typed: " << pre_eval << "\n\n";

  auto const post_eval = lambda::eval(pre_eval);
  std::cout << "eval'd: " << post_eval << '\n';
}
