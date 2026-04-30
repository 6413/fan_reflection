#include <meta>
#include <string>
#include <sstream>
#include <type_traits>

import fan;

namespace fan::refl {
  template<typename T>
  consteval auto members() {
    return std::define_static_array(std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::unchecked()));
  }

  consteval std::string_view name_of(std::meta::info m) {
    return std::meta::identifier_of(m);
  }

  template<typename T, typename F>
  constexpr void iterate_members(auto&& obj, F&& func) {
    template for (constexpr auto m : fan::refl::members<T>()) {
      func(std::meta::identifier_of(m), obj.[:m:]);
    }
  }
  template<typename T>
  consteval std::size_t member_count() {
    return std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::unchecked()).size();
  }
  template<typename T>
  constexpr auto& at_index(auto&& obj, std::size_t i) {
    void* ptr = nullptr;
    std::size_t cur = 0;

    using obj_t = std::remove_reference_t<decltype(obj)>;
    using cast_t = std::conditional_t<std::is_const_v<obj_t>, const T, T>;

    template for (constexpr auto m : members<T>()) {
      if (cur++ == i) {
        constexpr auto mp = &[:m:];
        ptr = std::addressof(static_cast<cast_t&>(obj).*mp);
      }
    }

    using M0 = decltype(std::declval<T>().*(&[:members<T>()[0]:]));
    return *static_cast<std::remove_reference_t<M0>*>(ptr);
  }
  template<typename T>
  constexpr void apply_at(auto&& obj, std::size_t i, auto&& func) {
    using D = std::remove_cvref_t<decltype(obj)>;
    std::size_t cur = 0;
    template for (constexpr auto m : members<D>()) {
      if (cur++ == i) func(obj.[:m:]);
    }
  }
  template<typename T>
  constexpr std::string_view member_name(std::size_t i) {
    std::size_t cur = 0;
    template for (constexpr auto m : members<T>()) {
      if (cur++ == i) return std::meta::identifier_of(m);
    }
    return "";
  }
}

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
      fan::refl::iterate_members<T>(obj, [&](auto name, auto& value) {
        out += fan::paint(fan::colors::amber, ind + "  ." + std::string{name} + " = ") + format_reflect(value, indent_level + 1) + "\n";
      });
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

template <typename T>
struct dme_t {
  consteval auto size() { return fan::refl::member_count<T>(); }
  constexpr auto& operator[](this auto&& self, std::size_t i) { return fan::refl::at_index<T>(self, i);}
  constexpr auto begin() { return &(*this)[0]; }
  constexpr auto end()   { return &(*this)[0] + size(); }
  constexpr std::string_view name(std::size_t i) const {
    std::string_view result;
    std::size_t cur = 0;
    template for (constexpr auto m : fan::refl::members<T>()) {
      if (cur++ == i) result = std::meta::identifier_of(m);
    }
    return result;
  }
  void print() { fan::print_reflect(static_cast<T&>(*this)); }
};

struct _t {
  int number;
};

struct st_t : dme_t<st_t> {
  _t a{.number=5};
  _t b{.number=5};
};

int main() {
  st_t st;
  st[0].number = 12;
  st[1].number = 15;
  static_assert(st.size() == 2);
  st.print();
  constexpr auto a_str = st.name(0);
  constexpr auto b_str = st.name(1);
  fan::print(a_str, b_str);

  int index = 0;
  for (auto& m : st) {
    fan::print(m.number);
    m.number = 20 + index;
    fan::print(m.number);
  }
}