#ifndef _INCLUDE_UGE_Z_HPP_
#define _INCLUDE_UGE_Z_HPP_

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

         Z pow(const Z& other) const;
         Z root(const Z& other) const;

         bool isZero(void) const;

         explicit operator uint64_t() const;

         // returns pointer that must be free'd
         char *print(void) const;

         // returns pointer that must be free'd
         char *dprint(void) const;

         // the following enum and method are for use with the Q class
         // for full details, see the implementation.

         enum boolop {
            BOOL_AND,
            BOOL_OR,
            BOOL_XOR
         };

         Z apply(const Z &b, bool inva, bool invb, enum boolop op, bool invo) const;
	 bool deci_lengths(Z &lead, Z &repeat, uint64_t max = 1024) const;
   };
};

#endif // _INCLUDE_UGE_Z_HPP_

// vim: expandtab:noai:ts=3:sw=3
