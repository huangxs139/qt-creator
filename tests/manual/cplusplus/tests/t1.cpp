
class Class {
  int a, b;

  enum zoo { a = 1, b = a + 2 + x::y<10>::value };

  enum {};

  typedef enum { k };

  void foo() {}
  inline void bar() {}

  void another_foo() {
    int a = static_cast<int>(1+2/3*4-5%6+(7&8));
  }

  void test_if() {
    if (a == 10) return 1;
    else if (b == 20) return 2;
    else if (c == 30) { x = 1; }
  }
};

class Derived: public Class {
};

class Derived2: public Class, public virtual Derived {};
