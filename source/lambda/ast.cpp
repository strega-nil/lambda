#include <lambda/ast.h>

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

using Context = std::vector<std::string_view>;

namespace {
  std::optional<int>
  find_in_context(Context const& ctxt, std::string_view to_find) {
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

  Ast reduce_rec(Parse_ast const& ast, Context& context) {
    return ublib::match(ast)(
        LAM(Parse_ast::Variable const& e) {
          if (auto idx = find_in_context(context, e.name())) {
            return Ast(Ast::Variable(*idx));
          } else {
            return Ast(Ast::Free_variable(e.name()));
          }
        },
        LAM(Parse_ast::Call const& e) {
          auto arg = reduce_rec(e.argument(), context);
          auto callee = reduce_rec(e.callee(), context);
          return Ast(Ast::Call(std::move(callee), std::move(arg)));
        },
        LAM(Parse_ast::Lambda const& e) {
          context.push_back(e.parameter());
          auto typed = reduce_rec(e.expression(), context);
          context.pop_back();
          return Ast(Ast::Lambda(e.parameter(), std::move(typed)));
        });
  }

} // namespace

Ast reduce(Parse_ast const& ast) {
  std::vector<std::string_view> context;
  return reduce_rec(ast, context);
}

Ast eval(Ast const& ast) {
  struct helper {
    static Ast substitute(Ast const& expr, Ast const& arg, int index) {
      return ublib::match(expr)(
          LAM(Ast::Lambda const& e) {
            return Ast(Ast::Lambda(
                e.variable(), substitute(e.expression(), arg, index + 1)));
          },
          LAM(Ast::Call const& e) {
            return Ast(Ast::Call(
                substitute(e.callee(), arg, index),
                substitute(e.argument(), arg, index)));
          },
          LAM(Ast::Variable const& e) {
            if (e.index() == index) {
              return arg;
            } else {
              return Ast(e);
            }
          },
          LAM(Ast::Free_variable const& e) { return Ast(e); });
    };

    static Ast do_call(Ast const& callee, Ast const& arg) {
      auto const callee_eval = eval(callee);
      auto const arg_eval = eval(arg);

      return ublib::match(callee_eval)(
          LAM(Ast::Lambda const& e) {
            return eval(substitute(e.expression(), arg_eval, 0));
          },
          LAM(Ast::Variable const&) {
            return ublib::unreachable<Ast>(); // should be impossible
          },
          LAM(Ast::Call const& e) { return Ast(Ast::Call(e, arg_eval)); },
          LAM(Ast::Free_variable const& e) {
            return Ast(Ast::Call(e, arg_eval));
          });
    };
  };

  return ublib::match(ast)(
      LAM(Ast::Call const& e) {
        return helper::do_call(e.callee(), e.argument());
      },
      LAM(Ast::Variable const&) {
        return ublib::throw_as<Ast>(
            Eval_error("evaluation found an unbound non-free variable"));
      },
      LAM(Ast::Free_variable const& e) { return Ast(e); },
      LAM(Ast::Lambda const& e) { return Ast(e); });
}

std::ostream& operator<<(std::ostream& os, Ast const& ast) {
  struct helper {
    static std::ostream& rec(std::ostream& os, Ast const& ast, Context& ctxt) {
      return ublib::match(ast)(
          RLAM(Ast::Variable const& e) {
            return os << ctxt.at(e.index()) << '_' << e.index();
          },
          RLAM(Ast::Free_variable const& e) { return os << e.name(); },
          RLAM(Ast::Call const& e) {
            rec(os, e.callee(), ctxt);
            os << ' ';
            return rec(os, e.argument(), ctxt);
          },
          RLAM(Ast::Lambda const& e) {
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
