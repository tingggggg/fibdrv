#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn.h"
#include "mem_pool.h"

MemoryPool *pool;

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#ifndef SWAP
#define SWAP(x, y)           \
    do {                     \
        typeof(x) __tmp = x; \
        x = y;               \
        y = __tmp;           \
    } while (0)
#endif

#ifndef DIV_ROUNDUP
#define DIV_ROUNDUP(x, len) (((x) + (len) -1) / len)
#endif

/* count leading zeros of src->number */
static int bn_clz(const bn *src)
{
    int cnt = 0;
    for (int i = src->size - 1; i >= 0; i--) {
        if (src->number[i]) {
            // avoid undefined behavior when src = 0
            return cnt + builtin_clz(src->number[i]);
        } else {
            cnt += DATA_BITS;
        }
    }
    return cnt;
}

/* count the digits of most significant bit */
// static int bn_msb(const bn *src)
// {
//     return src->size * 32 - bn_clz(src);
// }

/*
 * output bn to decimal string
 * Note: the returned string should be freed with the kfree()
 */
char *bn_to_string(const bn *src)
{
    size_t len = (8 * sizeof(bn_data) * src->size) / 3 + 2 + src->sign;
    char *s = (char *) malloc(len);
    char *p = s;

    memset(p, '0', len - 1);
    p[len - 1] = '\0';

    for (int i = src->size - 1; i >= 0; i--) {
        for (bn_data d = MSB_MASK; d; d >>= 1) {
            int carry = !!(d & src->number[i]);
            for (int j = len - 2; j >= 0; j--) {
                p[j] += p[j] - '0' + carry;
                carry = (p[j] > '9');
                if (carry)
                    p[j] -= 10;
            }
        }
    }
    // skip leading zero
    while (p[0] == '0' && p[1] != '\0')
        p++;
    if (src->sign)
        *(--p) = '-';
    memmove(s, p, strlen(p) + 1);
    return s;
}

/*
 * alloc a bn structure with the given size
 * the value is initialized to +0
 */
bn *bn_alloc(size_t size)
{
    bn *new = (bn *) malloc(sizeof(bn));
    new->size = size;
    new->capacity = size > INIT_ALLOC_SIZE ? size : INIT_ALLOC_SIZE;
    // new->number = (bn_data *) malloc(sizeof(bn_data) * new->capacity);
    new->number = (bn_data *) my_malloc(pool, sizeof(bn_data) * new->capacity);
    // if (!new->number)
    //     return NULL;
    memset(new->number, 0, sizeof(bn_data) * size);

    new->sign = 0;
    return new;
}

/*
 * free entire bn data structure
 * return 0 on success, -1 on error
 */
int bn_free(bn *src)
{
    if (src == NULL)
        return -1;
    // free(src->number);
    my_free(pool, src->number);
    free(src);
    return 0;
}

/*
 * resize bn
 * return 0 on success, -1 on error
 */
static int bn_resize(bn *src, size_t size)
{
    if (!src)
        return -1;
    if (size == src->size)
        return 0;
    if (size == 0)
        size = 1;

    if (size > src->capacity) {
        src->capacity =
            (size + (ALLOC_CHUNK_SIZE - 1)) & ~(ALLOC_CHUNK_SIZE - 1);
        // src->number = realloc(src->number, sizeof(bn_data) * src->capacity);
        src->number =
            my_realloc(pool, src->number, sizeof(bn_data) * src->capacity);

        // bn_data *tmp = (bn_data *) my_malloc(pool,  sizeof(bn_data) *
        // src->capacity); memcpy(tmp, src->number, sizeof(bn_data) *
        // src->size); my_free(pool, src->number); src->number = tmp;

        if (!src->number) /* realloc failed */
            return -1;
    }

    if (size > src->size)
        memset(src->number + src->size, 0,
               sizeof(bn_data) * (size - src->size));

    src->size = size;
    return 0;
}

/* trim unused space(size) */
static void bn_trim(bn *src)
{
    while (src->size > 1 && !src->number[src->size - 1])
        src->size--;
}

/*
 * copy the value from src to dest
 * return 0 on success, -1 on error
 */
int bn_cpy(bn *dest, bn *src)
{
    if (bn_resize(dest, src->size) < 0)
        return -1;
    dest->sign = src->sign;
    memcpy(dest->number, src->number, src->size * sizeof(bn_data));
    return 0;
}

/* swap bn ptr */
void bn_swap(bn *a, bn *b)
{
    bn tmp = *a;
    *a = *b;
    *b = tmp;
}

