#ifndef _INCLUDE_INTEGER_HPP_
#define _INCLUDE_INTEGER_HPP_

#include <stdint.h>

class Integer {
   private:
      uint64_t size;
      uint16_t *data;

      // helper function
      static void divide(
            const Integer &num,
            const Integer &den,
            Integer &quot,
            Integer &rem);

      // helper function
      void grow(void);

      // helper function
      void fixZero(void);

   public:
      Integer();
      Integer(const Integer &orig); // copy constructor
      Integer(const char *p);
      Integer(uint64_t d);
      ~Integer();

      Integer& operator=(const Integer& other); // assignment operator

      Integer operator + (Integer const & obj) const;
      Integer operator - (Integer const & obj) const;
      Integer operator * (Integer const & obj) const;
      Integer operator / (Integer const & obj) const;
      Integer operator % (Integer const & obj) const;

      Integer operator >> (int bits) const;
      Integer operator << (int bits) const;

      Integer sqrt(void) const;

      bool operator == (const Integer &other) const;
      bool operator != (const Integer &other) const;
      bool operator < (const Integer &other) const;
      bool operator > (const Integer &other) const;
      bool operator <= (const Integer &other) const;
      bool operator >= (const Integer &other) const;

      Integer& operator+=(const Integer& other);
      Integer& operator-=(const Integer& other);
      Integer& operator*=(const Integer& other);
      Integer& operator/=(const Integer& other);
      Integer& operator%=(const Integer& other);

      Integer& operator >>= (int bits);
      Integer& operator <<= (int bits);

      bool isZero(void) const;
      void setZero(void);

      explicit operator uint64_t() const;

      // returns pointer that must be free'd
      char *print(void) const;

      // returns pointer that must be free'd
      char *dprint(void) const;
};

#endif // _INCLUDE_INTEGER_HPP_
