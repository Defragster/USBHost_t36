// Simple test of USB Host Mouse/Keyboard
//
// This example is in the public domain

#include "USBHost_t36.h"

USBHost myusb;
USBHub hub1(myusb);
USBHub hub2(myusb);
USBHIDParser hid1(myusb);
USBHIDParser hid2(myusb);
USBHIDParser hid3(myusb);
USBSerial userial(myusb);

USBDriver *drivers[] = {&hub1, &hub2, &hid1, &hid2, &hid3, &userial};
#define CNT_DEVICES (sizeof(drivers)/sizeof(drivers[0]))
const char * driver_names[CNT_DEVICES] = {"Hub1","Hub2",  "HID1", "HID2", "HID3","USERIAL1" };
bool driver_active[CNT_DEVICES] = {false, false, false, false};

void setup()
{
  while (!Serial) ; // wait for Arduino Serial Monitor
  Serial.println("\n\nUSB Host Testing - Serial");
  myusb.begin();
  Serial1.begin(115200);  // We will echo stuff Through Serial1... 

}


void loop()
{
  myusb.Task();
    // Print out information about different devices.
    for (uint8_t i = 0; i < CNT_DEVICES; i++) {
    if (*drivers[i] != driver_active[i]) {
      if (driver_active[i]) {
        Serial.printf("*** Device %s - disconnected ***\n", driver_names[i]);
        driver_active[i] = false;
      } else {
        Serial.printf("*** Device %s %x:%x - connected ***\n", driver_names[i], drivers[i]->idVendor(), drivers[i]->idProduct());
        driver_active[i] = true;

        const uint8_t *psz = drivers[i]->manufacturer();
        if (psz && *psz) Serial.printf("  manufacturer: %s\n", psz);
        psz = drivers[i]->product();
        if (psz && *psz) Serial.printf("  product: %s\n", psz);
        psz = drivers[i]->serialNumber();
        if (psz && *psz) Serial.printf("  Serial: %s\n", psz);

        // If this is a new Serial device. 
        if (drivers[i] == &userial) {
          // Lets try first outputting something to our USerial to see if it will go out...
          userial.begin(115200);
          userial.println("abcdefghijklmnopqrstuvwxyz");
          userial.println("ABCDEFGHIJKLMNOPQURSTUVWYZ");
          userial.flush();  // force it out now. 
          userial.println("0123456789");
          userial.flush();
          delay(2);
          userial.println("abcdefghijklmnopqrstuvwxyz");
          userial.println("ABCDEFGHIJKLMNOPQURSTUVWYZ");
          delay(2);
          userial.println("!@#$%^&*()");
          userial.flush();
        }
      }
    }
  }

  while (Serial.available()) {
    userial.write(Serial.read());
  }

  while (Serial1.available()) {
    Serial1.write(Serial1.read());
  }

  while (userial.available()) {
    Serial.write(userial.read());
  }


}


