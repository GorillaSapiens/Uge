#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "gcstr.hpp"
#include "uge_q.hpp"

using namespace uge;

namespace {

static const uint64_t DEFAULT_PRINT_MAX = 1024;
static const uint64_t DEFAULT_PRECISION = 256;
static const uint64_t MAX_RADIX = 65536;
static const size_t MAX_HISTORY = 1000;

struct Context {
   uint64_t ibase;
   uint64_t obase;
   uint64_t print_max;
   uint64_t precision;
   std::map<std::string, Q> vars;
   Q last;

   Context()
      : ibase(10), obase(10), print_max(DEFAULT_PRINT_MAX),
        precision(DEFAULT_PRECISION), last((int64_t)0) {}
};

static std::string trim(const std::string &s) {
   size_t a = 0;
   while (a < s.size() && isspace((unsigned char)s[a])) a++;
   size_t b = s.size();
   while (b > a && isspace((unsigned char)s[b - 1])) b--;
   return s.substr(a, b - a);
}

static bool starts_word(const std::string &s, const std::string &word,
                        std::string &rest) {
   if (s.compare(0, word.size(), word) != 0) return false;
   if (s.size() == word.size()) {
      rest.clear();
      return true;
   }
   if (!isspace((unsigned char)s[word.size()])) return false;
   rest = trim(s.substr(word.size()));
   return true;
}

static bool q_is_zero(const Q &q) {
   return q.sgn() == 0;
}

static int64_t q_to_whole(const Q &q, const char *what) {
   if (q != q.floor()) {
      throw std::string(what) + " must be a whole number";
   }
   return (int64_t)q;
}

static Q q_bool(bool b) {
   return Q((int64_t)(b ? 1 : 0));
}

static void check_divisor(const Q &q) {
   if (q_is_zero(q)) {
      throw std::string("divide by zero");
   }
}

static Q q_pow(const Q &base, const Q &power, uint64_t precision) {
   if (power < Q((int64_t)0)) {
      check_divisor(base);
      Q positive = -power;
      Q one((int64_t)1);
      return one / base.pow(positive, precision);
   }
   return base.pow(power, precision);
}

static bool is_config_name(const std::string &name) {
   return name == "ibase" || name == "obase" || name == "base" ||
          name == "maxdigits" || name == "precision";
}

static Q config_value(Context &ctx, const std::string &name) {
   if (name == "ibase") return Q((int64_t)ctx.ibase);
   if (name == "obase") return Q((int64_t)ctx.obase);
   if (name == "base") return Q((int64_t)ctx.ibase);
   if (name == "maxdigits") return Q((int64_t)ctx.print_max);
   if (name == "precision") return Q((int64_t)ctx.precision);
   if (name == "last") return ctx.last;
   std::map<std::string, Q>::iterator i = ctx.vars.find(name);
   if (i == ctx.vars.end()) return Q((int64_t)0);
   return i->second;
}

static void set_named_value(Context &ctx, const std::string &name, const Q &value) {
   if (name == "last") {
      ctx.last = value;
      return;
   }
   if (!is_config_name(name)) {
      ctx.vars[name] = value;
      return;
   }

   int64_t n = q_to_whole(value, name.c_str());
   if (name == "ibase" || name == "obase" || name == "base") {
      if (n < 2 || n > (int64_t)MAX_RADIX) {
         std::ostringstream os;
         os << name << " must be between 2 and " << MAX_RADIX;
         throw os.str();
      }
      if (name == "ibase" || name == "base") ctx.ibase = (uint64_t)n;
      if (name == "obase" || name == "base") ctx.obase = (uint64_t)n;
      return;
   }

   if (n < 1 || n > 1000000) {
      throw std::string(name) + " must be between 1 and 1000000";
   }
   if (name == "maxdigits") ctx.print_max = (uint64_t)n;
   if (name == "precision") ctx.precision = (uint64_t)n;
}

struct Token {
   enum Kind { END, NUMBER, IDENT, OP, LPAREN, RPAREN, COMMA } kind;
   std::string text;
   size_t at;

   Token(Kind k = END, const std::string &t = "", size_t p = 0)
      : kind(k), text(t), at(p) {}
};

class Lexer {
   const std::string &src;
   size_t pos;
   uint64_t radix;

