not ready
/*-----------------------------------------------------------------------------------------------*
 * 7-Segment Flip-disc Clock by Marcin Saj https://flipo.io                                      *
 * https://github.com/marcinsaj/Flipo-Clock-4x7-Segment-Flip-Disc-Display                        *
 *                                                                                               *
 * Classic 24-hour Clock                                                                         *
 * + Flip Disc Speed/Delay Settings                                                              *
 * + Time Settings                                                                               *
 * + Temperature & Humidity Settings                                                             *
 *                                                                                               *
 * Attention!!! - Firmware Update Instructions - https://bit.ly/4x7SEG-CLOCK-FIRMWARE-UPDATE     *
 * Flip Disc Speed/Delay Settings Instructions - https://bit.ly/4x7SEG-CLOCK-SPEED-SET           *
 * Time Setting Instructions - https://bit.ly/4x7SEG-CLOCK-TIME-SET                              *
 * Temperature & Humidity Setting Instructions -                                                 *
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
bool modeSettingsStatus = false;
bool timeSettingsStatus = false;
bool speedSettingsStatus = false;
bool tempSettingsStatus = false;

// Flags for storing button press status
bool shortPressButton1Status = false;
bool shortPressButton3Status = false;
bool longPressButton1Status = false;
bool longPressButton2Status = false;
bool longPressButton3Status = false;

// RTC interrupt flag
bool interruptRtcStatus = false;






volatile bool clockStatus = true;










// A flag that stores the time display status, if the flag is set, 
// the current time will be displayed
volatile bool timeDisplayStatus = false;

// A flag that stores the temperature display status, if the flag is set, 
// the current temperature will be displayed
bool tempDisplayStatus = false;

// Declare structure that allows convenient access to the time elements:
// - tm.Hour - hours
// - tm.Minute - minutes
tmElements_t tm;

// An array to store individual digits for display
int digit[4] = {0, 0, 0, 0};

// Default disc delay/speed values ​​for Flip.Delay(value)
int flipSpeed[7] = {0, 5, 10, 25, 50, 75, 99};

// How long and after what time from displaying the temperature to display the humidity
const unsigned long wait_interval = 5000;

// Every 30 seconds - temperature and humidity display frequency
const unsigned long sequence_interval = 30000 - 3 * wait_interval;

// Time values for millis()
unsigned long sequence_start_time = 0;
unsigned long sequence_current_time = 0;

// Time, temperature and humidity sequence display status flags
bool sequenceRunning= false;         // Indicates if the sequence is currently running
bool sequenceSecondRun = false;      // Flag for second sequence if the temp/hum display interval is set to 30s
bool firstSequenceComplete = false;  // Flag to indicate the first sequence has completed
bool secondSequenceComplete = false; // Flag to indicate the second sequence has completed

// Variable defining the current level of the time/temperature/humidity display sequence
uint8_t sequence_level = 0;

// Aliases for individual option settings
static const uint8_t HR12 = 0;   // Display time in 12 hour format
static const uint8_t HR24 = 1;   // Display time in 24 hour format
static const uint8_t OFF = 0;    // Turn ON temperature and humidity display 
static const uint8_t ON = 1;     // Turn OFF temperature and humidity display
static const uint8_t TPF = 0;    // Temperature in Fahrenheit
static const uint8_t TPC = 1;    // Temperature in Celsius 
static const uint8_t THFQ60 = 0; // Every 60 seconds - temperature and humidity display frequency
static const uint8_t THFQ30 = 1; // Every 30 seconds - temperature and humidity display frequency
static const uint8_t AM = 0;
static const uint8_t PM = 1;

// The values is stored in eeprom memory and read during setup
uint8_t flip_disc_delay_time = 0; // Flip disc delay/speed effect [ms]
uint8_t leading_zero = 0;         // Leading zero display ON/OFF
uint8_t rest_period = 0;          // Clock rest period ON/OFF
uint8_t sleep_hour = 0;           // Hour to turn off the clock
uint8_t wake_hour = 0;            // Hour to turn on the clock
uint8_t time_hr = 0;              // Time 12/24 hour clock
uint8_t temp_on_off = 0;          // Temperature ON/OFF
uint8_t temp_c_f = 0;             // Temparature C/F - Celsius/Fahrenheit
uint8_t hum_on_off = 0;           // Humidity ON/OFF
uint8_t temp_hum_fq = 0;          // Temperature and humidity display frequency - 30/60 seconds

// Eeprom addresses where settings are stored
static const uint16_t ee_delay_address = 0;        // Flip disc delay/speed effect
static const uint16_t ee_leading_zero_address = 1; // Leading zero display
static const uint16_t ee_rest_period_address = 2;  // Clock rest period ON/OFF
static const uint16_t ee_sleep_hour_address = 3;   // Hour to turn off the clock
static const uint16_t ee_wake_hour_address = 4;    // Hour to turn on the clock
static const uint16_t ee_time_hr_address = 5;      // Time format 12/24 hour  
static const uint16_t ee_temp_on_off_address = 6;  // Temperature ON/OFF
static const uint16_t ee_temp_c_f_address = 7;     // Temparature C/F - Celsius/Fahrenheit
static const uint16_t ee_hum_on_off_address = 8;   // Humidity ON/OFF
static const uint16_t ee_temp_hum_fq_address = 9;  // Temperature and humidity display frequency - 30/60 seconds

// Required for EEPROM.begin(eeprom_size) for Arduino Nano ESP32
static const uint8_t eeprom_size = 10;

/************************************************************************************************/

