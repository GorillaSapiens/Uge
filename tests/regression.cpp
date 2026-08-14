#include <stdint.h>
#include <stdlib.h>

#include <cmath>
#include <functional>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "uge_n.hpp"
#include "uge_z.hpp"
#include "uge_q.hpp"
#include "uge_c.hpp"

using namespace uge;

static unsigned checks = 0;
static unsigned failures = 0;

static std::string take(char *p) {
   if (!p) return std::string();
   std::string s(p);
   free(p);
   return s;
}

static void fail(const std::string &name, const std::string &detail) {
   failures++;
   std::cerr << "[FAIL] " << name << ": " << detail << "\n";
}

static void expect_true(const std::string &name, bool value) {
   checks++;
   if (!value) fail(name, "condition is false");
}

static void expect_string(const std::string &name,
                          const std::string &got,
                          const std::string &expected) {
   checks++;
   if (got != expected) {
      fail(name, "expected '" + expected + "', got '" + got + "'");
   }
}

static void expect_i64(const std::string &name, int64_t got, int64_t expected) {
   checks++;
   if (got != expected) {
      fail(name, "expected " + std::to_string(expected) +
                 ", got " + std::to_string(got));
   }
}

static void expect_throw(const std::string &name,
                         const std::string &needle,
                         const std::function<void(void)> &fn) {
   checks++;
   try {
      fn();
      fail(name, "expected exception containing '" + needle + "'");
   }
   catch (const std::string &e) {
      if (e.find(needle) == std::string::npos) {
         fail(name, "exception did not contain '" + needle + "': " + e);
      }
   }
}


static Q make_q(int64_t num, int64_t den) {
   return Q((std::to_string(num) + "/" + std::to_string(den)).c_str());
}

static void test_random_properties(void) {
   // Deterministic seed: failures are reproducible in CI.
   std::mt19937_64 rng(UINT64_C(0x5547455245475245));
   const uint64_t radices[] = {2, 3, 10, 12, 16, 36, 37, 256, 65536};

   for (unsigned iter = 0; iter < 1000; ++iter) {
      uint64_t a = rng() % UINT64_C(1000000000);
      uint64_t b = rng() % UINT64_C(1000000000);
      N na(a), nb(b);
      expect_true("N random add", (uint64_t)(na + nb) == a + b);
      expect_true("N random multiply", (uint64_t)(na * nb) == a * b);
      expect_true("N random compare", (na < nb) == (a < b));
      if (a >= b) expect_true("N random subtract", (uint64_t)(na - nb) == a - b);
      if (b != 0) {
         expect_true("N random divide", (uint64_t)(na / nb) == a / b);
         expect_true("N random remainder", (uint64_t)(na % nb) == a % b);
      }

      int64_t za0 = (int64_t)(rng() % 2000001) - 1000000;
      int64_t zb0 = (int64_t)(rng() % 2000001) - 1000000;
      Z za(za0), zb(zb0);
      expect_i64("Z random add", (int64_t)(za + zb), za0 + zb0);
      expect_i64("Z random subtract", (int64_t)(za - zb), za0 - zb0);
      expect_i64("Z random multiply", (int64_t)(za * zb), za0 * zb0);
      expect_true("Z random compare", (za < zb) == (za0 < zb0));
      expect_i64("Z random and", (int64_t)(za & zb), za0 & zb0);
      expect_i64("Z random or", (int64_t)(za | zb), za0 | zb0);
      expect_i64("Z random xor", (int64_t)(za ^ zb), za0 ^ zb0);
      if (zb0 != 0) {
         expect_i64("Z random divide", (int64_t)(za / zb), za0 / zb0);
         expect_i64("Z random remainder", (int64_t)(za % zb), za0 % zb0);
      }

      int64_t an = (int64_t)(rng() % 2001) - 1000;
      int64_t ad = (int64_t)(rng() % 97) + 1;
      int64_t bn = (int64_t)(rng() % 2001) - 1000;
      int64_t bd = (int64_t)(rng() % 97) + 1;
      Q qa = make_q(an, ad);
      Q qb = make_q(bn, bd);
      expect_true("Q random add/sub identity", (qa + qb) - qb == qa);
      expect_true("Q random distributive identity", qa * (qb + Q((int64_t)1)) == qa * qb + qa);
      if (qa.sgn() != 0) expect_true("Q random reciprocal identity", qa / qa == Q((int64_t)1));
   }

   // Round-trip randomly generated values through representative radices,
   // including the multi-character {digit} notation above base 36.
   for (unsigned iter = 0; iter < 100; ++iter) {
      uint64_t nv = rng();
      int64_t zv = (int64_t)(rng() % UINT64_C(2000000001)) - INT64_C(1000000000);
      int64_t qn = (int64_t)(rng() % 2001) - 1000;
      int64_t qd = (int64_t)(rng() % 97) + 1;
      Q q = make_q(qn, qd);
      C c(q, make_q((int64_t)(rng() % 401) - 200,
                    (int64_t)(rng() % 31) + 1));

      for (uint64_t radix : radices) {
         std::string ns = take(N(nv).print(radix));
         expect_true("N random radix round-trip", N(ns.c_str(), radix) == N(nv));

         std::string zs = take(Z(zv).print(radix));
         expect_true("Z random radix round-trip", Z(zs.c_str(), radix) == Z(zv));

         std::string qfs = take(q.frac_print(radix));
         expect_true("Q random fraction radix round-trip", Q(qfs.c_str(), radix) == q);

         std::string qps = take(q.print(radix, 4096));
         expect_true("Q random positional radix round-trip", Q(qps.c_str(), radix) == q);

         std::string crs = take(c.real().frac_print(radix));
         std::string cis = take(c.imag().frac_print(radix));
         C cdecoded(Q(crs.c_str(), radix), Q(cis.c_str(), radix));
         expect_true("C random component radix round-trip", cdecoded == c);
      }
   }
}

