#pragma once

#include "MeshModule.h"
#include "SinglePortModule.h"
#include "configuration.h"
#include <Arduino.h>
#include "driver/ledc.h"
#include <cstring>
#include <Wire.h>
#include "concurrency/OSThread.h"

#define IFF_RGB_ADDR_A 0x74
#define IFF_RGB_ADDR_C 0x75

#define IFF_RGB_ID_REG 0x00
#define IFF_RGB_MONITOR_REG 0x01
#define IFF_RGB_CONTROL_REG 0x02
#define IFF_RGB_IREG_START 0x03

#define IFF_RGB_REG_IRED1             0x03
#define IFF_RGB_REG_IGRN1             0x04
#define IFF_RGB_REG_IBLU1             0x05

#define IFF_RGB_REG_PG_CNTL           0x0F
#define IFF_RGB_REG_PG_FADE           0x10
#define IFF_RGB_REG_PG_RGB1           0x11
#define IFF_RGB_REG_PG_RGB2           0x12
#define IFF_RGB_REG_PG_RGB3           0x13
#define IFF_RGB_REG_PG_RGB4           0x14
#define IFF_RGB_REG_PG_WD             0x15


class IFFModule : public SinglePortModule, private concurrency::OSThread
{

  struct
  {
    uint8_t r;
    uint8_t g;
    uint8_t b;
  } rgbleds[4];

  struct
  {
    uint8_t IR1;
    uint8_t IR2;
    uint8_t IR3;
  } irleds;

  typedef enum
  {
    IFF_CYCLE_UP,
    IFF_CYCLE_DOWN,
    IFF_CYCLE_ALTERNATING,
    IFF_CYCLE_BLINK
  } IFFcycleType;
  
  ledc_channel_config_t ledc_channel[3];

  char patterns[4][4] = {
      {0b10001000, 0b01000100, 0b00100010, 0b00010001}, // up
      {0b00010001, 0b00100010, 0b01000100, 0b10001000}, // down
      {0b10101010, 0b01010101, 0b10101010, 0b01010101}, // Alternating
      {0b11111111, 0b00000000, 0b11111111, 0b00000000}  // Alternating Inverse
  };

  public:
    IFFModule() : SinglePortModule("iff", meshtastic_PortNum_TEXT_MESSAGE_APP), OSThread("IFF") {}    
    virtual bool wantPacket(const meshtastic_MeshPacket *p) override;

  protected:
    virtual int32_t runOnce() override;
    virtual ProcessMessage handleReceived(const meshtastic_MeshPacket &mp) override;

  private:
    TwoWire *i2cBus = 0;
    bool firstTime = true;
    uint8_t currentIRLed = 0;
    uint8_t currentRGBLed = 0;
    IFFcycleType irLedCycleType = IFF_CYCLE_UP;
    IFFcycleType rgbLedCycleType = IFF_CYCLE_UP;
    bool irLedCycleEnabled = false;
    bool irLedEnabled = false;
    bool rgbLedEnabled = false;
    bool rgbCycleEnabled = false;
    bool ktdAPresent = false;
    bool ktdCPresent = false;

    int ktd_readRegister(uint8_t deviceAddress, uint8_t regAddress, uint8_t *data, size_t length);
    int ktd_writeRegister(uint8_t deviceAddress, uint8_t regAddress, uint8_t data);
    bool ktd_is_present(uint8_t deviceAddress);
    int ktd_set_rgb(uint8_t deviceAddress, uint8_t *values);
    int ktd_set_led(uint8_t deviceAddress, uint8_t ledIndex, uint8_t r, uint8_t g, uint8_t b);

};

extern IFFModule *iffModule;