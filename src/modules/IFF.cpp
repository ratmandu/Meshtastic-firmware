#include "IFF.h"
#include "MeshService.h"
#include "configuration.h"
#include "main.h"
#include "driver/ledc.h"
#include "detect/ScanI2C.h"
#include "detect/ScanI2CTwoWire.h"

#include <assert.h>

IFFModule *iffModule;

#define IFF_IR_CYCLE_TIME_MS 500

int32_t IFFModule::runOnce()
{
  if (firstTime) {
    // setup RGB LEDs
  
    // check to see which drivers are present
    if (!i2cBus) {
      i2cBus = &Wire; 
    }

    uint8_t controlreg = 0xFB;
    rgbleds[0] = {0, 0, 0};
    rgbleds[1] = {0, 0, 0};
    rgbleds[2] = {0, 0, 0};
    rgbleds[3] = {0, 0, 0};
    ktd_writeRegister(IFF_RGB_ADDR_A, IFF_RGB_CONTROL_REG, 0xFB); // Reset Defaults
    ktd_set_rgb(IFF_RGB_ADDR_A, (uint8_t *)&rgbleds[0]);
    ktd_writeRegister(IFF_RGB_ADDR_A, IFF_RGB_CONTROL_REG, 0xB1); // Enable

    ktd_writeRegister(IFF_RGB_ADDR_C, IFF_RGB_CONTROL_REG, 0xFB); // Reset Defaults
    ktd_set_rgb(IFF_RGB_ADDR_C, (uint8_t *)&rgbleds[0]);
    ktd_writeRegister(IFF_RGB_ADDR_C, IFF_RGB_CONTROL_REG, 0xB1); // Enable

    uint8_t buf = 0;
    ktd_readRegister(IFF_RGB_ADDR_A, 0x01, &buf, 1);
    LOG_INFO("KTDRGB A Monitor Reg: 0x%02x", buf);
    ktd_readRegister(IFF_RGB_ADDR_C, 0x01, &buf, 1);
    LOG_INFO("KTDRGB C Monitor Reg: 0x%02x", buf);

    // setup IR LEDs
    irleds.IR1 = 0;
    irleds.IR2 = 0;
    irleds.IR3 = 0;

    #if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
      ledcAttach(IFF_IR1, 1000, 8);
      ledcAttach(IFF_IR2, 1000, 8);
      ledcAttach(IFF_IR3, 1000, 8);
      ledcWrite(IFF_IR1, irleds.IR1);
      ledcWrite(IFF_IR2, irleds.IR2);
      ledcWrite(IFF_IR3, irleds.IR3);
    #else
      ledcSetup(4, 1000, 8);
      ledcSetup(5, 1000, 8);
      ledcSetup(6, 1000, 8);
      ledcAttachPin(IFF_IR1, 4);
      ledcAttachPin(IFF_IR2, 5);
      ledcAttachPin(IFF_IR3, 6);
      ledcWrite(4, irleds.IR1);
      ledcWrite(5, irleds.IR2);
      ledcWrite(6, irleds.IR3);
    #endif

    LOG_INFO("IFF Module Initialized");
    firstTime = false;
  }

  if (irLedEnabled) {
    if (irLedCycleEnabled) {
      // Cycle through IR LEDs
      if (irLedCycleType == IFF_CYCLE_DOWN) {
        currentIRLed++;
        if (currentIRLed == 3) {
          currentIRLed = 0;
        }
      } else if (irLedCycleType == IFF_CYCLE_UP) {
        if (currentIRLed == 0) {
          currentIRLed = 2;
        } else {
          currentIRLed--;
        }
      } else if (irLedCycleType == IFF_CYCLE_ALTERNATING) {
        if (currentIRLed == 0) {
          currentIRLed = 1;
        } else if (currentIRLed == 1) {
          currentIRLed = 0;
        } 
      }

      // Update IR LED brightness
      if (irLedCycleType == IFF_CYCLE_ALTERNATING) {
        // In alternating mode, turn off other LEDs
        if (currentIRLed == 0) {
          irleds.IR1 = 255;
          irleds.IR2 = 0;
          irleds.IR3 = 255;
        } else if (currentIRLed == 1) {
          irleds.IR1 = 0;
          irleds.IR2 = 255;
          irleds.IR3 = 0;
        }
      } else {
        irleds.IR1 = (currentIRLed == 0) ? 255 : 0;
        irleds.IR2 = (currentIRLed == 1) ? 255 : 0;
        irleds.IR3 = (currentIRLed == 2) ? 255 : 0;
      }
    } else {
      irleds.IR1 = 0;
      irleds.IR2 = 0;
      irleds.IR3 = 0;
    }
  }

  if (rgbLedEnabled)
  {
    if (rgbCycleEnabled)
    {
      // Cycle through RGB LEDs
      if (rgbLedCycleType == IFF_CYCLE_DOWN)
      {
        currentRGBLed++;
        if (currentRGBLed == 4)
        {
          currentRGBLed = 0;
        }
      }
      else if (rgbLedCycleType == IFF_CYCLE_UP)
      {
        if (currentRGBLed == 0)
        {
          currentRGBLed = 3;
        }
        else
        {
          currentRGBLed--;
        }
      }
      else if (rgbLedCycleType == IFF_CYCLE_ALTERNATING)
      {
        if (currentRGBLed == 0)
        {
          currentRGBLed = 1;
        }
        else
        {
          currentRGBLed = 0;
        }
      }
      else if (rgbLedCycleType == IFF_CYCLE_BLINK)
      {
        // In blink mode, toggle all LEDs on/off
        if (currentRGBLed == 0)
        {
          currentRGBLed = 1;
        }
        else
        {
          currentRGBLed = 0;
        }
      }
    }
  
    #if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
      ledcWrite(IFF_IR1, irleds.IR1);
      ledcWrite(IFF_IR2, irleds.IR2);
      ledcWrite(IFF_IR3, irleds.IR3);
    #else
      ledcWrite(4, irleds.IR1);
      ledcWrite(5, irleds.IR2);
      ledcWrite(6, irleds.IR3);
    #endif

    // update RGB leds
    if (rgbLedEnabled)
    {
      uint8_t values[12];

      if (rgbCycleEnabled && rgbLedCycleType == IFF_CYCLE_BLINK)
      {
        // In blink mode, toggle all LEDs on/off
        for (int i = 0; i < 4; i++)
        {
          values[i * 3 + 0] = (currentRGBLed == 1) ? rgbleds[i].r : 0; // R
          values[i * 3 + 1] = (currentRGBLed == 1) ? rgbleds[i].g : 0; // G
          values[i * 3 + 2] = (currentRGBLed == 1) ? rgbleds[i].b : 0; // B
        }
      }
      else if (rgbCycleEnabled && rgbLedCycleType == IFF_CYCLE_ALTERNATING)
      {
        for (int i = 0; i < 4; i++)
        {
          values[i * 3 + 0] = (currentRGBLed == i % 2) ? rgbleds[i].r : 0; // R
          values[i * 3 + 1] = (currentRGBLed == i % 2) ? rgbleds[i].g : 0; // G
          values[i * 3 + 2] = (currentRGBLed == i % 2) ? rgbleds[i].b : 0; // B
        }
      }
      else if (rgbCycleEnabled && (rgbLedCycleType == IFF_CYCLE_UP || rgbLedCycleType == IFF_CYCLE_DOWN))
      {
        for (int i = 0; i < 4; i++)
        {
          values[i * 3 + 0] = (i == currentRGBLed) ? rgbleds[i].r : 0; // R
          values[i * 3 + 1] = (i == currentRGBLed) ? rgbleds[i].g : 0; // G
          values[i * 3 + 2] = (i == currentRGBLed) ? rgbleds[i].b : 0; // B
        }
      }
      else
      {
        for (int i = 0; i < 4; i++)
        {
          values[i * 3 + 0] = rgbleds[i].r; // R
          values[i * 3 + 1] = rgbleds[i].g; // G
          values[i * 3 + 2] = rgbleds[i].b; // B
        }
      }

      ktd_set_rgb(IFF_RGB_ADDR_A, values);
      ktd_set_rgb(IFF_RGB_ADDR_C, values);
    }

    return IFF_IR_CYCLE_TIME_MS; 
  }

  return 1000;
}

