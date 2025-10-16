#define LED_PIN 18

#define _VARIANT_IFFMESH_REVA

// I2C
#define I2C_SDA SDA
#define I2C_SCL SCL

// OLED
#define USE_SSD1306 // IFFMesh has a SSD1306 display

#define VEXT_ENABLE 6 // active HIGH - powers the GPS and LoRA radio
#define VEXT_ON_VALUE HIGH
#define BUTTON_PIN 0

#define BATTERY_PIN 3 // A battery voltage measurement pin, voltage divider connected here to measure battery voltage
#define ADC_CHANNEL ADC1_GPIO3_CHANNEL
#define ADC_ATTENUATION ADC_ATTEN_DB_2_5 // lower dB for high resistance voltage divider
#define ADC_MULTIPLIER 4.9 * 1.045
#define ADC_CTRL 2     // active HIGH, powers the voltage divider.
#define ADC_USE_PULLUP // Use internal pullup/pulldown instead of actively driving the output

#undef GPS_RX_PIN
#undef GPS_TX_PIN
#define GPS_RX_PIN 34
#define GPS_TX_PIN 33
#define PIN_GPS_RESET 35
#define PIN_GPS_PPS 36
// #define PIN_GPS_EN 3    // Uncomment to power off the GPS with triple-click on Tracker v2, though we'll also lose the
// display.

#define GPS_RESET_MODE LOW
#define GPS_MODEL_UNKNOWN
#define GPS_BAUDRATE 9600

#define USE_SX1262
#define LORA_DIO0 -1 // a No connect on the SX1262 module
#define LORA_RESET 12
#define LORA_DIO1 14 // SX1262 IRQ
#define LORA_DIO2 13 // SX1262 BUSY
#define LORA_DIO3    // Not connected on PCB, but internally on the TTGO SX1262, if DIO3 is high the TCXO is enabled

#define LORA_SCK 9
#define LORA_MISO 11
#define LORA_MOSI 10
#define LORA_CS 8

#define SX126X_CS LORA_CS
#define SX126X_DIO1 LORA_DIO1
#define SX126X_BUSY LORA_DIO2
#define SX126X_RESET LORA_RESET

#define SX126X_DIO2_AS_RF_SWITCH
#define SX126X_DIO3_TCXO_VOLTAGE 1.8

#define HAS_VEML7700 1

// Buzzer
#define PIN_BUZZER 26

// Vibration
#define PIN_VIBRATION 21