void rtcInterruptISR(void)
{
  timeDisplayStatus = true;
}

/************************************************************************************************/
void setup() 
{
  Serial.begin(9600);
  
  // Required for Arduino Nano ESP32, initialize EEPROM with predefined size
  #if defined(ARDUINO_ARCH_ESP32)
    EEPROM.begin(eeprom_size);
  #endif

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
  
  // Assign an interrupt handler to the RTC output, 
  // an interrupt will be generated every minute to display the time
  attachInterrupt(digitalPinToInterrupt(RTC_PIN), rtcInterruptISR, FALLING);

  // Attention: do not change! Changing these settings may physical damage the flip-disc displays.
  Flip.Init(D7SEG, D7SEG, D3X1, D7SEG, D7SEG);

  delay(3000); 
  Serial.println("FLIP DISC 7-SEGMENT CLOCK");

  // Function allows you to control one, two or three discs of the selected D3X1 display
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
  button3.attachLongPressStart(LongPressButton3);

  // Read disc speed/delay value from eeprom memory
  flip_disc_delay_time = EEPROM.read(ee_delay_address);

  bool success_reading = false;

  // Check if the data read from memory is correct, 
  // if it is one of the defined values ​​0, 5, 10, 25, 50, 75, 99ms
  for (int i = 0; i < 7; i++)
  {
    if(flip_disc_delay_time == flipSpeed[i]) success_reading = true;
  }

  // If the read value is correct, set the speed/delay effect according to the read value
  if(success_reading == true) Flip.Delay(flip_disc_delay_time);
  else // If the data is incorrect, reset the time for the speed/delay effect
  {
    flip_disc_delay_time = 0;
    Flip.Delay(flip_disc_delay_time);  
  }

  // Read remaining setting options from memory
  leading_zero = EEPROM.read(ee_leading_zero_address);
  rest_period = EEPROM.read(ee_rest_period_address);
  sleep_hour = EEPROM.read(ee_sleep_hour_address);
  wake_hour = EEPROM.read(ee_wake_hour_address);
  time_hr = EEPROM.read(ee_time_hr_address);
  temp_on_off = EEPROM.read(ee_temp_on_off_address);
  temp_c_f = EEPROM.read(ee_temp_c_f_address);
  hum_on_off = EEPROM.read(ee_hum_on_off_address);
  temp_hum_fq = EEPROM.read(ee_temp_hum_fq_address);

  // If the read values ​​are incorrect, set the default values
  if(leading_zero != ON && leading_zero != OFF) leading_zero = ON;         // Turn ON leading zero display
  if(rest_period != OFF && rest_period != ON) rest_period = OFF;           // Clock rest period ON/OFF
  if(sleep_hour < 0 || sleep_hour > 23) sleep_hour = 0;                    // Set sleep hour to 0
  if(wake_hour < 0 || wake_hour > 23) wake_hour = 0;                       // Set wake hour to 0
  if(time_hr != HR12 && time_hr != HR24) time_hr = HR12;                   // Set the time display to 12 hour format
  if(temp_on_off != ON && temp_on_off != OFF) temp_on_off = OFF;           // Turn off the temperature and humidity display, the other options have no effect
  if(temp_c_f != TPF && temp_c_f != TPC) temp_c_f = TPF;                   // Set temperature display in fahrenheit
  if(hum_on_off != ON && hum_on_off != OFF) hum_on_off = OFF;              // Turn off humidity display
  if(temp_hum_fq != THFQ60 && temp_hum_fq != THFQ30) temp_hum_fq = THFQ60; // Set the temperature and humidity display frequency to 60 seconds

  
  DisplayTime();
}

