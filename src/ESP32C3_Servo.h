
#ifndef ESP32C3_Servo_h
#define ESP32C3_Servo_h
#include "esp_system.h"

#if (CONFIG_IDF_TARGET_ESP32C3 != 1)
    #error "This code is intended to run on ESP32-C3 platform only"
#endif

// Values for TowerPro MG995 large servos (and many other hobbyist servos)
#define DEFAULT_uS_LOW 500   // 1000us
#define DEFAULT_uS_HIGH 2500 // 2000us

// Values for TowerPro SG90 small servos
// #define DEFAULT_uS_LOW 400
// #define DEFAULT_uS_HIGH 2400

// 0-1023
#define DEFAULT_TIMER_WIDTH 10         // 定时器位宽
#define DEFAULT_TIMER_WIDTH_TICKS 1024 // 定时器最大定时Tick值

// #define ESP32_Servo_VERSION           1     // 库的软件版本号

#define MIN_PULSE_WIDTH 500          // 最小脉宽
#define MAX_PULSE_WIDTH 2500         // 最大脉宽
#define DEFAULT_PULSE_WIDTH 1500     // 默认脉宽
#define DEFAULT_PULSE_WIDTH_TICKS 25 // 默认脉宽的tick值
#define REFRESH_CPS 50               // 频率
#define REFRESH_USEC 20000           // 周期时间

#define MAX_SERVOS 6                  // ESP32C3 支持的最大PWMC通道数

class Servo
{
public:
  Servo();
  // Arduino Servo Library calls
  int attach(int pin);                   // attach the given pin to the next free channel, returns channel number or 0 if failure
  int attach(int pin, int min, int max); // as above but also sets min and max values for writes.
  void detach();
  void write(int value);             // if value is < MIN_PULSE_WIDTH its treated as an angle, otherwise as pulse width in microseconds
  void writeMicroseconds(int value); // Write pulse width in microseconds
  int read();                        // returns current pulse width as an angle between 0 and 180 degrees
  int readMicroseconds();            // returns current pulse width in microseconds for this servo
  bool attached();                   // return true if this servo is attached, otherwise false

  // ESP32 only functions
  void setTimerWidth(int value); // set the PWM timer width (ESP32 ONLY)
  int readTimerWidth();          // get the PWM timer width (ESP32 ONLY)

private:
  int usToTicks(int usec);
  int ticksToUs(int ticks);
  static int ServoCount;                             // the total number of attached servos
  static int ChannelUsed[];                          // used to track whether a channel is in service
  int servoChannel = 0;                              // channel number for this servo
  int min = DEFAULT_uS_LOW;                          // minimum pulse width for this servo
  int max = DEFAULT_uS_HIGH;                         // maximum pulse width for this servo
  int pinNumber = 0;                                 // GPIO pin assigned to this channel
  int timer_width = DEFAULT_TIMER_WIDTH;             // 定时器的位数
  int ticks = DEFAULT_PULSE_WIDTH_TICKS;             // current pulse width on this channel
  int timer_width_ticks = DEFAULT_TIMER_WIDTH_TICKS; // no. of ticks at rollover; varies with width
};
#endif
