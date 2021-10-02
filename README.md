
# Cpp Dyn
Cpp-Dyn tries to C++ improve runtime polymorphism.
Indeed, C++ runtime polymorphism, originally, uses inheritance and virtual methods.
Sean Parent, at least I believe he is the inventor of this idea, uses type erasure for runtime polymorphism. It is better because we can use value instead of pointer/reference, and we don't depend anymore on a base class.

## The Sean Parent Type Erasure Polymorphism
Say we have a Drawable interface, the original way to handle such a thing is something close to
```cpp
struct Drawable {
    void draw(std::ostream &stream) const = 0;
};

struct Square : Drawable {
    void draw(std::ostream &stream) const override;
};

struct Circle : Drawable {
    void draw(std::ostream &stream) const override;
};
```

Sean Parent proposes something close to:
```cpp
struct Drawable {
    struct Concept {
        virtual void draw(std::ostream &stream) const = 0;
    };
    
    template<typename T>
    struct Model {
         Model(T x) : m_object{std::move(x)}{}
         void draw(std::ostream &stream) const override {
             m_object.draw(stream);
         }
         T m_object;
    };

     template<typename T>
     Drawable(T x) : m_ptr{std::make_unique<Model<T>>(std::move(x)){}
};

struct Rectangle {
    void draw(std::ostream &stream) const;
};

struct Circle {
    void draw(std::ostream &stream) const;
};

Drawable rect = Rectangle{};
Drawable circle = Circle{};
```

As we can see, it does not use inheritance, and we can use directly `Drawable` in a vector instead of a `unique_ptr<Drawable>`.

## In rust
I am not a rust developer, but I like the idea behind runtime polymorphism in rust. You can achieve it through trait and `dyn` keyword.

For example:
```rust
pub trait Drawable {
    fn draw(&self);
}

Box<dyn Drawable> drawable;
```

## CppDyn
**CppDyn** tries to bring the best from the two worlds. One issue with the Sean Parent Type Erasure is that it requires a lot of boilerplates... Metaclass may solve this issue, but it is not in C++ 20 and it is not merged (yet?) in C++ 23 either. **CppDyn** will generate the `Concept`and `Model` ~~class~~ boilerplates for you.
We had two ways to generate such code: Python and extensive use of Macro.
The advantage of Python (scripting in general) is the ease to maintain the code even if it means having another language to use. We could generate the code from a JSON file. However, the disadvantage is that the build becomes more complex to understand, and it will be more difficult to integrate it into other projects.
The main issue with Macro is readability, however, all other issues are solved with this method. So, **CppDyn** uses Macro to generate the code.

### Basics
**CppDyn** will generate one `concept` and one class that embeds the dynamic behavior. This class is a template. The template parameter allows you to set a storage _policy_. As such, the storage may be on the heap, on the stack, or a mix depending on the size of the object.

### Create your first interface
The macro 
```cpp
#define DYN_MAKE_INTERFACE(name, methods...)
```
 creates an interface name with the methods.
The methods must be declared is as following: `(returnType, name, (args...) qualifier)`
Let's say we want to create an `Openable` interface. This interface will embed three methods : 
```cpp
bool open(const std::string &path, int mode);
bool close();
bool isOpen() const;
```
You are going to call the macro as follows:
```cpp
DYN_MAKE_INTERFACE(Openable,  //
  (bool, open, (const std::string&, int)),  //
  (bool, close, ()),  //
  (bool, isOpen, () const));
```
As explained prior, this will generate

1. The concept Openable
2. The class template dyn_Openable<Storage = dyn::heap_storage>

Now, you can write such things :

```cpp
struct AnOpenableThing{
/// code
};

void f(Openable auto x) {}
void g(dyn_Openable x) {}
```
The function `f` will be used for static polymorphism, the function `g` will be used for dynamic one.

### Some storage policies
Here are the storage policies provided by **CppDyn**

1. `dyn::heap_storage`: It is a wrapper over a `unique_ptr` and allocates everything on the heap.
2. `dyn::stack_storage<N>`: The maximum size of the `dyn_Object` will be N bytes. Generally, it will accept every object smaller than `N - sizeof(void*)`bytes.
3. `dyn::full_stack_storage<N>`: Equivalent to `stack_storage<N + sizeof(void*)>`. Thus, it will accept every object smaller than N bytes.
4. `dyn::small_object_storage<N>`: It is a mix between `full_stack_storage` and `heap_storage`. Every object smaller than N bytes will be allocated on the stack, and other ones will be allocated on the heap.

Objects that are too heavy for any of stack storage will raise a compile-time error.
For every storage policy but the `heap_storage`, **CppDyn** provides some macros to instance them easily.

```cpp
#define  DYN_STACK_STORAGE(size)
#define  DYN_FULL_STACK_STORAGE(size)
#define  DYN_SMALL_OBJECT_STORAGE(size)
```
Knowing that, you can write :
```cpp
using HeapOpenable = dyn_Openable;
using StackOpenable = dyn_Openable<DYN_FULL_STACK_STORAGE(16)>;
using StackSmallOptimizedOpenable = dyn_Openable<DYN_SMALL_OBJECT_STORAGE(16)>;
```
### Create an interface with a custom storage
You may want to use the following macro:
```cpp
#define DYN_MAKE_INTERFACE_WITH_STORAGE(name, storage, methods...)
```

If you experience any bugs, feel free to report them, or to fix them.
If you have any comments to say, feel free to report them too.

Thanks a lot :)