/************************************************************************************************/
void loop(void)
{
  WatchButtons();
  if(timeDisplayStatus == true) DisplayTimeAndTemperature();
  
  if(timeSettingsStatus == true) SettingTime();
  if(speedSettingsStatus == true) SettingSpeed();
  if(tempSettingsStatus == true) SettingTemp();
}

void DisplayRestPeriod(void)
{
  Flip.Matrix_7Seg(HLM,HLM,HLM,HLM);
}

/************************************************************************************************/
void DisplayTime(void)
{  
  // The function is used to set the delay effect between flip discs. 
  Flip.Delay(flip_disc_delay_time);
  
  // Get the time from the RTC and save it to the tm structure
  RTC_RX8025T.read(tm);
  
  uint8_t hour_time = tm.Hour;
  uint8_t minute_time = tm.Minute;
  
  // 12-Hour conversion
  if(time_hr == HR12)
  {
    if(hour_time > 12) hour_time = hour_time - 12;
    if(hour_time == 0) hour_time = 12; 
  }

  // Extract individual digits for the display
  digit[0] = (hour_time / 10) % 10;
  
  // Check if the leading zero hiding option is enabled
  // CLR - clear the leading display
  if(digit[0] == 0 && leading_zero == OFF) digit[0] = CLR;

  digit[1] = (hour_time / 1) % 10;
  digit[2] = (minute_time / 10) % 10;
  digit[3] = (minute_time / 1) % 10;

  // Display the current time
  Flip.Display_3x1(1, 1,1,0);
  Flip.Matrix_7Seg(digit[0],digit[1],digit[2],digit[3]);
  
  // Print time to the serial monitor
  Serial.print("TIME: ");
  if(hour_time < 10) Serial.print("0");
  Serial.print(hour_time);
  Serial.print(":");
  if(minute_time < 10) Serial.print("0");
  Serial.println(minute_time);
}
























/* The sequence is started every full minute after the RTC interrupt occurs or every 30 seconds. 
30 seconds is always counted down from the time of the RTC interrupt, this method was chosen 
to obtain the best visual effect. After the sequence is called, the millis() start time 
is recorded at the very beginning. The time is displayed first. Depending on the options 
that have been set: if temperature display has been enabled, then after 
the time interval "wait_interval" set by default to 5 seconds, the temperature will be displayed. 
Then after the time "wait_interval" the humidity will be displayed, if it has been enabled. 
Of course, if the temperature display has been disabled and the humidity display enabled, 
only humidity will be displayed - these are independent options. If the temperature 
and humidity display frequency has been set to 60 seconds, the sequence will end. 
If it is set to 30 seconds, a new sequence will start 30 seconds after the first sequence was started. 
After the second cycle, the sequence will end. The next loop will start at the next full minute.*/
void DisplayTimeAndTemperature(void)
{  
  if (sequenceRunning == false)     // If the sequence is not already running, initialize it 
  {
    sequence_start_time = millis(); // Record the start time
    sequence_level = 0;             // Start from the first state
    sequenceRunning = true;
  }

  // Update the current time
  sequence_current_time = millis();

  switch(sequence_level) 
  {
    case 0:
        DisplayTime();
        sequence_level = 1;
        sequence_start_time = sequence_current_time;  // Reset the start time for the next interval
      break;

    case 1:
      if(temp_on_off == OFF) sequence_level = 2;
      else if(temp_on_off == ON)
      {
        if(sequence_current_time - sequence_start_time >= wait_interval)
        {
          DisplayTemperature();
          sequence_level = 2; 
          sequence_start_time = sequence_current_time;  // Reset the start time for the next interval
        }
      }
      break;

    case 2:
      if(hum_on_off == OFF) sequence_level = 3;
      else if(hum_on_off == ON)
      {
        if(sequence_current_time - sequence_start_time >= wait_interval)
        {
          DisplayHumidity();
          sequence_level = 3; 
          sequence_start_time = sequence_current_time;  // Reset the start time for the next interval
        }
      }
      break;

    case 3:
      if(temp_on_off == OFF && hum_on_off == OFF) sequence_level = 5;
      else
      {
        if(sequence_current_time - sequence_start_time >= wait_interval)
        {
          DisplayTime();

          if(temp_hum_fq == THFQ30 && sequenceSecondRun == false) 
          {
            sequence_level = 4;
            sequence_start_time = sequence_current_time;  // Reset the start time for the next interval
          }
          else sequence_level = 5;
        }
      }
      break;   

    case 4:
      if(sequence_current_time - sequence_start_time >= sequence_interval)
      {
        sequence_level = 1;
        sequenceSecondRun = true; 
        sequence_start_time = sequence_current_time;  // Reset the start time for the next interval
      }

      break; 

    case 5:
      sequenceRunning = false;
      sequenceSecondRun = false;
      timeDisplayStatus = false;
      break;
  }
} 





























