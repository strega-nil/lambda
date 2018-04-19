#include <ublib/shared_string.h>

#include <string_view>

namespace ublib {

Shared_string::Shared_string(std::string_view s) : Shared_string() {
  if (not s.empty()) {
    auto sz = s.size();
    std::shared_ptr<char[]> tmp(new char[sz + 1]());
    std::copy(
        s.begin(), s.end(), ublib::make_checked_iterator(tmp.get(), sz + 1));
    underlying_ = std::move(tmp);
    length_ = sz;
  }
}

void swap(Shared_string& lhs, Shared_string& rhs) noexcept {
  using std::swap;
  swap(lhs.underlying_, rhs.underlying_);
  swap(lhs.length_, rhs.length_);
}

bool operator==(Shared_string const& lhs, Shared_string const& rhs) noexcept {
  return std::string_view(lhs) == std::string_view(rhs);
}
bool operator!=(Shared_string const& lhs, Shared_string const& rhs) noexcept {
  return std::string_view(lhs) != std::string_view(rhs);
}
bool operator<(Shared_string const& lhs, Shared_string const& rhs) noexcept {
  return std::string_view(lhs) < std::string_view(rhs);
}
bool operator>(Shared_string const& lhs, Shared_string const& rhs) noexcept {
  return std::string_view(lhs) > std::string_view(rhs);
}
bool operator<=(Shared_string const& lhs, Shared_string const& rhs) noexcept {
  return std::string_view(lhs) <= std::string_view(rhs);
}
bool operator>=(Shared_string const& lhs, Shared_string const& rhs) noexcept {
  return std::string_view(lhs) >= std::string_view(rhs);
}

std::ostream& operator<<(std::ostream& os, Shared_string const& rhs) {
  return os << std::string_view(rhs);
}

} // namespace ublib