#include <math.h>
#include <string.h>
#include <assert.h>

#include <string>

#include "err.hpp"
#include "ramprintf.hpp"
#include "integer.hpp"

Integer::Integer() {
   size = 0;
   data = NULL;
}

Integer::~Integer() {
   if (data) {
      free((void *) data);
      data = NULL;
   }
}

void Integer::grow(void) {
   // NB: caller responsible for changing size
   data = (uint16_t *) realloc(data, sizeof(uint16_t) * (size + 1));
   assert(data);
}

bool Integer::isZero(void) const {
   return (size == 0);
}

void Integer::setZero(void) {
   size = 0;
   if (data) {
      free((void *)data);
      data = 0;
   }
}

void Integer::fixZero(void) {
   while (size && data[size - 1] == 0) {
      size--;
      if (size == 0 && data) {
         free((void *)data);
         data = NULL;
      }
   }
}

Integer::Integer(const Integer &orig) {
   size = orig.size;
   data = (uint16_t *) malloc(sizeof(uint16_t) * size);
   memcpy(data, orig.data, sizeof(uint16_t) * size);
}

Integer& Integer::operator=(const Integer& other) {
   size = other.size;
   data = (uint16_t *) malloc(sizeof(uint16_t) * size);
   memcpy(data, other.data, sizeof(uint16_t) * size);

   return *this;
}

Integer::Integer(const char *orig) {

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

Integer::Integer(uint64_t i) {
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

Integer Integer::operator + (Integer const & obj) const {
   Integer res;

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

Integer Integer::operator - (Integer const & obj) const {
   if (*this < obj) {
      throw (ERR("subtraction underflow"));
   }

   Integer res;

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

Integer Integer::operator * (Integer const & obj) const {
   Integer res;

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

void Integer::divide(
   const Integer &num,
   const Integer &den,
   Integer &quot,
   Integer &rem) {

   // can't divide by zero
   if (den.size == 0) {
      throw(ERR("division by zero"));
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
   Integer pieces[16];
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
         Integer accumulator;
         accumulator.size = 0;
         accumulator.data = 0;

         for (int i = 15; i >= 0; i--) {
            Integer tmp = accumulator + pieces[i];
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

Integer Integer::sqrt(void) const {
   // we'll do this recursively...

   // trivial cases
   if (size == 0) {
      return *this;
   }
   if (size == 1) {
      uint64_t res = ::sqrt((double)data[0]);
      return Integer(res);
   }
   if (size == 2) {
      uint64_t res = ::sqrt((double)(((uint64_t)data[1] << 16) | data[0]));
      return Integer(res);
   }

   // and now the fun stuff...
   uint64_t place = size - 1;
   uint64_t digit;
   digit = data[place--];
   if ((place % 2) == 0) {
      digit <<= 16;
      digit |= data[place--];
   }
   digit = ::sqrt((double) digit);

   Integer a(digit);
   for (uint16_t i = place + 1; i > 2; i -= 2) {
      a.grow();
      a.data[0] = 0;
      a.size++;
   }

   // (a*base+x) * (a*base+x) =~ this
   // this - a*a*base*base = 2 * base * a * x + x * x
   // remainder = (2*base*a)*x + x*x
   // x^2 + (2 * base * a) x - remainder = 0
   // x = (-b +/- sqrt(b^2-4c)) / 2

   Integer c =  *this - a * 65536 * a * 65536;
   Integer b = Integer(2 * 65536) * a;
   Integer inner = b*b-Integer(4)*c;
   Integer x = (inner.sqrt() - b) / Integer(2);

   Integer result = a * 65536 + x;

   return result;
}

Integer Integer::operator / (Integer const & obj) const {
   Integer quot;
   Integer rem;
   divide(*this, obj, quot, rem);
   return quot;
}

Integer Integer::operator % (Integer const & obj) const {
   Integer quot;
   Integer rem;
   divide(*this, obj, quot, rem);
   return rem;
}

Integer Integer::operator >> (int smallbits) const {
   Integer result = *this;
   return result >>= smallbits;
}

Integer Integer::operator << (int smallbits) const {
   Integer result = *this;
   return result <<= smallbits;
}

bool Integer::operator == (const Integer &other) const {
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

bool Integer::operator != (const Integer &other) const {
   return (!(*this == other));
}


bool Integer::operator < (const Integer &other) const {
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

bool Integer::operator > (const Integer &other) const {
   return !(*this <= other);
}


bool Integer::operator <= (const Integer &other) const {
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

bool Integer::operator >= (const Integer &other) const {
   return !(*this < other);
}

Integer& Integer::operator+=(const Integer& other) {
   *this = *this + other;
   return *this;
}

Integer& Integer::operator-=(const Integer& other) {
   *this = *this - other;
   return *this;
}

Integer& Integer::operator*=(const Integer& other) {
   *this = *this * other;
   return *this;
}

Integer& Integer::operator/=(const Integer& other) {
   *this = *this / other;
   return *this;
}

Integer& Integer::operator%=(const Integer& other) {
   *this = *this % other;
   return *this;
}

Integer& Integer::operator >>= (int smallbits) {
   if (smallbits > 16) {
      throw(ERR("shift bigger than 16 bits"));
   }
   if (smallbits < 0) {
      return *this <<= (-smallbits);
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

Integer& Integer::operator <<= (int smallbits) {
   if (smallbits > 16) {
      throw(ERR("shift bigger than 16 bits"));
   }
   if (smallbits < 0) {
      return *this >>= (-smallbits);
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

Integer::operator uint64_t() const {
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

char *Integer::print(void) const {
   if (isZero()) {
      return strdup("0");
   }

   Integer copy = *this;
   Integer quot;
   Integer rem;

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

char *Integer::dprint(void) const {
   char *ret = NULL;
   raprintf(ret, "{%ld", size);
   for (uint64_t i = 0; i < size; i++) {
      raprintf(ret, ":%04x", data[i]);
   }
   raprintf(ret, "}");
   return ret;
}
