RFRPI is a shield of Raspberry PI allowing RF433 communication
See : http://www.disk91.com/?p=1323 for any information

The code provided in this git is the demonstration code for this board, it is working with 
Oregon Scientific sensors.

# A couple of explanations on the library

- RCSwitch is the lower end driver listening RF and decoding signals, it is where you can active some new layer 1 decoding protocol.
- RcOok is the layer 1 decoding system for miscellaneous protocols. It is where you can add you own layer 1 decoding protocol.
 - Core433 is the associated Thread interpreting RF messages and creating the message to the eventManager once a message is received.
- EventManager is the state machine receiving the messaging and executing the expected actions like extracting and printing sensors data from a RF message. It is where you can add your custom code.
- Sensor is the layer 2 decoding protocol, at this level we are extracting the sensor measure from the received data, like temperature, humidity … This is where you can add you own sensors definition.
-  LedManager is the led driver allowing functions like on/off/blinking/fast blinking…
- Singleton is a really bad way, but efficient one to start all the threads and to allow an object sharing across the Threads.

---

If you use a RPI B+ or RPI 2

This version is basically made to support a Kernel Driver to manage interrupt more efficiently. The way to build the kernel driver is describded in the following
post : http://www.disk91.com/?p=1940 

If you use a RPI 1

Edit version.h file and change the DEFINE to match your version. You don't need the kernel driver
