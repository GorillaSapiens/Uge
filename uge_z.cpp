#include <inttypes.h>
#include <limits.h>
#include <string.h>

#include <string>

#include "uge_err.hpp"
#include "uge_ramprintf.hpp"
#include "gcstr.hpp"
#include "uge_z.hpp"

using namespace uge;

void Z::fixZero(void) {
   if (mag.isZero()) {
      pos = true;
   }
}

Z::Z() : pos(true), mag() {
}

Z::Z(const Z &orig) : pos(orig.pos), mag(orig.mag) {
   fixZero();
}

Z::Z(bool p, const N &m) : pos(p), mag(m) {
   fixZero();
}

Z::Z(const N &n) : pos(true), mag(n) {
}

Z::Z(const char *p, uint64_t radix) : pos(true), mag() {
   if (*p == '-') {
      pos = false;
      p++;
   }
   else if (*p == '+') {
      p++;
   }

   mag = N(p, radix);
   fixZero();
}

Z::Z(int64_t i) : pos(true), mag() {
   uint64_t magnitude;

   if (i < 0) {
      pos = false;
      // Avoid overflowing when i == INT64_MIN.
      magnitude = (uint64_t)(-(i + 1));
      magnitude++;
   }
   else {
      magnitude = (uint64_t)i;
   }

   mag = N(magnitude);
   fixZero();
}

Z& Z::operator=(const Z& other) {
   if (this != &other) {
      pos = other.pos;
      mag = other.mag;
      fixZero();
   }
   return *this;
}

Z Z::operator + () const {
   return *this;
}

Z Z::operator - () const {
   if (isZero()) {
      return *this;
   }
   return Z(!pos, mag);
}

Z Z::operator ~ () const {
   // Infinite-width two's-complement identity: ~x == -x - 1.
   if (pos) {
      return Z(false, mag + 1);
   }
   return Z(true, mag - 1);
}

Z Z::operator + (Z const & obj) const {
   if (pos == obj.pos) {
      return Z(pos, mag + obj.mag);
   }

   if (mag > obj.mag) {
      return Z(pos, mag - obj.mag);
   }
   if (obj.mag > mag) {
      return Z(obj.pos, obj.mag - mag);
   }
   return Z();
}

Z Z::operator - (Z const & obj) const {
   return *this + (-obj);
}

Z Z::operator * (Z const & obj) const {
   return Z(pos == obj.pos, mag * obj.mag);
}

Z Z::operator / (Z const & obj) const {
   // N performs the divide-by-zero check.  Magnitude division truncates,
   // therefore signed division truncates toward zero.
   return Z(pos == obj.pos, mag / obj.mag);
}

Z Z::operator % (Z const & obj) const {
   // Match truncation-toward-zero division: the remainder has the sign of
   // the dividend (unless it is zero, which is canonicalized positive).
   return Z(pos, mag % obj.mag);
}

Z Z::operator & (Z const & obj) const {
   if (pos) {
      if (obj.pos) {
         return Z(true, mag.apply(obj.mag, false, false, N::BOOL_AND, false));
      }
      return Z(true, mag.apply(obj.mag, false, true, N::BOOL_AND, false));
   }

   if (obj.pos) {
      return Z(true, mag.apply(obj.mag, true, false, N::BOOL_AND, false));
   }
   return Z(false, mag.apply(obj.mag, true, true, N::BOOL_AND, true));
}

Z Z::operator | (Z const & obj) const {
   if (pos) {
      if (obj.pos) {
         return Z(true, mag.apply(obj.mag, false, false, N::BOOL_OR, false));
      }
      return Z(false, mag.apply(obj.mag, false, true, N::BOOL_OR, true));
   }

   if (obj.pos) {
      return Z(false, mag.apply(obj.mag, true, false, N::BOOL_OR, true));
   }
   return Z(false, mag.apply(obj.mag, true, true, N::BOOL_OR, true));
}

Z Z::operator ^ (Z const & obj) const {
   if (pos) {
      if (obj.pos) {
         return Z(true, mag.apply(obj.mag, false, false, N::BOOL_XOR, false));
      }
      return Z(false, mag.apply(obj.mag, false, true, N::BOOL_XOR, true));
   }

   if (obj.pos) {
      return Z(false, mag.apply(obj.mag, true, false, N::BOOL_XOR, true));
   }
   return Z(true, mag.apply(obj.mag, true, true, N::BOOL_XOR, false));
}

