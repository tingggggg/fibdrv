#ifndef _BN_H_
#define _BN_H_

#include <stddef.h>
#include <stdint.h>

#if defined(__LP64__) || defined(__x86_64__) || defined(__amd64__) || defined(__aarch64__)
#define BN_WSIZE 8
#else
#define BN_WSIZE 4
#endif

#if BN_WSIZE == 8
typedef uint64_t bn_data;
typedef unsigned __int128 bn_data_tmp;
#define DATA_BITS 64U
#define MSB_MASK __UINT64_C(0x8000000000000000)
#define DATA_MASK __UINT64_C(0xffffffffffffffff)
#define builtin_clz(x) __builtin_clzll(x)
#elif BN_WSIZE == 4
typedef uint32_t bn_data;
typedef uint64_t bn_data_tmp;
#define DATA_BITS 32U
#define MSB_MASK __UINT64_C(0x80000000)
#define DATA_MASK __UINT64_C(0xffffffff)
#define builtin_clz(x) __builtin_clz(x)
#else
#error "BN_WSIZE must be 4 or 8"
#endif

/*
 * bignum data structure
 * number[0] contains least significant bits
 * number[size - 1] contains most significant bits
 * sign = 1 for negative number
 */
typedef struct _bn {
    bn_data *number;
    int size;
    int capacity; /* Allocated length, size <= capacity */
    int sign;
} bn;

#define INIT_ALLOC_SIZE 4
#define ALLOC_CHUNK_SIZE 4

/*
 * output bn to decimal string
 * Note: the returned string should be freed with the kfree()
 */
char *bn_to_string(const bn *src);

/*
 * free entir bn data structur
 * return 0 on success, -1 on error
 */
int bn_free(bn *src);

/*
 * copy the value from src to dest
 * return 0 on success, -1 on error
 */
int bn_cpy(bn *dest, bn *src);

/*
 * compare length
 * return 1 if |a| > |b|
 * return -1 if |a| < |b|
 * return 0 if |a| = |b|
 */
int bn_cmp(const bn *a, const bn *b);

/*
 * alloc a bn structure with the given size
 * the value is initialized to +0
 */
bn *bn_alloc(size_t size);

/* left bit shift on bn (maximun shift 31) */
void bn_lshift(const bn *src, size_t shift, bn *dest);

/* c = a + b */
void bn_add(const bn *a, const bn *b, bn *c);

/* c = a - b */
void bn_sub(const bn *a, const bn *b, bn *c);

/* c = a x b */
void bn_mult(const bn *a, const bn *b, bn *c);

/* Calculate n-th Fibonacci number and save into dest */
void bn_fib(bn *dest, unsigned int n);
void bn_fib_fdoubling(bn *dest, unsigned int n);

#endif /* _BN_H_ */