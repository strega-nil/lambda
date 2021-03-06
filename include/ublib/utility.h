﻿#pragma once

#include <iostream>
#include <tuple>
#include <type_traits>
#include <variant>

namespace ublib {

template <typename T>
struct Visit_for {
  template <typename F, typename... Ts>
  static decltype(auto) f(F&& f, Ts&&... ts) {
    using type = std::decay_t<T>;
    static_assert(
        not std::is_same_v<T, type>,
        "Attempted to call Visit_for on an unsupported type.");
    return Visit_for<type>::f(std::forward<F>(f), std::forward<Ts>(ts)...);
  }
};

template <typename... Ts>
struct Visit_for<std::variant<Ts...>> {
  template <typename F, typename... Us>
  static decltype(auto) f(F&& f, Us&&... us) {
    return std::visit(std::forward<F>(f), std::forward<Us>(us)...);
  }
};

namespace impl {
  template <typename...>
  struct Overload;

  template <typename F, typename... Fs>
  struct Overload<F, Fs...> : F, Overload<Fs...> {
    template <typename F0, typename... F0s>
    Overload(F0&& f, F0s&&... fs)
        : F(std::forward<F0>(f)), Overload<Fs...>(std::forward<F0s>(fs)...) {}

    using F::operator();
    using Overload<Fs...>::operator();
  };

  template <typename F>
  struct Overload<F> : F {
    template <typename F0>
    Overload(F0&& f) : F(std::forward<F0>(f)) {}

    using F::operator();
  };

  template <typename... Ts>
  auto make_overload(Ts&&... ts) {
    return impl::Overload<std::remove_reference_t<Ts>...>(
        std::forward<Ts>(ts)...);
  }

  // TODO(ubsan): this will eventually be extended to multiple Ts
  template <typename T, typename... Ts>
  struct Matcher {
    std::tuple<T&&, Ts&&...> underlying;

    template <typename... Fs>
    decltype(auto) operator()(Fs&&... fs) && {
      auto const lam = [&](T&& t, Ts&&... ts) -> decltype(auto) {
        return Visit_for<T>::f(
            make_overload(std::forward<Fs>(fs)...),
            std::forward<T>(t),
            std::forward<Ts>(ts)...);
      };
      return std::apply(lam, std::move(underlying));
    }
  };
} // namespace impl

template <typename... Ts>
auto match(Ts&&... ts) {
  return impl::Matcher<Ts...>{std::tuple<Ts&&...>(std::forward<Ts>(ts)...)};
}

namespace prelude {
  using ::ublib::match;
}
} // namespace ublib
