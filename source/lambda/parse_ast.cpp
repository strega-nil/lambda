#include <ostream>

#include <lambda/parse_ast.h>
#include <ub/macros.h>
#include <ub/utility.h>

using namespace ub::prelude;

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
};

} // namespace lambda