   bool digit_at(size_t &p, uint32_t &digit, std::string &raw) const {
      if (p >= src.size()) return false;
      size_t start = p;
      unsigned char c = (unsigned char)src[p];
      if (c >= '0' && c <= '9') {
         digit = c - '0';
         p++;
      }
      else if (c >= 'A' && c <= 'Z') {
         if (c == 'E' && radix <= 14) return false;
         digit = c - 'A' + 10;
         p++;
      }
      else if (c == '{') {
         size_t q = p + 1;
         if (q >= src.size() || !isdigit((unsigned char)src[q])) {
            throw std::string("expected decimal digit value after '{'");
         }
         uint64_t v = 0;
         while (q < src.size() && isdigit((unsigned char)src[q])) {
            v = v * 10 + (src[q] - '0');
            if (v > 65535) throw std::string("digit value exceeds 65535");
            q++;
         }
         if (q >= src.size() || src[q] != '}') {
            throw std::string("missing '}' in radix digit");
         }
         digit = (uint32_t)v;
         p = q + 1;
      }
      else {
         return false;
      }
      raw.assign(src, start, p - start);
      if (digit >= radix) {
         std::ostringstream os;
         os << "digit " << raw << " is not valid in base " << radix;
         throw os.str();
      }
      return true;
   }

   bool numeric_start(size_t p) const {
      if (p >= src.size()) return false;
      unsigned char c = (unsigned char)src[p];
      if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || c == '{') return true;
      if (c == '.') {
         if (p + 1 >= src.size()) return false;
         unsigned char d = (unsigned char)src[p + 1];
         return (d >= '0' && d <= '9') || (d >= 'A' && d <= 'Z') || d == '{' || d == '(';
      }
      return false;
   }

   void scan_digits(size_t &p, bool require_one) const {
      bool any = false;
      for (;;) {
         size_t q = p;
         uint32_t d;
         std::string raw;
         if (!digit_at(q, d, raw)) break;
         p = q;
         any = true;
      }
      if (require_one && !any) throw std::string("expected radix digit");
   }

   Token scan_number() {
      size_t start = pos;
      bool have_int = false;
      for (;;) {
         size_t q = pos;
         uint32_t d;
         std::string raw;
         if (!digit_at(q, d, raw)) break;
         pos = q;
         have_int = true;
      }

      if (pos < src.size() && src[pos] == '\'') {
         if (!have_int) throw std::string("mixed fraction requires a whole part");
         pos++;
         scan_digits(pos, true);
         if (pos >= src.size() || src[pos] != '/') {
            throw std::string("mixed fraction requires '/'");
         }
         pos++;
         scan_digits(pos, true);
         return Token(Token::NUMBER, src.substr(start, pos - start), start);
      }

      if (pos < src.size() && src[pos] == '.') {
         pos++;
         scan_digits(pos, false);
      }
      else if (!have_int) {
         throw std::string("expected number");
      }

      if (pos < src.size() && src[pos] == '(') {
         pos++;
         scan_digits(pos, true);
         if (pos >= src.size() || src[pos] != ')') {
            throw std::string("missing ')' in repeating fraction");
         }
         pos++;
      }

      // Q supports scientific notation only while E is not a radix digit.
      if (radix <= 14 && pos < src.size() && (src[pos] == 'e' || src[pos] == 'E')) {
         pos++;
         if (pos < src.size() && (src[pos] == '+' || src[pos] == '-')) pos++;
         scan_digits(pos, true);
      }

      return Token(Token::NUMBER, src.substr(start, pos - start), start);
   }

public:
   Lexer(const std::string &s, uint64_t r) : src(s), pos(0), radix(r) {}

   uint64_t get_radix() const { return radix; }
   void set_radix(uint64_t r) { radix = r; }

