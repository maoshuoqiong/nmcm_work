#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <wchar.h>
#include <stdint.h>
#include <string.h>

#define bittab __fsmu8
/*
#include "libc.h"
*/
#define ATTR_LIBC_VISIBILITY __attribute__((visibility("hidden")))

extern const uint32_t bittab[] ATTR_LIBC_VISIBILITY;
/* Upper 6 state bits are a negative integer offset to bound-check next byte */
/*    equivalent to: ( (b-0x80) | (b+offset) ) & ~0x3f      */
#define OOB(c,b) (((((b)>>3)-0x10)|(((b)>>3)+((int32_t)(c)>>26))) & ~7)

/* Interval [a,b). Either a must be 80 or b must be c0, lower 3 bits clear. */
#define R(a,b) ((uint32_t)((a==0x80 ? 0x40-b : -a) << 23))
#define FAILSTATE R(0x80,0x80)

#define SA 0xc2u
#define SB 0xf4u

#define C(x) ( x<2 ? -1 : ( R(0x80,0xc0) | x ) )
#define D(x) C((x+16))
#define E(x) ( ( x==0 ? R(0xa0,0xc0) : \
                 x==0xd ? R(0x80,0xa0) : \
                 R(0x80,0xc0) ) \
             | ( R(0x80,0xc0) >> 6 ) \
             | x )
#define F(x) ( ( x>=5 ? 0 : \
                 x==0 ? R(0x90,0xc0) : \
                 x==4 ? R(0x80,0xa0) : \
                 R(0x80,0xc0) ) \
             | ( R(0x80,0xc0) >> 6 ) \
             | ( R(0x80,0xc0) >> 12 ) \
             | x )

const uint32_t bittab[] = {
	              C(0x2),C(0x3),C(0x4),C(0x5),C(0x6),C(0x7),
	C(0x8),C(0x9),C(0xa),C(0xb),C(0xc),C(0xd),C(0xe),C(0xf),
	D(0x0),D(0x1),D(0x2),D(0x3),D(0x4),D(0x5),D(0x6),D(0x7),
	D(0x8),D(0x9),D(0xa),D(0xb),D(0xc),D(0xd),D(0xe),D(0xf),
	E(0x0),E(0x1),E(0x2),E(0x3),E(0x4),E(0x5),E(0x6),E(0x7),
	E(0x8),E(0x9),E(0xa),E(0xb),E(0xc),E(0xd),E(0xe),E(0xf),
	F(0x0),F(0x1),F(0x2),F(0x3),F(0x4)
};

#ifdef BROKEN_VISIBILITY
__asm__(".hidden __fsmu8");
#endif


size_t mbsrtowcs(wchar_t *restrict ws, const char **restrict src, size_t wn, mbstate_t *restrict st)
{
	const unsigned char *s = (const void *) *src;
	size_t wn0 = wn;
	unsigned c = 0;

	if( st && (c = *(unsigned *)st))
	{
		if(ws)
		{
			*(unsigned *)st = 0;
			goto resume;
		}
		else
			goto resume0;
	}

	if(!ws)
	{
		for(;;)
		{
			if(*s -1u < 0x7f && (uintptr_t)s%4 == 0)
			{
				while(!(( *(uint32_t*)s | *(uint32_t*)s - 0x01010101) & 0x80808080))
				{
					s += 4;
					wn -= 4;
				}
			}

			if(*s-1u < 0x7f)
			{
				s++;
				wn--;
				continue;
			}

			if( *s - SA > SB-SA) break;
			c = bittab[*s++ - SA];
resume0:
			if (OOB(c,*s)) { s--; break; }
			s++;
			if (c&(1U<<25))
			{
				if (*s-0x80u >= 0x40) { s-=2; break; }
				s++;
				if (c&(1U<<19))
				{
					if (*s-0x80u >= 0x40) { s-=3; break; }
					s++;
				}	
			}
			wn--;
			c = 0;
		}
	}
	else
	{
		for(;;)
		{
			if (!wn)
			{
				*src = (const void*)s;
				return wn0;
			}
			if (*s-1u < 0x7f && (uintptr_t)s%4 == 0)
			{
				while (wn>=5 && !(( *(uint32_t*)s | *(uint32_t*)s-0x01010101) & 0x80808080))
				{
					*ws++ = *s++;
					*ws++ = *s++;
					*ws++ = *s++;
					*ws++ = *s++;
					wn -= 4;
				}
			}
			if (*s-1u < 0x7f)
			{
				*ws++ = *s++;
				wn--;
				continue;
			}
			if( *s - SA > SB-SA) break;
			c = bittab[*s++ - SA];
resume:
			if(OOB(c, *s)){s--; break;}
			c = (c<<6) | *s++ - 0x80;
			if(c &(1u<<31))
			{
				if (*s-0x80u >= 0x40) { s-=2; break; }
				c = (c<<6) | *s++-0x80;
				if (c&(1U<<31))
				{
					if (*s-0x80u >= 0x40) { s-=3; break; }
					c = (c<<6) | *s++-0x80;
				}
			}
			*ws++ = c;
			wn--;
			c=0;
		}
	}

	if(!c && !*s)
	{
		if(ws)
		{
			*ws = 0;
			*src = 0;
		}
		return wn0 - wn;
	}
	errno = EILSEQ;
	if(ws) *src = (const void*)s;
	return -1;
}

