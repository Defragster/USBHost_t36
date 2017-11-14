/* USB EHCI Host for Teensy 3.6
 * Copyright 2017 Paul Stoffregen (paul@pjrc.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <Arduino.h>
#include "USBHost_t36.h"  // Read this header first for key info



bool RawHIDController::claim_collection(USBHIDParser *driver, Device_t *dev, uint32_t topusage)
{
	// only claim RAWHID devices currently: 16c0:0486
#ifdef USBHOST_PRINT_DEBUG
	Serial.printf("Rawhid Claim: %x:%x usage: %x\n", dev->idVendor, dev->idProduct, topusage);
#endif

	if ((dev->idVendor != 0x16c0 || (dev->idProduct) != 0x486)) return false;
	if (mydevice != NULL && dev != mydevice) return false;
	if (usage_  && (usage_ != topusage)) return false; 	// only claim one for now... 
	mydevice = dev;
	collections_claimed++;
	usage_ = topusage;
	driver_ = driver;	// remember the driver. 
	return true;
}

void RawHIDController::disconnect_collection(Device_t *dev)
{
	if (--collections_claimed == 0) {
		mydevice = NULL;
		usage_ = 0;
	}
}

bool RawHIDController::hid_input_data_bypass(const uint8_t *data, uint32_t len) 
{
#ifdef USBHOST_PRINT_DEBUG
	Serial.printf("RawHIDController::hid_input_data_bypass: %x\n", usage_);
#endif

	if (receiveCB) {
		return (*receiveCB)(usage_, data, len);
	}
	return true;
}

void RawHIDController::hid_input_begin(uint32_t topusage, uint32_t type, int lgmin, int lgmax)
{
	Serial.printf("RawHID::hid_input_begin %x %x %x %x\n", topusage, type, lgmin, lgmax);
	hid_input_begin_ = true;
}

void RawHIDController::hid_input_data(uint32_t usage, int32_t value)
{
	Serial.printf("RawHID: usage=%X, value=%d", usage, value);
	if ((value >= ' ') && (value <='~')) Serial.printf("(%c)", value);
	Serial.println();
}

void RawHIDController::hid_input_end()
{
	Serial.println("RawHID::hid_input_end");
	if (hid_input_begin_) {
		mouseEvent = true;
		hid_input_begin_ = false;
	}
}


