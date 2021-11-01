#include "preprocessor_utils.h"
#include <array>
#include <concepts>
#include <iostream>
#include <memory>
#include <string>

#define DYN_FWD(x) ::std::forward<decltype(x) &&>(x)

namespace dyn {
template <typename Base, template <typename> typename Model>
class heap_storage {
  public:
    template <typename T>
    constexpr heap_storage(T &&x) : m_ptr{std::make_unique<Model<std::decay_t<T>>>(DYN_FWD(x))} {}
    constexpr Base *operator->() { return m_ptr.get(); }
    constexpr const Base *operator->() const { return m_ptr.get(); }

  private:
    std::unique_ptr<Base> m_ptr;
};

namespace details {
template <std::size_t Size, typename Base, template <typename> typename Model>
class stack_storage_impl {
    static_assert(Size > sizeof(Base *));
    inline static constexpr auto true_size = Size - sizeof(Base *);
    inline static constexpr auto deleter = [](Base *base) { base->~Base(); };

  public:
    template <typename T>
    requires(sizeof(T) <= true_size) constexpr stack_storage_impl(T &&x) :
        m_ptr{new (m_storage.data()) Model<std::decay_t<T>>(DYN_FWD(x))} {}

    constexpr Base *operator->() { return m_ptr.get(); }
    constexpr const Base *operator->() const { return m_ptr.get(); }

  private:
    std::array<std::byte, true_size> m_storage;
    std::unique_ptr<Base, decltype(deleter)> m_ptr;
};
} // namespace details

template <std::size_t N>
struct stack_storage {
    template <typename Base, template <typename> typename Model>
    using storage = details::stack_storage_impl<N, Base, Model>;
};

template <std::size_t N>
using full_stack_storage = stack_storage<N + sizeof(void *)>;

namespace details {
template <std::size_t Size, typename Base, template <typename> typename Model>
class small_object_storage_impl {
    static_assert(Size >= sizeof(std::unique_ptr<Base>));

    struct vtable_t {
        void (*dtor)(void *);
        Base *(*get)(void *);
        const Base *(*get_const)(const void *);
    };

    template <typename T, bool is_small = (sizeof(T) <= Size)>
    struct make_vtable;

    template <typename T>
    struct make_vtable<T, false> {
        static constexpr auto dtor = [](void *p) { static_cast<std::unique_ptr<Model<T>> *>(p)->~unique_ptr(); };

        static constexpr auto get = [](void *p) {
            return static_cast<Base *>(static_cast<std::unique_ptr<Model<T>> *>(p)->get());
        };

        static constexpr auto get_const = [](const void *p) {
            return static_cast<const Base *>(static_cast<const std::unique_ptr<Model<T>> *>(p)->get());
        };

        static constexpr auto vtable = vtable_t{dtor, get, get_const};
    };

    template <typename T>
    struct make_vtable<T, true> {
        static constexpr auto dtor = [](void *p) { static_cast<Model<T> *>(p)->~Model<T>(); };

        static constexpr auto get = [](void *p) { return static_cast<Base *>(static_cast<Model<T> *>(p)); };

        static constexpr auto get_const = [](const void *p) {
            return static_cast<const Base *>(static_cast<const Model<T> *>(p));
        };

        static constexpr auto vtable = vtable_t{dtor, get, get_const};
    };

    const vtable_t *m_vtable;
    std::array<std::byte, Size> m_storage;

  public:
    template <typename T>
    constexpr small_object_storage_impl(T &&t) : m_vtable{&make_vtable<std::decay_t<T>>::vtable} {
        if constexpr (sizeof(t) <= Size) {
            new (m_storage.data()) Model<T>{DYN_FWD(t)};
        } else {
            new (m_storage.data()) std::unique_ptr<Model<T>>{std::make_unique<Model<T>>(DYN_FWD(t))};
        }
    }

    constexpr Base *operator->() { return m_vtable->get(m_storage.data()); }
    constexpr const Base *operator->() const { return m_vtable->get_const(m_storage.data()); }
};
} // namespace details

template <std::size_t N>
struct small_object_storage {
    template <typename Base, template <typename> typename Model>
    using storage = details::small_object_storage_impl<N + sizeof(void *), Base, Model>;
};

} // namespace dyn

#define DYN_KEEP_CONST(...) DYN_EAT __VA_ARGS__
#define DYN_REMOVE_CONST_IMPL(...) __VA_ARGS__ DYN_EAT (
#define DYN_REMOVE_CONST(...) (DYN_REMOVE_CONST_IMPL __VA_ARGS__ ) )

#define DYN_MAKE_DECLVAL_CALL(type) ::std::declval<type>()
#define DYN_MAKE_CONCEPT_INTERFACE_IMPL(r, name, args)                                                                 \
    {                                                                                                                  \
        std::declval<T DYN_KEEP_CONST(args)>().name(DYN_MAP(DYN_MAKE_DECLVAL_CALL, DYN_REMOVE_CONST(args)))            \
        } -> ::std::same_as<r>;