/************************************************************************************************/
void DisplayTemperature(void)
{
  // The function is used to set the delay effect between flip discs. 
  Flip.Delay(flip_disc_delay_time);
  Flip.Matrix_7Seg(2,3,DEG,C);
  Serial.println("DisplayTemperature");

}

/************************************************************************************************/
void DisplayHumidity(void)
{
  // The function is used to set the delay effect between flip discs. 
  Flip.Delay(flip_disc_delay_time);
  Flip.Matrix_7Seg(5,2,CLR,H);
  Serial.println("DisplayHumidity");

}
























/************************************************************************************************/
void SettingTime(void)
{
  ClearPressButtonFlags();

  // If the settings were enabled during the time/temperature/humidity sequence, 
  // we need to reset the sequence flag
  sequenceRunning = false;

  // Settings are active
  modeSettingsStatus = true;
  
  Serial.println();
  Serial.println("TIME FORMAT SETTINGS");

  Flip.Delay(0);
  Flip.Display_3x1(1, 0,0,0); // Clear dots
  Flip.Matrix_7Seg(H,O,U,R);
  delay(1500);
  Flip.Display_3x1(1, 1,1,0); // Set dots

  uint8_t time_settings_level = 0;
  bool updateDisplay = true;

  do
  {
    WatchButtons();

    if(shortPressButton1Status == true || shortPressButton3Status == true)
    {
      if(time_settings_level == 0) time_hr = !time_hr;
      if(time_settings_level == 1) leading_zero = !leading_zero;

      ClearPressButtonFlags();
      updateDisplay = true;    
    }
    
    if(longPressButton2Status == true)
    {
      time_settings_level++;
      if(time_settings_level <= 1) updateDisplay = true;       // Stay in settings
      if(time_settings_level  > 1) timeSettingsStatus = false;  // Exit first part od time settings
      ClearPressButtonFlags();
    }

    if(updateDisplay == true)
    {
      updateDisplay = false;

      if(time_settings_level == 0)
      {
        Serial.print("Time format: ");
        if(time_hr == HR12) {Flip.Matrix_7Seg(H,R,1,2); Serial.println("12-hour");}
        if(time_hr == HR24) {Flip.Matrix_7Seg(H,R,2,4); Serial.println("24-hour");}
      }

      if(time_settings_level == 1)
      {
        Serial.print("Leading zero: ");
        if(leading_zero == ON)  {Flip.Matrix_7Seg(L,Z,O,N); Serial.println("ON"); }
        if(leading_zero == OFF) {Flip.Matrix_7Seg(L,Z,O,F); Serial.println("OFF");}
      }

      updateDisplay = false;
    } 
  } while(timeSettingsStatus == true);

  EEPROM.write(ee_time_hr_address, time_hr); // Save the selected 12/24 time format to memory
  EEPROM.write(ee_leading_zero_address, leading_zero);

  // Required for Arduino Nano ESP32, saves the changes to the EEPROM
  #if defined(ARDUINO_ARCH_ESP32)
    EEPROM.commit(); 
  #endif
  








  Serial.println();
  Serial.println("TIME SETTINGS");
  
  Flip.Delay(0);
  Flip.Display_3x1(1, 0,0,0); // Clear dots
  Flip.Matrix_7Seg(T,I,M,E);
  delay(1500);
  
  time_settings_level = 0;
  updateDisplay = true;
  timeSettingsStatus = true;
  int current_digit = 0;
  bool updateDigit = true;

  uint8_t hour_time = tm.Hour;
  uint8_t minute_time = tm.Minute;
  
  // 12-Hour conversion
  if(time_hr == HR12)
  {
    if(hour_time > 12) hour_time = hour_time - 12;
    if(hour_time == 0) hour_time = 12; 
  }

  // Extract individual digits for the display
  digit[0] = (hour_time / 10) % 10;
  digit[1] = (hour_time / 1) % 10;
  digit[2] = (minute_time / 10) % 10;
  digit[3] = (minute_time / 1) % 10;

  do  // Stay in the time settings until all digits are set
  {
    WatchButtons();
    
    if(shortPressButton1Status == true || shortPressButton3Status == true)
    {
      if(shortPressButton1Status == true) current_digit++; // Top button "+1"
      if(shortPressButton3Status == true) current_digit--; // Bottom button "-1"
      
      if(time_hr == HR12)
      {
        if(time_settings_level == 0) // First digit: 0-1
        {
          if(current_digit < 0) current_digit = 1;
          if(current_digit > 1) current_digit = 0;
          Serial.print(current_digit); Serial.println("---");
        }

        if(time_settings_level == 1) // Second digit: 1-9 or 0-2
        {
          // If the first digit is 0 then the second digit can be from 1 to 9
          if(digit[0] == 0)
          {
           if(current_digit < 1) current_digit = 9;
           if(current_digit > 9)  current_digit = 1;
          }

          // If the first digit is 1 then the second digit 
          // cannot be greater than 2 because the largest possible hour is 12
          if(digit[0] == 1)
          {
            if(current_digit > 2) current_digit = 0;
            if(current_digit < 0) current_digit = 2;
          }

          Serial.print("-"); Serial.print(current_digit); Serial.println("--");
        }
      }
      
      if(time_hr == HR24)
      {
        // First digit: 0-2
        if(time_settings_level == 0)
        {
          if(current_digit < 0) current_digit = 2;
          if(current_digit > 2) current_digit = 0;
          Serial.print(current_digit); Serial.println("---");
        }

        // Second digit: 0-9 or 0-3
        if(time_settings_level == 1)
        {
          // If the first digit is 0 or 1 then the second digit can be from 0 to 9
          if(digit[0] != 2)
          {
            if(current_digit < 0) current_digit = 9;
            if(current_digit > 9) current_digit = 0;
          }

          // If the first digit is 2 then the second digit 
          // cannot be greater than 3 because the largest possible hour is 23
          if(digit[0] == 2)
          {
            if(current_digit > 3) current_digit = 0;
            if(current_digit < 0) current_digit = 3;
          }

          Serial.print("-"); Serial.print(current_digit); Serial.println("--");
        }
      }  

      // Third digit: 0-5
      if(time_settings_level == 2)
      {
        if(current_digit < 0) current_digit = 5;
        if(current_digit > 5) current_digit = 0;
        Serial.print("--"); Serial.print(current_digit); Serial.println("-");
      }

      // Fourth digit: 0-9
      if(time_settings_level == 3)
      {
        if(current_digit < 0) current_digit = 9;
        if(current_digit > 9) current_digit = 0;
        Serial.print("---"); Serial.println(current_digit);
      }          

      // Update the display for only the currently set digit,
      // for more details see FlipDisc.h library
      Flip.Display_7Seg(time_settings_level + 1, current_digit);     
      digit[time_settings_level] = current_digit;

      ClearPressButtonFlags();
    }

    if(longPressButton2Status == true)
    {      
      time_settings_level++;      
      if(time_settings_level >  3) timeSettingsStatus = false;  // Exit settings
      if(time_settings_level <= 3) 
      {
        updateDigit = true;
        updateDisplay = true;
      }

      ClearPressButtonFlags();
    }

    if(updateDigit == true)
    { 
      current_digit = digit[time_settings_level];
      updateDigit = false;
    }
    
    if(updateDisplay == true)
    {
      if(time_settings_level == 0) {Flip.Matrix_7Seg(digit[0],HLM,HLM,HLM); Serial.print(digit[0]); Serial.println("---");}
      if(time_settings_level == 1) {Flip.Matrix_7Seg(HLM,digit[1],HLM,HLM); Serial.print("-"); Serial.print(digit[1]); Serial.println("--");}
      if(time_settings_level == 2) {Flip.Matrix_7Seg(HLM,HLM,digit[2],HLM); Serial.print("--"); Serial.print(digit[2]); Serial.println("-");}
      if(time_settings_level == 3) {Flip.Matrix_7Seg(HLM,HLM,HLM,digit[3]); Serial.print("---"); Serial.println(digit[3]);}

      updateDisplay = false;
    }      

  } while(timeSettingsStatus == true); // Stay in the speed settings until all digits are set













  Flip.Display_3x1(1, 1,1,0); // Set the dots

  updateDisplay = true;
  bool updateData = true;


  bool time_am_pm = 0;
  bool sleep_am_pm = 0;
  bool wake_am_pm = 0;

  bool rest_period = OFF;
  uint8_t sleep_set_hour = 0;
  uint8_t wake_set_hour = 0;

  int set_hour = 0;
  uint8_t digit_1 = 0;
  uint8_t digit_2 = 0;

  timeSettingsStatus = true;
  time_settings_level = 0;


  do
  {
    WatchButtons();

    if(shortPressButton1Status == true || shortPressButton3Status == true)
    {
      if(time_settings_level == 2 || time_settings_level == 4)
      {
        // Top button "+1", bottom button "-1"
        if(shortPressButton1Status == true) set_hour++;
        if(shortPressButton3Status == true) set_hour--;

        if(time_hr == HR24)
        {
          if(set_hour > 23) set_hour = 0;
          if(set_hour < 0) set_hour = 23;
        }

        if(time_hr == HR12)
        {
          if(set_hour > 12) set_hour = 1;
          if(set_hour < 1) set_hour = 12;
        } 
      }

      if(time_settings_level == 0) time_am_pm = !time_am_pm;
      if(time_settings_level == 1) rest_period = !rest_period;
      if(time_settings_level == 3) sleep_am_pm = !sleep_am_pm;
      if(time_settings_level == 5) wake_am_pm = !wake_am_pm;

      updateDisplay = true;
    }  
    
    if(longPressButton2Status == true)
    {      
      time_settings_level++;      
      if(time_settings_level >  5) timeSettingsStatus = false;  // Exit settings
      if(time_settings_level <= 5) 
      {
        updateData = true;
        updateDisplay = true;
      }

      ClearPressButtonFlags();
    }
    
    // If the time is set to 24 hours, there is no need to set the time format AM/PM
    if(time_hr == HR24) 
    {
      if(time_settings_level == 0) time_settings_level = 1;
      if(time_settings_level == 3) time_settings_level = 4;
      if(time_settings_level == 5) time_settings_level = 6;
    }

    if(updateData == true)
    {
      if(time_hr == HR12)
      {
        if(time_settings_level == 2)
        {
          if(sleep_hour >= 12) 
          {
            sleep_am_pm = PM;
            set_hour = sleep_hour - 12;
          }
          if(sleep_hour <  12) 
          {
            sleep_am_pm = AM;
            set_hour = sleep_hour; 
          }
        }
        
        if(time_settings_level == 4)
        {
          if(wake_hour >= 12) 
          {
            wake_am_pm = PM;
            set_hour = wake_hour - 12;
          }
          if(wake_hour <  12) 
          {
            wake_am_pm = AM;
            set_hour = wake_hour; 
          }
        }

        if(set_hour == 0) set_hour = 1;
      }

      updateData = false;
    }











    // If the clock rest period has been disabled, skip the rest of the options and exit the settings
    if(rest_period == OFF && time_settings_level == 2) 
    {
      timeSettingsStatus = false; 
      time_settings_level = 6;
    }

    if(updateDisplay == true)
    {      
      if(time_settings_level == 0)
      {
        Serial.print("Time format: ");
        if(time_am_pm == AM) {Flip.Matrix_7Seg(T,F,A,M); Serial.println("AM");}
        if(time_am_pm == PM) {Flip.Matrix_7Seg(T,F,P,M); Serial.println("PM");}
      }

      if(time_settings_level == 1)
      {
        Serial.print("Clock rest period: ");
        if(rest_period == OFF) {Flip.Matrix_7Seg(R,P,O,F); Serial.println("OFF");}
        if(rest_period == ON)  {Flip.Matrix_7Seg(R,P,O,N); Serial.println("ON");}
      }

      if(time_settings_level == 2)
      {
        Serial.print("Sleep time: ");
        sleep_hour = set_hour;
        digit_1 = (set_hour / 10) % 10;
          
        if(digit_1 == 0) digit_1 = CLR;
        digit_2 = (set_hour / 1 ) % 10;
        Serial.println(set_hour);
        Flip.Matrix_7Seg(S,H,digit_1,digit_2);
      }

      if(time_settings_level == 3)
      {
        Serial.print("Sleep time: ");
        if(sleep_am_pm == AM) {Flip.Matrix_7Seg(S,F,A,M); Serial.println("AM");}
        if(sleep_am_pm == PM) {Flip.Matrix_7Seg(S,F,P,M); Serial.println("PM");}
      }

      if(time_settings_level == 4)
      {
        Serial.print("Wake time: ");
        wake_hour = set_hour;
        digit_1 = (set_hour / 10) % 10;
          
        if(digit_1 == 0) digit_1 = CLR;
        digit_2 = (set_hour / 1 ) % 10;
        Serial.println(set_hour);
        Flip.Matrix_7Seg(W,H,digit_1,digit_2);
      }

      if(time_settings_level == 5)
      {
        Serial.print("Wake time: ");
        if(wake_am_pm == AM) {Flip.Matrix_7Seg(W,F,A,M); Serial.println("AM");}
        if(wake_am_pm == PM) {Flip.Matrix_7Seg(W,F,P,M); Serial.println("PM");}
      }

      updateDisplay = false;
    } 

    ClearPressButtonFlags();

  } while(timeSettingsStatus == true); // Stay in the speed settings until all digits are set




  Serial.println("Settings have been saved");
  Serial.println("------------------------");
  
  // Convert entered individual digits to the format supported by RTC
  hour_time = (digit[0] * 10) + digit[1];
  minute_time = (digit[2] * 10) + digit[3];

  if(time_hr == HR12)
  {
    if(time_am_pm == PM && hour_time != 12) hour_time = hour_time + 12;
    if(time_am_pm == AM && hour_time == 12) hour_time = 0;

    if(sleep_am_pm == PM && sleep_set_hour != 12) sleep_hour = sleep_set_hour + 12;
    if(sleep_am_pm == AM && sleep_set_hour == 12) sleep_hour = 0;

    if(wake_am_pm == PM && wake_set_hour != 12) wake_hour = wake_set_hour + 12;
    if(wake_am_pm == AM && wake_set_hour == 12) wake_hour = 0;
  }


/*

  EEPROM.write(ee_sleep_hour_address, sleep_hour); // Save the selected 12/24 time format to memory
  EEPROM.write(ee_wake_hour_address, wake_hour);

  // Required for Arduino Nano ESP32, saves the changes to the EEPROM
  #if defined(ARDUINO_ARCH_ESP32)
    EEPROM.commit(); 
  #endif

*/




  // setTime(hh, mm, ss, day, month, year) 
  // The date is skipped and the seconds are set by default to 0
  // We are only interested in hours and minutes
  setTime(hour_time, minute_time, 0, 0, 0, 0);

  // Set the RTC from the system time
  RTC_RX8025T.set(now());
  
  timeSettingsStatus = false;
  modeSettingsStatus = false;
  timeDisplayStatus = false;
  DisplayTime();
}









































