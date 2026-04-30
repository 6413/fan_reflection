#include <meta>
#include <string>
#include <sstream>
#include <type_traits>

import fan;

namespace fan {
  template<typename T>
  std::string format_reflect(const T& obj, int indent_level = 0) {
    if constexpr (requires { std::declval<std::ostream&>() << obj; }) {
      std::ostringstream val; val << obj;
      std::string t{std::meta::display_string_of(^^T)};
      if (t.starts_with("member_t {aka ")) t = t.substr(14, t.size() - 15);
      return fan::paint(fan::colors::white, val.str()) +
             fan::paint(fan::colors::gray,  " [") +
             fan::paint(fan::colors::teal,  t) +
             fan::paint(fan::colors::gray,  "]");
    } else {
      std::string ind(indent_level * 2, ' ');
      std::string out = fan::paint(fan::colors::cyan, std::string{std::meta::display_string_of(^^T)}) + fan::paint(fan::colors::gray, " {\n");
      constexpr auto ctx = std::meta::access_context::unchecked();
      template for (constexpr auto m : std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx))) {
        out += fan::paint(fan::colors::amber, ind + "  ." + std::string{std::meta::identifier_of(m)} + " = ") + format_reflect(obj.[:m:], indent_level + 1) + "\n";
      }
      return out + fan::paint(fan::colors::gray, ind + "}");
    }
  }

  template<typename... T>
  void print_reflect(const T&... args) {
    (fan::print_color_raw(fan::colors::white, format_reflect(args) + "\n"), ...);
  }
}

struct a_t {
  int a;
  double b;
  char c;
};

struct b_t {
  b_t() = default;
  int d;
  double e;
  char f;
  a_t a;
private:
  int aasdf;
};

struct base_t {
  int x;
  float y;
};

struct [[=base_t]] derived_t {
  int z;
};

int main() {
  b_t b;
  b.d = 10;
  b.e = 2.718;
  b.f = 'y';
  b.a.a = 5;
  b.a.b = 3.14;
  b.a.c = 'x';
  fan::print_reflect(derived_t{}, "1", 3, a_t{.a=5, .b=3.14, .c='x'}, b);
}