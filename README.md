# Things
This library provides a set of classes that implement Thingsboard's MQTT API, such that a device can easily interact with Thingsboard. To that end, the following classes are available:
- `ThingDevice`: This acts as the representation of a Thingsboard Device on the physical device. It can process telemetry and attribute updates, and process shared attributes updates from Thingsboard. Telemetry and attributes are represented in this library as `Property`s from the Properties library (https://github.com/thijer/Properties). Communication with a Thingsboard server is manged through the `ThingGateway` or `ThingMQTT` class.
- `ThingGateway`: This class facilitates the communication to Thingsboard on behalf of a number of `ThingDevice`s. 
- `ThingMQTT`: This is a single `ThingDevice` that can connect to a Thingsboard server.

This library depends on ArduinoJson 7 (https://github.com/bblanchon/ArduinoJson), PubSubClient (https://github.com/knolleary/pubsubclient), and Properties (https://github.com/thijer/Properties).