#ifndef _INCLUDE_UGE_N_HPP_
#define _INCLUDE_UGE_N_HPP_

#include <stdint.h>

namespace uge {

   class N {
      private:
         uint64_t size;
         uint16_t *data;

         // helper functions
         static void divide( const N &num, const N &den, N &quot, N &rem);
         void grow(void);
         void fixZero(void);

      public:
         N();
         N(const N &orig); // copy constructor
         N(const char *p, uint64_t radix = 10);
         N(uint64_t d);
         ~N();

         N& operator=(const N& other); // assignment operator

         N operator+ (N const & obj) const;
         N operator- (N const & obj) const;
         N operator* (N const & obj) const;
         N operator/ (N const & obj) const;
         N operator% (N const & obj) const;
         N operator& (N const & obj) const;
         N operator| (N const & obj) const;
         N operator^ (N const & obj) const;

         N operator>> (int64_t bits) const;
         N operator<< (int64_t bits) const;

         N sqrt(void) const;

         bool operator== (const N &other) const;
         bool operator!= (const N &other) const;
         bool operator< (const N &other) const;
         bool operator> (const N &other) const;
         bool operator<= (const N &other) const;
         bool operator>= (const N &other) const;

         N& operator+=(const N& other);
         N& operator-=(const N& other);
         N& operator*=(const N& other);
         N& operator/=(const N& other);
         N& operator%=(const N& other);
         N& operator&=(const N& other);
         N& operator|=(const N& other);
         N& operator^=(const N& other);

         N& operator>>= (int64_t bits);
         N& operator<<= (int64_t bits);

         N pow(const N& other) const;
         N root(const N& other) const;

         bool isZero(void) const;

         explicit operator uint64_t() const;

         // returns pointer that must be free'd
         char *print(uint64_t radix = 10) const;

         // returns pointer that must be free'd
         char *dprint(void) const;

         // the following enum and method are for use with the Q class
         // for full details, see the implementation.

         enum boolop {
            BOOL_AND,
            BOOL_OR,
            BOOL_XOR
         };

         N apply(const N &b, bool inva, bool invb, enum boolop op, bool invo) const;
	      bool deci_lengths(N &lead, N &repeat, uint64_t max = 1024) const;
   };
};

#endif // _INCLUDE_UGE_N_HPP_

// vim: expandtab:noai:ts=3:sw=3
