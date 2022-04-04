# reg

## Brief

Gives an identity to your value-types, turning them into objects stored in a `reg::Registry` that you can safely reference through a `reg::Id`.

You can see `reg::Id`s as references which will never get invalidated unless you explicitly ask to destroy the object. And even after an object has been destroyed, it is safe to query the registry for the destroyed object's id: the registry will simply return `std::nullopt` and you will have to handle the fact that the object no longer exists. Basically this is like a reference which knows whether it is dangling or not and will never let you read garbage memory.

This library allows you to manually control the lifetime of objects, and to keep references to those objects that will never get invalidated, even upon restarting your application. Those references are safe to serialize.

You should use a `reg::registry` when you want to have long-lasting objects that you will reference all over your application and share across different systems.

You should use a `reg::registry` when you want to have the control over the lifetime of your objects, instead of using scope-based automatic lifetime management (a.k.a. RAII).

## Use case

This library was designed for this specific use case:

You want to create user-facing objects that will be referenceable across the application, like in Blender's scene hierarchy:
![](./docs/img/blender-hierarchy.png)
> The *Cube*'s constraint references the *MyPosition* object. We want this reference to live forever (or at least until the user changes it).

## Why can't I use native pointers (*) or references (&)?

<!-- ## Tutorial

You can iterate over all the objects in the registry, but the order is not guaranteed. This is why you should probably maintain your own `std::vector<reg::Id<T>>` to have the control over the order the objects will be displayed in in your UI for example.

 

(TODO: test the objects_count() of the registry thanks to the begin() and end() iterators)

 

```cpp We want to have both ways to iterate: only the objects, or objects and ids

for (const T& object : registry) {}

for (const auto&[id, object] : registry.key_value_pairs()) {} // Or should it be named `id_object_pairs()`? And have also `.ids()` and `.objects()`? Do these through a proxy

```

## No automatic lifetime management

We don't provided `unique_ptr` and `shared_ptr`-like functionnality because you don't want all your objects to be remove from the database when the application shuts down and everything gets destroyed.

## TODO

- Use a real uuid library
- Decide between Database and Registry for the name
- Add the destroy function (and rename insert as create?) -->