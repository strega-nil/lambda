#include <lambda/typed_ast.h>

#include <ublib/failure.h>
#include <ublib/macros.h>
#include <ublib/utility.h>

#include <iostream>

#include <algorithm>
#include <iterator>
#include <limits>
#include <optional>
#include <vector>

using namespace std::literals;

namespace lambda {

using context_t = std::vector<std::string_view>;

namespace {
  std::optional<int> find_in_context(context_t const& ctxt, std::string_view to_find) {
    using std::crbegin;
    using std::crend;

    auto first = crbegin(ctxt);
    auto last = crend(ctxt);

    assert(last - first <= std::numeric_limits<int>::max());

    auto found = std::find(first, last, to_find);
    if (found == last) {
      return std::nullopt;
    } else {
      return static_cast<int>(found - first);
    }
  }

  typed_ast
  make_typed_rec(parse_ast const& ast, context_t& context) {
    return ublib::match(ast)(
        LAM(parse_ast::variable const& e) {
          if (auto idx = find_in_context(context, e.name())) {
            return typed_ast(typed_ast::variable(*idx));
          } else {
            return typed_ast(typed_ast::free_variable(e.name()));
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
          return typed_ast(typed_ast::lambda(e.parameter(), std::move(typed)));
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
      return ublib::match(expr)(
          LAM(typed_ast::lambda const& e) {
            return typed_ast(typed_ast::lambda(
                e.variable(), substitute(e.expression(), arg, index + 1)));
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
      auto const callee_eval = eval(callee);
      auto const arg_eval = eval(arg);

      return ublib::match(callee_eval)(
          LAM(typed_ast::lambda const& e) {
            return eval(substitute(e.expression(), arg_eval, 0));
          },
          LAM(typed_ast::variable const&) {
            return ublib::unreachable<typed_ast>(); // should be impossible
          },
          LAM(typed_ast::call const& e) {
            return typed_ast(typed_ast::call(e, arg_eval));
          },
          LAM(typed_ast::free_variable const& e) {
            return typed_ast(typed_ast::call(e, arg_eval));
          });
    };
  };

  return ublib::match(ast)(
      LAM(typed_ast::call const& e) {
        return helper::do_call(e.callee(), e.argument());
      },
      LAM(typed_ast::variable const&) {
        return ublib::throw_as<typed_ast>(
            eval_error("evaluation found an unbound non-free variable"));
      },
      LAM(typed_ast::free_variable const& e) { return typed_ast(e); },
      LAM(typed_ast::lambda const& e) { return typed_ast(e); });
}

std::ostream& operator<<(std::ostream& os, typed_ast const& ast) {
  struct helper {
    static std::ostream&
    rec(std::ostream& os,
        typed_ast const& ast,
        context_t& ctxt) {
      return ublib::match(ast)(
          RLAM(typed_ast::variable const& e) {
            return os << ctxt.at(e.index()) << '_' << e.index();
          },
          RLAM(typed_ast::free_variable const& e) { return os << e.name(); },
          RLAM(typed_ast::call const& e) {
            rec(os, e.callee(), ctxt);
            os << ' ';
            return rec(os, e.argument(), ctxt);
          },
          RLAM(typed_ast::lambda const& e) {
            os << "(/" << e.variable() << '.';
            ctxt.push_back(e.variable());
            rec(os, e.expression(), ctxt);
            ctxt.pop_back();
            return os << ')';
          });
    }
  };
  std::vector<std::string_view> ctxt;
  return helper::rec(os, ast, ctxt);
}

} // namespace lambda
