/*-----------------------------------------------------------------------------------------------*
 * 7-Segment Flip-disc Clock by Marcin Saj https://flipo.io                                      *
 * https://github.com/marcinsaj/Flipo-Clock-4x7-Segment-Flip-Disc-Display                        *
 *                                                                                               *
 * Classic 24-hour Clock + Time Settings + Flip Disc Speed/Delay Settings                        *
 *                                                                                               *
 * Attention!!! - Firmware Update Instructions - https://bit.ly/4x7SEG-CLOCK-FIRMWARE-UPDATE     *
 * Flip Disc Speed/Delay Settings Instructions - https://bit.ly/4x7SEG-CLOCK-SPEED-SET           *
 * Time Setting Instructions - https://bit.ly/4x7SEG-CLOCK-TIME-SET                              *
 *                                                                                               *
 * Setup:                                                                                        *
 * Assembly Instructions - https://bit.ly/Flip-Disc-Clock-Assembly                               *
 * Clock Diagram - https://bit.ly/4x7SEG-CLOCK-DIAGRAM                                           *
 * Dedicated Controller - https://bit.ly/AC1-FD                                                  *
 * 7-Segment Flip-disc Display - https://bit.ly/7SEG-FD                                          *
 * 3x1 Flip-disc Display - https://bit.ly/3x1DOT-FD                                              *
 * Arduino Nano Every - https://bit.ly/ARD-EVERY                                                 *
 * RTC Real Time Clock RX8025T - https://bit.ly/RX8025T                                          *
 * Temperature Sensor DHT22 - https://bit.ly/DHT22                                               *
 *-----------------------------------------------------------------------------------------------*/

#include <FlipDisc.h>         // https://github.com/marcinsaj/FlipDisc
#include <RTC_RX8025T.h>      // https://github.com/marcinsaj/RTC_RX8025T
#include <TimeLib.h>          // https://github.com/PaulStoffregen/Time
#include <Wire.h>             // https://arduino.cc/en/Reference/Wire (included with Arduino IDE)
#include <OneButton.h>        // https://github.com/mathertel/OneButton
#include <EEPROM.h>           // https://www.arduino.cc/en/Reference/EEPROM

// Attention: do not change! Changing these settings may physical damage the flip-disc displays.
// Pin declaration for a dedicated controller
#define EN_PIN A7  // Start & End SPI transfer data
#define CH_PIN A2  // Charging PSPS module - turn ON/OFF
#define PL_PIN A3  // Release the current pulse - turn ON/OFF 

// RTC
#define RTC_PIN A1 // RTC interrupt input

// Buttons - counting from the top
#define B1_PIN 10  // Top button
#define B2_PIN 9   // Middle button
#define B3_PIN 8   // Bottom button

// Initialize a new OneButton instance for a buttons 
// BUTTON_PIN - Input pin for the button
// false      - Button is active high
// false      - Disable internal pull-up resistor
OneButton button1(B1_PIN, false, false);
OneButton button2(B2_PIN, false, false);
OneButton button3(B3_PIN, false, false);

// The flags to store status of the settings
volatile bool modeSettingsStatus = false;
volatile bool timeSettingsStatus = false;
volatile bool speedSettingsStatus = false;

// Flags for storing button press status
volatile bool shortPressButton1Status = false;
volatile bool shortPressButton3Status = false;
volatile bool longPressButton1Status = false;
volatile bool longPressButton2Status = false;

// A flag that stores the time display status, if the flag is set, 
// the current time will be displayed
volatile bool timeDisplayStatus = false;

// Declare structure that allows convenient access to the time elements:
// - tm.Hour - hours
// - tm.Minute - minutes
tmElements_t tm;

// An array to store individual digits for display
int digit[4] = {0, 0, 0, 0};

// Default disc delay/speed values ​​for Flip.Delay(value)
int flipSpeed[7] = {0, 5, 10, 25, 50, 75, 99};

// Delay effect between flip discs [ms]
// The value is stored in eeprom memory and read during setup
uint8_t flip_disc_delay_time = 0;

// Eeprom address where the disc delay/speed value is stored
static const uint8_t eeprom_address = 0;
/************************************************************************************************/

void rtcInterruptISR(void)
{
  // Set the status of the flag only if we are not in setting mode
  if(timeSettingsStatus == false) timeDisplayStatus = true; 
}

