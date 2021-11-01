#include <iostream>

#include <dyn/dyn.h>
#include <gtest/gtest.h>

using namespace std;

DYN_MAKE_INTERFACE(Openable,                                 //
                   (bool, open, (const std::string &, int)), //
                   (bool, close, ()),                        //
                   (bool, isOpen, () const));

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
    bool isOpen() const { return true; }
};

static_assert(!Openable<Test1>);
static_assert(!Openable<Test2>);
static_assert(!Openable<Test3>);
static_assert(Openable<Test4>);

DYN_MAKE_INTERFACE(Computer, (int, compute, (int) const));

TEST(DYN_Tests, test_return) {
    struct C1 {
        int compute(int a) const { return a * 2; }
    };

    struct C2 {
        int compute(int a) const { return a / 2; }
    };

    dyn_Computer c1 = C1{};
    dyn_Computer c2 = C2{};

    ASSERT_EQ(c1.compute(10), 20);
    ASSERT_EQ(c2.compute(10), 5);
}

TEST(DYN_Tests, test_stack_storage) {
    struct C1 {
        int compute(int a) const { return a * 2; }
    };

    struct C2 {
        int compute(int a) const { return a / 2; }
    };

    dyn_Computer<DYN_STACK_STORAGE(16)> c1 = C1{};
    dyn_Computer<DYN_FULL_STACK_STORAGE(16)> c2 = C2{};

    ASSERT_EQ(c1.compute(10), 20);
    ASSERT_EQ(c2.compute(10), 5);
}

TEST(DYN_Tests, test_small_object_storage) {
    struct C1 {
        int compute(int a) const { return a * 2; }
    };

    struct C2 {
        std::array<int, 128> unused;
        int compute(int a) const { return a / 2; }
    };

    dyn_Computer<DYN_SMALL_OBJECT_STORAGE(16)> c1 = C1{};
    dyn_Computer<dyn::small_object_storage<16>::template storage> c2 = C2{};

    ASSERT_EQ(c1.compute(10), 20);
    ASSERT_EQ(c2.compute(10), 5);
}
