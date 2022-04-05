# reg

- [Brief](#brief)
- [Use case](#use-case)
- [Tutorial](#tutorial)
  * [Creating an object](#creating-an-object)
  * [Accessing an object](#accessing-an-object)
  * [Destroying an object](#destroying-an-object)
  * [Iterating over all the objects](#iterating-over-all-the-objects)
  * [More examples](#more-examples)
- [Notes](#notes)
  * [Why can't I just use native pointers (*) or references (&)?](#why-can-t-i-use-native-pointers-----or-references-----)
  * [We don't provide automatic lifetime management](#there-is-no-automatic-lifetime-management)

## Brief

This library gives an identity to your value-types, turning them into objects stored in a `reg::Registry`. You can reference and access your objects through a `reg::Id`.

You can see `reg::Id`s as references which will never get invalidated unless you explicitly ask the registry to destroy the object. And even after an object has been destroyed it is safe to query the registry for the destroyed object through its id: the registry will simply return `nullptr` and you will have to handle the fact that the object no longer exists. Basically this is like a reference which knows whether it is dangling or not and will never let you read garbage memory.

This library allows you to manually control the lifetime of objects, and to keep references to those objects that will never get invalidated, even upon restarting your application. Those references are safe to serialize.

## Use case

This library was designed for this specific use case:

**_You want to create user-facing objects that will be referenceable across the application, as in Blender's Scene Collection:_**

![](./docs/img/blender-hierarchy.png)
> The *Cube*'s constraint references the *MyPosition* object. We want this reference to live forever (or at least until the user changes it).

## Tutorial

### Creating an object

A `reg::Registry` stores values of a given type. For example you can create a
```cpp
reg::Registry<float> registry{};
```

(NB: if you want to store multiple types in the same registry you can always use a `std::variant` or `std::any`).

You can then create objects in the registry:
```cpp
reg::Id<float> id{ registry.create(5.f) };
```

this will return you an id that you can later use to get your object back from the registry.

### Accessing an object

Since the object could have been destroyed at any time by another part of the application, you always have to check that `get()` actually returned you a value:

```cpp
const float* const maybe_value{ registry.get(id) };
if (maybe_value) // Check that `maybe_value` contains a value
{
    std::cout << "My float is "
              << std::to_string(*maybe_value) // use the value `*maybe_value`
              << '\n';
}
```

**NB:** you should never store a reference to the object returned by `get()`: this would defeat the whole point of this library! Always query the current value by using `get()` when you need it.

### Destroying an object

You can always call `destroy()`, even if the id isn't valid. This will remove the object from the registry (if it was there in the first place).

```cpp
registry.destroy(id);
```

### Iterating over all the objects

You can iterate over all the objects in the registry, but the order is not guaranteed. This is why you should probably maintain your own `std::vector<reg::Id<T>>` to have the control over the order the objects will be displayed in in your UI for example.

```cpp

for (const auto&[id, value] : registry) 
{
    // ...
}

```

### Thread safety

:warning: **Using a `reg::Registry` is not thread-safe!**

Trying to create or destroy objects concurrently is undefined behaviour ([as for all the `std` containers](https://stackoberflow.com/a/9685609)), and the pointer returned by `get()` could be invalidated by another thread at any time.

If you are only reading on all the threads (using the const version of `get()`) then this is fine. But if you want to mutate the registry by using `create()`, `destroy()` or the non-const version of `get()` then you need to be very careful!
Use a mutex or another synchronization mechanism when you want to call `create()` or `destroy()` or modify an object through the non-const `get()`, and take copies of the values returned by `get()` if another thread might destroy the pointed object.

We made that choice because we wanted `get()` to return by reference instead of value. This is to allow the use of registries to store big objects like meshes that we can't afford to copy all the time.

### More examples

Check out [our tests](./tests/test.cpp) for more examples of how to use the library.

## Notes

### Why can't I just use native pointers (*) or references (&)?

Pointers and references are tied to an address in memory, which is something that can change. For example if you close and restart your application your objects will likely not be created in the same location in memory as they were before, so if you saved a pointer and try to reuse it it will likely be pointing to garbage memory now.

Even during the lifetime of the application pointers can get invalidated. For example if you store some objects in a `std::vector`, and then later add one more object to that vector it might need to resize, which will make all the objects it contains move to a new location in memory. If you had pointers pointing to the objects in the vector they will now be dangling!

And references (&) suffer from the exact same problems.

### We don't provide automatic lifetime management

We don't provide `unique_ptr` and `shared_ptr`-like functionnalities because you don't want all your objects to be removed from the registry when the application shuts down and everything gets destroyed.

## TODO

- add inline documentation