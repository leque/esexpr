#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

#include <pegtl.hh>
#include <pegtl/file_parser.hh>

#include <esexpr.hh>

using namespace std;

enum number_type {
  integer_type, float_type
};

struct esexpr_number {
  esexpr_number(int_least32_t n) : type(integer_type) {
    v.integer_value = n;
  }
  esexpr_number(double n) : type(float_type) {
    v.float_value = n;
  }
  number_type type;
  union {
    int_least64_t integer_value;
    double float_value;
  } v;
};

static int
digit_to_number(const char c, const int base) {
  int res = -1;
  if ('0' <= c && c <= '9') {
    res = c - '0';
  } else if ('a' <= c && c <= 'f') {
    res = 10 + c - 'a';
  } else if ('A' <= c && c <= 'F') {
    res = 10 + c - 'A';
  }
  return res < base ? res : -1;
}

static esexpr_number
string_to_number(const string &s, const int base, size_t pos) {
  esexpr_number res(0);
  const size_t end = s.length();
  bool positive = true;
  if (pos < end) {
    switch (s[pos]) {
    case '+':
      positive = true;
      ++pos;
      break;
    case '-':
      positive = false;
      ++pos;
      break;
    default:
      break;
    }
  }

  while (pos < end) {
    int d = digit_to_number(s[pos], base);
    if (d < 0)
      break;

    switch (res.type) {
    case integer_type:
      res.v.integer_value *= base;
      res.v.integer_value += d;
      if (res.v.integer_value
          > (positive ? esexpr::int_max : -esexpr::int_min)) {
        res.v.float_value = res.v.integer_value;
        res.type = float_type;
      }
      break;
    case float_type:
      res.v.float_value *= base;
      res.v.float_value += d;
      break;
    }

    ++pos;
  }

  const int sign = positive ? 1 : -1;
  switch (res.type) {
  case integer_type:
    res.v.integer_value *= sign;
    break;
  case float_type:
    res.v.float_value *= sign;
    break;
  }

  if (pos < end && s[pos] == '.')
    ++pos;

  if (pos != end) {
    throw invalid_argument("cannot parse as number: " + s);
  }

  return res;
}

static void
print_int(const string &s, size_t pos, const int radix) {
    esexpr_number num = string_to_number(s, radix, pos);
    string typ = num.type == integer_type ? "INTEGER" : "FLOAT";
    cout << typ << ": " << s
         << " (" << (num.type == integer_type
                     ? num.v.integer_value
                     : num.v.float_value)
         << ")" << endl;
}

const int tabstop = 2;

static void
do_indent(int amount) {
  for (int i = 0; i < amount; ++i) {
    cout << ' ';
  }
}

template<typename Rule>
struct print_actions : pegtl::nothing< Rule > {};

template<> struct print_actions<esexpr::comment> {
  static void apply(const pegtl::input &in, int &indent) {
    do_indent(indent);
    cout << "COMMENT: " << in.string();
  }
};

template<> struct print_actions<esexpr::quote> {
  static void apply(const pegtl::input &in, int &indent) {
    do_indent(indent);
    cout << "QUOTE" << endl;
  }
};

template<> struct print_actions<esexpr::label_def> {
  static void apply(const pegtl::input &in, int &indent) {
    do_indent(indent);
    cout << "LABEL-DEF: " << in.string() << endl;
  }
};

template<> struct print_actions<esexpr::label_ref> {
  static void apply(const pegtl::input &in, int &indent) {
    do_indent(indent);
    cout << "LABEL-REF: " << in.string() << endl;
  }
};

template<> struct print_actions<esexpr::list::begin> {
  static void apply(const pegtl::input &in, int &indent) {
    do_indent(indent);
    cout << "LIST:" << endl;
    indent += tabstop;
  }
};

template<> struct print_actions<esexpr::list::end> {
  static void apply(const pegtl::input &in, int &indent) {
    indent -= tabstop;
  }
};

template<> struct print_actions<esexpr::vector::begin> {
  static void apply(const pegtl::input &in, int &indent) {
    do_indent(indent);
    cout << "VECTOR:" << endl;
    indent += tabstop;
  }
};

template<> struct print_actions<esexpr::vector::end> {
  static void apply(const pegtl::input &in, int &indent) {
    indent -= tabstop;
  }
};
template<> struct print_actions<esexpr::string> {
  static void apply(const pegtl::input &in, int &indent) {
    do_indent(indent);
    cout << "STRING: " << in.string() << endl;
  }
};

template<> struct print_actions<esexpr::character> {
  static void apply(const pegtl::input &in, int &indent) {
    do_indent(indent);
    cout << "CHARACTER: " << in.string() << endl;
  }
};

template<> struct print_actions<esexpr::float_> {
  static void apply(const pegtl::input &in, int &indent) {
    do_indent(indent);
    cout << "FLOAT: " << in.string()
              << " (" << stod(in.string()) << ")"
              << endl;
  }
};

template<> struct print_actions<esexpr::integer2> {
  static void apply(const pegtl::input &in, int &indent) {
    do_indent(indent);
    print_int(in.string(), 2, 2);
  }
};

template<> struct print_actions<esexpr::integer16> {
  static void apply(const pegtl::input &in, int &indent) {
    do_indent(indent);
    print_int(in.string(), 2, 16);
  }
};

template<> struct print_actions<esexpr::integer10> {
  static void apply(const pegtl::input &in, int &indent) {
    do_indent(indent);
    print_int(in.string(), 0, 10);
  }
};

template<> struct print_actions<esexpr::symbol> {
  static void apply(const pegtl::input &in, int &indent) {
    do_indent(indent);
    cout << "SYMBOL: " << in.string() << endl;
  }
};

int
main(int argc, char **argv)
{
  cout.precision(numeric_limits<double>::max_digits10);

  if (argc == 3 && strcmp(argv[1], "-f") == 0) {
    pegtl::file_parser p(argv[2]);
    p.parse<esexpr::sexprs, print_actions>(0);
  } else if (argc == 2) {
    pegtl::data_parser p(argv[1], argv[0]);
    p.parse<esexpr::sexpr, print_actions>(0);
  } else {
    cerr << "usage: " << argv[0] << " file-to-parse" << endl;
    return 1;
  }

  return 0;
}
