#pragma once

#include <memory>
#include <string_view>

#include <ublib/checked_iterators.h>

namespace ublib {

// NOTE(ubsan): this string *is* a NTBS
class Shared_string {
  std::size_t length_;
  std::shared_ptr<char const[]> underlying_;

public:
  Shared_string() noexcept : length_(0), underlying_(nullptr) {}

  Shared_string(Shared_string const&) noexcept = default;
  Shared_string(Shared_string&&) noexcept = default;
  Shared_string& operator=(Shared_string const&) noexcept = default;
  Shared_string& operator=(Shared_string&&) noexcept = default;
  ~Shared_string() = default;

  Shared_string(std::string_view s);

  Shared_string(char const* first, char const* last)
      : Shared_string(std::string_view(first, last - first)) {}
  Shared_string(char const* ptr, std::size_t length)
      : Shared_string(std::string_view(ptr, length)) {}
  Shared_string(std::string const& s) : Shared_string(std::string_view(s)) {}
  Shared_string(char const* s) : Shared_string(std::string_view(s)) {}

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

  bool empty() const noexcept { return length_ == 0; }
  std::size_t length() const noexcept { return length_; }
  std::size_t size() const noexcept { return length_; }
  char const& operator[](std::size_t idx) const noexcept {
    return underlying_.get()[idx];
  }

  // @throw std::out_of_range if idx >= size()
  char const& at(std::size_t idx) const {
    return std::string_view(*this).at(idx);
  }

  char const& front() const noexcept { return underlying_.get()[0]; }
  char const& back() const noexcept { return underlying_.get()[size() - 1]; }

  // returns a nul terminated string
  char const* data() const noexcept { return underlying_.get(); }
  char const* c_str() const noexcept { return data(); }

  iterator cbegin() const noexcept { return std::string_view(*this).cbegin(); }
  iterator begin() const noexcept { return cbegin(); }
  reverse_iterator crbegin() const noexcept {
    return std::string_view(*this).crbegin();
  }
  reverse_iterator rbegin() const noexcept { return crbegin(); }

  iterator cend() const noexcept { return std::string_view(*this).cend(); }
  iterator end() const noexcept { return cend(); }
  reverse_iterator crend() const noexcept {
    return std::string_view(*this).crend();
  }
  reverse_iterator rend() const noexcept { return crend(); }

  friend void swap(Shared_string& lhs, Shared_string& rhs) noexcept;
};

bool operator==(Shared_string const& lhs, Shared_string const& rhs) noexcept;
bool operator!=(Shared_string const& lhs, Shared_string const& rhs) noexcept;
bool operator<(Shared_string const& lhs, Shared_string const& rhs) noexcept;
bool operator>(Shared_string const& lhs, Shared_string const& rhs) noexcept;
bool operator<=(Shared_string const& lhs, Shared_string const& rhs) noexcept;
bool operator>=(Shared_string const& lhs, Shared_string const& rhs) noexcept;

std::ostream& operator<<(std::ostream& os, Shared_string const& rhs);

} // namespace ublib

namespace std {

template <>
struct hash<::ublib::Shared_string> {
  auto operator()(::ublib::Shared_string const& s) {
    return ::std::hash<::std::string_view>()(s);
  }
};

} // namespace std