   Token next() {
      while (pos < src.size() && isspace((unsigned char)src[pos])) pos++;
      if (pos >= src.size()) return Token(Token::END, "", pos);

      size_t at = pos;
      if (numeric_start(pos)) return scan_number();

      unsigned char c = (unsigned char)src[pos];
      if ((c >= 'a' && c <= 'z') || c == '_') {
         pos++;
         while (pos < src.size()) {
            unsigned char d = (unsigned char)src[pos];
            if (!((d >= 'a' && d <= 'z') || (d >= '0' && d <= '9') || d == '_')) break;
            pos++;
         }
         return Token(Token::IDENT, src.substr(at, pos - at), at);
      }

      if (c == '(') { pos++; return Token(Token::LPAREN, "(", at); }
      if (c == ')') { pos++; return Token(Token::RPAREN, ")", at); }
      if (c == ',') { pos++; return Token(Token::COMMA, ",", at); }

      static const char *two_ops[] = {
         "++", "--", "==", "!=", "<=", ">=", "&&", "||",
         "<<", ">>", "+=", "-=", "*=", "/=", "%=", "^=", "**"
      };
      for (size_t i = 0; i < sizeof(two_ops) / sizeof(two_ops[0]); i++) {
         if (src.compare(pos, 2, two_ops[i]) == 0) {
            pos += 2;
            return Token(Token::OP, two_ops[i], at);
         }
      }

      if (strchr("+-*/%^<>=!~&|", c)) {
         pos++;
         return Token(Token::OP, std::string(1, (char)c), at);
      }

      std::ostringstream os;
      os << "unexpected character '" << (char)c << "'";
      throw os.str();
   }
};

struct Value {
   Q q;
   std::string lvalue;
   bool assignment;

   Value(const Q &v = Q((int64_t)0), const std::string &l = "", bool a = false)
      : q(v), lvalue(l), assignment(a) {}
};

class Parser {
   Context &ctx;
   Lexer lex;
   Token tok;

   void advance() { tok = lex.next(); }

   bool op(const char *s) const {
      return tok.kind == Token::OP && tok.text == s;
   }

   bool take_op(const char *s) {
      if (!op(s)) return false;
      advance();
      return true;
   }

   void expect(Token::Kind k, const char *what) {
      if (tok.kind != k) {
         std::ostringstream os;
         os << "expected " << what << " near column " << (tok.at + 1);
         throw os.str();
      }
   }

   Value parse_assignment() {
      Value left = parse_shift();
      if (!(op("=") || op("+=") || op("-=") || op("*=") || op("/=") ||
            op("%=") || op("^="))) {
         return left;
      }
      if (left.lvalue.empty()) throw std::string("left side of assignment is not a variable");

      std::string assignop = tok.text;
      bool decimal_rhs = is_config_name(left.lvalue);
      if (decimal_rhs) lex.set_radix(10);
      advance();
      Value right = parse_assignment();

      Q result = right.q;
      if (assignop != "=") {
         Q old = left.q;
         if (assignop == "+=") result = old + right.q;
         else if (assignop == "-=") result = old - right.q;
         else if (assignop == "*=") result = old * right.q;
         else if (assignop == "/=") { check_divisor(right.q); result = old / right.q; }
         else if (assignop == "%=") { check_divisor(right.q); result = old % right.q; }
         else if (assignop == "^=") result = q_pow(old, right.q, ctx.precision);
      }

      set_named_value(ctx, left.lvalue, result);
      lex.set_radix(ctx.ibase);
      left.q = result;
      left.lvalue.clear();
      left.assignment = true;
      return left;
   }

   Value parse_logical_or() {
      Value v = parse_logical_and();
      while (take_op("||")) {
         Value r = parse_logical_and();
         v = Value(q_bool(!q_is_zero(v.q) || !q_is_zero(r.q)));
      }
      return v;
   }

   Value parse_logical_and() {
      Value v = parse_not();
      while (take_op("&&")) {
         Value r = parse_not();
         v = Value(q_bool(!q_is_zero(v.q) && !q_is_zero(r.q)));
      }
      return v;
   }

   Value parse_not() {
      if (take_op("!")) {
         Value v = parse_bit_or();
         return Value(q_bool(q_is_zero(v.q)));
      }
      return parse_bit_or();
   }

   Value parse_bit_or() {
      Value v = parse_bit_and();
      while (take_op("|")) {
         Value r = parse_bit_and();
         v = Value(v.q | r.q);
      }
      return v;
   }

   Value parse_bit_and() {
      Value v = parse_compare();
      while (take_op("&")) {
         Value r = parse_compare();
         v = Value(v.q & r.q);
      }
      return v;
   }

