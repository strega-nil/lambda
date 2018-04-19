#pragma once

#include <iterator>

namespace ublib {

#if defined(_MSC_VER) and defined(_SECURE_SCL) and _SECURE_SCL

template <typename It>
using Checked_array_iterator = stdext::checked_array_iterator<It>;

template <typename It>
Checked_array_iterator<It> make_checked_iterator(It it, std::size_t sz) {
  return Checked_array_iterator<It>(it, sz);
}

#else

template <typename It>
using Checked_array_iterator = It;

template <typename It>
It make_checked_iterator(It it, std::size_t) {
  return it;
}

#endif

} // namespace ublib