static void test_n(void) {
   expect_string("N zero", take(N().print()), "0");
   expect_string("N huge add",
      take((N("18446744073709551616") + N("18446744073709551616")).print()),
      "36893488147419103232");
   expect_string("N multiply", take((N("123456789") * N("987654321")).print()),
      "121932631112635269");
   expect_string("N divide", take((N("100000000000000000000") / N("97")).print()),
      "1030927835051546391");
   expect_string("N remainder", take((N("100000000000000000000") % N("97")).print()),
      "73");
   expect_string("N shift left", take((N("1") << 80).print()),
      "1208925819614629174706176");
   expect_string("N shift right", take((N("1208925819614629174706176") >> 80).print()), "1");
   expect_string("N pow", take(N("12").pow(N("12")).print()), "8916100448256");
   expect_string("N root", take(N("8916100448256").root(N("12")).print()), "12");

   N n("1234567890123456789012345678901234567890");
   const uint64_t radices[] = {2, 3, 8, 10, 12, 16, 36, 256, 65536};
   for (uint64_t radix : radices) {
      std::string encoded = take(n.print(radix));
      N decoded(encoded.c_str(), radix);
      expect_true("N radix round-trip " + std::to_string(radix), decoded == n);
   }

   for (uint64_t a = 0; a <= 40; ++a) {
      for (uint64_t b = 0; b <= 40; ++b) {
         N na(a), nb(b);
         expect_i64("N small add", (int64_t)(uint64_t)(na + nb), (int64_t)(a + b));
         expect_i64("N small multiply", (int64_t)(uint64_t)(na * nb), (int64_t)(a * b));
         expect_true("N small compare", (na < nb) == (a < b));
         if (a >= b) {
            expect_i64("N small subtract", (int64_t)(uint64_t)(na - nb), (int64_t)(a - b));
         }
         if (b != 0) {
            expect_i64("N small divide", (int64_t)(uint64_t)(na / nb), (int64_t)(a / b));
            expect_i64("N small remainder", (int64_t)(uint64_t)(na % nb), (int64_t)(a % b));
         }
      }
   }

   expect_throw("N subtraction underflow", "subtraction underflow", [] { (void)(N("1") - N("2")); });
   expect_throw("N division by zero", "division by zero", [] { (void)(N("1") / N("0")); });
   expect_throw("N unsupported radix", "unsupported radix", [] { N bad("1", 1); (void)bad; });
}