void setup() 
{
  Serial.begin(9600);

  // Attention: do not change! Changing these settings may physical damage the flip-disc displays.
  // Flip.Pin(...); it is the most important function and first to call before everything else. 
  // The function is used to declare pin functions.
  Flip.Pin(EN_PIN, CH_PIN, PL_PIN);
  
  pinMode(RTC_PIN, INPUT_PULLUP);

  // RTC RX8025T initialization
  RTC_RX8025T.init();

  // Time update interrupt initialization. Interrupt generated by RTC (INT output): 
  // "INT_SECOND" - every second,
  // "INT_MINUTE" - every minute.
  RTC_RX8025T.initTUI(INT_MINUTE);

  // "INT_ON" - turn ON interrupt generated by RTC (INT output),
  // "INT_OFF" - turn OFF interrupt.
  RTC_RX8025T.statusTUI(INT_ON);
  
  // ssign an interrupt handler to the RTC output, 
  // an interrupt will be generated every minute to display the time
  attachInterrupt(digitalPinToInterrupt(RTC_PIN), rtcInterruptISR, FALLING);

  // Attention: do not change! Changing these settings may physical damage the flip-disc displays. 
  // Flip.Init(...); it is the second most important function. Initialization function for a series 
  // of displays. The function also prepares SPI to control displays. Correct initialization requires 
  // code names of the serially connected displays:
  // - D7SEG - 7-segment display
  // - D3X1 - 3x1 display
  Flip.Init(D7SEG, D7SEG, D3X1, D7SEG, D7SEG);

  delay(2000);
  
  // Function allows you to control one, two or three discs of the selected D3X1 display.
  // - Flip.Display_3x1(module_number, disc1, disc2, disc3);
  Flip.Display_3x1(1, 0,0,0);

  // his function allows you to display numbers and symbols
  // Flip.Matrix_7Seg(data1,data2,data3,data4); 
  Flip.Matrix_7Seg(F,L,I,P);
  delay(1500);
  Flip.Matrix_7Seg(D,I,S,C);
  delay(1500);

  // Link the button functions
  button1.attachClick(ShortPressButton1);
  button3.attachClick(ShortPressButton3);
  button1.attachLongPressStart(LongPressButton1);
  button2.attachLongPressStart(LongPressButton2);

  Serial.println("Flip-disc 7-Segment Clock");

  // Read disc speed/delay value from eeprom memory
  flip_disc_delay_time = EEPROM.read(eeprom_address);

  bool success_reading = false;

  // Check if the data read from memory is correct, 
  // if it is one of the defined values ​​0, 5, 10, 25, 50, 75, 99ms
  for (int i = 0; i < 7; i++)
  {
    if(flip_disc_delay_time == flipSpeed[i]) success_reading = true;
  }

  // If the read value is correct, set the speed/delay effect according to the read value
  if(success_reading == true) 
  {
    Flip.Delay(flip_disc_delay_time);
  }
  else // If the data is incorrect, reset the time for the speed/delay effect
  {
    flip_disc_delay_time = 0;
    Flip.Delay(flip_disc_delay_time);  
  }

  DisplayTime();
}

void loop(void)
{
  WatchButtons();

  if(timeDisplayStatus == true) DisplayTime();
  if(timeSettingsStatus == true) SettingTime();
  if(speedSettingsStatus == true) SettingSpeed();
}

void DisplayTime(void)
{
  // The function is used to set the delay effect between flip discs. 
  // The default value without calling the function is 0. Can be called multiple times 
  // anywhere in the code. Recommended delay range: 0 - 100ms, max 255ms
  Flip.Delay(flip_disc_delay_time);
  
  // Get the time from the RTC and save it to the tm structure
  RTC_RX8025T.read(tm);
  
  // Extract individual digits for the display
  digit[0] = (tm.Hour   / 10) % 10;
  digit[1] = (tm.Hour   / 1 ) % 10;
  digit[2] = (tm.Minute / 10) % 10;
  digit[3] = (tm.Minute / 1 ) % 10;

  // Display the current time
  Flip.Display_3x1(1, 1,1,0);
  Flip.Matrix_7Seg(digit[0],digit[1],digit[2],digit[3]);
  
  // Print time to the serial monitor
  Serial.print("Time: ");
  if(tm.Hour < 10) Serial.print("0");
  Serial.print(tm.Hour);
  Serial.print(":");
  if(tm.Minute < 10) Serial.print("0");
  Serial.println(tm.Minute);

  // Clear the flag
  timeDisplayStatus = false;
}

