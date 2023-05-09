#include <math.h>
#include <string.h>
#include <assert.h>

#include <string>

#include "uge_err.hpp"
#include "uge_ramprintf.hpp"
#include "uge_z.hpp"

using namespace uge;

Z::Z() {
   size = 0;
   data = NULL;
}

Z::~Z() {
   if (data) {
      free((void *) data);
      data = NULL;
   }
}

void Z::grow(void) {
   // NB: caller responsible for changing size
   data = (uint16_t *) realloc(data, sizeof(uint16_t) * (size + 1));
   data[size] = 0;
   assert(data);
}

bool Z::isZero(void) const {
   return (size == 0);
}

void Z::fixZero(void) {
   while (size && data[size - 1] == 0) {
      size--;
      if (size == 0 && data) {
         free((void *)data);
         data = NULL;
      }
   }
}

Z::Z(const Z &orig) {
   size = orig.size;
   data = (uint16_t *) malloc(sizeof(uint16_t) * size);
   memcpy(data, orig.data, sizeof(uint16_t) * size);
}

Z& Z::operator=(const Z& other) {
   if (data) {
      free((void *) data);
   }
   size = other.size;
   data = (uint16_t *) malloc(sizeof(uint16_t) * size);
   memcpy(data, other.data, sizeof(uint16_t) * size);

   return *this;
}

Z::Z(const char *orig) {

   size = 0;
   data = NULL;

   while (*orig) {
      if ((*orig < '0') || (*orig > '9')) {
         break;
      }
      // times 10
      if (size) {
         uint64_t place = 0;
         uint32_t carry = 0;
         while (place != size) {
            carry += data[place] * 10;
            data[place] = carry; // truncation here
            carry >>= 16;
            place++;
         }
         if (carry) {
            grow();
            data[place] = carry;
            size++;
         }
      }
      // plus string data
      if (!size) {
         size = 1;
         data = (uint16_t *) malloc(sizeof(uint16_t));
         data[0] = (uint16_t) (*orig - '0');
      }
      else {
         uint64_t place = 0;
         uint32_t carry = (uint32_t) (*orig - '0');
         while (place != size && carry) {
            carry += data[place];
            data[place] = carry; // truncation here
            carry >>= 16;
            place++;
         }
         if (carry) {
            grow();
            data[place] = carry;
            size++;
         }
      }
      orig++;
   }

   fixZero();
}

Z::Z(uint64_t i) {
   size = 0;
   data = NULL;

   while (i) {
      grow();
      data[size] = i; // truncation happens here
      size++;
      i >>= 16;
   }

   fixZero();
}

Z Z::operator + (Z const & obj) const {
   Z res;

   uint32_t carry = 0;

   for (uint64_t i = 0; i < size || i < obj.size; i++) {
      uint32_t a = (i < size) ? data[i] : 0;
      uint32_t b = (i < obj.size) ? obj.data[i] : 0;
      uint32_t result = a + b + carry;

      res.grow();
      res.data[res.size] = result; // truncation happens here
      res.size++;
      carry = result >> 16;
   }

   if (carry) {
      res.grow();
      res.data[res.size] = carry; // truncation happens here
      res.size++;
   }

   res.fixZero();
   return res;
}

Z Z::operator - (Z const & obj) const {
   if (*this < obj) {
      throw (UGE_ERR("subtraction underflow"));
   }

   Z res;

   uint16_t borrow = 0;

   for (uint64_t i = 0; i < size || i < obj.size; i++) {
      uint32_t a = (i < size) ? data[i] : 0;
      if (borrow) {
         if (a) {
            borrow = 0;
            a--;
         }
         else {
            a = 0xFFFF;
         }
      }
      uint32_t b = (i < obj.size) ? obj.data[i] : 0;

      if (a < b) {
         borrow = 1;
         a += 0x10000;
      }

      uint32_t result = a - b;

      res.grow();
      res.data[res.size] = result; // truncation happens here
      res.size++;
   }

   res.fixZero();
   return res;
}

