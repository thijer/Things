#ifndef THINGDEVICE_H
#define THINGDEVICE_H
#include "ArduinoJson.h"
#include "property.hpp"
#include "propertystore.hpp"
#include "ThingGateway.hpp"
#include "utility.h"

template<size_t SIZE> class ThingGateway;

/// @brief A device that can interact with Thingsboard. This device can be added to a 
/// `ThingGateway` that facilitates the communication with Thingsboard.
class ThingDevice
{
    public:
        /// @brief Setup a device.
        /// @param name The name of this device. This device will appear in Thingsboard under this name. 
        /// @param type The device type. This can correspond to a device profile in Thingsboard.
        ThingDevice(const char* name, const char* type = nullptr);

        ~ThingDevice(){}

        /// @brief Initializes the ThingDevice
        void begin() {};

        /// @brief Runs the main functionality of the device. This function should run in the main loop with 
        /// little or no blocking code in between. 
        void loop();

        /// @brief Add the set of `Property`s that should be uploaded to Thingsboard as telemetry.
        /// @param store Either a `PropertyStore` or a `TelemetryStore` object which contains the relevant `Property`s.
        void add_telemetry_store(BaseStore& store) { telemetrystore = &store; };
        
        /// @brief Add the set of `Property`s that should be uploaded to Thingsboard as attributes.
        /// @param store Either a `PropertyStore` or a `TelemetryStore` object which contains the relevant `Property`s.
        void add_attribute_store(BaseStore& store) { attributestore = &store; };
        
    protected:
        
    private:
        // Add the given `Property` to the given JSON document.
        void add_to_document(JsonDocument& doc, BaseProperty* p);
        
        // Make the `ThingGateway` a friend so it has access to the privates.
        template<size_t SIZE>
        friend class ThingGateway;

        // Desired connection status. if `true`, it will be connected to Thingsboard.
        bool enabled = true;

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
        
        // The set of `Property`s that constitute this device's attributes.
        BaseStore* attributestore;

};

ThingDevice::ThingDevice(const char* name, const char* type):
    name(name), 
    type(type)
{}

void ThingDevice::add_to_document(JsonDocument& doc, BaseProperty* p)
{
    if(doc.isNull())
    {
        // Do doc initialization if necessary
    }
    p->save_to_doc(doc);
    p->saved();
}

void ThingDevice::loop()
{
    if(enabled)
    {
        // Telemetry updates
        if(telemetrystore != nullptr)
        {
            for(uint32_t i = 0; i < telemetrystore->size; i++)
            {
                BaseProperty* p = telemetrystore->get_property(i);
                if(p == nullptr) PRINT("[ThingDevice] ERROR: nullptr in telemetrystore");
                else if(p->is_updated())
                {
                    add_to_document(telemetry_doc, p);
                }
            }
        }
        // Attribute updates
        if(attributestore != nullptr)
        {
            for(uint32_t i = 0; i < attributestore->size; i++)
            {
                BaseProperty* p = attributestore->get_property(i);
                if(p == nullptr) PRINT("[ThingDevice] ERROR: nullptr in attributestore");
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

#endif