ProcessMessage IFFModule::handleReceived(const meshtastic_MeshPacket &mp)
{
  auto &p = mp.decoded;
  LOG_INFO("IFF Received text msg from=0x%0x, id=0x%x, msg=%.*s", mp.from, mp.id, p.payload.size, p.payload.bytes);

  if (p.payload.size < 6) {
    return ProcessMessage::CONTINUE; // Not an IFF command, ignore
  }

  if (memcmp(p.payload.bytes, "IFF!", 4) != 0) {
    return ProcessMessage::CONTINUE; // Not an IFF command, ignore
  } else {
    LOG_INFO("IFF Command Received: %.*s", p.payload.size, p.payload.bytes);
    if (p.payload.bytes[4] == 'I') {
      // IR Command
      if (p.payload.bytes[5] == 'C') {
        // Toggle IR Cycle
        irLedCycleEnabled = !irLedCycleEnabled;
        LOG_INFO("IFF IR Cycle %s", irLedCycleEnabled ? "Enabled" : "Disabled");
      } else if (p.payload.bytes[5] == 'U') {
        // Set IR Cycle Direction Up
        irLedCycleType = IFF_CYCLE_UP;
        LOG_INFO("IFF IR Cycle Direction Set to Up");
      } else if (p.payload.bytes[5] == 'D') {
        // Set IR Cycle Direction Down
        irLedCycleType = IFF_CYCLE_DOWN;
        LOG_INFO("IFF IR Cycle Direction Set to Down");
      } else if (p.payload.bytes[5] == 'A') {
        // Set IR Cycle Alternating
        irLedCycleType = IFF_CYCLE_ALTERNATING;
        LOG_INFO("IFF IR Cycle Direction Set to Alternating");
      } else if (p.payload.bytes[5] == 'E') {
        // Enable IR LED
        irLedEnabled = true;
        LOG_INFO("IFF IR LED Enabled");
      } else if (p.payload.bytes[5] == 'D') {
        // Disable IR LED
        irLedEnabled = false;
        LOG_INFO("IFF IR LED Disabled");
      } else {
        LOG_ERROR("IFF Invalid IR Command: %c", p.payload.bytes[5]);
      }
    } else if (p.payload.bytes[4] == 'R') {
      // RGB Command
      // get led index
      int ledIndex = p.payload.bytes[5] - '0'; // '0' to '4' -> 0 to 4 This is a terrible way to do this, but i'll fix it later
      if (p.payload.bytes[5] == 'C') {
        // Control pattern generator
        // Get pattern index
        int patternIndex = p.payload.bytes[6] - '0'; // '0' to '4' -> 0 to 4
        if (patternIndex == 0) {
          rgbCycleEnabled = false;

          return ProcessMessage::STOP; // Stop further processing
        } else if (patternIndex >= 1 && patternIndex <= 4) {
          rgbLedEnabled = true;
          rgbCycleEnabled = true;
          rgbLedCycleType = (IFFcycleType)(patternIndex - 1);
          LOG_INFO("IFF RGB Cycle Enabled with Pattern %d", patternIndex);
          return ProcessMessage::STOP; // Stop further processing
        }
      } else if (p.payload.bytes[4] == 'N') {
        // Switch to night mode
        ktd_writeRegister(IFF_RGB_ADDR_A, IFF_RGB_CONTROL_REG, 0x71); 
        ktd_writeRegister(IFF_RGB_ADDR_C, IFF_RGB_CONTROL_REG, 0x71); 
        return ProcessMessage::STOP; // Stop further processing
      } else if (p.payload.bytes[4] == 'B') {
        // Switch to bright mode
        ktd_writeRegister(IFF_RGB_ADDR_A, IFF_RGB_CONTROL_REG, 0xB1); 
        ktd_writeRegister(IFF_RGB_ADDR_C, IFF_RGB_CONTROL_REG, 0xB1); 
        return ProcessMessage::STOP; // Stop further processing
      } else if (ledIndex < 0 || ledIndex > 4) {
        LOG_ERROR("IFF Invalid RGB LED Index: %d", ledIndex);
        return ProcessMessage::STOP; // Stop further processing
      }
      if (ledIndex == 0) {
        // Set all LEDs
        for (int i = 0; i < 4; i++) {
          rgbleds[i].r = (p.payload.bytes[6] - '0') * 22; // '0' to '9' -> 0 to 225
          rgbleds[i].g = (p.payload.bytes[7] - '0') * 22;
          rgbleds[i].b = (p.payload.bytes[8] - '0') * 22;
          LOG_INFO("IFF Set All LEDs to R=%d, G=%d, B=%d", rgbleds[i].r, rgbleds[i].g, rgbleds[i].b);
        }
      } else if (ledIndex >= 1 && ledIndex <= 4) {
        ledIndex = ledIndex - 1; // Adjust for 0-based index
        rgbleds[ledIndex].r = (p.payload.bytes[6] - '0') * 22; // '0' to '9' -> 0 to 225
        rgbleds[ledIndex].g = (p.payload.bytes[7] - '0') * 22;
        rgbleds[ledIndex].b = (p.payload.bytes[8] - '0') * 22;
        LOG_INFO("IFF Set RGB LED %d to R=%d, G=%d, B=%d", ledIndex + 1, rgbleds[ledIndex].r, rgbleds[ledIndex].g, rgbleds[ledIndex].b);
      } else {
        LOG_ERROR("IFF Invalid RGB Command: %c", p.payload.bytes[5]);
        return ProcessMessage::STOP; // Stop further processing
      }
      
      // Update the LED
      ktd_set_rgb(IFF_RGB_ADDR_A, (uint8_t *)&rgbleds[0]);
      ktd_set_rgb(IFF_RGB_ADDR_C, (uint8_t *)&rgbleds[0]);
      // ktd_set_led(IFF_RGB_ADDR_A, ledIndex, rgbleds[ledIndex].r, rgbleds[ledIndex].g, rgbleds[ledIndex].b);
      // ktd_set_led(IFF_RGB_ADDR_C, ledIndex, rgbleds[ledIndex].r, rgbleds[ledIndex].g, rgbleds[ledIndex].b);
      return ProcessMessage::STOP; // Stop further processing
    }
    return ProcessMessage::STOP; // Stop further processing
  }

  return ProcessMessage::CONTINUE; // Let others look at this message also if they want
}

