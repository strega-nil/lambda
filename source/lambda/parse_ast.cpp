#include <lambda/parse_ast.h>

#include <ublib/macros.h>
#include <ublib/utility.h>

#include <cctype>
#include <iostream>
#include <optional>
#include <string>

using namespace ublib::prelude;

namespace lambda {

std::ostream& operator<<(std::ostream& os, parse_ast const& a) noexcept {
  return match(a)(
      RLAM(parse_ast::variable const& v) { return os << v.name(); },
      RLAM(parse_ast::call const& v) {
        return os << v.callee() << ' ' << v.argument();
      },
      RLAM(parse_ast::lambda const& v) {
        return os << '(' << '/' << v.parameter() << '.' << v.expression()
                  << ')';
      });
}

namespace {

  void eat_whitespace(std::istream& inp) {
    while (std::isspace(inp.peek())) {
      inp.get();
    }
  }

  std::optional<parse_ast> parse_from_optional(std::istream& inp) {
    eat_whitespace(inp);
    if (not inp.eof()) {
      auto ch = inp.get();
      switch (ch) {
      case '/':
      case '\\':
        throw parse_error("lambda expressions not yet supported");
      case '(':
        throw parse_error("parentheses not yet supported");
      case ')':
        throw parse_error("unexpected close parenthesis");
      case '.':
        throw parse_error("unexpected dot");
      default:
        if (std::isalpha(ch)) {
          throw parse_error("variables not yet supported");
        } else {
          throw parse_error("unknown character");
        }
      }
    } else {
      return std::nullopt;
    }
  }
}


parse_ast parse_from(std::istream& inp) {
  if (auto ret = parse_from_optional(inp)) {
    return std::move(*ret);
  } else {
    throw parse_error("no non-whitespace input");
  }
}

std::ostream& operator<<(std::ostream& os, parse_error const& e) {
  return os << "Parse error: " << e.what();
}

} // namespace lambda