   Value parse_compare() {
      Value v = parse_assignment();
      for (;;) {
         if (take_op("==")) { Value r = parse_assignment(); v = Value(q_bool(v.q == r.q)); }
         else if (take_op("!=")) { Value r = parse_assignment(); v = Value(q_bool(v.q != r.q)); }
         else if (take_op("<=")) { Value r = parse_assignment(); v = Value(q_bool(v.q <= r.q)); }
         else if (take_op(">=")) { Value r = parse_assignment(); v = Value(q_bool(v.q >= r.q)); }
         else if (take_op("<")) { Value r = parse_assignment(); v = Value(q_bool(v.q < r.q)); }
         else if (take_op(">")) { Value r = parse_assignment(); v = Value(q_bool(v.q > r.q)); }
         else break;
      }
      return v;
   }

   Value parse_shift() {
      Value v = parse_add();
      for (;;) {
         if (take_op("<<")) {
            Value r = parse_add();
            int64_t n = q_to_whole(r.q, "shift count");
            if (n < 0) throw std::string("shift count must not be negative");
            v = Value(v.q << n);
         }
         else if (take_op(">>")) {
            Value r = parse_add();
            int64_t n = q_to_whole(r.q, "shift count");
            if (n < 0) throw std::string("shift count must not be negative");
            v = Value(v.q >> n);
         }
         else break;
      }
      return v;
   }

   Value parse_add() {
      Value v = parse_mul();
      for (;;) {
         if (take_op("+")) { Value r = parse_mul(); v = Value(v.q + r.q); }
         else if (take_op("-")) { Value r = parse_mul(); v = Value(v.q - r.q); }
         else break;
      }
      return v;
   }

   Value parse_mul() {
      Value v = parse_power();
      for (;;) {
         if (take_op("*")) { Value r = parse_power(); v = Value(v.q * r.q); }
         else if (take_op("/")) { Value r = parse_power(); check_divisor(r.q); v = Value(v.q / r.q); }
         else if (take_op("%")) { Value r = parse_power(); check_divisor(r.q); v = Value(v.q % r.q); }
         else break;
      }
      return v;
   }

   Value parse_power() {
      Value v = parse_unary();
      if (op("^") || op("**")) {
         advance();
         Value r = parse_power();
         v = Value(q_pow(v.q, r.q, ctx.precision));
      }
      return v;
   }

   Value parse_unary() {
      if (take_op("+")) return Value(+parse_unary().q);
      if (take_op("-")) return Value(-parse_unary().q);
      if (take_op("~")) return Value(~parse_unary().q);
      // Prefix ++/-- need to preserve which operator before advancing.
      if (op("++") || op("--")) {
         std::string which = tok.text;
         advance();
         Value v = parse_unary();
         if (v.lvalue.empty()) throw std::string("increment requires a variable");
         Q n = v.q + Q((int64_t)(which == "++" ? 1 : -1));
         set_named_value(ctx, v.lvalue, n);
         lex.set_radix(ctx.ibase);
         return Value(n);
      }

      return parse_postfix();
   }

   Value parse_postfix() {
      Value v = parse_primary();
      while (op("++") || op("--")) {
         if (v.lvalue.empty()) throw std::string("increment requires a variable");
         std::string which = tok.text;
         advance();
         Q old = v.q;
         Q n = old + Q((int64_t)(which == "++" ? 1 : -1));
         set_named_value(ctx, v.lvalue, n);
         lex.set_radix(ctx.ibase);
         v = Value(old);
      }
      return v;
   }

   std::vector<Q> parse_args() {
      std::vector<Q> args;
      expect(Token::LPAREN, "'('");
      advance();
      if (tok.kind == Token::RPAREN) {
         advance();
         return args;
      }
      for (;;) {
         args.push_back(parse_logical_or().q);
         if (tok.kind == Token::COMMA) {
            advance();
            continue;
         }
         expect(Token::RPAREN, "')'");
         advance();
         return args;
      }
   }

