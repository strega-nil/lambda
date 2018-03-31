#pragma once

// NOTE(ubsan): not yet typed (if it'll ever be typed)
// at this point, it's just a de-bruijnified version

#include <lambda/parse_ast.h>

#include <ub/utility.h>
#include <ub/shared_string.h>

#include <cassert>
#include <exception>
#include <memory>
#include <string_view>
#include <variant>

namespace lambda {

class typed_ast {
public:
  class variable;
  class free_variable;
  class call;
  class lambda;

  using underlying_type =
      std::variant<variable, free_variable, call, lambda> const;

  typed_ast(variable e);
  typed_ast(free_variable e);
  typed_ast(call e);
  typed_ast(lambda e);

  template <typename T>
  friend struct ::ub::visit_for;

private:
  std::shared_ptr<underlying_type> underlying_;
};

class typed_ast::variable {
  int index_;

public:
  int index() const noexcept { return index_; }

  explicit variable(int index) noexcept : index_(index) {}
};
inline typed_ast::typed_ast(variable e)
    : underlying_(std::make_shared<underlying_type>(std::move(e))) {}

class typed_ast::free_variable {
  ub::shared_string name_;

public:
  ub::shared_string name() const noexcept { return name_; }

  explicit free_variable(ub::shared_string name)
      : name_(std::move(name)) {}
};
inline typed_ast::typed_ast(free_variable e)
    : underlying_(std::make_shared<underlying_type>(std::move(e))) {}

class typed_ast::call {
  typed_ast callee_;
  typed_ast argument_;

public:
  typed_ast const& callee() const noexcept { return callee_; }
  typed_ast const& argument() const noexcept { return argument_; }

  call(typed_ast callee, typed_ast argument)
      : callee_(std::move(callee)), argument_(std::move(argument)) {}
};
inline typed_ast::typed_ast(call e)
    : underlying_(std::make_shared<underlying_type>(std::move(e))) {}

class typed_ast::lambda {
  ub::shared_string parameter_;
  typed_ast expression_;

public:
  ub::shared_string variable() const noexcept { return parameter_; }
  typed_ast const& expression() const noexcept { return expression_; }

  lambda(ub::shared_string variable, typed_ast expression)
      : parameter_(std::move(variable)),
        expression_(std::move(expression)) {}
};
inline typed_ast::typed_ast(lambda e)
    : underlying_(std::make_shared<underlying_type>(std::move(e))) {}

class make_typed_error : public std::exception {
  ub::shared_string what_;

public:
  make_typed_error(ub::shared_string what)
      : what_(std::move(what)) {}
  virtual char const* what() const noexcept { return what_.c_str(); }
};

// @throw make_typed_error if the parse_ast is not well-typed
// NOTE(ubsan): doesn't yet actually do typeck, soooo
typed_ast make_typed(parse_ast const&);

class eval_error : public std::exception {
  ub::shared_string what_;

public:
  eval_error(ub::shared_string what)
      : what_(std::move(what)) {}
  virtual char const* what() const noexcept { return what_.c_str(); }
};

// @throw eval_error if the typed_ast is not well-formed
// if the ast is taken from `make_typed`, then this should not happen
typed_ast eval(typed_ast const&);

std::ostream& operator<<(std::ostream&, typed_ast const&);

} // namespace lambda

namespace ub {

template <>
struct visit_for<::lambda::typed_ast> {
  template <typename F, typename... Ts>
  static decltype(auto) f(F&& f, Ts const&... ts) {
    return std::visit(std::forward<F>(f), *ts.underlying_...);
  }
};

} // namespace ub