/*
 * compare length
 * return 1 if |a| > |b|
 * return -1 if |a| < |b|
 * return 0 if |a| = |b|
 */
int bn_cmp(const bn *a, const bn *b)
{
    if (a->size > b->size) {
        return 1;
    } else if (a->size < b->size) {
        return -1;
    } else {
        for (int i = a->size - 1; i >= 0; i--) {
            if (a->number[i] > b->number[i])
                return 1;
            if (a->number[i] < b->number[i])
                return -1;
        }
        return 0;
    }
}

/* left bit shift on bn (maximun shift 31) */
void bn_lshift(const bn *src, size_t shift, bn *dest)
{
    size_t z = bn_clz(src);
    shift %= DATA_BITS;  // only handle shift within DATA_BITS bits
    if (!shift)
        return;

    /* add size if space not enough */
    if (shift > z) {
        bn_resize(dest, src->size + 1);
        dest->number[src->size] =
            src->number[src->size - 1] >> (DATA_BITS - shift);
    } else {
        bn_resize(dest, src->size);
    }

    /* bit shift */
    for (int i = src->size - 1; i > 0; i--)
        dest->number[i] =
            src->number[i] << shift | src->number[i - 1] >> (DATA_BITS - shift);
    dest->number[0] = src->number[0] << shift;
}

/* |c| = |a| + |b| */
static void bn_do_add(const bn *a, const bn *b, bn *c)
{
    if (a->size < b->size)  // a->size >= b->size
        SWAP(a, b);
    int asize = a->size, bsize = b->size;
    if ((asize + 1) > c->capacity) {  // only change the capacity, not the size
        c->capacity = (asize + 1 + (ALLOC_CHUNK_SIZE - 1)) &
                      ~(ALLOC_CHUNK_SIZE - 1);  // ceil to 4*n
        // c->number = realloc(c->number, sizeof(bn_data) * c->capacity);
        c->number = my_realloc(pool, c->number, sizeof(bn_data) * c->capacity);

        // bn_data *tmp = (bn_data *) my_malloc(pool, sizeof(bn_data) *
        // c->capacity); memcpy(tmp, c->number, sizeof(bn_data) * c->size);
        // my_free(pool, c->number);
        // c->number = tmp;
    }
    c->size = asize;

    bn_data carry = 0;
    for (int i = 0; i < bsize; i++) {
        bn_data tmp1 = a->number[i];
        bn_data tmp2 = b->number[i];
        carry = (tmp1 += carry) < carry;
        carry += (c->number[i] = tmp1 + tmp2) < tmp2;
    }
    for (int i = bsize; i < asize; i++) {
        bn_data tmp1 = a->number[i];
        carry = (c->number[i] = tmp1 + carry) < carry;
    }

    c->number[asize] = carry;
    c->size += !!(carry);
}

/*
 * |c| = |a| - |b|
 * Note: |a| > |b| must be true
 */
static void bn_do_sub(const bn *a, const bn *b, bn *c)
{
    // max digits = max(sizeof(a), sizeof(b))
    int asize = a->size, bsize = b->size;
    bn_resize(c, asize);

    bn_data borrow = 0;
    for (int i = 0; i < c->size; i++) {
        // bn_data tmp1 = (i < asize) ? a->number[i] : 0;
        // bn_data tmp2 = (i < bsize) ? b->number[i] : 0;

        // borrow = (long long int) tmp1 - tmp2 - borrow;
        // if (borrow < 0) {  // borrow from next
        //     c->number[i] = borrow + (1LL << 32);
        //     borrow = 1;
        // } else {
        //     c->number[i] = borrow;
        //     borrow = 0;
        // }
        bn_data_tmp tmp1 = (i < asize) ? a->number[i] : 0;
        bn_data_tmp tmp2 = (i < bsize) ? b->number[i] : 0;
        tmp1 += DATA_MASK + 1;  // pre-borrow
        tmp2 += borrow;
        borrow = tmp1 - tmp2;
        c->number[i] = borrow;
        borrow = (borrow <= DATA_MASK);
    }

    bn_trim(c);
}

/*
 * c = a + b
 * work for (c == a) or (c == b)
 */