   Value call_function(const std::string &name) {
      std::vector<Q> a = parse_args();
      if (name == "sqrt") {
         if (a.size() != 1) throw std::string("sqrt() takes one argument");
         return Value(a[0].sqrt(ctx.precision));
      }
      if (name == "abs") {
         if (a.size() != 1) throw std::string("abs() takes one argument");
         return Value(a[0].abs());
      }
      if (name == "floor") {
         if (a.size() != 1) throw std::string("floor() takes one argument");
         return Value(a[0].floor());
      }
      if (name == "sgn") {
         if (a.size() != 1) throw std::string("sgn() takes one argument");
         return Value(Q((int64_t)a[0].sgn()));
      }
      if (name == "pow") {
         if (a.size() != 2) throw std::string("pow() takes two arguments");
         return Value(q_pow(a[0], a[1], ctx.precision));
      }
      if (name == "xor") {
         if (a.size() != 2) throw std::string("xor() takes two arguments");
         return Value(a[0] ^ a[1]);
      }
      throw std::string("unknown function '") + name + "'";
   }

   Value parse_primary() {
      if (tok.kind == Token::NUMBER) {
         std::string text = tok.text;
         uint64_t r = lex.get_radix();
         advance();
         return Value(Q(text.c_str(), r));
      }

      if (tok.kind == Token::IDENT) {
         std::string name = tok.text;
         advance();
         if (tok.kind == Token::LPAREN) return call_function(name);
         return Value(config_value(ctx, name), name);
      }

      if (tok.kind == Token::LPAREN) {
         advance();
         Value v = parse_logical_or();
         expect(Token::RPAREN, "')'");
         advance();
         v.lvalue.clear();
         v.assignment = false;
         return v;
      }

      std::ostringstream os;
      os << "expected a number, variable, or '(' near column " << (tok.at + 1);
      throw os.str();
   }

public:
   Parser(Context &c, const std::string &s, uint64_t radix)
      : ctx(c), lex(s, radix) { advance(); }

   Value parse() {
      Value v = parse_logical_or();
      if (tok.kind != Token::END) {
         std::ostringstream os;
         os << "unexpected '" << tok.text << "' near column " << (tok.at + 1);
         throw os.str();
      }
      return v;
   }
};

static bool wrapped_call(const std::string &s, const char *name, std::string &inside) {
   size_t n = strlen(name);
   if (s.compare(0, n, name) != 0) return false;
   size_t p = n;
   while (p < s.size() && isspace((unsigned char)s[p])) p++;
   if (p >= s.size() || s[p] != '(') return false;

   int depth = 0;
   size_t close = std::string::npos;
   for (size_t i = p; i < s.size(); i++) {
      if (s[i] == '(') depth++;
      else if (s[i] == ')') {
         depth--;
         if (depth == 0) {
            close = i;
            break;
         }
      }
   }
   if (close == std::string::npos || !trim(s.substr(close + 1)).empty()) return false;
   inside = s.substr(p + 1, close - p - 1);
   return true;
}

static void output_positional(Context &ctx, const Q &q, uint64_t radix) {
   printf("%s\n", GCSTR q.print(radix, ctx.print_max));
}

static bool starts_assignment_statement(const std::string &s) {
   size_t p = 0;
   while (p < s.size() && isspace((unsigned char)s[p])) p++;
   if (p >= s.size() || !((s[p] >= 'a' && s[p] <= 'z') || s[p] == '_')) return false;
   p++;
   while (p < s.size()) {
      unsigned char c = (unsigned char)s[p];
      if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_')) break;
      p++;
   }
   while (p < s.size() && isspace((unsigned char)s[p])) p++;
   static const char *ops[] = { "=", "+=", "-=", "*=", "/=", "%=", "^=" };
   for (size_t i = 0; i < sizeof(ops) / sizeof(ops[0]); i++) {
      size_t n = strlen(ops[i]);
      if (s.compare(p, n, ops[i]) == 0) {
         if (ops[i][0] == '=' && p + 1 < s.size() && s[p + 1] == '=') continue;
         return true;
      }
   }
   return false;
}

