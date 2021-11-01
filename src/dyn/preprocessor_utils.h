#pragma once

#define DYN_STRINGIFY_IMPL(...) #__VA_ARGS__
#define DYN_STRINGIFY(...) DYN_STRINGIFY_IMPL(__VA_ARGS__)

#define DYN_CAT_IMPL(x, ...) x##__VA_ARGS__
#define DYN_CAT(...) DYN_CAT_IMPL(__VA_ARGS__)

#define DYN_EMPTY()
#define DYN_EAT(...)
#define DYN_DEFER(...) __VA_ARGS__ DYN_EMPTY()

#define DYN_EXPAND(...) __VA_ARGS__
#define DYN_EVAL1(...) DYN_EXPAND(DYN_EXPAND(__VA_ARGS__))
#define DYN_EVAL2(...) DYN_EVAL1(DYN_EVAL1(__VA_ARGS__))
#define DYN_EVAL3(...) DYN_EVAL2(DYN_EVAL2(__VA_ARGS__))
#define DYN_EVAL4(...) DYN_EVAL3(DYN_EVAL3(__VA_ARGS__))
#define DYN_EVAL5(...) DYN_EVAL4(DYN_EVAL4(__VA_ARGS__))
#define DYN_EVAL6(...) DYN_EVAL5(DYN_EVAL5(__VA_ARGS__))
#define DYN_EVAL(...) DYN_EVAL6(DYN_EVAL6(__VA_ARGS__))

#define DYN_INC0 1
#define DYN_INC1 2
#define DYN_INC2 3
#define DYN_INC3 4
#define DYN_INC4 5
#define DYN_INC5 6
#define DYN_INC6 7
#define DYN_INC7 8
#define DYN_INC8 9
#define DYN_INC(x) DYN_CAT(DYN_INC, x)

#define DYN_HEAD(x, ...) x
#define DYN_TAIL(x, ...) __VA_ARGS__

#define DYN_STRIP_PARENTHESIS(list) DYN_EXPAND list

#define DYN_FOR_EACH_IMPL(macro, x, ...) macro(x) __VA_OPT__(DYN_DEFER(DYN_FOR_EACH_I)()(macro, __VA_ARGS__))
#define DYN_FOR_EACH_I() DYN_FOR_EACH_IMPL
#define DYN_TRY_FOR_EACH(macro, ...) __VA_OPT__(DYN_FOR_EACH_IMPL(macro, __VA_ARGS__))
#define DYN_FOR_EACH(macro, list) DYN_EVAL(DYN_DEFER(DYN_TRY_FOR_EACH)(macro, DYN_STRIP_PARENTHESIS(list)))

#define DYN_MAP_IMPL(macro, x, ...) macro(x) __VA_OPT__(DYN_DEFER(DYN_MAP_I)()(macro, __VA_ARGS__))
#define DYN_MAP_I() , DYN_MAP_IMPL
#define DYN_TRY_MAP(macro, ...) __VA_OPT__(DYN_MAP_IMPL(macro, __VA_ARGS__))
#define DYN_MAP(macro, list) DYN_DEFER(DYN_TRY_MAP)(macro, DYN_STRIP_PARENTHESIS(list))

#define DYN_ENUMERATE_IMPL(n, x, ...) (n, x) __VA_OPT__(DYN_DEFER(DYN_ENUMERATE_I)()(DYN_INC(n), __VA_ARGS__))
#define DYN_ENUMERATE_I() , DYN_ENUMERATE_IMPL
#define DYN_TRY_ENUMERATE(...) __VA_OPT__(DYN_ENUMERATE_IMPL(0, __VA_ARGS__))
#define DYN_ENUMERATE(list) (DYN_DEFER(DYN_TRY_ENUMERATE)(DYN_STRIP_PARENTHESIS(list)))
