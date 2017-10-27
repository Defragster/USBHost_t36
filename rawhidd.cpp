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



bool RawHIDController::claim_collection(Device_t *dev, uint32_t topusage)
{
	// only claim top report given to use in constructor
	if (topusage != topusage_) return false;
	// only claim from one physical device
	if (mydevice != NULL && dev != mydevice) return false;
	mydevice = dev;
	collections_claimed++;
	return true;
}

void RawHIDController::disconnect_collection(Device_t *dev)
{
	if (--collections_claimed == 0) {
		mydevice = NULL;
	}
}

void RawHIDController::hid_input_begin(uint32_t topusage, uint32_t type, int lgmin, int lgmax)
{
	// TODO: check if absolute coordinates
	Serial.printf("Raw Begin: usage=%X, type=%d, min=%d Max=%d\n", topusage, type, lgmin, lgmax);
	hid_input_begin_ = true;
}

void RawHIDController::hid_input_data(uint32_t usage, int32_t value)
{
	Serial.printf("Raw: usage=%X, value=%d\n", usage, value);
	if (rx_buffer_index_ < RAWHID_MAX_LEN) {
		rxBuffer_[rx_buffer_index_++] = value;
	}
}

void RawHIDController::hid_input_end()
{
	if (hid_input_begin_) {
		rawEvent = true;
		hid_input_begin_ = false;
	}
}

void RawHIDController::clear() {
	rawEvent = false;
}

uint8_t RawHIDController::receiveData(uint8_t *rx_buffer, uint8_t size) {
	if (size > rx_buffer_index_) size = rx_buffer_index_;
	memcpy(rx_buffer, rxBuffer_, size);
	return size;
}

void 	RawHIDController::sendData(uint8_t *tx_buffer, uint8_t size) {

}

