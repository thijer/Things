#ifndef THINGMQTT_HPP
#define THINGMQTT_HPP
#include <functional>
#include "config.h"
#include "Client.h"
#include "ArduinoJson.h"
#include "PubSubClient.h"
#include "ThingDevice.hpp"

class ThingMQTT: public ThingDevice
{
    public:
        ThingMQTT(
            Client& client, 
            const char* server, 
            const char* accesstoken, 
            const char* devicename,
            const char* devicetype = nullptr
        );
        ~ThingMQTT(){}
        
        /// @brief Start the device and connect to Thingsboard
        void begin();
        
        /// @brief Runs the main functionality of the device. This function should run in the main loop with 
        /// little or no blocking code in between. 
        void loop();

        /// @brief Indicates if the gateway is currently connected to thingsboard.
        /// @return `true` if connected, `false` otherwise.
        bool connected() { return mqtt.connected(); }

        /// @brief Add a function that generates the time as a timestamp when called.
        /// @param timefunction The function that generates the timestamp. Should take no input 
        /// arguments and return a single `time_t` object.
        void add_timesource(std::function<time_t()> timefunction) { timesource = timefunction; }
    protected:
    
    private:
        // Callback function used by the PubSubClient to process messages from thingsboard.
        void callback(char* topic, uint8_t* payload, unsigned int length);

        // Publish a finished JSON structure to the given MQTT topic.
        bool publish(const char* topic, JsonDocument& doc);
    
        // SUB: download attributes related to the gateway device itself from Thingsboard.
        const char* topic_device_attributes = "v1/devices/me/attributes";

        // PUB: push telemetry to Thingsboard
        const char* topic_device_telemetry = "v1/devices/me/telemetry";
        
        // The MQTT client.
        PubSubClient mqtt;

        // Indicates if the gateway has subscribed to the relevant topics.
        bool subscribed = false;

        // Accesstoken to authenticate this gateway to Thingsboard.
        const char* accesstoken;

        // Pointer to a function that provides a timestamp.
        std::function<time_t()> timesource;
};

ThingMQTT::ThingMQTT(
    Client& client, 
    const char* server, 
    const char* accesstoken, 
    const char* devicename, 
    const char* devicetype
):
    ThingDevice(devicename, devicetype),
    mqtt(server, 1883, client),
    accesstoken(accesstoken)
{
    using namespace std::placeholders;
    mqtt.setCallback(std::bind(&ThingMQTT::callback, this, _1, _2, _3));
}

void ThingMQTT::begin()
{
    ThingDevice::begin();
    mqtt.connect(name, accesstoken, "");
}

void ThingMQTT::loop()
{
    ThingDevice::loop();

    mqtt.loop();
    // reconnect if not connected.
    if(!mqtt.connected())
    {
        mqtt.connect(name, accesstoken, "");
    }
    else
    {
        // Subscribe to topics.
        if(!subscribed)
        {
            subscribed = mqtt.subscribe(topic_device_attributes);
            PRINT("[ThingGateway]: subscribing ", subscribed ? "success." : "failed.");
        }

        // Check for attribute updates.
        if(!attribute_doc.isNull())
        {
            PRINT("[ThingGateway] publishing attributes");
            publish(topic_device_attributes, attribute_doc);
            attribute_doc.clear();
            
        }
        // Check for telemetry updates.
        if(!telemetry_doc.isNull())
        {
            PRINT("[ThingGateway] publishing telemetry");
            if(timesource)
            {
                JsonDocument doc;
                doc["ts"] = timesource();
                doc["values"] = telemetry_doc;
                publish(topic_device_telemetry, doc);
                telemetry_doc.clear();
            }
            else
            {
                publish(topic_device_telemetry, telemetry_doc);
                telemetry_doc.clear();
            }
        }
    }
}

void ThingMQTT::callback(char* topic, uint8_t* payload, unsigned int length)
{
    PRINT("[ThingMQTT] received message on ", topic, ":");
    PRINT(payload, length);

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, payload, length);
    if(err) { PRINT("[ThingMQTT] ERROR: Deserialization ", err.c_str()); }
    else
    {
        // Attributes belonging to a connected device.
        if(strncmp(topic, topic_device_attributes, strlen(topic_device_attributes)) == 0)
        {
            process_attributes(doc.as<JsonObject>());
        }
    }

}

bool ThingMQTT::publish(const char* topic, JsonDocument& doc)
{
    PRINT("[ThingMQTT]: publishing to ", topic);
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