static void test_z(void) {
   expect_string("Z negative zero", take(Z("-0").print()), "0");
   expect_i64("Z negative zero sign", Z("-0").sgn(), 0);
   expect_string("Z add opposite signs", take((Z("-100") + Z("37")).print()), "-63");
   expect_string("Z subtract", take((Z("-100") - Z("37")).print()), "-137");
   expect_string("Z multiply", take((Z("-123456789") * Z("987654321")).print()),
      "-121932631112635269");
   expect_string("Z division truncates toward zero", take((Z("-7") / Z("3")).print()), "-2");
   expect_string("Z remainder follows dividend", take((Z("-7") % Z("3")).print()), "-1");
   expect_string("Z bitwise complement", take((~Z("5")).print()), "-6");
   expect_string("Z arithmetic right shift", take((Z("-9") >> 2).print()), "-3");
   expect_string("Z odd negative root", take(Z("-125").root(N("3")).print()), "-5");

   Z z("-123456789012345678901234567890");
   const uint64_t radices[] = {2, 10, 12, 36, 65536};
   for (uint64_t radix : radices) {
      std::string encoded = take(z.print(radix));
      Z decoded(encoded.c_str(), radix);
      expect_true("Z radix round-trip " + std::to_string(radix), decoded == z);
   }

   for (int64_t a = -25; a <= 25; ++a) {
      for (int64_t b = -25; b <= 25; ++b) {
         Z za(a), zb(b);
         expect_i64("Z small add", (int64_t)(za + zb), a + b);
         expect_i64("Z small subtract", (int64_t)(za - zb), a - b);
         expect_i64("Z small multiply", (int64_t)(za * zb), a * b);
         expect_true("Z small compare", (za < zb) == (a < b));
         expect_i64("Z small and", (int64_t)(za & zb), a & b);
         expect_i64("Z small or", (int64_t)(za | zb), a | b);
         expect_i64("Z small xor", (int64_t)(za ^ zb), a ^ b);
         if (b != 0) {
            expect_i64("Z small divide", (int64_t)(za / zb), a / b);
            expect_i64("Z small remainder", (int64_t)(za % zb), a % b);
         }
      }
   }

   expect_throw("Z division by zero", "division by zero", [] { (void)(Z("1") / Z("0")); });
   expect_throw("Z even root negative", "even root of negative integer", [] { (void)Z("-16").root(N("2")); });
}

static void test_q(void) {
   expect_string("Q simplify", take(Q("10/20").frac_print()), "1/2");
   expect_string("Q exact add", take((Q("1/2") + Q("1/3")).frac_print()), "5/6");
   expect_string("Q exact subtract", take((Q("1/2") - Q("2/3")).frac_print()), "-1/6");
   expect_string("Q exact multiply", take((Q("14/15") * Q("25/21")).frac_print()), "1'1/9");
   expect_string("Q exact divide", take((Q("7/9") / Q("14/15")).frac_print()), "5/6");
   expect_string("Q repeating parse", take(Q("0.8(3)").frac_print()), "5/6");
   expect_string("Q negative repeating parse", take(Q("-0.(3)").frac_print()), "-1/3");
   expect_string("Q repeating print", take(Q("1/7").print()), "0.(142857)");
   expect_string("Q base-12 positional", take(Q("0.5", 12).frac_print(10)), "5/12");
   expect_string("Q negative floor", take(Q("-7/3").floor().frac_print()), "-3");

   Q q("-12345/6789");
   const uint64_t radices[] = {2, 10, 12, 36, 65536};
   for (uint64_t radix : radices) {
      std::string encoded = take(q.frac_print(radix));
      Q decoded(encoded.c_str(), radix);
      expect_true("Q radix fraction round-trip " + std::to_string(radix), decoded == q);
   }

   expect_string("Q sinpi exact", take(Q("1/6").sinpi(128).frac_print()), "1/2");
   expect_string("Q cospi exact", take(Q("1").cospi(128).frac_print()), "-1");
   expect_string("Q sintau exact", take(Q("1/4").sintau(128).frac_print()), "1");
   expect_string("Q costau exact", take(Q("1/2").costau(128).frac_print()), "-1");
   expect_string("Q sindeg exact", take(Q("30").sindeg(128).frac_print()), "1/2");
   expect_string("Q tandeg exact", take(Q("45").tandeg(128).frac_print()), "1");
   expect_string("Q sqrt exact rational", take(Q("9/16").sqrt(128).frac_print()), "3/4");
   expect_true("Q sqrt requested precision",
               std::fabs((double)Q("3").sqrt(64) - std::sqrt(3.0)) < 1e-15);
   Q sqrt3 = Q("3").sqrt(128);
   expect_string("Q sintau exact-companion identity",
                 take((Q("1/6").sintau(128) / sqrt3).frac_print()), "1/2");
   expect_string("Q costau exact-companion identity",
                 take((Q("1/12").costau(128) / sqrt3).frac_print()), "1/2");
   expect_string("Q sinpi inherits exact-companion identity",
                 take((Q("1/3").sinpi(128) / sqrt3).frac_print()), "1/2");

   Q p = Q::pi(96);
   expect_true("Q double conversion for 1/3", std::fabs((double)Q("1/3") - (1.0 / 3.0)) < 1e-15);
   expect_true("Q double conversion for large rational", std::fabs((double)p - 3.14159265358979323846) < 1e-15);
   expect_true("Q pi approximation lower bound", p > Q("3.141592653589793"));
   expect_true("Q pi approximation upper bound", p < Q("3.141592653589794"));
   expect_true("Q tau is exactly twice pi at same precision", Q::tau(96) == p * Q((int64_t)2));
   Q exp1 = Q((int64_t)1).e(96);
   expect_true("Q exp approximation lower bound", exp1 > Q("2.718281828459045"));
   expect_true("Q exp approximation upper bound", exp1 < Q("2.718281828459046"));
   Q ln2 = Q((int64_t)2).ln(96);
   expect_true("Q ln approximation lower bound", ln2 > Q("0.693147180559945"));
   expect_true("Q ln approximation upper bound", ln2 < Q("0.693147180559946"));

   for (int64_t a = -12; a <= 12; ++a) {
      for (int64_t b = -12; b <= 12; ++b) {
         Q qa(a), qb(b);
         expect_i64("Q integer add", (int64_t)(qa + qb), a + b);
         expect_i64("Q integer subtract", (int64_t)(qa - qb), a - b);
         expect_i64("Q integer multiply", (int64_t)(qa * qb), a * b);
         expect_true("Q integer compare", (qa < qb) == (a < b));
         if (b != 0 && a % b == 0) {
            expect_i64("Q exact integer divide", (int64_t)(qa / qb), a / b);
         }
      }
   }

   expect_throw("Q logarithm domain", "logarithm of non-positive number", [] { (void)Q("-1").ln(64); });
   expect_throw("Q negative sqrt", "square root of negative number", [] { (void)Q("-1").sqrt(64); });
   expect_throw("Q atan2 origin", "atan2 undefined", [] { (void)Q("0").atan2(Q("0"), 64); });
}

