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
   // TODO FIX
}

Integer::Integer(uint64_t i) {
   while (i) {
      data = (uint16_t *)realloc(data, sizeof(uint16_t) * (size + 1));
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

      res.data = (uint16_t *)realloc(res.data, sizeof(uint16_t) * (res.size + 1));
      res.data[res.size] = result; // truncation happens here
      res.size++;
      carry = result >> 16;
   }

   if (carry) {
      res.data = (uint16_t *)realloc(res.data, sizeof(uint16_t) * (res.size + 1));
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

      res.data = (uint16_t *)realloc(res.data, sizeof(uint16_t) * (res.size + 1));
      res.data[res.size] = result; // truncation happens here
      res.size++;
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
            res.data = (uint16_t *)realloc(res.data, sizeof(uint16_t) * (res.size + 1));
            res.data[res.size] = 0;
            res.size++;
         }

         uint32_t result = a * b + res.data[place];
         res.data[place] = result; // truncation happens here
         result >>= 16;
         place++;

         while (result) {
            while (res.size <= place) {
               res.data = (uint16_t *)realloc(res.data, sizeof(uint16_t) * (res.size + 1));
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

   // trivial case
   if (num < den) {
      rem = num;
      return;
   }
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
   return true;
}

bool Integer::operator >= (const Integer &other) const {
   return !(*this < other);
}

Integer::operator int64_t() const {
   int64_t ret = 0;
   for (uint64_t i = 0; i < 4; i++) {
      uint64_t place = 3LL - i;
      if (place < size) {
         ret <<= 16;
         ret |= (int64_t)data[place];
      }
   }
   return ret;
}
