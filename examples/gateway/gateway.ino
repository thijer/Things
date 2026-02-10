#include "Arduino.h"
#include "WiFi.h"
#include "WiFiClient.h"

/* This example demonstrates the use of multiple Thingsboard devices in combination with a gateway.
Two equivalent ThingDevices are set up that both have two client attributes (prop0 and prop01) that are
uploaded to Thingsboard when they are updated, and one (prop2) shared attribute that gets downloaded to
the correct device when it is modified in Thingsboard.

This example is for use on the ESP32.
*/

// Set THING_DEBUG to a `Print` interface of your choice before 
// including Thing headers to receive debug output from the library.
#define THING_DEBUG Serial
#include "ThingGateway.hpp"
#include "property.hpp"         // Needs to be included after ThingGateway.hpp to ensure arduinojson support compiles.
#include "propertystore.hpp"
#include "PropertyTextInterface.hpp"

// Contains Wifi and thingsboard parameters.
#include "secrets.h"

WiFiClient cl;

// Set up a gateway with a Client interface, the address of the Thingsboard server, and the accesstoken.
ThingGateway<2> gateway(cl, tb_address, tb_accesstoken);

// Set up client attribute properties.
IntegerProperty prop00("prop0");
RealProperty prop01("prop1");
// Set up a propertystore that manages the properties belonging to device0.
PropertyStore<2> dev0_cl_attributes({&prop00, &prop01});

// Set up shared attribute properties.
IntegerProperty prop02("prop2");
PropertyStore<1> dev0_sh_attributes({&prop02});
PropertyTextInterface dev0_interface(dev0_sh_attributes);
// Create a Thingsboard device with a name and optionally a type identifier.
ThingDevice device0("device0", "test-device");

// Repeat for device1
IntegerProperty prop10("prop0");
RealProperty prop11("prop1");
PropertyStore<2> dev1_cl_attributes({&prop10, &prop11});

IntegerProperty prop12("prop2");
PropertyStore<1> dev1_sh_attributes({&prop12});
PropertyTextInterface dev1_interface(dev1_sh_attributes);

ThingDevice device1("device1", "test-device");

uint32_t last_update = 0;
String serial_buffer;

// Provides an Unix timestamp in ms.
time_t timesource()
{
    time_t time_now;
    time(&time_now);
    return time_now * 1000; // Convert seconds to milliseconds with this sophisticated conversion;
}

void parse_command(String& message)
{
    if(message == "print")
    {
        Serial.println("device0 properties");
        dev0_interface.print_to(Serial);

        Serial.println("device1 properties");
        dev1_interface.print_to(Serial);   
    }    
}

void serial_input()
{
    while(Serial.available() > 0)
    {
        char c = Serial.read();
        if(c == '\r' || c == '\n') // carriage return
        {
            Serial.print("[Serial] processing line.");
            parse_command(serial_buffer);
            serial_buffer.clear();
        }
        else
        {
            serial_buffer += c;
        }
    }
}

void setup()
{
    Serial.begin(115200);
    delay(5000);

    // Connect to Wifi.
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, passphrase);
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print('.');
        delay(1000);
    }
    Serial.println(WiFi.localIP());

    // Configure time using NTP.
    configTime(3600, 0, "pool.ntp.org");

    Serial.println("[Main] Adding properties");
    device0.add_client_attributes(dev0_cl_attributes);
    device0.add_shared_attributes(dev0_sh_attributes);
    
    device1.add_client_attributes(dev1_cl_attributes);
    device1.add_shared_attributes(dev1_sh_attributes);

    Serial.println("[Main] Adding ThingDevices");
    gateway.add_devices({&device0, &device1});
    gateway.add_timesource(timesource);

    Serial.print("[Main] Connecting gateway: ");
    gateway.begin();
    Serial.println(gateway.connected() ? "Success" : "Failure");
}

void loop()
{
    serial_input();
    if(millis() - last_update >= 10000ul)
    {
        last_update = millis();
        Serial.println("\n[Main] Updating attributes.");
        prop00.set(prop00.get() + 1);
        prop01.set(prop01.get() + 0.1);
        prop10.set(prop10.get() + 2);
        prop11.set(prop11.get() + 0.2);
    }
    
    // The main loop should not contain code that block or pause execution.
    device0.loop();
    device1.loop();
    gateway.loop();
}