#pragma once

#include <memory>
#include <string_view>

#include <ub/checked_iterators.h>

namespace ub {

// NOTE(ubsan): this string *is* a NTBS
class shared_string {
  std::size_t length_;
  std::shared_ptr<char const[]> underlying_;
public:
  shared_string() noexcept : length_(0), underlying_(nullptr) {}

  shared_string(shared_string const&) noexcept = default;
  shared_string(shared_string&&) noexcept = default;
  shared_string& operator=(shared_string const&) noexcept = default;
  shared_string& operator=(shared_string&&) noexcept = default;
  ~shared_string() = default;

  shared_string(std::string_view s) : shared_string() {
    if (not s.empty()) {
      auto sz = s.size();
      std::shared_ptr<char[]> tmp(new char[sz + 1]());
      std::copy(s.begin(), s.end(), ub::make_checked_iterator(tmp.get(), sz + 1));
      underlying_ = std::move(tmp);
      length_ = sz;
    }
  }

  shared_string(char const* first, char const* last) : shared_string(std::string_view(first, last - first)) {}
  shared_string(char const* ptr, std::size_t length) : shared_string(std::string_view(ptr, length)) {}
  shared_string(std::string const& s) : shared_string(std::string_view(s)) {}
  shared_string(char const* s) : shared_string(std::string_view(s)) {}

  operator std::string_view() const noexcept {
    if (empty()) {
      return std::string_view();
    } else {
      return std::string_view(underlying_.get(), length_);
    }
  }

  using iterator = std::string_view::const_iterator;
  using const_iterator = iterator;
  using reverse_iterator = std::string_view::const_reverse_iterator;
  using const_reverse_iterator = reverse_iterator;

  bool empty() const noexcept {
    return length_ == 0;
  }
  std::size_t length() const noexcept{
    return length_;
  }
  std::size_t size() const noexcept {
    return length_;
  }
  char const& operator[](std::size_t idx) const noexcept {
    return underlying_.get()[idx];
  }

  // @throw std::out_of_range if idx >= size()
  char const& at(std::size_t idx) const {
    return std::string_view(*this).at(idx);
  }

  char const& front() const noexcept {
    return underlying_.get()[0];
  }
  char const& back() const noexcept {
    return underlying_.get()[size() - 1];
  }
  
  // returns a nul terminated string
  char const* data() const noexcept {
    return underlying_.get();
  }
  char const* c_str() const noexcept {
    return data();
  }

  iterator cbegin() const noexcept { return std::string_view(*this).cbegin(); }
  iterator begin() const noexcept { return cbegin(); }
  reverse_iterator crbegin() const noexcept { return std::string_view(*this).crbegin(); }
  reverse_iterator rbegin() const noexcept { return crbegin(); }

  iterator cend() const noexcept { return std::string_view(*this).cend(); }
  iterator end() const noexcept { return cend(); }
  reverse_iterator crend() const noexcept { return std::string_view(*this).crend(); }
  reverse_iterator rend() const noexcept { return crend(); }

  friend void swap(shared_string& lhs, shared_string& rhs) noexcept;
};

inline void swap(shared_string& lhs, shared_string& rhs) noexcept {
  using std::swap;
  swap(lhs.underlying_, rhs.underlying_);
  swap(lhs.length_, rhs.length_);
}

inline bool operator==(shared_string const& lhs, shared_string const& rhs) noexcept {
  return std::string_view(lhs) == std::string_view(rhs);
}
inline bool operator!=(shared_string const& lhs, shared_string const& rhs) noexcept {
  return std::string_view(lhs) != std::string_view(rhs);
}
inline bool operator<(shared_string const& lhs, shared_string const& rhs) noexcept {
  return std::string_view(lhs) < std::string_view(rhs);
}
inline bool operator>(shared_string const& lhs, shared_string const& rhs) noexcept {
  return std::string_view(lhs) > std::string_view(rhs);
}
inline bool operator<=(shared_string const& lhs, shared_string const& rhs) noexcept {
  return std::string_view(lhs) <= std::string_view(rhs);
}
inline bool operator>=(shared_string const& lhs, shared_string const& rhs) noexcept {
  return std::string_view(lhs) >= std::string_view(rhs);
}

inline std::ostream& operator<<(std::ostream& os, shared_string const& rhs) {
  return os << std::string_view(rhs);
}

}

namespace std {

template <>
struct hash<::ub::shared_string> {
  auto operator()(::ub::shared_string const& s) {
    return ::std::hash<::std::string_view>()(s);
  }
};

}