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

         Z operator+ (Z const & obj) const;
         Z operator- (Z const & obj) const;
         Z operator* (Z const & obj) const;
         Z operator/ (Z const & obj) const;
         Z operator% (Z const & obj) const;
         Z operator& (Z const & obj) const;
         Z operator| (Z const & obj) const;
         Z operator^ (Z const & obj) const;

         Z operator>> (int64_t bits) const;
         Z operator<< (int64_t bits) const;

         Z sqrt(void) const;

         bool operator== (const Z &other) const;
         bool operator!= (const Z &other) const;
         bool operator< (const Z &other) const;
         bool operator> (const Z &other) const;
         bool operator<= (const Z &other) const;
         bool operator>= (const Z &other) const;

         Z& operator+=(const Z& other);
         Z& operator-=(const Z& other);
         Z& operator*=(const Z& other);
         Z& operator/=(const Z& other);
         Z& operator%=(const Z& other);
         Z& operator&=(const Z& other);
         Z& operator|=(const Z& other);
         Z& operator^=(const Z& other);

         Z& operator>>= (int64_t bits);
         Z& operator<<= (int64_t bits);

         Z pow(const Z& other);

         bool isZero(void) const;

         explicit operator uint64_t() const;

         // returns pointer that must be free'd
         char *print(void) const;

         // returns pointer that must be free'd
         char *dprint(void) const;

         enum boolop {
                            // a 1100
                            // b 1010
            BOOL_NULL        = 0b0000,
            BOOL_NOR         = 0b0001,
            BOOL_BANDNOTA    = 0b0010,
            BOOL_NOTA        = 0b0011,
            BOOL_AANDNOTB    = 0b0100,
            BOOL_NOTB        = 0b0101,
            BOOL_XOR         = 0b0110,
            BOOL_NAND        = 0b0111,
            BOOL_AND         = 0b1000,
            BOOL_XNOR        = 0b1001,
            BOOL_TRANB       = 0b1010,
            BOOL_BORNOTA     = 0b1011,
            BOOL_TRANA       = 0b1100,
            BOOL_AORNOTB     = 0b1101,
            BOOL_OR          = 0b1110,
            BOOL_ONE         = 0b1111
         };

         Z apply(const Z &b, enum boolop op) const;
   };
};

#endif // _INCLUDE_INTEGER_HPP_