Z Z::operator >> (int64_t bits) const {
   Z result = *this;
   return result >>= bits;
}

Z Z::operator << (int64_t bits) const {
   Z result = *this;
   return result <<= bits;
}

bool Z::operator == (const Z &other) const {
   return pos == other.pos && mag == other.mag;
}

bool Z::operator != (const Z &other) const {
   return !(*this == other);
}

bool Z::operator < (const Z &other) const {
   if (pos != other.pos) {
      return !pos;
   }
   if (pos) {
      return mag < other.mag;
   }
   return mag > other.mag;
}

bool Z::operator > (const Z &other) const {
   return other < *this;
}

bool Z::operator <= (const Z &other) const {
   return !(*this > other);
}

bool Z::operator >= (const Z &other) const {
   return !(*this < other);
}

Z& Z::operator+=(const Z& other) {
   *this = *this + other;
   return *this;
}

Z& Z::operator-=(const Z& other) {
   *this = *this - other;
   return *this;
}

Z& Z::operator*=(const Z& other) {
   *this = *this * other;
   return *this;
}

Z& Z::operator/=(const Z& other) {
   *this = *this / other;
   return *this;
}

Z& Z::operator%=(const Z& other) {
   *this = *this % other;
   return *this;
}

Z& Z::operator&=(const Z& other) {
   *this = *this & other;
   return *this;
}

Z& Z::operator|=(const Z& other) {
   *this = *this | other;
   return *this;
}

Z& Z::operator^=(const Z& other) {
   *this = *this ^ other;
   return *this;
}

Z& Z::operator >>= (int64_t bits) {
   if (bits < 0) {
      return *this <<= -bits;
   }

   if (isZero() || bits == 0) {
      return *this;
   }

   if (pos) {
      mag >>= bits;
      fixZero();
      return *this;
   }

   // Arithmetic right shift of a negative integer is floor(x / 2^bits).
   // Shift the magnitude, then round its quotient up if any low bits were
   // discarded: -(ceil(|x| / 2^bits)).
   N shifted = mag >> bits;
   if ((shifted << bits) != mag) {
      shifted += 1;
   }
   mag = shifted;
   fixZero();
   return *this;
}

Z& Z::operator <<= (int64_t bits) {
   if (bits < 0) {
      return *this >>= -bits;
   }
   mag <<= bits;
   fixZero();
   return *this;
}

Z Z::sqrt(void) const {
   return root(2);
}

Z Z::pow(const N& other) const {
   N result = mag.pow(other);
   bool result_pos = true;

   if (!pos && !((other % 2).isZero())) {
      result_pos = false;
   }

   return Z(result_pos, result);
}

Z Z::root(const N& other) const {
   if (other.isZero()) {
      // Preserve N's established error for a zeroth root.
      return Z(true, mag.root(other));
   }

   if (!pos && (other % 2).isZero()) {
      throw(UGE_ERR("even root of negative integer"));
   }

   return Z(pos, mag.root(other));
}

bool Z::isZero(void) const {
   return mag.isZero();
}

int Z::sgn(void) const {
   if (isZero()) {
      return 0;
   }
   return pos ? 1 : -1;
}

Z Z::abs(void) const {
   return Z(true, mag);
}

N Z::magnitude(void) const {
   return mag;
}

static int64_t signed_bits(uint64_t bits) {
   if (bits <= (uint64_t)INT64_MAX) {
      return (int64_t)bits;
   }

   uint64_t magnitude = (~bits) + 1;
   if (magnitude == (UINT64_C(1) << 63)) {
      return INT64_MIN;
   }
   return -(int64_t)magnitude;
}

Z::operator int64_t() const {
   uint64_t bits = (uint64_t)mag;
   if (!pos) {
      bits = (~bits) + 1;
   }
   return signed_bits(bits);
}

char *Z::print(uint64_t radix) const {
   if (isZero()) {
      return strdup("0");
   }

   if (radix < 2 || radix > 65536) {
      char *ret = NULL;
      return rpprintf(ret, "{radix %" PRIu64 " not supported}", radix);
   }

   char *m = mag.print(radix);
   if (pos) {
      return m;
   }

   char *ret = mprintf("-%s", m);
   free((void *)m);
   return ret;
}

char *Z::dprint(void) const {
   return mprintf("[%c%s]", pos ? '+' : '-', GCSTR mag.dprint());
}

// vim: expandtab:noai:ts=3:sw=3
