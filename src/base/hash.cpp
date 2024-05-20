#include "hash.h"

#include <vector>
#include <algorithm>

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


unsigned long long cantor_pairing(unsigned long long a, unsigned long long b) {
    unsigned long long hash = (a + b + 1) * (a + b) / 2 + b; 
    return hash;
}

unsigned long  compute_hash(std::vector<unsigned int> conn)
{
    std::sort(conn.begin(), conn.end());
    unsigned int len   = conn.size();
    unsigned int *keys = conn.data();

    uint32_t a = 0,b=0,c=0;
    a = b = c = 0xdeadbeef + (((uint32_t)len)<<2);

    while(len > 3)
    {
        a += keys[0];
        b += keys[1];
        c += keys[2];
        len -=3;
        keys+=3;
    }

    switch (len)
    {
        case 3 : c+=keys[2];
        case 2 : b+=keys[1];
        case 1 : a+=keys[0];
        final(a,b,c);
        case 0:     /* case 0: nothing left to add */
        break;
    }

    return c;

}

