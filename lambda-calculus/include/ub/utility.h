#pragma once

#include <type_traits>

namespace ub {

template <typename T>
struct visit_for {
  template <typename F, typename... Ts>
  static decltype(auto) f(F&& f, Ts&&... ts) {
    using type = std::decay_t<T>;
    static_assert(
        not std::is_same_v<T, type>,
        "Attempted to call visit_for on an unsupported type.");
    return visit_for<type>::f(std::forward<F>(f), std::forward<Ts>(ts)...);
  }
};

template <typename... Ts>
struct visit_for<std::variant<Ts...>> {
  template <typename F, typename... Ts>
  static decltype(auto) f(F&& f, Ts&&... ts) {
    return std::visit(std::forward<F>(f), std::forward<Ts>(ts)...);
  }
};

namespace impl {
  template <typename...>
  struct overload;

  template <typename F, typename... Fs>
  struct overload<F, Fs...> : F, overload<Fs...> {
    template <typename F0, typename... F0s>
    overload(F0&& f, F0s&&... fs)
        : F(std::forward<F0>(f)), overload<Fs...>(std::forward<F0s>(fs)...) {}

    using F::operator();
    using overload<Fs...>::operator();
  };

  template <typename F>
  struct overload<F> : F {
    template <typename F0>
    overload(F0&& f) : F(std::forward<F0>(f)) {}

    using F::operator();
  };

  template <typename... Ts>
  auto make_overload(Ts&&... ts) {
    return impl::overload<std::remove_reference_t<Ts>...>(
        std::forward<Ts>(ts)...);
  }

  // TODO(ubsan): this will eventually be extended to multiple Ts
  template <typename T, typename... Ts>
  struct matcher {
    std::tuple<T&&, Ts&&...> underlying;

    template <typename... Fs>
    decltype(auto) operator()(Fs&&... fs) && {
      auto const lam = [&](T&& t, Ts&&... ts) -> decltype(auto) {
        return visit_for<T>::f(
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
  return impl::matcher<Ts...>{std::tuple<Ts&&...>(std::forward<Ts>(ts)...)};
}

template <typename T>
[[noreturn]] T abort_as() { std::abort(); }

template <typename T, typename Exn>
[[noreturn]] T throw_as(Exn&& exception) { throw std::forward<Exn>(exception); }

namespace prelude {
  using ::ub::match;
}
} // namespace ub