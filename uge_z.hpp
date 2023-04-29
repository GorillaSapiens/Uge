#ifndef _INCLUDE_INTEGER_HPP_
#define _INCLUDE_INTEGER_HPP_

#include <stdint.h>

namespace uge {

   class Z {
      private:
         uint64_t size;
         uint16_t *data;

         // helper functions
         static void divide( const Z &num, const Z &den, Z &quot, Z &rem);
         void grow(void);
         void fixZero(void);

      public:
         Z();
         Z(const Z &orig); // copy constructor
         Z(const char *p);
         Z(uint64_t d);
         ~Z();

         Z& operator=(const Z& other); // assignment operator

         Z operator + (Z const & obj) const;
         Z operator - (Z const & obj) const;
         Z operator * (Z const & obj) const;
         Z operator / (Z const & obj) const;
         Z operator % (Z const & obj) const;

         Z operator >> (int bits) const;
         Z operator << (int bits) const;

         Z sqrt(void) const;

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

         Z& operator >>= (int bits);
         Z& operator <<= (int bits);

         bool isZero(void) const;

         explicit operator uint64_t() const;

         // returns pointer that must be free'd
         char *print(void) const;

         // returns pointer that must be free'd
         char *dprint(void) const;
   };

};

#endif // _INCLUDE_INTEGER_HPP_
