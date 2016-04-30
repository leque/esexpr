/*
 * Copyright (C) 2016  OOHASHI Daichi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef ESEXPR_HH
#define ESEXPR_HH

#include <cstdint>

#include <pegtl.hh>
#include <pegtl/contrib/abnf.hh>

namespace esexpr {
  // -2**29
  const int_least32_t int_min = -536870912;
  // +2**29-1
  const int_least32_t int_max = +536870911;

  using namespace pegtl;

  struct sexpr;

  // space
  //     = [ \t\v\r\n]
  struct space : one<' ', '\t', '\v', '\r', '\n'> {};

  // nl
  //     = '\r\n' | '\r' | '\n'
  struct nl : sor<seq<one<'\r'>, one<'\n'> >, one<'\r'>, one<'\n'> > {};

  // digit
  //     = [0-9]
  using digit = ascii::digit;

  // bit
  //     = [01]
  struct bit : ascii::range<'0', '1'> {};

  // hexdigit
  //     = [0-9a-fA-F]
  using hexdigit = ascii::xdigit;

  struct quote : one<'\''> {};
  struct string_quote : one<'"'> {};
  struct begin_comment : one<';'> {};
  struct begin_list : one<'('> {};
  struct end_list : one<')'> {};
  struct begin_vector : one<'['> {};
  struct end_vector : one<']'> {};

  // comment
  //     = ';' (!nl .)* nl
  struct comment : if_must<begin_comment, star<not_at<nl>, utf8::any>, nl> {};

  // atomosphere
  //     = (space | comment)*
  struct atomosphere : star<sor<space, comment> > {};

  struct delimiter :
    sor<space,
        quote,
        one<'#'>,
        string_quote,
        begin_comment,
        begin_list,
        end_list,
        begin_vector,
        end_vector,
        eof
        > {};

  // quoted
  //     = "'" sexpr
  struct quoted : if_must<quote, atomosphere, sexpr> {};

  // label
  //     = '#' digit+
  struct label : seq<one<'#'>, plus<digit> > {};

  struct label_def : seq<label, one<'='> > {};

  // labeled
  //     = label '=' sexpr
  struct labeled : if_must<label_def, atomosphere, sexpr> {};

  // label_ref
  //     = label '#'
  struct label_ref : if_must<label, one<'#'>, at<delimiter> > {};

  // list
  //     = '(' atomosphere sexpr* ')'
  struct list_content : until<at<end_list>, sexpr> {};
  struct list : seq<begin_list, atomosphere, list_content, must<end_list> >
  {
    using begin = begin_list;
    using end = end_list;
    using element = sexpr;
    using content = list_content;
  };

  // vector
  //     = '[' atomosphere sexpr* ']'
  struct vector_content : until<at<end_vector>, sexpr> {};
  struct vector : seq<begin_vector, atomosphere, vector_content, must<end_vector> >
  {
    using begin = begin_vector;
    using end = end_vector;
    using element = sexpr;
    using content = vector_content;
  };

  // char
  //     = '\\' [\\"abdefntrv]
  //     | '\\u' hexdigit{4}
  //     | '\\U00' hexdigit{6}
  //     | (!'\\' .)
  struct escaped_char :
    one<'"', '\\', 'a', 'b', 'd', 'e', 'f', 'n', 't', 'r', 'v'> {};
  struct unicode :
    sor< seq<one<'u'>, rep<4, must<hexdigit> > >,
         if_must<one<'U'>, rep<2, one<'0'> >, rep<6, hexdigit > >
         >{};
  struct escaped : sor<escaped_char, unicode, utf8::any> {};
  struct unescaped : utf8::any {};
  struct char_ : if_then_else<one<'\\'>, must<escaped>, unescaped> {};

  // string
  //     = [\"] (![\"] char)* [\"]
  struct string_content : until<at<string_quote>, must<char_> > {};
  struct string : seq<string_quote, must<string_content>, any>
  {
    using content = string_content;
  };

  struct character : if_must<one<'?'>, char_, at<delimiter> > {};

  // sign
  //     = ('+' | '-')?
  struct sign : opt<one<'+', '-'> > {};

  struct unsupported_inf : failure {};
  struct unsupported_nan : failure {};

  // exponent
  //     = [Ee] sign [0-9]+
  struct exponent :
    seq<one<'E', 'e'>,
        sign,
        sor<
          seq<pegtl_string_t("INF"), at<delimiter>, must<unsupported_inf> >,
          seq<pegtl_string_t("NaN"), at<delimiter>, must<unsupported_nan> >,
          plus<digit>
          >
        > {};

  // float
  //     = sign [0-9]* '.' [0-9]+ exponent?
  //     | sign [0-9]+ ('.' [0-9]+)? exponent
  struct float_ :
    seq<sign,
        sor<
          seq<star<digit>, one<'.'>, plus<digit>, opt<exponent> >,
          seq<plus<digit>, opt<one<'.'>, plus<digit> >, exponent>
          >,
        at<delimiter> > {};

  // integer2
  //     = '#' [Bb] sign bit+
  struct integer2 :
    seq<one<'#'>, one<'B', 'b'>, sign, plus<bit>, at<delimiter> > {};

  // integer16
  //     = '#' [Xx] sign hexdigit+
  struct integer16 :
    seq<one<'#'>, one<'X', 'x'>, sign, plus<hexdigit>, at<delimiter> > {};

  // integer10
  //     = sign [0-9]+ '.'?
  struct integer10 :
    seq<sign, plus<digit>, opt<one<'.'> >, at<delimiter> > {};

  // integer
  //     = integer2
  //     | integer16
  //     | integer10
  struct integer : sor<integer2, integer16, integer10> {};

  // symbol_char
  //     = [!$%&*+-./0-9:<=>?@A-Z^_a-z{}~]
  //     | '\\' .
  struct symbol_char :
    if_then_else<one<'\\'>,
                 must<utf8::any>,
                 sor<one<'!', '$', '%', '&', '*', '+', '-', '=', '.', '/',
                         ':', '<', '=', '>', '?', '@', '^', '_', '{', '}', '~'>,
                     ascii::alnum> > {};

  struct unsupported_dot : failure {};

  // symbol
  //     = symbol_char+
  // NB: `.' is not a symbol.
  struct symbol :
    sor<seq<one<'.'>, at<delimiter>, must<unsupported_dot> >,
        must<symbol_char,
             until<at<delimiter>, must<symbol_char> >,
             at<delimiter>
             > > {};

  // atom
  //     = character
  //     | float
  //     | integer
  //     | symbol
  struct atom : sor<character, float_, integer, symbol> {};

  // sexpr
  //     = (quoted | labeled | label_ref | list | vector | string | atom)
  struct sexpr0 : sor<quoted, labeled, label_ref, list, vector, string, atom> {};
  struct sexpr : must<sexpr0, atomosphere> {};

  struct sexpr1 : must<atomosphere, sexpr, eof> {};

  struct sexprs : must<atomosphere, star<sexpr>, eof> {};
}

#endif /* ESEXPR_HH */
