#pragma once

#include <iterator>

namespace ub {

#if \
  defined(_MSC_VER) \
  and defined(_SECURE_SCL) \
  and _SECURE_SCL

using stdext::checked_array_iterator;

template <typename It>
checked_array_iterator<It> make_checked_iterator(It it, std::size_t sz) {
  return checked_array_iterator<It>(it, sz);
}

#else

template <typename It>
using checked_array_iterator = It;

template <typename It>
It make_checked_iterator(It it, std::size_t) {
  return it;
}

#endif

}