/************************************************************************************************/
void SettingSpeed(void)
{
  ClearPressButtonFlags();

  // If the settings were enabled during the time/temperature/humidity sequence, 
  // we need to reset the sequence flag
  sequenceRunning = false;

  // Settings are active
  modeSettingsStatus = true;

  Serial.println();
  Serial.println("FLIP DISC SPEED/DELAY EFFECT SETTINGS");
  
  Flip.Delay(0);
  Flip.Display_3x1(1, 0,0,0);                     // Clear dots
  Flip.Matrix_7Seg(S,P,E,D);
  delay(1500);
  Flip.Display_3x1(1, 1,1,0);                     // Set dots
  Flip.Display_7Seg(1,S);                         // Display "S"

  int speed_index = 0;
  bool updateDisplay = true;

  // To correctly display the current settings we need to find speed_index, 
  // which is the place where the current value is in flipSpeed[] array. 
  // Then after entering the settings we start from the current value.
  for(int i = 0; i < 6; i++)
  {
    if(flip_disc_delay_time == flipSpeed[i]) speed_index = i;
  }

  do // Stay in the speed/delay settings until the value is set
  {
    WatchButtons();
    
    if(shortPressButton1Status == true || shortPressButton3Status == true)
    {      
      if(shortPressButton1Status == true) speed_index++; // Top button "+1"
      if(shortPressButton3Status == true) speed_index--; // Bottom button "-1"

      ClearPressButtonFlags();
      updateDisplay = true;
    }
    
    if(longPressButton2Status == true)
    {
      ClearPressButtonFlags();
      speedSettingsStatus = false;
    } 

    if(updateDisplay == true)
    {
      if(speed_index > 6) speed_index = 0;
      if(speed_index < 0) speed_index = 6;

      flip_disc_delay_time = flipSpeed[speed_index];
      Flip.Delay(flip_disc_delay_time);

      uint8_t digit3 = (flip_disc_delay_time / 10) % 10;
      uint8_t digit4 = (flip_disc_delay_time / 1 ) % 10;

      Serial.print("Speed/delay: "); Serial.print(flipSpeed[speed_index]); Serial.println("ms");  

      Flip.Display_7Seg(3, digit3);
      Flip.Display_7Seg(4, digit4);

      updateDisplay = false;
    }
  } while(speedSettingsStatus == true); // Stay in the speed settings until all digits are set 

  Serial.println("Settings have been saved");
  Serial.println("------------------------");
  
  EEPROM.write(ee_delay_address, flip_disc_delay_time);
  
  // Required for Arduino Nano ESP32, saves the changes to the EEPROM
  #if defined(ARDUINO_ARCH_ESP32)
    EEPROM.commit(); 
  #endif

  modeSettingsStatus = false;
  timeDisplayStatus = false;
  
  if(clockStatus == ON) DisplayTime();
  if(clockStatus == OFF) DisplayRestPeriod();
}

