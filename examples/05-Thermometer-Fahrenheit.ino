/*-----------------------------------------------------------------------------------------------*
 * 7-Segment Flip-disc Clock by Marcin Saj https://flipo.io                                      *
 * https://github.com/marcinsaj/Flipo-Clock-4x7-Segment-Flip-Disc-Display                        *
 *                                                                                               *
 * Thermometer (Fahrenheit)                                                                      *
 * You can set two parameters: the measurement and display frequency                             * 
 * and the delay effect time between flip discs                                                  *
 *                                                                                               *
 * REQUIRES the following Arduino libraries:                                                     *
 * - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library                          *
 * - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor                    *
 *                                                                                               *
 * Setup:                                                                                        *
 * Assembly Instructions - https://bit.ly/Flip-Disc-Clock-Assembly                               *
 * Clock diagram - https://bit.ly/3RhW7H2                                                        *
 * Dedicated Controller - https://bit.ly/AC1-FD                                                  *
 * 7-Segment Flip-disc Display - https://bit.ly/7SEG-FD                                          *
 * 3x1 Flip-disc Display - https://bit.ly/3x1DOT-FD                                              *
 * Arduino Nano Every - https://bit.ly/ARD-EVERY                                                 *
 * RTC real Time Clock RX8025T - https://bit.ly/RX8025T                                          *
 * Temperature Sensor DHT22 - https://bit.ly/DHT22                                               *
 *-----------------------------------------------------------------------------------------------*/

#include <FlipDisc.h>         // https://github.com/marcinsaj/FlipDisc
#include <Adafruit_Sensor.h>  // https://github.com/adafruit/Adafruit_Sensor
#include <DHT.h>              // https://github.com/adafruit/DHT-sensor-library

/************************************************************************************************/
/* Select how often to measure and display the temperature - time in seconds                    */
int display_delay_time = 30;                                                                      
/************************************************************************************************/

/************************************************************************************************/
/* Set the delay effect between flip discs. Recommended delay range: 0 - 100ms, max 255ms       */
int flip_disc_delay_time = 50;
/************************************************************************************************/


#define DHT_PIN A0            // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22         // Sensor type

DHT dht(DHT_PIN, DHTTYPE);    // Initialize DHT sensor

int new_temperature_value = 0;
int old_temperature_value = 0;

/* Attention: do not change! Changing these settings may physical damage the flip-disc displays.
Pin declaration for a dedicated controller */
#define EN_PIN A7  // Start & End SPI transfer data
#define CH_PIN A2  // Charging PSPS module - turn ON/OFF
#define PL_PIN A3  // Release the current pulse - turn ON/OFF 


void setup() 
{
  /* Attention: do not change! Changing these settings may physical damage the flip-disc displays.
  Flip.Pin(...); it is the most important function and first to call before everything else. 
  The function is used to declare pin functions. */
  Flip.Pin(EN_PIN, CH_PIN, PL_PIN);
  
  /* Attention: do not change! Changing these settings may physical damage the flip-disc displays. 
  Flip.Init(...); it is the second most important function. Initialization function for a series 
  of displays. The function also prepares SPI to control displays. Correct initialization requires 
  code names of the serially connected displays:
  - D7SEG - 7-segment display
  - D3X1 - 3x1 display */
  Flip.Init(D7SEG, D7SEG, D3X1, D7SEG, D7SEG);
  delay(3000);

  /* The function is used to set the delay effect between flip discs. 
  The default value without calling the function is 0. Can be called multiple times 
  anywhere in the code. Recommended delay range: 0 - 100ms, max 255ms */
  Flip.Delay(flip_disc_delay_time);

  /* This function allows you to display numbers and symbols
  Flip.Matrix_7Seg(data1,data2,data3,data4); */ 
  Flip.Matrix_7Seg(T,E,M,P);

  /* Function allows you to control one, two or three discs of the selected D3X1 display. 
  The first argument is the relative number "module_number" of the display in the series 
  of all displays. For example, if we have a combination of D3X1, D7SEG, D3X1, then 
  the second D3X1 display will have a relative number of 2 even though there is a D7SEG display 
  between the D3X1 displays. In the configuration of the D7SEG, D7SEG, D3X1, D7SEG, D7SEG displays, 
  the D3X1 display has the relative number 1.
  - Flip.Display_3x1(module_number, disc1, disc2, disc3); */
  Flip.Display_3x1(1, 0,0,0); 

  Serial.begin(9600);
  
  /* Initialize DHT22 sensor*/
  dht.begin();

  delay(3000);
}

void loop() 
{
  DisplayTemperature();
  delay(1000 * display_delay_time);
}

void DisplayTemperature(void)
{
  // Reading temperature and humidity
  float humidity = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float celsius = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float fahrenheit = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(humidity) || isnan(celsius) || isnan(fahrenheit)) return;

  // Compute heat index in Fahrenheit (the default)
  float heat_fahrenheit = dht.computeHeatIndex(fahrenheit, humidity);
  // Compute heat index in Celsius (isFahreheit = false)
  float heat_celsius = dht.computeHeatIndex(celsius, humidity, false);

  Serial.print(F("Humidity: "));
  Serial.print(humidity);
  Serial.print(F("%  Temperature: "));
  Serial.print(heat_celsius);
  Serial.print(F("°C  "));
  Serial.print(heat_fahrenheit);
  Serial.println(F("°F"));
  
  // Rounding the temperature value
  float temperature_fahrenheit = heat_fahrenheit + 0.05;

  // ​​Preparing the temperature for display
  if(temperature_fahrenheit < 100) temperature_fahrenheit = temperature_fahrenheit * 10;

  // Rounding
  new_temperature_value = int (temperature_fahrenheit);

  // Display only if the temperature has changed since the last measurement
  if(new_temperature_value != old_temperature_value)
  {
    // Extract individual digits
    uint8_t digit1 = (new_temperature_value / 100) % 10;
    uint8_t digit2 = (new_temperature_value / 10) % 10;
    uint8_t digit3 = (new_temperature_value / 1) % 10;

    Flip.Matrix_7Seg(digit1, digit2, digit3, F);

    // If the temperature is equal to or higher than 100F (100 * 10), do not display a dot
    if(heat_fahrenheit >= 100) Flip.Display_3x1(1, 0,0,0);
    else Flip.Display_3x1(1, 0,0,1);
  }

  old_temperature_value = new_temperature_value;
}