void bn_add(const bn *a, const bn *b, bn *c)
{
    if (a->sign == b->sign) {  // both a & b are positive or negative
        bn_do_add(a, b, c);
        c->sign = a->sign;
    } else {          // different sign
        if (a->sign)  // let a > 0, b < 0
            SWAP(a, b);
        int cmp = bn_cmp(a, b);  // compare a & b length
        if (cmp > 0) {
            /* |a| > |b| and b < 0, hence c = a - |b| */
            bn_do_sub(a, b, c);
            c->sign = 0;
        } else if (cmp < 0) {
            /* |a| < |b| and b < 0, hence c = -(|b| - |a|) */
            bn_do_sub(b, a, c);
            c->sign = 1;
        } else {
            /* |a| == |b| */
            bn_resize(c, 1);
            c->number[0] = 0;
            c->sign = 0;
        }
    }
}

/*
 * c = a - b
 * work for (c == a) or (c == b)
 */
void bn_sub(const bn *a, const bn *b, bn *c)
{
    bn tmp = *b;
    tmp.sign ^= 1;  // a - b = a + (-b)
    bn_add(a, &tmp, c);
}

/* c[size] += a[size] * k, and return the carry */
static bn_data _mult_partial(const bn_data *a,
                             bn_data asize,
                             const bn_data k,
                             bn_data *c)
{
    if (k == 0)
        return 0;

    bn_data carry = 0;
    for (int i = 0; i < asize; i++) {
        bn_data high, low;
        bn_data_tmp prod = (bn_data_tmp) a[i] * k;
        low = prod;
        high = prod >> DATA_BITS;
        carry = high + ((low += carry) < carry);
        carry += ((c[i] += low) < low);
    }

    return carry;
}

/* c = a x b */
void bn_mult(const bn *a, const bn *b, bn *c)
{
    // max digits = sizeof(a) + sizeof(b)
    int d = a->size + b->size;
    bn *tmp;

    /* make it work properly when c == a or c == b */
    if (c == a || c == b) {
        tmp = c;
        c = bn_alloc(d);
    } else {
        tmp = NULL;
        for (int i = 0; i < c->size; i++)
            c->number[i] = 0;  // clean up c
        bn_resize(c, d);
    }

    for (int j = 0; j < b->size; j++) {
        c->number[a->size + j] =
            _mult_partial(a->number, a->size, b->number[j], c->number + j);
    }
    c->sign = a->sign ^ b->sign;

    bn_trim(c);
    if (tmp) {
        bn_cpy(tmp, c);  // copy value to original c
        bn_free(c);
    }
}

/* calculate n-th Fibonacci number and save into dest */
void bn_fib(bn *dest, unsigned int n)
{
    bn_resize(dest, 1);
    if (n < 2) {
        dest->number[0] = n;
        return;
    }

    bn *f0 = bn_alloc(1);
    bn *f1 = bn_alloc(1);
    f1->number[0] = 1;

    for (unsigned int i = 2; i <= n; i++) {
        bn_add(f0, f1, dest);
        bn_swap(f0, f1);
        bn_cpy(f1, dest);
    }

    bn_free(f0);
    bn_free(f1);
}

/*
 * calculate n-th Fibonacci number and save into dest
 * using fast doubling algorithm
 */
void bn_fib_fdoubling(bn *dest, unsigned int n)
{
    bn_resize(dest, 1);
    if (n < 2) {
        dest->number[0] = n;
        return;
    }

    bn *f1 = bn_alloc(1); /* F(k - 1) */
    bn *f2 = dest;        /* F(k) */
    f1->number[0] = 0;
    f2->number[0] = 1;
    bn *k1 = bn_alloc(1);
    bn *k2 = bn_alloc(1);

    /* walk through the digit of n */
    for (unsigned int i = 1U << (30 - __builtin_clz(n)); i; i >>= 1) {
        /* F(2k-1) = F(k)^2 + F(k-1)^2 */
        /* F(2k) = F(k) * [ 2 * F(k-1) + F(k) ] */
        bn_lshift(f1, 1, k1);  // k1 = 2 * F(k-1)
        bn_add(k1, f2, k1);    // k1 = 2 * F(k-1) + F(k)
        bn_mult(k1, f2, k2);   // k2 = k1 * f2 = F(2k)
        bn_mult(f2, f2, k1);   // k1 = F(k)^2
        bn_swap(f2, k2);       // k2 <-> f2, f2 = F(2k)
        bn_mult(f1, f1, k2);   // k2 = F(k - 1)^2
        bn_add(k2, k1, f1);    // f1 = k1 + k2 = F(2k - 1)
        if (n & i) {
            bn_swap(f1, f2);     // f1 = F(2k)
            bn_add(f1, f2, f2);  // f2 = F(2k + 1)
        }
    }

    bn_free(f1);
    bn_free(k1);
    bn_free(k2);
}