/************************************************************************************************/
void SettingTemp(void)
{
  ClearPressButtonFlags();

  // If the settings were enabled during the time/temperature/humidity sequence, 
  // we need to reset the sequence flag
  sequenceRunning = false;

  // Settings are active
  modeSettingsStatus = true;

  Serial.println();
  Serial.println("TEMPERATURE & HUMIDITY SETTINGS");

  Flip.Delay(0);
  Flip.Matrix_7Seg(T,E,M,P);
  Flip.Display_3x1(1, 0,0,0); // Clear the dots
  delay(1500); 
  Flip.Display_3x1(1, 1,1,0); // Set the dots

  uint8_t temp_settings_level = 0;
  bool updateDisplay = true;

  do // Stay in settings until all values are set
  {
    WatchButtons();

    if(shortPressButton1Status == true || shortPressButton3Status == true)
    {
      if(temp_settings_level == 0) temp_on_off = !temp_on_off;
      if(temp_settings_level == 1) temp_c_f = !temp_c_f;
      if(temp_settings_level == 2) hum_on_off = !hum_on_off;
      if(temp_settings_level == 3) temp_hum_fq = !temp_hum_fq; 

      updateDisplay = true;
      ClearPressButtonFlags();
    }

    if(longPressButton2Status == true)
    {
      temp_settings_level++;
      if(temp_settings_level <= 3) updateDisplay = true;        // Stay in settings
      if(temp_settings_level >  3) tempSettingsStatus = false;  // Exit settings

      ClearPressButtonFlags();
    }

  if(updateDisplay == true)
  {      
    if(temp_settings_level == 0)
    {
      Serial.print("Temperature display: ");
      if(temp_on_off == OFF) {Flip.Matrix_7Seg(T,CLR,O,F); Serial.println("OFF");}
      if(temp_on_off == ON) {Flip.Matrix_7Seg(T,CLR,O,N); Serial.println("ON");}
    }

    if(temp_settings_level == 1)
    {
      Serial.print("Temperature: ");
      if(temp_c_f == TPF) {Flip.Matrix_7Seg(D,CLR,DEG,F); Serial.println("°F");}
      if(temp_c_f == TPC) {Flip.Matrix_7Seg(D,CLR,DEG,C); Serial.println("°C");}
    }

    if(temp_settings_level == 2)
    {
      Serial.print("Humidity display: ");
      if(hum_on_off == OFF) {Flip.Matrix_7Seg(H,CLR,O,F); Serial.println("OFF");}
      if(hum_on_off == ON) {Flip.Matrix_7Seg(H,CLR,O,N); Serial.println("ON");}
    }

    if(temp_settings_level == 3)
    {
      Serial.print("Temperature and/or Humidity display frequency: "); 
      if(temp_hum_fq == THFQ60) {Flip.Matrix_7Seg(F,CLR,6,0); Serial.println("60 seconds");}
      if(temp_hum_fq == THFQ30) {Flip.Matrix_7Seg(F,CLR,3,0); Serial.println("30 seconds");}
    }

    updateDisplay = false;
  }  

  } while(tempSettingsStatus == true); // Stay in the speed settings until all digits are set

  Serial.println("Settings have been saved");
  Serial.println("------------------------");
  
  // Save settings
  EEPROM.write(ee_temp_on_off_address, temp_on_off);
  EEPROM.write(ee_temp_c_f_address, temp_c_f);
  EEPROM.write(ee_hum_on_off_address, hum_on_off);
  EEPROM.write(ee_temp_hum_fq_address, temp_hum_fq);

  // Required for Arduino Nano ESP32, saves the changes to the EEPROM
  #if defined(ARDUINO_ARCH_ESP32)
    EEPROM.commit(); 
  #endif

  modeSettingsStatus = false;
  timeDisplayStatus = false;
  
  if(clockStatus == ON) DisplayTime();
  if(clockStatus == OFF) DisplayRestPeriod();
}

/************************************************************************************************/
// Button flags clearing function
void ClearPressButtonFlags(void)
{
  shortPressButton1Status = false;
  shortPressButton3Status = false;
  longPressButton1Status = false;
  longPressButton2Status = false;
  longPressButton3Status = false;
}

/************************************************************************************************/
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
    if(longPressButton1Status == true) speedSettingsStatus = true;
    if(longPressButton2Status == true) timeSettingsStatus = true;
    if(longPressButton3Status == true) tempSettingsStatus = true;
  }
}

/************************************************************************************************/
// Button press handling functions
void ShortPressButton1(void){shortPressButton1Status = true;}
void ShortPressButton3(void){shortPressButton3Status = true;}
void LongPressButton1(void){longPressButton1Status = true;}
void LongPressButton2(void){longPressButton2Status = true;}
void LongPressButton3(void){longPressButton3Status = true;}