static bool execute_statement(Context &ctx, std::string stmt) {
   stmt = trim(stmt);
   if (stmt.empty()) return true;

   if (stmt == "quit" || stmt == "halt") return false;
   if (stmt == "help") {
      printf("Type expressions using bc-like syntax.  See UGE.md for full help.\n");
      printf("ibase/obase/base assignments are always interpreted in decimal.\n");
      printf("Output: positional(x), fraction(x), decimal(x).\n");
      printf("maxdigits controls rendering; precision controls approximations.\n");
      return true;
   }
   if (stmt == "warranty") {
      printf("Uge is free software; see LICENSE for copying and warranty terms.\n");
      return true;
   }

   // Friendly command forms inherited from ztest/qtest.  The Parser's
   // special-variable assignment path deliberately parses their RHS in base 10.
   static const char *commands[] = { "ibase", "obase", "base", "maxdigits", "precision" };
   for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
      std::string rest;
      if (starts_word(stmt, commands[i], rest) && !rest.empty()) {
         stmt = std::string(commands[i]) + "=" + rest;
         break;
      }
   }

   bool assignment_statement = starts_assignment_statement(stmt);

   std::string inside;
   enum OutMode { NORMAL, POSITIONAL, FRACTION, DECIMAL, DEBUG } mode = NORMAL;
   if (wrapped_call(stmt, "fraction", inside) || wrapped_call(stmt, "frac", inside)) {
      mode = FRACTION;
      stmt = inside;
   }
   else if (wrapped_call(stmt, "positional", inside) || wrapped_call(stmt, "pos", inside)) {
      mode = POSITIONAL;
      stmt = inside;
   }
   else if (wrapped_call(stmt, "decimal", inside)) {
      mode = DECIMAL;
      stmt = inside;
   }
   else if (wrapped_call(stmt, "debug", inside)) {
      mode = DEBUG;
      stmt = inside;
   }
   else {
      std::string rest;
      if (starts_word(stmt, "print", rest) && !rest.empty()) {
         mode = POSITIONAL;
         stmt = rest;
      }
   }

   Parser p(ctx, stmt, ctx.ibase);
   Value v = p.parse();

   if (mode == FRACTION) {
      printf("%s\n", GCSTR v.q.frac_print(ctx.obase));
      ctx.last = v.q;
   }
   else if (mode == POSITIONAL) {
      output_positional(ctx, v.q, ctx.obase);
      ctx.last = v.q;
   }
   else if (mode == DECIMAL) {
      output_positional(ctx, v.q, 10);
      ctx.last = v.q;
   }
   else if (mode == DEBUG) {
      printf("%s\n", GCSTR v.q.debu_print());
      ctx.last = v.q;
   }
   else if (!assignment_statement) {
      output_positional(ctx, v.q, ctx.obase);
      ctx.last = v.q;
   }

   return true;
}

class CommentStripper {
   bool block;
public:
   CommentStripper() : block(false) {}

   std::string strip(const std::string &line) {
      std::string out;
      for (size_t i = 0; i < line.size();) {
         if (block) {
            size_t p = line.find("*/", i);
            if (p == std::string::npos) return out;
            block = false;
            i = p + 2;
            continue;
         }
         if (i + 1 < line.size() && line[i] == '/' && line[i + 1] == '*') {
            block = true;
            i += 2;
            continue;
         }
         if (line[i] == '#') break;
         if (i + 1 < line.size() && line[i] == '/' && line[i + 1] == '/') break;
         out += line[i++];
      }
      return out;
   }
};

static std::vector<std::string> split_statements(const std::string &line) {
   std::vector<std::string> out;
   size_t start = 0;
   int paren = 0;
   for (size_t i = 0; i < line.size(); i++) {
      if (line[i] == '(') paren++;
      else if (line[i] == ')' && paren > 0) paren--;
      else if (line[i] == ';' && paren == 0) {
         out.push_back(line.substr(start, i - start));
         start = i + 1;
      }
   }
   out.push_back(line.substr(start));
   return out;
}

static int paren_balance(const std::string &s) {
   int n = 0;
   for (size_t i = 0; i < s.size(); i++) {
      if (s[i] == '(') n++;
      else if (s[i] == ')') n--;
   }
   return n;
}

static void redraw_line(const std::string &prompt, const std::string &line, size_t cursor) {
   std::string out = "\r" + prompt + line + "\x1b[K";
   size_t back = line.size() - cursor;
   if (back) {
      char buf[64];
      snprintf(buf, sizeof(buf), "\x1b[%zuD", back);
      out += buf;
   }
   (void)!write(STDOUT_FILENO, out.data(), out.size());
}