void SettingTime(void)
{
  ClearPressButtonFlags();
  modeSettingsStatus = true;

  uint8_t time_settings_level = 1;

  // The speed/delay effect is used only when displaying the time 
  // During time settings, the default value is 0
  Flip.Delay(0);
  
  Flip.Display_3x1(1, 0,0,0); // Clear the dots
  Flip.Matrix_7Seg(T,I,M,E);
  delay(1500);
  
  // Clear digit[] before setting the time
  for(int i = 0; i < 4; i++) digit[i] = 0;
  
  // Display the first digit to set
  Flip.Matrix_7Seg(digit[0],HLM,HLM,HLM);

  // Stay in the time settings until all digits are set
  do
  {
    WatchButtons();
    
    if(shortPressButton1Status == true || shortPressButton3Status == true)
    {
      uint8_t digit_number = time_settings_level - 1;

      // Top button "+1", bottom button "-1"
      if(shortPressButton1Status == true) {digit[digit_number] = digit[digit_number] + 1;}
      if(shortPressButton3Status == true) {digit[digit_number] = digit[digit_number] - 1;}

      // First digit: 0-2
      if(time_settings_level == 1)
      {
        if(digit[digit_number] < 0) digit[digit_number] = 2;
        if(digit[digit_number] > 2) digit[digit_number] = 0;
      }

      // Second digit: 0-9 or 0-3
      if(time_settings_level == 2)
      {
        // If the first digit is 0 or 1 then the second digit can be from 0 to 9
        if(digit[digit_number-1] != 2)
        {
          if(digit[digit_number] < 0) digit[digit_number] = 9;
          if(digit[digit_number] > 9) digit[digit_number] = 0;
        }

        // If the first digit is 2 then the second digit 
        // cannot be greater than 3 because the largest possible hour is 23
        if(digit[digit_number-1] == 2)
        {
          if(digit[digit_number] > 3) digit[digit_number] = 0;
          if(digit[digit_number] < 0) digit[digit_number] = 3;
        }
      }

      // Third digit: 0-5
      if(time_settings_level == 3)
      {
        if(digit[digit_number] < 0) digit[digit_number] = 5;
        if(digit[digit_number] > 5) digit[digit_number] = 0;
      }

      // Fourth digit: 0-9
      if(time_settings_level == 4)
      {
        if(digit[digit_number] < 0) digit[digit_number] = 9;
        if(digit[digit_number] > 9) digit[digit_number] = 0;
      }          

      // Update the display for only the currently set digit,
      // for more details see FlipDisc.h library
      Flip.Display_7Seg(time_settings_level, digit[digit_number]);  

      ClearPressButtonFlags();
    }

    if(longPressButton2Status == true)
    {
      time_settings_level = time_settings_level + 1;
      if(time_settings_level > 4) time_settings_level = 0;

      if(time_settings_level == 2) Flip.Matrix_7Seg(HLM,digit[1],HLM,HLM);
      if(time_settings_level == 3) Flip.Matrix_7Seg(HLM,HLM,digit[2],HLM);
      if(time_settings_level == 4) Flip.Matrix_7Seg(HLM,HLM,HLM,digit[3]);
    
      ClearPressButtonFlags();
    }
  } while(time_settings_level != 0); // Stay in the time settings until all digits are set
  
  // Convert entered individual digits to the format supported by RTC
  uint8_t hour_time = (digit[0] * 10) + digit[1];
  uint8_t minute_time = (digit[2] * 10) + digit[3];

  // setTime(hh, mm, ss, day, month, year) 
  // The date is skipped and the seconds are set by default to 0
  // We are only interested in hours and minutes
  setTime(hour_time, minute_time, 0, 0, 0, 0);

  // Set the RTC from the system time
  RTC_RX8025T.set(now());
  
  timeSettingsStatus = false;
  modeSettingsStatus = false;
  timeDisplayStatus = true;
}

void SettingSpeed(void)
{
  ClearPressButtonFlags(); 
  modeSettingsStatus = true;

  int speed_index = 0;
  
  // The delay/speed effect is used only when displaying the time,
  // during delay/speed settings, the default value is 0
  flip_disc_delay_time = 0;
  Flip.Delay(flip_disc_delay_time);

  Flip.Display_3x1(1, 1,0,1); // Clear the dots
  Flip.Matrix_7Seg(S,P,E,D);
  delay(1500);
  
  // Display the first digit to set
  Flip.Matrix_7Seg(S,P,0,0);

  // Stay in the time settings until all digits are set
  do
  {
    WatchButtons();

    if(shortPressButton1Status == true || shortPressButton3Status == true)
    {
      // Top button "+1", bottom button "-1"
      if(shortPressButton1Status == true) speed_index++;
      if(shortPressButton3Status == true) speed_index--;

      if(speed_index > 6) speed_index = 0;
      if(speed_index < 0) speed_index = 6;

      flip_disc_delay_time = flipSpeed[speed_index];
      Flip.Delay(flip_disc_delay_time);

      uint8_t digit3 = (flip_disc_delay_time / 10) % 10;
      uint8_t digit4 = (flip_disc_delay_time / 1 ) % 10;

      Flip.Display_7Seg(3, digit3);
      Flip.Display_7Seg(4, digit4);

      ClearPressButtonFlags();
    }

    if(longPressButton2Status == true)
    {
      speedSettingsStatus = false;
      ClearPressButtonFlags();
    }

  } while(speedSettingsStatus == true); // Stay in the speed settings until all digits are set 

  EEPROM.write(eeprom_address, flip_disc_delay_time);

  modeSettingsStatus = false;
  timeDisplayStatus = true; 
}

// Button flags clearing function
void ClearPressButtonFlags(void)
{
  shortPressButton1Status = false;
  shortPressButton3Status = false;
  longPressButton1Status = false;
  longPressButton2Status = false;
}

// Keep watching the buttons
void WatchButtons(void)
{
  button1.tick();
  button2.tick();
  button3.tick();

  // If the time settings are not currently active,
  // and if a long press of the middle or top button is detected, set corresponding flag
  if(modeSettingsStatus == false)
  {
    if(longPressButton2Status == true) timeSettingsStatus = true;
    if(longPressButton1Status == true) speedSettingsStatus = true;
  }
}

// Button press handling functions
void ShortPressButton1(void){shortPressButton1Status = true;}
void ShortPressButton3(void){shortPressButton3Status = true;}
void LongPressButton1(void){longPressButton1Status = true;}
void LongPressButton2(void){longPressButton2Status = true;}
