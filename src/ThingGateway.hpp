#ifndef THINGGATEWAY_HPP
#define THINGGATEWAY_HPP
#include "config.h"

#include <functional>
#include "Client.h"
#include "ArduinoJson.h"
#include "PubSubClient.h"
#include "ThingDevice.hpp"
#include "utility.h"


/// @brief 
/// @tparam SIZE The number of separate Thingsboard devices that connect to THingsboard through this gateway.
template<size_t SIZE>
class ThingGateway
{
    public:
        /// @brief Setup a Thingsboard Gateway that manages a set of devices.
        /// @param client A Client interface that can be used to connect to Thingsboard.
        /// @param server The URL/IP address of the server.
        /// @param accesstoken The accesstoken to authenticate the gateway.
        /// @param devicename The name of the gateway. Defaults to "gateway".
        ThingGateway(Client& client, const char* server, const char* accesstoken, const char* devicename = "Gateway");
        
        ~ThingGateway();

        /// @brief Initializes the gateway
        void begin();
        
        /// @brief Runs the main functionality of the gateway. This function should run in the main loop with 
        /// little or no blocking code in between. 
        void loop();
        
        /// @brief Add a set of `ThingDevice`s to the gateway. 
        /// @param device An array with pointers to the thingsboard device instances.
        /// @details The `ThingDevice`s appear as normal devices in thingsboard, even though they connect to THingsboard through this gateway.
        void add_devices(ThingDevice* const (&device)[SIZE]);

        /// @brief Indicates if the gateway is currently connected to thingsboard.
        /// @return `true` if connected, `false` otherwise.
        bool is_connected() { return connected; }

        /// @brief Add a function that generates the time as a timestamp when called.
        /// @param timefunction The function that generates the timestamp. Should take no input 
        /// arguments and return a single `time_t` object.
        void add_timesource(std::function<time_t()> timefunction); 
    
    protected:
    
    private:
        // Callback function used by the PubSubClient to process messages from thingsboard.
        void callback(char* topic, uint8_t* payload, unsigned int length);

        // Connect a device to thingsboard.
        void connect_device(ThingDevice* device);

        // Disconnect a device from thingsboard.
        void disconnect_device(ThingDevice* device);

        // Publish a finished JSON structure to the given MQTT topic.
        bool publish(const char* topic, JsonDocument& doc);

        // MQTT Topics
        // PUB: Connect a device to thingsboard.
        const char* topic_connect = "v1/gateway/connect";

        // PUB: Disconnect
        const char* topic_disconnect = "v1/gateway/disconnect";

        // PUB: Upload attributes to thingsboard. SUB: download attribute updates from thingsboard
        const char* topic_attributes = "v1/gateway/attributes";

        // PUB: Upload telemetry to thingsboard.
        const char* topic_telemetry = "v1/gateway/telemetry";
        
        // THe set of Thingsboard devices belonging to this gateway.
        ThingDevice* devices[SIZE];

        // The JSON structures that hold the data before sending it to thingsboard. 
        JsonDocument attribute_doc;
        JsonDocument telemetry_doc;
        
        // The MQTT client.
        PubSubClient mqtt;

        // Connection status
        bool connected = false;

        // Accesstoken to authenticate this gateway to Thingsboard.
        const char* accesstoken;
        
        // The name of this device.
        const char* devicename;

        // Pointer to a function that provides a timestamp.
        std::function<time_t()> timesource;
};

template<size_t SIZE>
ThingGateway<SIZE>::ThingGateway(Client& client, const char* server, const char* accesstoken, const char* devicename):
    mqtt(server, 1883, client),
    accesstoken(accesstoken),
    devicename(devicename)
{
    using namespace std::placeholders;
    mqtt.setCallback(std::bind(&ThingGateway<SIZE>::callback, this, _1, _2, _3));
}

template<size_t SIZE>
ThingGateway<SIZE>::~ThingGateway()
{

}

template<size_t SIZE>
void ThingGateway<SIZE>::begin()
{
    connected = mqtt.connect(devicename, accesstoken, "");
}

