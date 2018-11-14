#ifndef pinouthelper_h
#define pinouthelper_h

#include <Arduino.h>

// Comment/uncomment the board you want to flash onto.
//#define ARDUINO
#define NODEMCU

#ifdef NODEMCU // for nodeMCU
#include <pins_arduino.h>
#endif

// #define USE_IO_EXPANDER

#define IO_EXPANDER_ADDRESS 0x20

#define ESP_PIN_0 0   //D3
#define ESP_PIN_1 1   //Tx
#define ESP_PIN_2 2   //D4 -> Led on ESP8266
#define ESP_PIN_3 3   //D9(Rx)
#define ESP_PIN_4 4   //D2
#define ESP_PIN_5 5   //D1
#define ESP_PIN_9 9   //S2
#define ESP_PIN_10 10 //S3
#define ESP_PIN_12 12 //D6
#define ESP_PIN_13 13 //D7
#define ESP_PIN_14 14 //D5
#define ESP_PIN_15 15 //D8
#define ESP_PIN_16 16 //D0 -> Led on NodeMcu

#define IO_EXPANDER_PIN_0 0
#define IO_EXPANDER_PIN_1 1
#define IO_EXPANDER_PIN_2 2
#define IO_EXPANDER_PIN_3 3
#define IO_EXPANDER_PIN_4 4
#define IO_EXPANDER_PIN_5 5
#define IO_EXPANDER_PIN_6 6
#define IO_EXPANDER_PIN_7 7

#ifdef USE_IO_EXPANDER

#define pinA IO_EXPANDER_PIN_7
#define pinB IO_EXPANDER_PIN_6
#define C IO_EXPANDER_PIN_5
#define D IO_EXPANDER_PIN_4
#define OE ESP_PIN_2   //D4 -> Led on NodeMcu
#define R1 ESP_PIN_12  //D6
#define SOFTSER_TX ESP_PIN_15 //D8
#define SOFTSER_RX ESP_PIN_13 //D7
#define SDA ESP_PIN_0  //D3
#define SCL ESP_PIN_4  //D2

#define BUZZER ESP_PIN_14 //D5
#define RTC_SQW_PIN IO_EXPANDER_PIN_0
#define ENCODER_SW_PIN IO_EXPANDER_PIN_1
#define ENCODER_CLK_PIN IO_EXPANDER_PIN_2
#define ENCODER_DT_PIN IO_EXPANDER_PIN_3
#define MCU_INTERRUPT_PIN ESP_PIN_5 //D1
#define LED_1 ESP_PIN_16            //D0 -> Led on ESP8266

#else

// #define LED_1 16      //D0
// // #define B 5       //D1 // red
// // #define SCL 4       //D2 // orange
// #define ENC_CLK 0       //D3 // yellow
// #define ENC_DT 14      //D5 // orange
// #define ENC_SW 2     //D4 // green
// #define SOFTSER_TX ESP_PIN_15 //D8
// // #define CLK 3     //D9(Rx)
// #define SOFTSER_RX ESP_PIN_13 //D7
// #define buzzerPin 12    //D6
// // #define buzzerPin 10 //1 //Tx

// #define LED_1 16      //D0
// // #define B 5       //D1 // red
// // #define SCL 4       //D2 // orange
// #define ENC_CLK 14       //D5 // yellow
// #define ENC_DT 12      //D6 // orange
// #define ENC_SW 2     //D4 // green
// #define SOFTSER_TX ESP_PIN_15 //D8
// // #define CLK 3     //D9(Rx)
// #define SOFTSER_RX ESP_PIN_13 //D7
// #define buzzerPin 0    //D3
// // #define buzzerPin 10 //1 //Tx

#define LED_1 16      //D0
#define buzzerPin 5       //D1
#define ENC_SW 4       //D2
#define ENC_CLK 0       //D3
#define SCL 14      //D5
#define ENC_DT 2     //D4
#define SOFTSER_TX ESP_PIN_15 //D8
// #define CLK 3     //D9(Rx)
#define SOFTSER_RX ESP_PIN_13 //D7
#define SDA 12    //D6
// #define buzzerPin 10 //1 //Tx



#endif

#endif //pinconfig_h