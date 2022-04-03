# reg

Allows you to manually control the lifetime of objects that you want to share accross your application. Allows you to store references to those objects that will never get invalidated, even upon restarting your application: those references are safe to serialize.

## No automatic lifetime management

We don't provided `unique_ptr` and `shared_ptr`-like functionnality because you don't want all your objects to be remove from the database when the application shuts down and everything gets destroyed.

## TODO

- Use a real uuid library
- Decide between Database and Registry for the name
- Add the destroy function (and rename insert as create?)