Z Z::operator * (Z const & obj) const {
   Z res;

   for (uint64_t ai = 0; ai < size; ai++) {
      uint32_t a = data[ai];
      for (uint64_t bi = 0; bi < obj.size; bi++) {
         uint32_t b = obj.data[bi];

         uint64_t place = ai + bi;
         while (res.size <= place) {
            res.grow();
            res.data[res.size] = 0;
            res.size++;
         }

         uint32_t result = a * b + res.data[place];
         res.data[place] = result; // truncation happens here
         result >>= 16;
         place++;

         while (result) {
            while (res.size <= place) {
               res.grow();
               res.data[res.size] = 0;
               res.size++;
            }

            result += res.data[place];
            res.data[place] = result; // truncation happens here
            result >>= 16;
            place++;
         }
      }
   }

   res.fixZero();
   return res;
}

void Z::divide(
   const Z &num,
   const Z &den,
   Z &quot,
   Z &rem) {

   // can't divide by zero
   if (den.size == 0) {
      throw(UGE_ERR("division by zero"));
   }

   // trivial case, numerator less than denominator
   // this includes num == 0
   if (num < den) {
      quot = (uint64_t) 0;
      rem = num;
      return;
   }

   // easy case, let's not mess about
   if (num.size <= 2 && den.size <= 2) {
      uint32_t n = num.data[0];
      if (num.size == 2) {
         n |= ((uint32_t) num.data[1]) << 16;
      }

      uint32_t d = den.data[0];
      if (den.size == 2) {
         d |= ((uint32_t) den.data[1]) << 16;
      }

      quot = n / d;
      rem = n % d;
      return;
   }

   // and now the fun stuff...

   // zero out quotient
   quot.size = 0;
   if (quot.data) {
      free((void *)quot.data);
      quot.data = NULL;
   }

   // zero out remainder
   rem.size = 0;
   if (rem.data) {
      free((void *)rem.data);
      rem.data = NULL;
   }

   // calculate 16 shifted versions of the denominator
   Z pieces[16];
   pieces[0] = den;
   for (int i = 1; i < 16; i++) {
      // zero it out
      pieces[i].size = 0;
      pieces[i].data = NULL;

      uint32_t carry = 0;
      for (uint64_t o = 0; o < pieces[i - 1].size; o++) {
         pieces[i].grow();
         uint32_t tmp = pieces[i-1].data[o];
         tmp = (tmp << 1) | carry;
         pieces[i].data[pieces[i].size] = tmp; // truncation here
         carry = tmp >> 16;
         pieces[i].size++;
      }
      if (carry) {
         pieces[i].grow();
         pieces[i].data[pieces[i].size] = carry; // truncation here
         pieces[i].size++;
      }
   }

   // our starting index in the numerator
   uint64_t place = num.size;

   while (place > 0) {
      while (rem < den && place > 0) {
         // shift rem over
         if (rem.size == 0) {
            rem.size = 1;
            rem.data = (uint16_t *) malloc(sizeof(uint16_t));
         }
         else {
            rem.grow();
            memmove(rem.data + 1, rem.data, sizeof(uint16_t) * rem.size);
            rem.size++;
         }
         rem.data[0] = num.data[--place];
         rem.fixZero();

         // put a zero in quot, if needed
         if (quot.size) {
            if (rem < den) {
               quot.grow();
               memmove(quot.data + 1, quot.data, sizeof(uint16_t) * quot.size);
               quot.size++;
               quot.data[0] = 0;
            }
         }
      }

      if (rem >= den) {
         uint16_t digit = 0;
         Z accumulator;
         accumulator.size = 0;
         accumulator.data = 0;

         for (int i = 15; i >= 0; i--) {
            Z tmp = accumulator + pieces[i];
            if (tmp <= rem) {
               accumulator = tmp;
               digit |= (1 << i);
            }
         }

         rem = rem - accumulator;
         quot.grow();
         if (quot.size) {
            memmove(quot.data + 1, quot.data, sizeof(uint16_t) * quot.size);
         }
         quot.size++;
         quot.data[0] = digit;
      }
   }

   quot.fixZero();
   rem.fixZero();

   // that's it, we're done!
}

