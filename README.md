# _reg_, a registry library

The _reg_ library gives an identity to your value-types, turning them into objects stored in a `reg::Registry`. You can reference and access your objects through a `reg::Id`.

You can see `reg::Id`s as references which will never get invalidated (unless their owner gets destroyed (`reg::UniqueId` or `reg::SharedId`)). Even after an object has been destroyed it is safe to query the registry for the destroyed object through its id: the registry will simply return null (`nullptr` or `std::nullopt`) and you will have to handle the fact that the object no longer exists. Basically this is like a reference which knows whether it is dangling or not and will never let you read garbage memory.

These references will never get invalidated, even upon restarting your application: they are safe to serialize.

## Table of Content

- [Table of Content](#table-of-content)
- [Use case](#use-case)
  - [When to prefer a registry to a `std::vector`](#when-to-prefer-a-registry-to-a-stdvector)
  - [When to prefer a registry to a `std::list`](#when-to-prefer-a-registry-to-a-stdlist)
  - [Summary](#summary)
- [Including](#including)
- [Tutorial](#tutorial)
  - [Creating an object](#creating-an-object)
  - [Accessing an object](#accessing-an-object)
  - [Modifying an object](#modifying-an-object)
  - [Owning IDs](#owning-ids)
  - [Checking for the existence of an object](#checking-for-the-existence-of-an-object)
  - [Iterating over all the objects](#iterating-over-all-the-objects)
  - [Manual lifetime management](#manual-lifetime-management)
  - [Thread safety](#thread-safety)
  - [`AnyId`](#anyid)
  - [`Registries`](#registries)
  - [`to_string()`](#to_string)
  - [`is_empty()`](#is_empty)
  - [`clear()`](#clear)
  - [Serialization and _cereal_ support](#serialization-and-cereal-support)
  - [`underlying_xxx()`](#underlying_xxx)
  - [More examples](#more-examples)
- [Notes](#notes)
  - [Why can't I just use native pointers (\*) or references (\&)?](#why-cant-i-just-use-native-pointers--or-references-)
  - [Performance is not our main concern](#performance-is-not-our-main-concern)
- [Future developments](#future-developments)
  - [`for_each` functions](#for_each-functions)
- [Running the tests](#running-the-tests)

## Use case

This library was designed for this specific use case:

**_You want to create user-facing objects that will be referenceable across the application, as in Blender's Scene Collection:_**

![](./docs/img/blender-hierarchy.png)

> The _Cube_'s constraint references the _MyPosition_ object. We want this reference to live forever (or at least until the user changes it).

### When to prefer a registry to a `std::vector`

Say you stored your objects in a `std::vector`, and stored iterators to elements of that vector in order to reference them. Adding or removing elements to a `std::vector` can **invalidate all of the iterators**, so this is no good for our use case.

But there is more! **Indices get invalidated too**, so they are no better than iterators to store references to elements. For example if you remove the first element of the vector, all indices should be decremented by one if you want them to keep referencing the same elements.

### When to prefer a registry to a `std::list`

Based on what we said about `std::vector`, we might think that `std::list` is a better match for our needs. And indeed, the list's iterators never get invalidated... unless you remove the element they were pointing to. And when iterators get invalidated, you have no way of knowing that! This is one of the points where a registry shines: **it always allows you to know if an id is valid or not**. This is very important in the case where any part of your application could delete the object at any time, and the other parts that had a reference to that object need to realize that and react accordingly. None of the STL iterators allow you to know if they are valid or not, nor if it is safe to dereference them or not.

The second advantage of registries is **serialization**. Iterators are tied to an address in memory and cannot be serialized (a.k.a. saved to a file or sent over a network). An id on the other hand is just a unique number that always allows you to reference the object. When a registry is loaded from a file, all the ids are kept as they were when saving the file and they can still be used to reference the same objects.

### Summary

If you have a need for at least one of thoses properties, you might prefer registries and ids to the STL containers and their iterators:
- **Robust references** that only get invalidated when the referenced object is destroyed, and that allow you to know if the reference is still valid or not.
- **Serialization**.

## Including

To add this library to your project, simply add these two lines to your *CMakeLists.txt*:
```cmake
add_subdirectory(path/to/reg)
target_link_libraries(${PROJECT_NAME} PRIVATE reg::reg)
```

Then include it as:
```cpp
#include <reg/reg.hpp>
```

## Tutorial

### Creating an object

A `reg::Registry` stores values of a given type. For example you can create a

```cpp
reg::Registry<float> registry{};
```

You can then create objects in the registry:

```cpp
reg::UniqueId<float> id = registry.create_unique(5.f);
```

This will return you an id that you can later use to get your object back from the registry.

### Accessing an object

The preferred way is to use `get()`.

Since the object could have been destroyed at any time by another part of the application you always have to check that `get()` actually returned you a value:

```cpp
std::optional<float> const maybe_value = registry.get(id);
if (maybe_value) // Check that `maybe_value` contains a value
{
    std::cout << "My value is "
              << std::to_string(*maybe_value) // Use the value `*maybe_value`
              << '\n';
}
```

If you cannot afford to pay the cost of the copy (for example when storing big `std::vector`s in your registry) you can use `with_ref()` instead:

```cpp
bool const success = registry.with_ref(id, [](float const& value) { // The callback will only be called if the id was found in the registry
    std::cout << "My value is "
                << std::to_string(value)
                << '\n';
});

if (!success) // You can check if with_ref() successfully found the id
{
    std::cout << "Object not found!\n";
}
```

A third-alternative is to use `get_ref()`, but it is not recommended because you have to handle the thread-safety manually:

```cpp
{
    std::shared_lock lock{registry.mutex()}; // I only want to read so I can use a shared_lock
    float const* const maybe_value = registry.get_ref(id);
    if (maybe_value) // Check that `maybe_value` contains a value
    {
        std::cout << "My value is "
                  << std::to_string(*maybe_value) // Use the value `*maybe_value`
                  << '\n';
    }
}
```

**NB:** you should never store a reference returned by `get_ref()`: this would defeat the whole point of this library! Always query for the object by using `get_ref()` when you need it.

### Modifying an object

The preferred way is to use `set()`:

```cpp
registry.set(id, 22.f);
```

If the object was not present in the registry `set()` will do nothing and return `false`; otherwise it will set the value and return `true`.

If your objects are big and you don't want to perform a full assignment (for example if you are storing `std::vector`s and only want to modify one element in one vector) you can use `with_mutable_ref()` instead:

```cpp
bool const success = registry.with_mutable_ref(id, [](float& value) { // The callback will only be called if the id was found in the registry
    value = 22.f;
});

if (!success) // You can check if with_mutable_ref() successfully found the id
{
    std::cout << "Object not found!\n";
}
```

A third-alternative is to use `get_mutable_ref()`, but it is not recommended because you have to handle the thread-safety manually:

```cpp
{
    std::unique_lock lock{registry.mutex()}; // I want to modify so I need a unique_lock
    float* const maybe_value = registry.get_mutable_ref(id);
    if (maybe_value) // Check that `maybe_value` contains a value
    {
        *maybe_value = 22.f;
    }
}
```

**NB:** you should never store a reference returned by `get_mutable_ref()`: this would defeat the whole point of this library! Always query for the object by using `get_mutable_ref()` when you need it.

### Owning IDs

An object gets destroyed when the id(s) that own(s) it is (are) destroyed. You can either use `reg::UniqueId` or `reg::SharedId`. These types behave just like `std::unique_ptr` and `std::shared_ptr`.

```cpp
reg::UniqueId<float> owning_id1 = registry.create_unique(1.f);
reg::SharedId<float> owning_id2 = registry.create_shared(1.f);
```

You can then get a non-owning version of the id with `owning_id.get()`. These non-owning versions are just as fine as the owning ones, but they might be referring to an object that has been destroyed (if the owner(s) of said object has (have) been destroyed). This is not a problem though, you can still use these ids safely as long as you check if `registry.get(id)` returns you a valid object or not.

### Checking for the existence of an object

```cpp
if (registry.contains(id))
{
    // ...
}
```

`contains` returns true if and only if the registry contains an object referenced by `id`.

### Iterating over all the objects

You can iterate over all the objects in the registry, but the order is not guaranteed. This is why you should probably maintain your own `std::vector<reg::Id<T>>` to have the control over the order the objects will be displayed in in your UI for example.<br/>
(**NB:** If you want the guarantee that the objects will keep the order they were created in, you can use a `reg::OrderedRegistry` instead of a `reg::Registry`. The API is the same, but it uses a `std::vector` internally instead of a `std::unordered_map`.)

```cpp
{
    std::shared_lock lock{registry.mutex()}; // I only want to read so I can use a shared_lock
    for (auto const&[id, value] : registry)
    {
        // ...
    }
}
```

```cpp
{
    std::unique_lock lock{registry.mutex()}; // I want to modify so I need a unique_lock
    for (auto&[id, value] : registry)
    {
        // ...
    }
}
```

### Manual lifetime management

You can also create a non-owned id with `create_raw()`. You will then have to destroy the object manually by calling `destroy()` whenever you want.

```cpp
reg::Id<float> id = registry.create_raw(1.f);
registry.destroy(id);
```

### Thread safety

**TLDR: Using a `reg::Registry` is thread-safe except for get_ref(), get_mutable_ref(), begin(), end(), cbegin(), cend() and using a range-based for loop.**

**Most operations on a `reg::Registry` are thread-safe.** Internally we use a `std::shared_mutex` to lock the registry while we do some operations on it. The fact that the mutex is shared means that multiple threads can read from the registry at the same time, but locking will occur when you try to modify the registry or one of the objects it stores.

We cannot do the locking automatically in the cases where we hand out references to objects in the registry, because we do not control how long those references will live. In those cases it is _your_ responsibility to care about thread-safety. You can use `registry.mutex()` to get the mutex and lock it yourself.<br/>
You should only use the references while your are locking the mutex: once the lock is gone any thread could invalidate your reference at any time. (Or alternatively you can make sure that only one thread ever accesses your registry, which is another way of solving the thread-safety problem.)

### `AnyId`

`reg::AnyId` is a type that can store any `reg::Id<T>`. It has the exact same memory footprint as a `reg::Id<T>` and doesn't do any dynamic allocation either. (It basically stores the same uint128 as a `reg::Id<T>` does).<br/>
Note that this removes the type-safety of a `reg::Id` and is intended to only be used in cases where you want to store ids from different registries and don't care about their types. (For example when listing all the objects that some piece of code uses).

```cpp
auto       registry = reg::Registry<float>{};
auto const id       = registry.create(1.f);
auto const any_id   = reg::AnyId{id};
assert(id == any_id); // They can be compared
```

### `Registries`

`reg::Registries` is a type that holds a set of registries of different types:

```cpp
    using Registries = reg::Registries< // Type that holds 3 registries:
        reg::Registry<int>,             // 1 for int,
        reg::Registry<float>,           // 1 for float
        reg::Registry<double>           // and 1 for double
    >;
    Registries registries{};

    reg::Registry<int> const& const_registry = registries.of<int>(); // You can access each of the registries
    reg::Registry<int>&       registry       = registries.of<int>(); // with of<T>()
```

As a convenience, `reg::Registries` provides the thread-safe functions of a `reg::Registry` and will automatically call them on the right registry:

```cpp
using Registries = reg::Registries<
    reg::Registry<int>,
    reg::Registry<float>,
    reg::Registry<double>
>;
Registries registries{};

auto const id = registries.create_unique(5.f);             //
auto const id = registries.create_unique<float>(5.f);      // Those 3 lines are equivalent
auto const id = registries.of<float>().create_unique(5.f); //
```

### `to_string()`

Allows you to convert a `reg::Id<T>` or a `reg:AnyId` to their string representation:

```cpp
auto registry = reg::Registry<float>{};
auto const id = registry.create_unique(1.f);
std::cout << reg::to_string(id) << '\n'; // "00020b79-be62-4749-95f9-938b042f3b6e"
```

### `is_empty()`

```cpp
if (registry.is_empty())
{
    // ...
}
```

### `clear()`

```cpp
registry.clear(); /// Destroys all the objects in the registry.
```

### Serialization and _cereal_ support

[_cereal_ is a serialization library](https://uscilab.github.io/cereal/index.html). _reg_ provides out of the box support for it and you can use _cereal_ to save and load _reg_ types without any efforts. You simply have to `#include <reg/cereal.hpp>` to import the serialization functions.

If you have another way of serializing your objects, see the `underlying_xxx()` section below.

### `underlying_xxx()`

These functions were added to allow you to add serialization support for the `reg` types; you can use them whenever you need access to the internals of the ids and registries.<br/>
But beware that we do not offer any guarantee that the return types of these functions won't change. Code relying on them is susceptible to be broken by futur releases of the library. This sould be rare though and not hard to update.

**Only use these functions when you cannot do otherwise, but feel free to use them when such cases arise.**

### More examples

Check out [our tests](./tests/test.cpp) for more examples of how to use the library.

## Notes

### Why can't I just use native pointers (\*) or references (&)?

Pointers and references are tied to an address in memory, which is something that can change. For example if you close and restart your application your objects will likely not be created in the same location in memory as they were before, so if you saved a pointer and try to reuse it it will likely be pointing to garbage memory now.

Even during the lifetime of the application pointers can get invalidated. For example if you store some objects in a `std::vector`, and then later on add one more object to that vector it might need to resize, which will make all the objects it contains move to a new location in memory. If you had pointers pointing to the objects in the vector they will now be dangling!

And references (&) suffer from the exact same problems.

### Performance is not our main concern

Since a registry is designed to store user-visible values, there likely won't be millions of them. We can therefore afford to prioritize safety and ease of use over performance.

## Future developments

### `for_each` functions

Should we add `for_each_value()`, `for_each_id()` and `for_each_object()` (a.k.a.`for_each_id_value_pair()`)?
I can't think of a use case for these right now, so I will wait for one before implementing them. If you have a use case, please raise an issue! I will be happy to hear what you have to say about these functions.

## Running the tests

Simply use "tests/CMakeLists.txt" to generate a project, then run it.<br/>
If you are using VSCode and the CMake extension, this project already contains a *.vscode/settings.json* that will use the right CMakeLists.txt automatically.
