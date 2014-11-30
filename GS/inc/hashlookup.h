#ifndef _HASH_LOOKUP_H
#define _HASH_LOOKUP_H
#include <stdint.h>
#define rot(x,k) (((x)<<(k)) | ((x)>>(32-(k))))
#define mix(a,b,c) \
{ \
  a -= c;  a ^= rot(c, 4);  c += b; \
  b -= a;  b ^= rot(a, 6);  a += c; \
  c -= b;  c ^= rot(b, 8);  b += a; \
  a -= c;  a ^= rot(c,16);  c += b; \
  b -= a;  b ^= rot(a,19);  a += c; \
  c -= b;  c ^= rot(b, 4);  b += a; \
}

#define final(a,b,c) \
{ \
  c ^= b; c -= rot(b,14); \
  a ^= c; a -= rot(c,11); \
  b ^= a; b -= rot(a,25); \
  c ^= b; c -= rot(b,16); \
  a ^= c; a -= rot(c,4);  \
  b ^= a; b -= rot(a,14); \
  c ^= b; c -= rot(b,24); \
}


static inline uint32_t hashword(const uint32_t *k, /* the key, an array of uint32_t values */
                                size_t          length,               /* the length of the key, in uint32_ts */
                                uint32_t        initval)         /* the previous hash, or an arbitrary value */
{
  uint32_t a,b,c;

  /* Set up the internal state */
  a = b = c = 0xdeadbeef + (((uint32_t)length)<<2) + initval;

  /*------------------------------------------------- handle most of the key */
  while (length > 3)
  {
    a += k[0];
    b += k[1];
    c += k[2];
    mix(a,b,c);
    length -= 3;
    k += 3;
  }

  /*------------------------------------------- handle the last 3 uint32_t's */
  switch(length)                     /* all the case statements fall through */
  { 
  case 3 : c+=k[2];
  case 2 : b+=k[1];
  case 1 : a+=k[0];
    final(a,b,c);
  case 0:     /* case 0: nothing left to add */
    break;
  }
  /*------------------------------------------------------ report the result */
  return c;
}


#endif 