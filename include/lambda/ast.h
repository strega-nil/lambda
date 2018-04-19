#pragma once

// NOTE(ubsan): not yet typed (if it'll ever be typed)
// at this point, it's just a de-bruijnified version

#include <lambda/parse_ast.h>

#include <ublib/shared_string.h>
#include <ublib/utility.h>

#include <cassert>
#include <exception>
#include <memory>
#include <string_view>
#include <variant>

namespace lambda {

class Ast {
public:
  class Variable;
  class Free_variable;
  class Call;
  class Lambda;

  using Underlying_type =
      std::variant<Variable, Free_variable, Call, Lambda> const;

  Ast(Variable e);
  Ast(Free_variable e);
  Ast(Call e);
  Ast(Lambda e);

  template <typename T>
  friend struct ::ublib::Visit_for;

private:
  std::shared_ptr<Underlying_type> underlying_;
};

class Ast::Variable {
  int index_;

public:
  int index() const noexcept { return index_; }

  explicit Variable(int index) noexcept : index_(index) {}
};
inline Ast::Ast(Variable e)
    : underlying_(std::make_shared<Underlying_type>(std::move(e))) {}

class Ast::Free_variable {
  ublib::Shared_string name_;

public:
  ublib::Shared_string name() const noexcept { return name_; }

  explicit Free_variable(ublib::Shared_string name) : name_(std::move(name)) {}
};
inline Ast::Ast(Free_variable e)
    : underlying_(std::make_shared<Underlying_type>(std::move(e))) {}

class Ast::Call {
  Ast callee_;
  Ast argument_;

public:
  Ast const& callee() const noexcept { return callee_; }
  Ast const& argument() const noexcept { return argument_; }

  Call(Ast callee, Ast argument)
      : callee_(std::move(callee)), argument_(std::move(argument)) {}
};
inline Ast::Ast(Call e)
    : underlying_(std::make_shared<Underlying_type>(std::move(e))) {}

class Ast::Lambda {
  ublib::Shared_string parameter_;
  Ast expression_;

public:
  ublib::Shared_string variable() const noexcept { return parameter_; }
  Ast const& expression() const noexcept { return expression_; }

  Lambda(ublib::Shared_string variable, Ast expression)
      : parameter_(std::move(variable)), expression_(std::move(expression)) {}
};
inline Ast::Ast(Lambda e)
    : underlying_(std::make_shared<Underlying_type>(std::move(e))) {}

class reduce_error : public std::exception {
  ublib::Shared_string what_;

public:
  reduce_error(ublib::Shared_string what) : what_(std::move(what)) {}
  virtual char const* what() const noexcept { return what_.c_str(); }
};

// @throw reduce_error if the Parse_ast is not well-formed
Ast reduce(Parse_ast const&);

class Eval_error : public std::exception {
  ublib::Shared_string what_;

public:
  Eval_error(ublib::Shared_string what) : what_(std::move(what)) {}
  virtual char const* what() const noexcept { return what_.c_str(); }
};

// @throw Eval_error if the ast is not well-formed
// if the ast is taken from `make_typed`, then this should not happen
Ast eval(Ast const&);

std::ostream& operator<<(std::ostream&, Ast const&);

} // namespace lambda

namespace ublib {

template <>
struct Visit_for<::lambda::Ast> {
  template <typename F, typename... Ts>
  static decltype(auto) f(F&& f, Ts const&... ts) {
    return std::visit(std::forward<F>(f), *ts.underlying_...);
  }
};

} // namespace ublib