#define DYN_MAKE_CONCEPT_INTERFACE(...) DYN_MAKE_CONCEPT_INTERFACE_IMPL __VA_ARGS__

#define DYN_MAKE_CONCEPT(nameConcept, listInterface)                                                                   \
    template <typename T>                                                                                              \
    concept nameConcept = requires() {                                                                                 \
        DYN_FOR_EACH(DYN_MAKE_CONCEPT_INTERFACE, listInterface)                                                        \
    }

#define DYN_MAKE_INTERFACE_VIRTUAL_IMPL(r, name, args) virtual r name DYN_REMOVE_CONST(args) DYN_KEEP_CONST(args) = 0;
#define DYN_MAKE_INTERFACE_VIRTUAL(...) DYN_MAKE_INTERFACE_VIRTUAL_IMPL __VA_ARGS__

#define DYN_MAKE_ARGUMENT_IMPL(n, type) type DYN_CAT(_, n)
#define DYN_MAKE_ARGUMENT(nAndType) DYN_MAKE_ARGUMENT_IMPL nAndType

#define DYN_MAKE_FWD_ARGUMENT_IMPL(n, type) DYN_FWD(DYN_CAT(_, n))
#define DYN_MAKE_FWD_ARGUMENT(nAndType) DYN_MAKE_FWD_ARGUMENT_IMPL nAndType

#define DYN_MAKE_INTERFACE_IMPLEMENTATION_IMPL(r, name, args)                                                          \
    r name(DYN_MAP(DYN_MAKE_ARGUMENT, DYN_ENUMERATE(DYN_REMOVE_CONST(args)))) DYN_KEEP_CONST(args) override {          \
        return object.name(DYN_MAP(DYN_MAKE_FWD_ARGUMENT, DYN_ENUMERATE(DYN_REMOVE_CONST(args))));                     \
    }
#define DYN_MAKE_INTERFACE_IMPLEMENTATION(...) DYN_MAKE_INTERFACE_IMPLEMENTATION_IMPL __VA_ARGS__

#define DYN_MAKE_INTERFACE_IMPLEMENTATION_PTR_IMPL(r, name, args)                                                      \
    r name(DYN_MAP(DYN_MAKE_ARGUMENT, DYN_ENUMERATE(DYN_REMOVE_CONST(args)))) DYN_KEEP_CONST(args) {                   \
        return m_storage->name(DYN_MAP(DYN_MAKE_FWD_ARGUMENT, DYN_ENUMERATE(DYN_REMOVE_CONST(args))));                 \
    }
#define DYN_MAKE_INTERFACE_IMPLEMENTATION_PTR(...) DYN_MAKE_INTERFACE_IMPLEMENTATION_PTR_IMPL __VA_ARGS__

#define DYN_MAKE_INTERFACE_CLASS(name, nameConcept, storage, listInterface)                                            \
    template <template <typename, template <typename> typename> typename Storage = storage>                            \
    class name {                                                                                                       \
        struct virtual_base {                                                                                          \
            virtual ~virtual_base() {}                                                                                 \
            DYN_FOR_EACH(DYN_MAKE_INTERFACE_VIRTUAL, listInterface)                                                    \
        };                                                                                                             \
        template <nameConcept T>                                                                                       \
        struct model : virtual_base {                                                                                  \
            model(T x) : object{std::move(x)} {}                                                                       \
            T object;                                                                                                  \
            DYN_FOR_EACH(DYN_MAKE_INTERFACE_IMPLEMENTATION, listInterface)                                             \
        };                                                                                                             \
        Storage<virtual_base, model> m_storage;                                                                        \
                                                                                                                       \
      public:                                                                                                          \
        template <nameConcept T>                                                                                       \
        name(T &&x) : m_storage{DYN_FWD(x)} {}                                                                         \
                                                                                                                       \
        DYN_FOR_EACH(DYN_MAKE_INTERFACE_IMPLEMENTATION_PTR, listInterface)                                             \
    }

#define DYN_MAKE_INTERFACE_WITH_STORAGE(name, storage, ...)                                                            \
    DYN_MAKE_CONCEPT(name, (__VA_ARGS__));                                                                             \
    DYN_MAKE_INTERFACE_CLASS(DYN_CAT(dyn_, name), name, storage, (__VA_ARGS__))

#define DYN_MAKE_INTERFACE(name, ...) DYN_MAKE_INTERFACE_WITH_STORAGE(name, ::dyn::heap_storage, __VA_ARGS__)

#define DYN_STACK_STORAGE(size) ::dyn::stack_storage<size>::template storage
#define DYN_FULL_STACK_STORAGE(size) ::dyn::full_stack_storage<size>::template storage

#define DYN_SMALL_OBJECT_STORAGE(size) ::dyn::small_object_storage<size>::template storage
