/*
	g++-10 -O2 -Warray-bounds array-bounds-bug.cpp
	g++-11.2 shows the same warnings

array-bounds-bug.cpp: In function 'void f()':
array-bounds-bug.cpp:8:29: warning: array subscript 2 is outside array bounds of 'X [1]' [-Warray-bounds]
    8 |   bool isX() const { return isX_; }
      |                             ^~~~
array-bounds-bug.cpp:28:9: note: while referencing '<anonymous>'
   28 |   X(base) == X(base);
      |         ^
array-bounds-bug.cpp:8:29: warning: array subscript 2 is outside array bounds of 'X [1]' [-Warray-bounds]
    8 |   bool isX() const { return isX_; }
*/
struct Base {
  bool isX_;
  Base(bool isX = false) : isX_(isX) { }
  bool isX() const { return isX_; }
  bool operator==(const Base& rhs) const;
};

struct X : public Base {
  X(const Base& b) : Base(true), b_(b) { }
  bool operator==(const X& rhs) const { return b_ == rhs.b_; }
  Base b_;
};

inline bool Base::operator==(const Base& rhs) const
{
    return isX() && rhs.isX() && static_cast<const X&>(*this) == static_cast<const X&>(rhs);
}

Base base;

#ifndef A
void f()
{
  X(base) == X(base);
}
#endif

int main()
{
#ifdef A
  X(base) == X(base);
#endif
}
