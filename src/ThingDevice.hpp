#ifndef THINGDEVICE_HPP
#define THINGDEVICE_HPP
#include "ArduinoJson.h" // ArduinoJson needs to be included before property.hpp to ensure property.hpp compiles ArduinoJson supporting functions.
#include "property.hpp"
#include "propertystore.hpp"
#include "utility.h"


/// @brief A device that can interact with Thingsboard. This device can be added to a 
/// `ThingGateway` that facilitates the communication with Thingsboard.
class ThingDevice
{
    public:
        /// @brief Setup a device.
        /// @param name The name of this device. This device will appear in Thingsboard under this name. 
        /// @param type The device type. This can correspond to a device profile in Thingsboard.
        /// @param enabled Initial state of the device, false means deactivated.
        ThingDevice(const char* name, const char* type = nullptr, bool enabled = true);

        ~ThingDevice(){}

        /// @brief Initializes the ThingDevice
        void begin() {};

        /// @brief Runs the main functionality of the device. This function should run in the main loop with 
        /// little or no blocking code in between. 
        void loop();

        /// @brief Add the set of `Property`s that should be uploaded to Thingsboard as telemetry.
        /// @param store Either a `PropertyStore` or a `TelemetryStore` object which contains the relevant `Property`s.
        void add_telemetry(BaseStore& store) { telemetrystore = &store; };
        
        /// @brief Add the set of `Property`s that should be uploaded to Thingsboard as attributes.
        /// @param store Either a `PropertyStore` or a `TelemetryStore` object which contains the relevant `Property`s.
        void add_client_attributes(BaseStore& store) { client_attributes = &store; };

        /// @brief Add the set of `Property`s of which the values should be downloaded from Thingsboard.
        /// @param store Either a `PropertyStore` or a `TelemetryStore` object which contains the relevant `Property`s.
        void add_shared_attributes(BaseStore& store) { shared_attributes = &store; };
        
    protected:
        // Desired connection status. if `true`, it will be connected to Thingsboard.
        BooleanProperty enabled;

        // Add the given `Property` to the given JSON document.
        void add_to_document(JsonDocument& doc, BaseProperty* p);
        
        // Pass the received shared attributes to the correct Property.
        void process_attributes(JsonObject obj);

        // Make the `ThingGateway` a friend so it has access to the privates.
        template<size_t SIZE>
        friend class ThingGateway;

        // Actual connection status. 
        bool connected = false;
        
        // Device name.
        const char* name;

        // Device type and/or desired device profile.
        const char* type;

        // JSON structures that hold the data before transmission.
        JsonDocument telemetry_doc, attribute_doc;

        // The set of `Property`s that constitute this device's telemetry.
        BaseStore* telemetrystore;
        
        // The set of `Property`s that are uploaded to Thingsboard as attributes.
        BaseStore* client_attributes;

        // The set of `Property`s whose values are downloaded from thingsboard.
        BaseStore* shared_attributes;

};

ThingDevice::ThingDevice(const char* name, const char* type, bool enabled):
    name(name), 
    type(type),
    enabled(name, enabled)
{}

void ThingDevice::add_to_document(JsonDocument& doc, BaseProperty* p)
{
    JsonObject obj;
    if(doc.isNull())    obj = doc.to<JsonObject>();
    else                obj = doc.as<JsonObject>();

    p->save_to_json(obj);
    p->saved();
}

void ThingDevice::loop()
{
    if(enabled.get())
    {
        // Telemetry updates
        if(telemetrystore != nullptr)
        {
            for(uint32_t i = 0; i < telemetrystore->size; i++)
            {
                BaseProperty* p = telemetrystore->get_property(i);
                if(p == nullptr) { PRINT("[ThingDevice] ERROR: nullptr in telemetrystore"); }
                else if(p->is_updated())
                {
                    add_to_document(telemetry_doc, p);
                }
            }
        }
        // Attribute updates
        if(client_attributes != nullptr)
        {
            for(uint32_t i = 0; i < client_attributes->size; i++)
            {
                BaseProperty* p = client_attributes->get_property(i);
                if(p == nullptr) { PRINT("[ThingDevice] ERROR: nullptr in client_attributes"); }
                else if(p->is_updated())
                {
                    add_to_document(attribute_doc, p);
                }
            }
        }
        // ThingGateway will detect that the attribute and/or telemetry JSON documents 
        // contain data and send them to thingsboard.
    }
}

void ThingDevice::process_attributes(JsonObject obj)
{
    // Do nothing if no shared attributes are attached.
    if(shared_attributes == nullptr) return;
    
    // Loop through attributes in the JSON object.
    for(JsonPair att : obj)
    {
        // Loop through attributes
        for(uint32_t i = 0; i < shared_attributes->size; i++)
        {
            BaseProperty* p = shared_attributes->get_property(i);
            // Compare names.
            if(!strcmp(p->get_name(), att.key().c_str()))
            {
                p->set_from_json(att.value());
            }
        }
    }
}

#endif