#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <memory.h>
#include <assert.h>

/*
	m(x) := x^8 + x^4 + x^3 + x + 1
*/

#define MAX_DEGREE 8
typedef struct {
	uint8_t c[MAX_DEGREE];
} Poly;

void clear(Poly *p)
{
	memset(p, 0, sizeof(*p));
}

void set(Poly *p, int pos, int v)
{
	assert(0 <= pos && pos < MAX_DEGREE);
	p->c[pos] = v & 1;
}

int get(const Poly *p, int pos)
{
	return p->c[pos];
}

void incPos(Poly *p, int pos)
{
	assert(0 <= pos && pos < MAX_DEGREE);
	p->c[pos] = 1 - p->c[pos];
}

/*
	deg (0) = -1
*/
int degree(const Poly *p)
{
	int i;
	for (i = MAX_DEGREE - 1; i >= 0; i--) {
		if (p->c[i]) return i;
	}
	return -1;
}

bool isEqual(const Poly *x, const Poly *y)
{
	int i;
	for (i = 0; i < MAX_DEGREE; i++) {
		if (x->c[i] != y->c[i]) return false;
	}
	return true;
}

bool isOne(const Poly *x)
{
	int i;
	if (!x->c[0]) return false;
	for (i = 1; i < MAX_DEGREE; i++) {
		if (x->c[i]) return false;
	}
	return true;
}

int getValue(const Poly *p)
{
	int i;
	int v = 0;
	for (i = 0; i < MAX_DEGREE; i++) {
		if (p->c[i]) v |= 1 << i;
	}
	return v;
}

void setValue(Poly *p, int v)
{
	int i;
	for (i = 0; i < MAX_DEGREE; i++) {
		set(p, i, (v >> i) & 1);
	}
}

void put(const Poly *p)
{
	bool first = true;
	int i;
	for (i = MAX_DEGREE - 1; i >= 0; i--) {
		if (p->c[i]) {
			if (first) {
				first = false;
			} else {
				printf("+");
			}
			if (i == 0) {
				printf("1");
			} else if (i == 1) {
				printf("x");
			} else {
				printf("x^%d", i);
			}
		}
	}
	if (first) printf("0");
}

void add(Poly *z, const Poly *x, const Poly *y)
{
	int i;
	for (i = 0; i < MAX_DEGREE; i++) {
		z->c[i] = (x->c[i] + y->c[i]) & 1;
	}
}

void sub(Poly *z, const Poly *x, const Poly *y)
{
	add(z, x, y);
}

/*
	output f(x)x mod m(x) for input f(x)
*/
void xtime(Poly *z, const Poly *x)
{
	int top = x->c[MAX_DEGREE - 1];
	int i;
	/*
		shift x
	*/
	for (i = MAX_DEGREE - 2; i >= 0; i--) {
		z->c[i + 1] = x->c[i];
	}
	z->c[0] = 0;
	if (top == 0) return;
	/*
		add x^8 = x^4 + x^3 + x + 1
	*/
	incPos(z, 4);
	incPos(z, 3);
	incPos(z, 1);
	incPos(z, 0);
}

/*
	z = x * y mod m(x)
*/
void mul(Poly *z, const Poly *x, const Poly *y)
{
	Poly out;
	Poly shift = *x;
	int i;
	clear(&out);
	for (i = 0; i < MAX_DEGREE; i++) {
		if (y->c[i]) add(&out, &out, &shift);
		xtime(&shift, &shift);
	}
	*z = out;
}

/*
	add (y << n) to x
*/
void shiftAndAdd(Poly *x, const Poly *y, int degY, int n)
{
	int i;
	for (i = 0; i <= degY; i++) {
		if (y->c[i]) incPos(x, i + n);
	}
}
/*
	return false if y = 0 else true
	x = y q + r where deg(r) < deg(x)
*/
bool divMod(Poly *q, Poly *r, const Poly *x, const Poly *y)
{
	Poly xx, yy;
	const int degY = degree(y);
	if (degY < 0) return false;
	if (degY == 0) {
		/* y = 1 */
		*q = *x;
		clear(r);
		return true;
	}
	xx = *x;
	yy = *y;
	clear(q);
	for (;;) {
		int degX = degree(&xx);
		int n;
		if (degX < degY) break;
		n = degX - degY;
		set(q, n, 1);
		shiftAndAdd(&xx, &yy, degY, n);
	}
	*r = xx;
	return true;
}

bool inv(Poly *y, const Poly *x)
{
	/*
		slow algorithm
	*/
	Poly r, rx;
	int i;
	for (i = 1; i < 256; i++) {
		setValue(&r, i);
		mul(&rx, &r, x);
		if (isOne(&rx)) {
			*y = r;
			return true;
		}
	}
	return false;
}

int main()
{
	int i;
	for (i = 1; i < 256; i++) {
		Poly x, y, xy;
		setValue(&x, i);
		inv(&y, &x);
		mul(&xy, &x, &y);
		if (!isOne(&xy)) {
			printf("ERR %d\n", i);
		}
		printf("inv(");
		put(&x);
		printf(")=");
		put(&y);
		printf("\n");
	}
	return 0;
}