Z Z::sqrt(void) const {
   // we'll do this recursively...

   // trivial cases
   if (size == 0) {
      return *this;
   }
   if (size == 1) {
      uint64_t res = ::sqrt((double)data[0]);
      return Z(res);
   }
   if (size == 2) {
      uint64_t res = ::sqrt((double)(((uint64_t)data[1] << 16) | data[0]));
      return Z(res);
   }

   // and now the fun stuff...

   // sqrt(z) = 2 * sqrt(z/4)
   Z result = (*this >> 2).sqrt() << 1;
   Z result_plus_1 = result + 1;

   if (result_plus_1 * result_plus_1 < *this) {
      result = result + 1;
   }

   return result;
}

Z Z::operator / (Z const & obj) const {
   Z quot;
   Z rem;
   divide(*this, obj, quot, rem);
   return quot;
}

Z Z::operator % (Z const & obj) const {
   Z quot;
   Z rem;
   divide(*this, obj, quot, rem);
   return rem;
}

Z Z::operator & (Z const & obj) const {
   Z res = *this;

   while (res.size < obj.size) {
      res.grow();
      res.size++;
   }

   for (uint64_t i = 0; i < res.size; i++) {
      if (i >= obj.size) {
         res.data[i] = 0;
      }
      else {
         res.data[i] &= obj.data[i];
      }
   }

   res.fixZero();

   return res;
}

Z Z::operator | (Z const & obj) const {
   Z res = *this;

   while (res.size < obj.size) {
      res.grow();
      res.size++;
   }

   for (uint64_t i = 0; i < res.size; i++) {
      if (i < obj.size) {
         res.data[i] |= obj.data[i];
      }
   }

   res.fixZero();

   return res;
}

Z Z::operator ^ (Z const & obj) const {
   Z res = *this;

   while (res.size < obj.size) {
      res.grow();
      res.size++;
   }

   for (uint64_t i = 0; i < res.size; i++) {
      if (i < obj.size) {
         res.data[i] ^= obj.data[i];
      }
   }

   res.fixZero();

   return res;
}

Z Z::operator >> (int64_t smallbits) const {
   Z result = *this;
   return result >>= smallbits;
}

Z Z::operator << (int64_t smallbits) const {
   Z result = *this;
   return result <<= smallbits;
}

bool Z::operator == (const Z &other) const {
   if (size != other.size) {
      return false;
   }
   for (uint64_t i = 0; i < size; i++) {
      if (data[i] != other.data[i]) {
         return false;
      }
   }
   return true;
}

bool Z::operator != (const Z &other) const {
   return (!(*this == other));
}


bool Z::operator < (const Z &other) const {
   if (size < other.size) {
      return true;
   }
   if (size == other.size) {
      for (uint64_t i = 0; i < size; i++) {
         uint64_t place = size - i - 1;
         if (data[place] < other.data[place]) {
            return true;
         }
         else if (data[place] > other.data[place]) {
            return false;
         }
      }
   }
   return false;
}

bool Z::operator > (const Z &other) const {
   return !(*this <= other);
}


bool Z::operator <= (const Z &other) const {
   if (size < other.size) {
      return true;
   }
   else if (size > other.size) {
      return false;
   }
   else {
      for (uint64_t i = 0; i < size; i++) {
         uint64_t place = size - i - 1;
         if (data[place] < other.data[place]) {
            return true;
         }
         else if (data[place] > other.data[place]) {
            return false;
         }
      }
      return true;
   }
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
      return *this <<= (-bits);
   }

   uint64_t wordshift = bits / 16;
   int smallbits = bits % 16;

   if (wordshift < size) {
      for (uint64_t i = 0; i < size; i++) {
         if (i + wordshift < size) {
            data[i] = data[i + wordshift];
         }
         else {
            data[i] = 0;
         }
      }
      fixZero();
   }
   else {
      *this = (int) 0;
   }

   uint32_t carry = 0;
   for (uint64_t j = 0 ; j < size; j++) {
      uint64_t i = size - 1 - j;
      carry |= data[i];
      data[i] = carry >> smallbits;
      carry &= (1 << smallbits) - 1;
      carry <<= 16;
   }

   fixZero();
   return *this;
}

