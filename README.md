# reg

## Brief

Gives an identity to your value-types, turning them into objects stored in a `reg::Registry`. You can reference and access your objects through a `reg::Id`.

You can see `reg::Id`s as references which will never get invalidated unless you explicitly ask the registry to destroy the object. And even after an object has been destroyed it is safe to query the registry for the destroyed object through its id: the registry will simply return `nullptr` and you will have to handle the fact that the object no longer exists. Basically this is like a reference which knows whether it is dangling or not and will never let you read garbage memory.

This library allows you to manually control the lifetime of objects, and to keep references to those objects that will never get invalidated, even upon restarting your application. Those references are safe to serialize.

## Use case

This library was designed for this specific use case:

**_You want to create user-facing objects that will be referenceable across the application, as in Blender's Scene Collection:_**

![](./docs/img/blender-hierarchy.png)
> The *Cube*'s constraint references the *MyPosition* object. We want this reference to live forever (or at least until the user changes it).

## Why can't I use native pointers (*) or references (&)?

Pointers and references are tied to an address in memory, which is something that can change. For example if you close and restart your application your objects will likely not be created in the same location in memory as they were before, so if you saved a pointer and try to reuse it, it will likely be pointing to garbage memory now.

Even during the lifetime of the application pointers can get invalidated. For example if you store some objects in a `std::vector`, and then later add one more object to that vector, it might need to resize, which will make all the objects it contains move to a new location in memory. If you had pointers pointing to the objects in the vector they will now be dangling!

And references (&) suffer from the exact same problems.

## Tutorial

### Creating an object

### Accessing an object

### Destroying an object

### Iterating over all the objects

You can iterate over all the objects in the registry, but the order is not guaranteed. This is why you should probably maintain your own `std::vector<reg::Id<T>>` to have the control over the order the objects will be displayed in in your UI for example.

```cpp We want to have both ways to iterate: only the objects, or objects and ids

for (const T& object : registry) {}

for (const auto&[id, object] : registry.key_value_pairs()) {} // Or should it be named `id_object_pairs()`? And have also `.ids()` and `.objects()`? Do these through a proxy

```

## Notes

### There is no automatic lifetime management

We don't provide `unique_ptr` and `shared_ptr`-like functionnalities because you don't want all your objects to be removed from the registry when the application shuts down and everything gets destroyed.

## TODO

- Use a real uuid library
- Add the destroy function (and rename insert as create?)
-  test the objects_count() of the registry thanks to the begin() and end() iterators