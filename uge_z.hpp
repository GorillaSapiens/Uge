#ifndef _INCLUDE_UGE_Z_HPP_
#define _INCLUDE_UGE_Z_HPP_

#include <stdint.h>

#include "uge_n.hpp"

namespace uge {

   // Arbitrary-precision signed integer (Z / mathematical integers).
   // The representation is deliberately simple: an N magnitude plus a
   // sign boolean.  Zero is always canonicalized as positive.
   class Z {
      private:
         bool pos;
         N mag;

         void fixZero(void);

      public:
         Z();
         Z(const Z &orig);
         Z(bool p, const N &m);
         Z(const N &n);
         Z(const char *p, uint64_t radix = 10);
         Z(int64_t i);

         Z& operator=(const Z& other);

         Z operator + () const;
         Z operator - () const;
         Z operator ~ () const;

         Z operator + (Z const & obj) const;
         Z operator - (Z const & obj) const;
         Z operator * (Z const & obj) const;
         Z operator / (Z const & obj) const;
         Z operator % (Z const & obj) const;
         Z operator & (Z const & obj) const;
         Z operator | (Z const & obj) const;
         Z operator ^ (Z const & obj) const;

         Z operator >> (int64_t bits) const;
         Z operator << (int64_t bits) const;

         bool operator == (const Z &other) const;
         bool operator != (const Z &other) const;
         bool operator < (const Z &other) const;
         bool operator > (const Z &other) const;
         bool operator <= (const Z &other) const;
         bool operator >= (const Z &other) const;

         Z& operator+=(const Z& other);
         Z& operator-=(const Z& other);
         Z& operator*=(const Z& other);
         Z& operator/=(const Z& other);
         Z& operator%=(const Z& other);
         Z& operator&=(const Z& other);
         Z& operator|=(const Z& other);
         Z& operator^=(const Z& other);
         Z& operator>>=(int64_t bits);
         Z& operator<<=(int64_t bits);

         Z sqrt(void) const;
         Z pow(const N& other) const;
         Z root(const N& other) const;

         bool isZero(void) const;
         int sgn(void) const;
         Z abs(void) const;
         N magnitude(void) const;

         explicit operator int64_t() const;

         // returns pointer that must be free'd
         char *print(uint64_t radix = 10) const;

         // returns pointer that must be free'd
         char *dprint(void) const;
   };
};

#endif // _INCLUDE_UGE_Z_HPP_

// vim: expandtab:noai:ts=3:sw=3