int IFFModule::ktd_readRegister(uint8_t deviceAddress, uint8_t regAddress, uint8_t *data, size_t length)
{
  i2cBus->beginTransmission(deviceAddress);
  i2cBus->write(regAddress);
  i2cBus->endTransmission(false); // Restart for read
  i2cBus->requestFrom(deviceAddress, length);
  for (size_t i = 0; i < length; i++) {
    if (i2cBus->available()) {
      data[i] = i2cBus->read();
    } else {
      return -1; // Error: not enough data
    }
  }
  return i2cBus->endTransmission(); // Stop transmission
}

int IFFModule::ktd_writeRegister(uint8_t deviceAddress, uint8_t regAddress, uint8_t data)
{
  i2cBus->beginTransmission(deviceAddress);
  i2cBus->write(regAddress);
  i2cBus->write(data);
  return i2cBus->endTransmission(); // Returns 0 on success
}

bool IFFModule::ktd_is_present(uint8_t deviceAddress)
{
  uint8_t buf[1];
  int ret = ktd_readRegister(deviceAddress, IFF_RGB_ID_REG, buf, 1);
  if (ret == 0) {
    LOG_INFO("KTDRGB Driver detected at 0x%02x, ID=0x%02x", deviceAddress, buf[0]);
    return true;
  } else {
    LOG_INFO("No KTDRGB Driver detected at 0x%02x, returned %u", deviceAddress, ret);
    return false;
  }
}

int IFFModule::ktd_set_rgb(uint8_t deviceAddress, uint8_t *values)
{
  for (int i = 0; i < 12; i++) {
    uint8_t ret = ktd_writeRegister(deviceAddress, IFF_RGB_IREG_START + i, values[i]);
    if (ret != 0) {
      LOG_ERROR("Failed to set RGB value index %d on device 0x%02x", i, deviceAddress);
      // return -1;
    }
  }
  return 0;
}

int IFFModule::ktd_set_led(uint8_t deviceAddress, uint8_t ledIndex, uint8_t r, uint8_t g, uint8_t b)
{
  int ret = 0;
  ret = ktd_writeRegister(deviceAddress, IFF_RGB_REG_IRED1 + (ledIndex * 3), r);
  ret = ktd_writeRegister(deviceAddress, IFF_RGB_REG_IGRN1 + (ledIndex * 3), g);
  ret = ktd_writeRegister(deviceAddress, IFF_RGB_REG_IBLU1 + (ledIndex * 3), b);
  
  return ret;
}

bool IFFModule::wantPacket(const meshtastic_MeshPacket *p)
{
  return MeshService::isTextPayload(p);
}
