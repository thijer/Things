#include "Arduino.h"
#include "WiFi.h"
#include "WiFiClient.h"
#include "ThingMQTT.hpp"
#include "PropertyTextInterface.hpp"
#include "secrets.h"
/* 
This example shows a simple Thingsboard device that uses an `ThingMQTT` instance to 
connect to Thingsboard.
*/

WiFiClient cl;
ThingMQTT device(cl, tb_address, tb_accesstoken, "Devicename", "default-type");

IntegerProperty prop0("prop0");
RealProperty prop1("prop1");
// Set up a propertystore that manages the properties belonging to device0.
PropertyStore<2> cl_attributes({&prop0, &prop1});

// Set up shared attribute properties.
IntegerProperty prop2("prop2");
PropertyStore<1> sh_attributes({&prop2});

uint32_t last_update = 0;
PropertyTextInterface prop_interface(sh_attributes);
String serial_buffer;

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
        Serial.println("device properties");
        prop_interface.print_to(Serial);
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
    device.add_client_attributes(cl_attributes);
    device.add_shared_attributes(sh_attributes);

    device.add_timesource(timesource);

    device.begin();
    Serial.print("[Main] Connecting Thing: ");
    Serial.println(device.connected() ? "Success" : "Failure");
}

void loop()
{
    serial_input();
    if(millis() - last_update >= 10000ul)
    {
        last_update = millis();
        Serial.println("\n[Main] Updating attributes.");
        prop0.set(prop0.get() + 1);
        prop1.set(prop1.get() + 0.1);
    }
    
    // The main loop should not contain code that block or pause execution.
    device.loop();
}

