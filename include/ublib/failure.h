#include <iostream>

namespace ublib {

template <typename T = void>
[[noreturn]] T unreachable() { std::abort(); }

template <typename T = void, typename Exn>
[[noreturn]] T throw_as(Exn&& exception) { throw std::forward<Exn>(exception); }

namespace impl {
  template <typename T>
  void failwith_rec() {}
  template <typename T>
  void failwith_rec(T const& t) {
    std::cerr << t << '\n';
  }
  template <typename T, typename... Ts>
  void failwith_rec(T const& t, Ts const&... ts) {
    std::cerr << t;
    failwith_rec(ts...);
  }
}

template <typename T = void, typename... Ts>
[[noreturn]] T failwith(Ts const&... ts) {
  impl::failwith_rec(ts...);
  std::abort();
}

} // namespace ublib