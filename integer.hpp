#ifndef _INCLUDE_INTEGER_HPP_
#define _INCLUDE_INTEGER_HPP_

#include <stdint.h>

class Integer {
   private:
      uint64_t size;
      uint16_t *data;

   public:
      Integer();
      Integer(const Integer &orig); // copy constructor
      Integer(const char *p);
      Integer(uint64_t d);

      Integer& operator=(const Integer& other); // assignment operator

      Integer operator + (Integer const & obj) const;
      Integer operator - (Integer const & obj) const;
      Integer operator * (Integer const & obj) const;
      Integer operator / (Integer const & obj) const;
      bool operator == (const Integer &other) const;
      bool operator != (const Integer &other) const;
      bool operator < (const Integer &other) const;
      bool operator > (const Integer &other) const;
      bool operator <= (const Integer &other) const;
      bool operator >= (const Integer &other) const;

      explicit operator int64_t() const;
};

#endif // _INCLUDE_INTEGER_HPP_