class RawTerminal {
   bool active;
   struct termios oldt;
public:
   RawTerminal() : active(false) {
      if (!isatty(STDIN_FILENO)) return;
      if (tcgetattr(STDIN_FILENO, &oldt) != 0) return;
      struct termios t = oldt;
      t.c_lflag &= ~(ICANON | ECHO | ISIG);
      t.c_iflag &= ~(IXON | ICRNL);
      t.c_cc[VMIN] = 1;
      t.c_cc[VTIME] = 0;
      if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &t) == 0) active = true;
   }
   ~RawTerminal() {
      if (active) tcsetattr(STDIN_FILENO, TCSAFLUSH, &oldt);
   }
   bool ok() const { return active; }
};

static bool read_byte(char &c) {
   for (;;) {
      ssize_t n = read(STDIN_FILENO, &c, 1);
      if (n == 1) return true;
      if (n == 0) return false;
      if (errno != EINTR) return false;
   }
}

static bool read_interactive_line(const std::string &prompt,
                                  std::vector<std::string> &history,
                                  std::string &result) {
   RawTerminal raw;
   if (!raw.ok()) return (bool)std::getline(std::cin, result);

   std::string line;
   std::string saved;
   size_t cursor = 0;
   size_t hist = history.size();
   (void)!write(STDOUT_FILENO, prompt.data(), prompt.size());

   for (;;) {
      char c;
      if (!read_byte(c)) {
         (void)!write(STDOUT_FILENO, "\n", 1);
         return false;
      }

      if (c == '\r' || c == '\n') {
         (void)!write(STDOUT_FILENO, "\r\n", 2);
         result = line;
         return true;
      }
      if (c == 4) { // Ctrl-D
         if (line.empty()) {
            (void)!write(STDOUT_FILENO, "\r\n", 2);
            return false;
         }
         continue;
      }
      if (c == 3) { // Ctrl-C: cancel current line without killing the shell's tty state
         (void)!write(STDOUT_FILENO, "^C\r\n", 4);
         result.clear();
         return true;
      }
      if (c == 1) { cursor = 0; redraw_line(prompt, line, cursor); continue; } // Ctrl-A
      if (c == 5) { cursor = line.size(); redraw_line(prompt, line, cursor); continue; } // Ctrl-E
      if (c == 11) { line.erase(cursor); redraw_line(prompt, line, cursor); continue; } // Ctrl-K
      if (c == 21) { line.erase(0, cursor); cursor = 0; redraw_line(prompt, line, cursor); continue; } // Ctrl-U
      if (c == 12) { // Ctrl-L
         const char *clear = "\x1b[2J\x1b[H";
         (void)!write(STDOUT_FILENO, clear, strlen(clear));
         redraw_line(prompt, line, cursor);
         continue;
      }
      if (c == 127 || c == 8) {
         if (cursor) {
            line.erase(cursor - 1, 1);
            cursor--;
            redraw_line(prompt, line, cursor);
         }
         continue;
      }
      if (c == 27) {
         char a, b;
         if (!read_byte(a) || a != '[' || !read_byte(b)) continue;
         if (b == 'A') { // up
            if (!history.empty() && hist > 0) {
               if (hist == history.size()) saved = line;
               hist--;
               line = history[hist];
               cursor = line.size();
               redraw_line(prompt, line, cursor);
            }
         }
         else if (b == 'B') { // down
            if (hist < history.size()) {
               hist++;
               line = hist == history.size() ? saved : history[hist];
               cursor = line.size();
               redraw_line(prompt, line, cursor);
            }
         }
         else if (b == 'C') { if (cursor < line.size()) cursor++; redraw_line(prompt, line, cursor); }
         else if (b == 'D') { if (cursor) cursor--; redraw_line(prompt, line, cursor); }
         else if (b == 'H') { cursor = 0; redraw_line(prompt, line, cursor); }
         else if (b == 'F') { cursor = line.size(); redraw_line(prompt, line, cursor); }
         else if (b == '3') { // delete: ESC [ 3 ~
            char tilde;
            if (read_byte(tilde) && tilde == '~' && cursor < line.size()) {
               line.erase(cursor, 1);
               redraw_line(prompt, line, cursor);
            }
         }
         continue;
      }

      if ((unsigned char)c >= 32) {
         line.insert(line.begin() + cursor, c);
         cursor++;
         redraw_line(prompt, line, cursor);
      }
   }
}

static std::string history_path() {
   const char *home = getenv("HOME");
   if (!home || !*home) return "";
   return std::string(home) + "/.uge_history";
}

