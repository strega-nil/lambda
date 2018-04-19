#include <lambda/parse_ast.h>
#include <lambda/ast.h>

#include <ublib/failure.h>

#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <variant>
#include <vector>

constexpr static auto default_program = R"(
(/x.x) y
)";

std::unique_ptr<std::istream> get_program(int argc, char const* const* argv) {
  if (argc > 2) {
    auto const program_name = (argc > 0) ? argv[0] : "[program]";
    ublib::failwith("Usage: ", program_name, " [filename=code.lc]");
  } else if (argc == 2) {
    return std::make_unique<std::fstream>(argv[1], std::ios_base::in);
  } else {
    return std::make_unique<std::stringstream>(default_program);
  }
}

int main(int argc, char** argv) {
  auto file = get_program(argc, argv);

  auto parse = [&] {
    try {
      return lambda::parse_from(*file);
    } catch (lambda::Parse_error const& e) {
      ublib::failwith(e);
    }
  }();

  std::cout << "parse: " << parse << "\n\n";

  auto const pre_eval = lambda::reduce(parse);
  std::cout << "typed: " << pre_eval << "\n\n";

  auto const post_eval = eval(pre_eval);
  std::cout << "eval'd: " << post_eval << '\n';
}
