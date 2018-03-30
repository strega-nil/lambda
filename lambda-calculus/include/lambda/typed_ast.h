#pragma once

// NOTE(ubsan): not yet typed (if it'll ever be typed)
// at this point, it's just a de-bruijnified version

#include <lambda/parse_ast.h>

#include <ub/utility.h>

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
  std::shared_ptr<std::string const> name_;

public:
  std::string_view name() const noexcept { return *name_; }

  explicit free_variable(std::string name)
      : name_(std::make_shared<std::string const>(std::move(name))) {}
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
  std::shared_ptr<std::string const> parameter_;
  typed_ast expression_;

public:
  std::string_view variable() const noexcept { return *parameter_; }
  typed_ast const& expression() const noexcept { return expression_; }

  lambda with_expression(typed_ast new_expression) const {
    auto new_lam = *this;
    new_lam.expression_ = std::move(new_expression);
    return new_lam;
  }

  lambda(std::string variable, typed_ast expression)
      : parameter_(std::make_shared<std::string const>(std::move(variable))),
        expression_(std::move(expression)) {}
};
inline typed_ast::typed_ast(lambda e)
    : underlying_(std::make_shared<underlying_type>(std::move(e))) {}

class make_typed_error : public std::exception {
  std::shared_ptr<std::string const> what_;

public:
  make_typed_error(std::string what)
      : what_(std::make_shared<std::string const>(std::move(what))) {}
  virtual char const* what() const noexcept { return what_->c_str(); }
};

// @throw make_typed_error if the parse_ast is not well-typed
// NOTE(ubsan): doesn't yet actually do typeck, soooo
typed_ast make_typed(parse_ast const&);

class eval_error : public std::exception {
  std::shared_ptr<std::string const> what_;

public:
  eval_error(std::string what)
      : what_(std::make_shared<std::string const>(std::move(what))) {}
  virtual char const* what() const noexcept { return what_->c_str(); }
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