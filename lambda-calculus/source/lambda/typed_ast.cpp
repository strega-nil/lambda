#include <lambda/typed_ast.h>

#include <ub/macros.h>
#include <ub/utility.h>

#include <iostream>

#include <algorithm>
#include <iterator>
#include <optional>
#include <vector>

using namespace std::literals;

namespace lambda {

namespace {
  template <typename Range, typename Ty>
  std::optional<int> find_in_context(Range const& range, Ty const& to_find) {
    using std::crbegin;
    using std::crend;

    auto first = crbegin(range);
    auto last = crend(range);

    assert(last - first <= INT_MAX);

    auto found = std::find(first, last, to_find);
    if (found == last) {
      return std::nullopt;
    } else {
      return static_cast<int>(found - first);
    }
  }

  typed_ast
  make_typed_rec(parse_ast const& ast, std::vector<std::string_view>& context) {
    return ub::match(ast)(
        LAM(parse_ast::variable const& e) {
          if (auto idx = find_in_context(context, e.name())) {
            return typed_ast(typed_ast::variable(*idx));
          } else {
            return typed_ast(typed_ast::free_variable(std::string(e.name())));
          }
        },
        LAM(parse_ast::call const& e) {
          auto arg = make_typed_rec(e.argument(), context);
          auto callee = make_typed_rec(e.callee(), context);
          return typed_ast(typed_ast::call(std::move(callee), std::move(arg)));
        },
        LAM(parse_ast::lambda const& e) {
          context.push_back(e.parameter());
          auto typed = make_typed_rec(e.expression(), context);
          context.pop_back();
          return typed_ast(
              typed_ast::lambda(std::string(e.parameter()), std::move(typed)));
        });
  }

} // namespace

typed_ast make_typed(parse_ast const& ast) {
  std::vector<std::string_view> context;
  return make_typed_rec(ast, context);
}

typed_ast eval(typed_ast const& ast) {
  struct helper {
    static typed_ast
    substitute(typed_ast const& expr, typed_ast const& arg, int index) {
      return ub::match(expr)(
          LAM(typed_ast::lambda const& e) {
            return typed_ast(
                e.with_expression(substitute(e.expression(), arg, index + 1)));
          },
          LAM(typed_ast::call const& e) {
            return typed_ast(typed_ast::call(
                substitute(e.callee(), arg, index),
                substitute(e.argument(), arg, index)));
          },
          LAM(typed_ast::variable const& e) {
            if (e.index() == index) {
              return arg;
            } else {
              return typed_ast(e);
            }
          },
          LAM(typed_ast::free_variable const& e) { return typed_ast(e); });
    };

    static typed_ast do_call(typed_ast const& callee, typed_ast const& arg) {
      auto const arg_eval = eval(arg);
      auto const callee_eval = eval(callee);

      return ub::match(callee_eval)(
          LAM(typed_ast::lambda const& e) {
            return eval(substitute(e.expression(), arg_eval, 0));
          },
          LAM(typed_ast::variable const&) {
            return ub::abort_as<typed_ast>(); // should be impossible
          },
          LAM(typed_ast::call const& e) {
            return typed_ast(typed_ast::call(e, arg_eval));
          },
          LAM(typed_ast::free_variable const& e) {
            return typed_ast(typed_ast::call(e, arg_eval));
          });
    };
  };

  return ub::match(ast)(
      LAM(typed_ast::call const& e) {
        return helper::do_call(e.callee(), e.argument());
      },
      LAM(typed_ast::variable const&) {
        return ub::throw_as<typed_ast>(
            eval_error("evaluation found an unbound non-free variable"));
      },
      LAM(typed_ast::free_variable const& e) { return typed_ast(e); },
      LAM(typed_ast::lambda const& e) { return typed_ast(e); });
}

std::ostream& operator<<(std::ostream& os, typed_ast const& ast) {
  return ub::match(ast)(
      RLAM(typed_ast::variable const& e) { return os << e.index(); },
      RLAM(typed_ast::free_variable const& e) { return os << e.name(); },
      RLAM(typed_ast::call const& e) {
        return os << e.callee() << ' ' << e.argument();
      },
      RLAM(typed_ast::lambda const& e) {
        return os << "(/." << e.expression() << ')';
      });
}

} // namespace lambda