static void test_c(void) {
   C a(Q((int64_t)1), Q((int64_t)2));
   C b(Q((int64_t)3), Q((int64_t)-4));
   C i(Q((int64_t)0), Q((int64_t)1));

   expect_string("C print", take(a.frac_print()), "1+2i");
   expect_string("C exact add", take((a + b).frac_print()), "4-2i");
   expect_string("C exact multiply", take((a * b).frac_print()), "11+2i");
   expect_string("C exact divide", take((a / b).frac_print()), "-1/5+2/5i");
   expect_string("C conjugate", take(a.conj().frac_print()), "1-2i");
   expect_string("C norm", take(a.norm().frac_print()), "5");
   expect_string("C sqrt negative one", take(C((int64_t)-1).sqrt(128).frac_print()), "i");
   expect_string("C i squared", take((i * i).frac_print()), "-1");
   expect_string("C integer power exact", take(i.pow(C((int64_t)2), 128).frac_print()), "-1");
   expect_string("C real normalized trig", take(C(Q("1/6")).sinpi(128).frac_print()), "1/2");
   C cos_i = i.cos(96);
   C sin_i = i.sin(96);
   expect_true("C cos(i) is real", cos_i.imag().sgn() == 0);
   expect_true("C cos(i) lower bound", cos_i.real() > Q("1.543080634815243"));
   expect_true("C cos(i) upper bound", cos_i.real() < Q("1.543080634815244"));
   expect_true("C sin(i) is imaginary", sin_i.real().sgn() == 0);
   expect_true("C sin(i) lower bound", sin_i.imag() > Q("1.175201193643801"));
   expect_true("C sin(i) upper bound", sin_i.imag() < Q("1.175201193643802"));
   expect_true("C equality both components", a == C(Q((int64_t)1), Q((int64_t)2)));
   expect_true("C inequality imaginary component", a != C(Q((int64_t)1), Q((int64_t)3)));

   expect_throw("C complex ordering rejected", "operation requires a real value", [&] { (void)(a < b); });
   expect_throw("C complex modulo rejected", "operation requires a real value", [&] { (void)(a % b); });
   expect_throw("C division by zero", "division by zero", [&] { (void)(a / C((int64_t)0)); });
}

int main(void) {
   try {
      test_n();
      test_z();
      test_q();
      test_c();
      test_random_properties();
   }
   catch (const std::string &e) {
      fail("uncaught library exception", e);
   }
   catch (const std::exception &e) {
      fail("uncaught std::exception", e.what());
   }
   catch (...) {
      fail("uncaught exception", "unknown exception type");
   }

   if (failures) {
      std::cerr << failures << " failure(s) in " << checks << " checks\n";
      return 1;
   }

   std::cout << "[PASS] library regression: " << checks << " checks\n";
   return 0;
}
