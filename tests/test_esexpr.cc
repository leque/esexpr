#include <test.hh>

#include <pegtl/analyze.hh>

#include <esexpr.hh>

namespace pegtl
{
  using G = esexpr::sexpr1;

  template<typename Rule>
  void verify_rule(const std::size_t line, const char *file, const std::string &data)
  {
    verify_rule<Rule>(line, file, data, result_type::SUCCESS, 0);
  }

  void unit_test()
  {
    const auto p = analyze<G>();
    assert(p == 0);

    verify_rule<G>(__LINE__, __FILE__, "()");
    verify_rule<G>(__LINE__, __FILE__, "'()");
    verify_rule<G>(__LINE__, __FILE__, "[]");
    verify_rule<G>(__LINE__, __FILE__, "42");
    verify_rule<G>(__LINE__, __FILE__, "3.14");
    verify_rule<G>(__LINE__, __FILE__, "?p");
    verify_rule<G>(__LINE__, __FILE__, "(42)");
    verify_rule<G>(__LINE__, __FILE__, "( 42)");
    verify_rule<G>(__LINE__, __FILE__, "(42 )");
    verify_rule<G>(__LINE__, __FILE__, "( 42 )");
    verify_rule<G>(__LINE__, __FILE__, "( 42 3.14)");
    verify_rule<G>(__LINE__, __FILE__, "( 42 3.14 )");
    verify_rule<G>(__LINE__, __FILE__, "(42'3.14)");
    verify_rule<G>(__LINE__, __FILE__, "(42[])");
    verify_rule<G>(__LINE__, __FILE__, "[42]");
    verify_rule<G>(__LINE__, __FILE__, "(42())");
    verify_rule<G>(__LINE__, __FILE__, "(42\"\")");
    verify_rule<G>(__LINE__, __FILE__, "(42#b1)");
    verify_rule<G>(__LINE__, __FILE__, "42;comment\n");
    verify_rule<G>(__LINE__, __FILE__, "(list)");
    verify_rule<G>(__LINE__, __FILE__, "(call-with-current-continuation call-with-current-continuation)");
    verify_rule<G>(__LINE__, __FILE__, "(call/cc call/cc)");
    verify_rule<G>(__LINE__, __FILE__, "#b10");
    verify_rule<G>(__LINE__, __FILE__, "#x02");
    verify_rule<G>(__LINE__, __FILE__, "#B10");
    verify_rule<G>(__LINE__, __FILE__, "#X02");
    verify_rule<G>(__LINE__, __FILE__, "#Xff");
    verify_rule<G>(__LINE__, __FILE__, "#xff");
    verify_rule<G>(__LINE__, __FILE__, "(call/cc call/cc)");
    verify_rule<G>(__LINE__, __FILE__, "(call/cc call/cc)");
    verify_rule<G>(__LINE__, __FILE__, "'#0=(#0#)");

    verify_rule<esexpr::comment>(__LINE__, __FILE__, "; comment\n");

    verify_fail<G>(__LINE__, __FILE__, "");
    verify_fail<G>(__LINE__, __FILE__, "(");
    verify_fail<G>(__LINE__, __FILE__, ")");
    verify_fail<G>(__LINE__, __FILE__, "[");
    verify_fail<G>(__LINE__, __FILE__, "]");
    verify_fail<G>(__LINE__, __FILE__, "\"");

    // dotted-pairs are not supported.
    verify_fail<G>(__LINE__, __FILE__, "(1 . 2)");
    verify_fail<G>(__LINE__, __FILE__, ".");

    // radixes other than 2, 10, or 16 are not supported
    verify_fail<G>(__LINE__, __FILE__, "#o641");
    verify_fail<G>(__LINE__, __FILE__, "#24r1k");

    // infinities and NaNs are not supported.
    verify_fail<G>(__LINE__, __FILE__, " 1.0e+INF");
    verify_fail<G>(__LINE__, __FILE__, "+1.0e+INF");
    verify_fail<G>(__LINE__, __FILE__, "-1.0e+INF");
    verify_fail<G>(__LINE__, __FILE__, " 1.0e+NaN");
    verify_fail<G>(__LINE__, __FILE__, "+1.0e+NaN");
    verify_fail<G>(__LINE__, __FILE__, "-1.0e+NaN");

    // infinities and NaNs are case sensitive
    verify_rule<G>(__LINE__, __FILE__, "+1.0e+inf");
    verify_rule<G>(__LINE__, __FILE__, "-1.0e+nan");

    // infinities and NaNs must be followed by delimiter
    verify_rule<G>(__LINE__, __FILE__, "+1.0e+INFi");
    verify_rule<G>(__LINE__, __FILE__, "-1.0e+NaNi");

    // backquotes are not supported
    verify_fail<G>(__LINE__, __FILE__, "`42");
    verify_fail<G>(__LINE__, __FILE__, ",42");

    // 2e4 and 2.0e4 are floats, 2.e4 is a symbol.
    verify_rule<esexpr::float_>(__LINE__, __FILE__, "2e4");
    verify_rule<esexpr::float_>(__LINE__, __FILE__, "2e+4");
    verify_rule<esexpr::float_>(__LINE__, __FILE__, "2e-4");
    verify_rule<esexpr::float_>(__LINE__, __FILE__, "2.0e4");
    verify_rule<esexpr::float_>(__LINE__, __FILE__, "2.e4",
                                result_type::LOCAL_FAILURE, 4);
    verify_rule<esexpr::symbol>(__LINE__, __FILE__, "2.e4");
    verify_rule<esexpr::float_>(__LINE__, __FILE__, ".2e5");

    // 2. is an integer, not a float.
    verify_rule<esexpr::integer>(__LINE__, __FILE__, "2.");
    verify_rule<esexpr::float_>(__LINE__, __FILE__, "2.",
                                result_type::LOCAL_FAILURE, 2);

    // 1+ is a symbol, not an integer.
    verify_rule<esexpr::symbol>(__LINE__, __FILE__, "1+");
    verify_rule<esexpr::integer>(__LINE__, __FILE__, "1+",
                                 result_type::LOCAL_FAILURE, 2);
  }

} // pegtl

#include "main.hh"
