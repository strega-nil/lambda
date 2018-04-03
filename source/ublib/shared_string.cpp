#include <ublib/shared_string.h>

#include <string_view>

namespace ublib {

shared_string::shared_string(std::string_view s) : shared_string() {
  if (not s.empty()) {
    auto sz = s.size();
    std::shared_ptr<char[]> tmp(new char[sz + 1]());
    std::copy(
        s.begin(), s.end(), ublib::make_checked_iterator(tmp.get(), sz + 1));
    underlying_ = std::move(tmp);
    length_ = sz;
  }
}

void swap(shared_string& lhs, shared_string& rhs) noexcept {
  using std::swap;
  swap(lhs.underlying_, rhs.underlying_);
  swap(lhs.length_, rhs.length_);
}

bool operator==(shared_string const& lhs, shared_string const& rhs) noexcept {
  return std::string_view(lhs) == std::string_view(rhs);
}
bool operator!=(shared_string const& lhs, shared_string const& rhs) noexcept {
  return std::string_view(lhs) != std::string_view(rhs);
}
bool operator<(shared_string const& lhs, shared_string const& rhs) noexcept {
  return std::string_view(lhs) < std::string_view(rhs);
}
bool operator>(shared_string const& lhs, shared_string const& rhs) noexcept {
  return std::string_view(lhs) > std::string_view(rhs);
}
bool operator<=(shared_string const& lhs, shared_string const& rhs) noexcept {
  return std::string_view(lhs) <= std::string_view(rhs);
}
bool operator>=(shared_string const& lhs, shared_string const& rhs) noexcept {
  return std::string_view(lhs) >= std::string_view(rhs);
}

std::ostream& operator<<(std::ostream& os, shared_string const& rhs) {
  return os << std::string_view(rhs);
}

} // namespace ublib