size_t wcrtomb(char *s, wchar_t wc, mbstate_t *st)
{
	if (!s) return 1;
	if ((unsigned)wc < 0x80) {
		*s = wc;
		return 1;
	} else if ((unsigned)wc < 0x800) {
		*s++ = 0xc0 | (wc>>6);
		*s = 0x80 | (wc&0x3f);
		return 2;
	} else if ((unsigned)wc < 0xd800 || (unsigned)wc-0xe000 < 0x2000) {
		*s++ = 0xe0 | (wc>>12);
		*s++ = 0x80 | ((wc>>6)&0x3f);
		*s = 0x80 | (wc&0x3f);
		return 3;
	} else if ((unsigned)wc-0x10000 < 0x100000) {
		*s++ = 0xf0 | (wc>>18);
		*s++ = 0x80 | ((wc>>12)&0x3f);
		*s++ = 0x80 | ((wc>>6)&0x3f);
		*s = 0x80 | (wc&0x3f);
		return 4;
	}
	errno = EILSEQ;
	return -1;
}

size_t wcsrtombs(char *s, const wchar_t **ws, size_t n, mbstate_t *st)
{
	const wchar_t *ws2;
	char buf[4];
	size_t N = n, l;
	if (!s) {
		for (n=0, ws2=*ws; *ws2; ws2++) {
			if (*ws2 >= 0x80u) {
				l = wcrtomb(buf, *ws2, 0);
				if (!(l+1)) return -1;
				n += l;
			} else n++;
		}
		return n;
	}
	while (n>=4) {
		if (**ws-1u >= 0x7fu) {
			if (!**ws) {
				*s = 0;
				*ws = 0;
				return N-n;
			}
			l = wcrtomb(s, **ws, 0);
			if (!(l+1)) return -1;
			s += l;
			n -= l;
		} else {
			*s++ = **ws;
			n--;
		}
		(*ws)++;
	}
	while (n) {
		if (**ws-1u >= 0x7fu) {
			if (!**ws) {
				*s = 0;
				*ws = 0;
				return N-n;
			}
			l = wcrtomb(buf, **ws, 0);
			if (!(l+1)) return -1;
			if (l>n) return N-n;
			wcrtomb(s, **ws, 0);
			s += l;
			n -= l;
		} else {
			*s++ = **ws;
			n--;
		}
		(*ws)++;
	}
	return N;
}

/*
testread 52 34
testread 54 36
testread 48 30
testread 48 30
testread 50 32
testread 56 38
testread 48 30
testread 48 30
testread 48 30
testread 57 39
testread 52 34
testread 55 37
testread 53 35
testread 54 36
testread 55 37
*/

int main(void)
{
	char *buf = "DF@@BH@@@IDGEFG";
	for(int i=0;i<strlen(buf);i++)
        printf("%X ", buf[i]);
    printf("\n");
	
	wchar_t *buf4 = L"你好啊，邻居";
    printf("%lu\n", wcslen(buf4));
    printf("%ls\n", buf4);

    for(int i=0;i<wcslen(buf4);i++)
        printf("%X ", buf4[i]);
    printf("\n");

	wchar_t ar[256] = {L'\0'};
    int read = mbsrtowcs(ar, &buf, strlen(buf), 0 );
    if(read < 0 )
        perror("mbstowcs error");
    printf("%d %lu\n", read, sizeof(wchar_t));

/*
	for (i=0; i<read; i++) 
	{
		tmpData[i] = ar[i]^key[keyIndex];
		if (tmpData[i] == 0) 
		{
			tmpData[i] = ar[i];
		}
		if (++keyIndex == strlen(key)) 
		{
			keyIndex = 0;
		}
	}
*/

    for(int i=0;i<read;i++)
        printf("%X ", ar[i]);
    printf("\n");

	wchar_t *p = &ar;

	char bufs[256] = {0x00};
	int n = wcsrtombs(bufs, &p,18 ,NULL);
	printf("%d \n",n);
	for(int i=0;i<n;i++)
        printf("%X ", bufs[i]);
    printf("\n");

	return 0;
}

