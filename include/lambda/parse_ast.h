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
class parse_ast {
public:
  class variable {
    std::string name_;

  public:
    std::string_view name() const { return name_; }

    explicit variable(std::string name) noexcept : name_(std::move(name)) {}
  };

  class call {
    std::unique_ptr<parse_ast> callee_;
    std::unique_ptr<parse_ast> argument_;

  public:
    parse_ast const& callee() const { return *callee_; }
    parse_ast const& argument() const { return *argument_; }

    explicit call(parse_ast callee, parse_ast argument)
        : callee_(std::make_unique<parse_ast>(std::move(callee))),
          argument_(std::make_unique<parse_ast>(std::move(argument))) {}
  };

  class lambda {
    std::string parameter_;
    std::unique_ptr<parse_ast> expression_;

  public:
    std::string_view parameter() const { return parameter_; }
    parse_ast const& expression() const { return *expression_; }

    explicit lambda(std::string parameter, parse_ast expression)
        : parameter_(std::move(parameter)),
          expression_(std::make_unique<parse_ast>(std::move(expression))) {}
  };

  parse_ast(variable v) : underlying_(std::move(v)) {}
  parse_ast(call v) : underlying_(std::move(v)) {}
  parse_ast(lambda v) : underlying_(std::move(v)) {}

  template <typename T>
  friend struct ::ublib::visit_for;

private:
  std::variant<variable, call, lambda> underlying_;
};

std::ostream& operator<<(std::ostream&, parse_ast const&) noexcept;


class parse_error : public std::exception {
  ublib::shared_string what_;

public:
  parse_error(ublib::shared_string what) : what_(std::move(what)) {}
  virtual char const* what() const noexcept { return what_.c_str(); }
};

std::ostream& operator<<(std::ostream&, parse_error const&);

// @throw parse_error if the input is invalid lambda calculus
parse_ast parse_from(std::istream&);

} // namespace lambda

namespace ublib {

template <>
struct visit_for<::lambda::parse_ast> {
  template <typename F, typename... Ts>
  static decltype(auto) f(F&& f, Ts&&... ts) {
    return std::visit(std::forward<F>(f), std::forward<Ts>(ts).underlying_...);
  }
};

} // namespace ublib
