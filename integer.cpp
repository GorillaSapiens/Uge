#include <string>

#include <math.h>
#include <string.h>
#include <assert.h>

#include <string>

#include "integer.hpp"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AT __FILE__ ":" TOSTRING(__LINE__)

#define ERR(x) std::string(x " at " AT)

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
}

Integer::Integer(const Integer &orig) {
   size = orig.size;
   data = (uint16_t *) malloc(sizeof(uint16_t) * size);
   memcpy(data, orig.data, sizeof(uint16_t) * size);
}

Integer& Integer::operator=(const Integer& other) {
   size = other.size;
   data = (uint16_t *) realloc(data, sizeof(uint16_t) * size);
   memcpy(data, other.data, sizeof(uint16_t) * size);

   return *this;
}

Integer::Integer(const char *orig) {

   size = 0;
   data = NULL;

   while (*orig) {
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

   // shrink to fit
   while (res.size && !res.data[res.size - 1]) {
      res.size--;
   }
   if (!res.size) {
      if (res.data) {
         free((void *) res.data);
      }
      res.data = NULL;
   }

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

   // that's it, we're done!
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

char *Integer::print(char *buf, size_t buflen) {
   Integer copy = *this;
   Integer quot;
   Integer rem;

   buf[0] = 0;
   while (copy > (uint64_t)0) {
      divide(copy, (uint64_t) 10, quot, rem);
      memmove(buf + 1, buf, strlen(buf) + 1);
      if (rem.size) {
         *buf = '0' + rem.data[0];
      }
      else {
         *buf = '0';
      }
      copy = quot;
   }

   return buf;
}