static void load_history(std::vector<std::string> &h) {
   std::string path = history_path();
   if (path.empty()) return;
   std::ifstream f(path.c_str());
   std::string s;
   while (std::getline(f, s)) {
      if (!s.empty()) h.push_back(s);
      if (h.size() > MAX_HISTORY) h.erase(h.begin());
   }
}

static void save_history(const std::vector<std::string> &h) {
   std::string path = history_path();
   if (path.empty()) return;
   std::ofstream f(path.c_str(), std::ios::trunc);
   size_t start = h.size() > MAX_HISTORY ? h.size() - MAX_HISTORY : 0;
   for (size_t i = start; i < h.size(); i++) f << h[i] << '\n';
}

static bool process_line(Context &ctx, CommentStripper &comments,
                         std::string &pending, const std::string &raw) {
   std::string line = comments.strip(raw);

   bool continuation = false;
   size_t end = line.find_last_not_of(" \t\r");
   if (end != std::string::npos && line[end] == '\\') {
      line.erase(end);
      continuation = true;
   }

   if (!pending.empty()) pending += " ";
   pending += line;

   if (continuation || paren_balance(pending) > 0) return true;

   std::vector<std::string> stmts = split_statements(pending);
   pending.clear();
   for (size_t i = 0; i < stmts.size(); i++) {
      try {
         if (!execute_statement(ctx, stmts[i])) return false;
      }
      catch (const std::string &e) {
         fprintf(stderr, "uge: %s\n", e.c_str());
      }
      catch (const std::exception &e) {
         fprintf(stderr, "uge: %s\n", e.what());
      }
   }
   return true;
}

static bool process_stream(Context &ctx, std::istream &in) {
   CommentStripper comments;
   std::string pending, line;
   while (std::getline(in, line)) {
      if (!process_line(ctx, comments, pending, line)) return false;
   }
   if (!trim(pending).empty()) {
      try {
         if (!execute_statement(ctx, pending)) return false;
      }
      catch (const std::string &e) {
         fprintf(stderr, "uge: %s\n", e.c_str());
      }
   }
   return true;
}

static void usage(const char *argv0) {
   printf("usage: %s [-q] [-l] [file ...]\n", argv0);
   printf("  -q, --quiet   suppress interactive banner\n");
   printf("  -l            accepted for bc compatibility; Uge functions are built in\n");
   printf("  -h, --help    show this help\n");
}

} // namespace

int main(int argc, char **argv) {
   bool quiet = false;
   std::vector<std::string> files;

   for (int i = 1; i < argc; i++) {
      std::string a = argv[i];
      if (a == "-q" || a == "--quiet") quiet = true;
      else if (a == "-l") { /* compatibility no-op */ }
      else if (a == "-h" || a == "--help") { usage(argv[0]); return 0; }
      else if (!a.empty() && a[0] == '-') {
         fprintf(stderr, "uge: unknown option '%s'\n", a.c_str());
         usage(argv[0]);
         return 2;
      }
      else files.push_back(a);
   }

   Context ctx;
   for (size_t i = 0; i < files.size(); i++) {
      std::ifstream f(files[i].c_str());
      if (!f) {
         fprintf(stderr, "uge: cannot open '%s'\n", files[i].c_str());
         return 1;
      }
      if (!process_stream(ctx, f)) return 0;
   }

   bool interactive = isatty(STDIN_FILENO);
   if (!interactive) {
      process_stream(ctx, std::cin);
      return 0;
   }

   if (!quiet) {
      printf("uge exact rational calculator\n");
      printf("Copyright (C) GorillaSapiens; type 'help' for help.\n");
   }

   std::vector<std::string> history;
   load_history(history);
   CommentStripper comments;
   std::string pending;
   bool running = true;

   while (running) {
      std::string line;
      const std::string prompt = ""; // GNU bc normally has no prompt.
      if (!read_interactive_line(prompt, history, line)) break;
      if (!line.empty() && (history.empty() || history.back() != line)) {
         history.push_back(line);
         if (history.size() > MAX_HISTORY) history.erase(history.begin());
      }
      running = process_line(ctx, comments, pending, line);
   }

   save_history(history);
   return 0;
}

// vim: expandtab:noai:ts=3:sw=3
