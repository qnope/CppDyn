#include <iostream>

#include <dyn/dyn.h>
#include <gtest/gtest.h>

using namespace std;

DYN_MAKE_INTERFACE(Openable,                                 //
                   (bool, open, (const std::string &, int)), //
                   (bool, close, ()));

struct Test1 {
  void open() {}
};

struct Test2 {
  void open(std::string) {}
  void close();
};

struct Test3 {
  bool open(std::string, int) { return true; }
};

struct Test4 {
  bool open(std::string, int) { return true; }
  bool close() { return true; }
};

static_assert(!OpenableConcept<Test1>);
static_assert(!OpenableConcept<Test2>);
static_assert(!OpenableConcept<Test3>);
static_assert(OpenableConcept<Test4>);

DYN_MAKE_INTERFACE(Computer, (int, compute, (int)const));

TEST(DYN_Tests, test_return) {
  struct C1 {
    int compute(int a) const { return a * 2; }
  };

  struct C2 {
    int compute(int a) const { return a / 2; }
  };

  Computer c1 = C1{};
  Computer c2 = C2{};

  ASSERT_EQ(c1.compute(10), 20);
  ASSERT_EQ(c2.compute(10), 5);
}