template<size_t SIZE>
void ThingGateway<SIZE>::loop()
{
    // reconnect if not connected.
    if(!connected)
    {
        connected = mqtt.connect(devicename, accesstoken, "");
    }
    time_t currenttime = 0;
    if(timesource) currenttime = timesource();
    
    for(ThingDevice* device : devices)
    {
        // Check if device should be connected to Thingsboard.
        if(device->enabled && !device->connected)
        {
            PRINT("[ThingGateway]", device->name, ": connecting.");
            connect_device(device);
        }
        // Check if device should be disconnected from Thingsboard.
        else if(!device->enabled && device->connected)
        {
            PRINT("[ThingGateway]", device->name, ": disconnecting.");
            disconnect_device(device);
        }
        
        else
        {
            // Check for attribute updates.
            if(!device->attribute_doc.isNull())
            {
                PRINT("[ThingGateway] ", device->name, ": attribute updates");
                // [TODO] Prepare doc if necessary.
                // Create JSON structure as defined in https://thingsboard.io/docs/reference/gateway-mqtt-api/#publish-attribute-to-the-thingsboard-platform
                attribute_doc[device->name] = device->attribute_doc;
                device->attribute_doc.clear();
            }
            // Check for telemetry updates.
            // The gateway can only send telemetry if a timestamp can be assigned.
            if(!device->telemetry_doc.isNull() && timesource)
            {
                PRINT("[ThingGateway] ", device->name, ": telemetry updates");
                // [TODO] Prepare doc if necessary.
                // Create JSON structure as defined in https://thingsboard.io/docs/reference/gateway-mqtt-api/#telemetry-upload-api
                JsonObject obj = telemetry_doc[device->name].add<JsonObject>();
                obj["ts"] = currenttime;
                obj["values"] = device->telemetry_doc;
                device->telemetry_doc.clear();
            }
        }
    }
    // Send attributes and/or telemetry to Thingsboard.
    if(!attribute_doc.isNull())
    {
        PRINT("[ThingGateway]: publishing attributes");
        if(publish(topic_attributes, attribute_doc))
        {
            // Publishing success
            attribute_doc.clear();
        }
        else
        {
            PRINT("[ThingGateway] ERROR: publishing failed");
        }
    }
    if(!telemetry_doc.isNull())
    {
        PRINT("[ThingGateway]: publishing telemetry");
        if(publish(topic_telemetry, telemetry_doc))
        {
            // Publishing success
            telemetry_doc.clear();
        }
        else
        {
            PRINT("[ThingGateway] ERROR: publishing failed");
        }
    }
}

template<size_t SIZE>
void ThingGateway<SIZE>::add_devices(ThingDevice* const (&device)[SIZE])
{
    for(uint32_t i = 0; i < SIZE; i++)
    {
        devices[i] = device[i];
    }
}

template<size_t SIZE>
void ThingGateway<SIZE>::add_timesource(std::function<time_t()> timefunction)
{
    timesource = timefunction;
}

template<size_t SIZE>
void ThingGateway<SIZE>::callback(char* topic, uint8_t* payload, unsigned int length)
{

}

template<size_t SIZE>
void ThingGateway<SIZE>::connect_device(ThingDevice* device)
{
    JsonDocument connect_doc;
    connect_doc["device"] = device->name;
    if(device->type != nullptr) connect_doc["type"] = device->type;
    device->connected = publish(topic_connect, connect_doc);
}

template<size_t SIZE>
void ThingGateway<SIZE>::disconnect_device(ThingDevice* device)
{
    JsonDocument disconnect_doc;
    disconnect_doc["device"] = device->name;
    device->connected = !publish(topic_disconnect, disconnect_doc);
}

template<size_t SIZE>
bool ThingGateway<SIZE>::publish(const char* topic, JsonDocument& doc)
{
    PRINT("[ThingGateway]: publishing to ", topic);
    #ifdef THING_DEBUG
    serializeJson(doc, THING_DEBUG); 
    PRINT();
    #endif

    uint32_t length = measureJson(doc);
    mqtt.beginPublish(topic, length, false);
    size_t written = serializeJson(doc, mqtt);
    return mqtt.endPublish();
}


#endif