#pragma once

#include <ublib/shared_string.h>
#include <ublib/utility.h>

#include <iosfwd>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

namespace lambda {
class Parse_ast {
public:
  class Variable {
    std::string name_;

  public:
    std::string_view name() const { return name_; }

    explicit Variable(std::string name) noexcept : name_(std::move(name)) {}
  };

  class Call {
    std::unique_ptr<Parse_ast> callee_;
    std::unique_ptr<Parse_ast> argument_;

  public:
    Parse_ast const& callee() const { return *callee_; }
    Parse_ast const& argument() const { return *argument_; }

    explicit Call(Parse_ast callee, Parse_ast argument)
        : callee_(std::make_unique<Parse_ast>(std::move(callee))),
          argument_(std::make_unique<Parse_ast>(std::move(argument))) {}
  };

  class Lambda {
    std::string parameter_;
    std::unique_ptr<Parse_ast> expression_;

  public:
    std::string_view parameter() const { return parameter_; }
    Parse_ast const& expression() const { return *expression_; }

    explicit Lambda(std::string parameter, Parse_ast expression)
        : parameter_(std::move(parameter)),
          expression_(std::make_unique<Parse_ast>(std::move(expression))) {}
  };

  Parse_ast(Variable v) : underlying_(std::move(v)) {}
  Parse_ast(Call v) : underlying_(std::move(v)) {}
  Parse_ast(Lambda v) : underlying_(std::move(v)) {}

  template <typename T>
  friend struct ::ublib::Visit_for;

private:
  std::variant<Variable, Call, Lambda> underlying_;
};

std::ostream& operator<<(std::ostream&, Parse_ast const&) noexcept;

class Parse_error : public std::exception {
  ublib::Shared_string what_;

public:
  Parse_error(ublib::Shared_string what) : what_(std::move(what)) {}
  virtual char const* what() const noexcept { return what_.c_str(); }
};

std::ostream& operator<<(std::ostream&, Parse_error const&);

// @throw Parse_error if the input is invalid lambda calculus
Parse_ast parse_from(std::istream&);

} // namespace lambda

namespace ublib {

template <>
struct Visit_for<::lambda::Parse_ast> {
  template <typename F, typename... Ts>
  static decltype(auto) f(F&& f, Ts&&... ts) {
    return std::visit(std::forward<F>(f), std::forward<Ts>(ts).underlying_...);
  }
};

} // namespace ublib
