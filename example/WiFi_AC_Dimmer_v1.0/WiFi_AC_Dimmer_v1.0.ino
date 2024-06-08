#include <arduino.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// IO Configuration

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ESP32
#define SCR_Pin 25
#define RELAY_PIN 26
#define LED_PIN 32
#define ZCD_PIN 33

#define AC_CTRL_OFF digitalWrite(SCR_Pin, LOW)
#define AC_CTRL_ON digitalWrite(SCR_Pin, HIGH)

#define RELAY_OFF digitalWrite(RELAY_PIN, LOW)
#define RELAY_ON digitalWrite(RELAY_PIN, HIGH)

#define LED_OFF digitalWrite(LED_PIN, HIGH)
#define LED_ON digitalWrite(LED_PIN, LOW)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Parameter Initialization

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned char dim = 0; // Initial brightness level from 0 to 100, change as you like!
unsigned char i;
unsigned long int zcd_num = 0;
int delay_flag = 0;
long command_time = 0;

void setup()
{
  pinMode(LED_PIN, OUTPUT);   // initialize the ledPin as an output:
  pinMode(RELAY_PIN, OUTPUT); // initialize the RELAYPin as an output:
  pinMode(SCR_Pin, OUTPUT);   // initialize the RELAYPin as an output:
  LED_OFF;
  RELAY_OFF;
  AC_CTRL_OFF;

  Serial.begin(115200); // initialize the serial communication:
  delay(1000);

  RELAY_ON;
  AC_CTRL_ON;
  delay(3000);

  pinMode(ZCD_PIN, INPUT);                     // initialize the ZCDPin as an input:
  attachInterrupt(ZCD_PIN, zero_cross_int, RISING); // CHANGE FALLING RISING

  Serial.println("test begin v1.2");
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Main Program

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop()
{

  for (i = 1; i < 10; i++)
  {
    set_power(i);
    delay(500);
  }
  set_power(0);
  delay(1000);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Zero_Cross

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void zero_cross_int() // function to be fired at the zero crossing to dim the light
{
  if (dim < 5)
    return;
  if (dim > 90)
    return;

  int dimtime = (100 * dim);  // For 60Hz =>65
  delayMicroseconds(dimtime); // Off cycle
  AC_CTRL_ON;                 // triac firing
  delayMicroseconds(500);     // triac On propagation delay (for 60Hz use 8.33)
  AC_CTRL_OFF;                // triac Off
}

// 0 ~ 10
void set_power(int level)
{
  if (level == 0)
  {
    dim = 95;
    RELAY_OFF;
  }
  if (level == 10)
  {
    dim = 5;
    RELAY_ON;
  }
  if (level < 10 && level > 0)
  {
    dim = 10 * (9 - level);
    RELAY_ON;
  }
}