Z& Z::operator <<= (int64_t bits) {
   if (bits < 0) {
      return *this >>= (-bits);
   }

   uint64_t wordshift = bits / 16;
   int smallbits = bits % 16;

   for (int i = 0; i < wordshift; i++) {
      grow();
      memmove(data + 1, data, sizeof(uint16_t) * size);
      data[0] = 0;
      size++;
   }

   uint32_t carry = 0;
   for (uint64_t i = 0; i < size; i++) {
      carry |= ((uint32_t)data[i]) << smallbits;
      data[i] = carry;
      carry >>= 16;
   }

   if (carry) {
      grow();
      data[size] = carry;
      size++;
   }

   return *this;
}

Z::operator uint64_t() const {
   uint64_t ret = 0;
   for (uint64_t i = 0; i < 4; i++) {
      uint64_t place = 3LL - i;
      if (place < size) {
         ret <<= 16;
         ret |= (uint64_t)data[place];
      }
   }
   return ret;
}

Z Z::pow(const Z& other) {
   if (other.isZero()) {
      return 1;
   }
   if (other == 1) {
      return *this;
   }
   Z halfpow = this->pow(other / 2);
   Z ret = halfpow * halfpow;
   if (other.data[0] & 1) {
      ret = ret * (*this);
   }
   return ret;
}

char *Z::print(void) const {
   if (isZero()) {
      return strdup("0");
   }

   Z copy = *this;
   Z quot;
   Z rem;

   char *ret = NULL;

   while (copy > (uint64_t)0) {
      divide(copy, (uint64_t) 10, quot, rem);
      if (!rem.isZero()) {
         raprintf(ret, "%d", rem.data[0]);
      }
      else {
         raprintf(ret, "0");
      }
      copy = quot;
   }

   strrev(ret);

   return ret;
}

char *Z::dprint(void) const {
   char *ret = NULL;

   if (isZero()) {
      return strdup("{zero}");
   }

   for (uint64_t j = 0; j < size; j++) {
      uint64_t i = size - 1 - j;
      raprintf(ret, "%s%04x%s",
         j == 0 ? "{" : ":",
         data[i],
         i == 0 ? "}" : "");

   }
   return ret;
}

Z Z::apply(const Z &b, enum boolop op) const {
   Z left = *this;
   Z right = b;
   Z ret;

   while (left.size < right.size) {
      left.grow();
      left.size++;
   }
   while (right.size < left.size) {
      right.grow();
      right.size++;
   }
   while (ret.size < left.size) {
      ret.grow();
      ret.size++;
   }

   for (uint64_t i = 0; i < left.size; i++) {
      switch (op) {
         case BOOL_NULL:       // 0b0000
            ret.data[i] = -1;
            break;
         case BOOL_NOR:        // 0b0001
            ret.data[i] = ~ (left.data[i] | right.data[i]);
            break;
         case BOOL_BANDNOTA:   // 0b0010
            ret.data[i] = (~left.data[i]) & right.data[i];
            break;
         case BOOL_NOTA:       // 0b0011
            ret.data[i] = ~ (left.data[i]);
            break;
         case BOOL_AANDNOTB:   // 0b0100
            ret.data[i] = left.data[i] & (~right.data[i]);
            break;
         case BOOL_NOTB:       // 0b0101
            ret.data[i] = ~ (right.data[i]);
            break;
         case BOOL_XOR:        // 0b0110
            ret.data[i] = left.data[i] ^ right.data[i];
            break;
         case BOOL_NAND:       // 0b0111
            ret.data[i] = ~(left.data[i] & right.data[i]);
            break;
         case BOOL_AND:        // 0b1000
            ret.data[i] = left.data[i] & right.data[i];
            break;
         case BOOL_XNOR:       // 0b1001
            ret.data[i] = ~(left.data[i] ^ right.data[i]);
            break;
         case BOOL_TRANB:      // 0b1010
            ret.data[i] = right.data[i];
            break;
         case BOOL_BORNOTA:    // 0b1011
            ret.data[i] = (~left.data[i]) | right.data[i];
            break;
         case BOOL_TRANA:      // 0b1100
            ret.data[i] = left.data[i];
            break;
         case BOOL_AORNOTB:    // 0b1101
            ret.data[i] = left.data[i] | (~right.data[i]);
            break;
         case BOOL_OR:         // 0b1110
            ret.data[i] = left.data[i] | right.data[i];
            break;
         case BOOL_ONE:        // 0b1111
            ret.data[i] = -1;
            break;
      }
   }

   ret.fixZero();
   return ret;
}
