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
      bool operator == (const Integer &other) const;
      bool operator != (const Integer &other) const;
      bool operator < (const Integer &other) const;
      bool operator > (const Integer &other) const;
      bool operator <= (const Integer &other) const;
      bool operator >= (const Integer &other) const;

      explicit operator uint64_t() const;

      char *print(char *buf, size_t buflen);
};

#endif // _INCLUDE_INTEGER_HPP_
