#include "preprocessor_utils.h"
#include <concepts>
#include <iostream>
#include <memory>
#include <string>

#define DYN_KEEP_CONST(...) DYN_EAT __VA_ARGS__
#define DYN_REMOVE_CONST_IMPL(...) __VA_ARGS__ DYN_EAT (
#define DYN_REMOVE_CONST(...) (DYN_REMOVE_CONST_IMPL __VA_ARGS__ ) )

#define DYN_MAKE_DECLVAL_CALL(type) ::std::declval<type>()
#define DYN_MAKE_CONCEPT_INTERFACE_IMPL(r, name, args)                         \
  {                                                                            \
    std::declval<T DYN_KEEP_CONST(args)>().name(                               \
        DYN_MAP(DYN_MAKE_DECLVAL_CALL, DYN_REMOVE_CONST(args)))                \
  }                                                                            \
  ->::std::same_as<r>;

#define DYN_MAKE_CONCEPT_INTERFACE(...)                                        \
  DYN_MAKE_CONCEPT_INTERFACE_IMPL __VA_ARGS__

#define DYN_MAKE_CONCEPT(nameConcept, listInterface)                           \
  template <typename T> concept nameConcept = requires() {                     \
    DYN_FOR_EACH(DYN_MAKE_CONCEPT_INTERFACE, listInterface)                    \
  }

#define DYN_MAKE_INTERFACE_VIRTUAL_IMPL(r, name, args)                         \
  virtual r name DYN_REMOVE_CONST(args) DYN_KEEP_CONST(args) = 0;
#define DYN_MAKE_INTERFACE_VIRTUAL(...)                                        \
  DYN_MAKE_INTERFACE_VIRTUAL_IMPL __VA_ARGS__

#define DYN_MAKE_ARGUMENT_IMPL(n, type) type DYN_CAT(_, n)
#define DYN_MAKE_ARGUMENT(nAndType) DYN_MAKE_ARGUMENT_IMPL nAndType

#define DYN_FWD(x) ::std::forward<decltype(x) &&>(x)

#define DYN_MAKE_FWD_ARGUMENT_IMPL(n, type) DYN_FWD(DYN_CAT(_, n))
#define DYN_MAKE_FWD_ARGUMENT(nAndType) DYN_MAKE_FWD_ARGUMENT_IMPL nAndType

#define DYN_MAKE_INTERFACE_IMPLEMENTATION_IMPL(r, name, args)                  \
  r name(DYN_MAP(DYN_MAKE_ARGUMENT, DYN_ENUMERATE(DYN_REMOVE_CONST(args))))    \
      DYN_KEEP_CONST(args) override {                                          \
    return object.name(DYN_MAP(DYN_MAKE_FWD_ARGUMENT,                          \
                               DYN_ENUMERATE(DYN_REMOVE_CONST(args))));        \
  }
#define DYN_MAKE_INTERFACE_IMPLEMENTATION(...)                                 \
  DYN_MAKE_INTERFACE_IMPLEMENTATION_IMPL __VA_ARGS__

#define DYN_MAKE_INTERFACE_IMPLEMENTATION_PTR_IMPL(r, name, args)              \
  r name(DYN_MAP(DYN_MAKE_ARGUMENT, DYN_ENUMERATE(DYN_REMOVE_CONST(args))))    \
      DYN_KEEP_CONST(args) {                                                   \
    return m_ptr->name(DYN_MAP(DYN_MAKE_FWD_ARGUMENT,                          \
                               DYN_ENUMERATE(DYN_REMOVE_CONST(args))));        \
  }
#define DYN_MAKE_INTERFACE_IMPLEMENTATION_PTR(...)                             \
  DYN_MAKE_INTERFACE_IMPLEMENTATION_PTR_IMPL __VA_ARGS__

#define DYN_MAKE_INTERFACE_CLASS(name, nameConcept, listInterface)             \
  class name {                                                                 \
    struct virtual_base {                                                      \
      virtual ~virtual_base() {}                                               \
      DYN_FOR_EACH(DYN_MAKE_INTERFACE_VIRTUAL, listInterface)                  \
    };                                                                         \
    template <typename T> struct model : virtual_base {                        \
      model(T x) : object{std::move(x)} {}                                     \
      T object;                                                                \
      DYN_FOR_EACH(DYN_MAKE_INTERFACE_IMPLEMENTATION, listInterface)           \
    };                                                                         \
    std::unique_ptr<virtual_base> m_ptr;                                       \
                                                                               \
  public:                                                                      \
    template <nameConcept T>                                                   \
    name(T &&x)                                                                \
        : m_ptr{std::make_unique<model<std::decay_t<T>>>(DYN_FWD(x))} {}       \
                                                                               \
    DYN_FOR_EACH(DYN_MAKE_INTERFACE_IMPLEMENTATION_PTR, listInterface)         \
  }

#define DYN_MAKE_INTERFACE(name, ...)                                          \
  DYN_MAKE_CONCEPT(DYN_CAT(name, Concept), (__VA_ARGS__));                     \
  DYN_MAKE_INTERFACE_CLASS(name, DYN_CAT(name, Concept), (__VA_ARGS__))
