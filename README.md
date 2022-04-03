# reg

Allows you to manually control the lifetime of objects that you want to share accross your application. Allows you to store references to those objects that will never get invalidated, even upon restarting your application: those references are safe to serialize.