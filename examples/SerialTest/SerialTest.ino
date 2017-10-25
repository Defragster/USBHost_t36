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
const char * driver_names[CNT_DEVICES] = {"Hub1", "Hub2",  "HID1", "HID2", "HID3", "USERIAL1" };
bool driver_active[CNT_DEVICES] = {false, false, false, false};

void setup()
{
  pinMode(13, OUTPUT);
  while (!Serial && (millis() < 5000)) ; // wait for Arduino Serial Monitor
  Serial.println("\n\nUSB Host Testing - Serial");
  myusb.begin();
  Serial1.begin(115200);  // We will echo stuff Through Serial1...

}


void loop()
{
  digitalWrite(13, !digitalRead(13));
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
          userial.begin(1000000);
#if 0
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
#endif
        }
      }
    }
  }

  while (Serial.available()) {
    Serial.println("Serial Available");
    int ch = Serial.read();
    if (ch == '$') {
      BioloidTest();
      while (Serial.read() != -1);
    }
    else userial.write(ch);
  }

  while (Serial1.available()) {
//    Serial.println("Serial1 Available");
    Serial1.write(Serial1.read());
  }

  while (userial.available()) {
//    Serial.println("USerial Available");

    Serial.write(userial.read());
  }
}

#define ID_MASTER 200 
//#define ID_MASTER 0xfd

void BioloidTest() {
  Serial.println("\n*** Bioloid Test ***");
  for (uint8_t reg = 0; reg < 32; reg++) {
    Serial.print(ax12GetRegister(ID_MASTER, reg, 1), HEX);
    Serial.print(" ");
  }

}

// Extract stuff from Bioloid library..
#define AX12_BUFFER_SIZE 128

/** Instruction Set **/
#define AX_PING                     1
#define AX_READ_DATA                2
#define AX_WRITE_DATA               3
#define AX_REG_WRITE                4
#define AX_ACTION                   5
#define AX_RESET                    6
#define AX_SYNC_WRITE               131

/** Error Levels **/
#define ERR_NONE                    0
#define ERR_VOLTAGE                 1
#define ERR_ANGLE_LIMIT             2
#define ERR_OVERHEATING             4
#define ERR_RANGE                   8
#define ERR_CHECKSUM                16
#define ERR_OVERLOAD                32
#define ERR_INSTRUCTION             64

/** AX-S1 **/
#define AX_LEFT_IR_DATA             26
#define AX_CENTER_IR_DATA           27
#define AX_RIGHT_IR_DATA            28
#define AX_LEFT_LUMINOSITY          29
#define AX_CENTER_LUMINOSITY        30
#define AX_RIGHT_LUMINOSITY         31
#define AX_OBSTACLE_DETECTION       32
#define AX_BUZZER_INDEX             40

#define COUNTER_TIMEOUT 12000

unsigned char ax_rx_buffer[AX12_BUFFER_SIZE];
int ax12GetRegister(int id, int regstart, int length) {
  // 0xFF 0xFF ID LENGTH INSTRUCTION PARAM... CHECKSUM
  int checksum = ~((id + 6 + regstart + length) % 256);
  userial.write(0xFF);
  userial.write(0xFF);
  userial.write(id);
  userial.write(4);    // length
  userial.write(AX_READ_DATA);
  userial.write(regstart);
  userial.write(length);
  userial.write(checksum);
  userial.flush();  // make sure the data goes out.

  if (ax12ReadPacket(length + 6) > 0) {
    //    ax12Error = ax_rx_buffer[4];
    if (length == 1)
      return ax_rx_buffer[5];
    else
      return ax_rx_buffer[5] + (ax_rx_buffer[6] << 8);
  } else {
    return -1;
  }
}
int ax12ReadPacket(int length) {
  unsigned long ulCounter;
  unsigned char offset, checksum;
  unsigned char *psz;
  unsigned char *pszEnd;
  int ch;


  offset = 0;

  psz = ax_rx_buffer;
  pszEnd = &ax_rx_buffer[length];

  while (userial.read() != -1) ;
  uint32_t ulStart = millis();
  // Need to wait for a character or a timeout...
  do {
    ulCounter = COUNTER_TIMEOUT;
    while ((ch = userial.read()) == -1) {
      if ((millis() - ulStart) > 10) {
        //if (!--ulCounter) {
        Serial.println("Timeout");
        return 0;   // Timeout
      }
    }
  } while (ch != 0xff) ;
  *psz++ = 0xff;
  while (psz != pszEnd) {
    ulCounter = COUNTER_TIMEOUT;
    while ((ch = userial.read()) == -1) {
      Serial.printf("Read ch: %x\n", ch);
      if (!--ulCounter)  {
        return 0;   // Timeout
      }
    }
    *psz++ = (unsigned char)ch;
  }
  checksum = 0;
  for (offset = 2; offset < length; offset++)
    checksum += ax_rx_buffer[offset];
  if (checksum != 255) {
    return 0;
  } else {
    return 1;
  }
}

