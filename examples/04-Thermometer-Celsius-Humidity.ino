/*-----------------------------------------------------------------------------------------------*
 * 7-Segment Flip-disc Clock by Marcin Saj https://flipo.io                                      *
 * https://github.com/marcinsaj/Flipo-Clock-4x7-Segment-Flip-Disc-Display                        *
 *                                                                                               *
 * Thermometer (Celsius) & Hygrometer                                                            *
 * You can set three parameters: the measurement and display frequency                           *
 * time how long to display humidity                                                             * 
 * and the delay effect time between flip discs                                                  *
 *                                                                                               *
 * Attention!!! - Firmware Update Instructions - https://bit.ly/4x7SEG-CLOCK-FIRMWARE-UPDATE     *
 *                                                                                               *
 * Required Arduino libraries:                                                                   *
 * - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library                          *
 * - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor                    *
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
#include <Adafruit_Sensor.h>  // https://github.com/adafruit/Adafruit_Sensor
#include <DHT.h>              // https://github.com/adafruit/DHT-sensor-library

/************************************************************************************************/
// How often to measure and display temperature and humidity - seconds
int measurement_frequency = 30;                                                                      
/************************************************************************************************/

/************************************************************************************************/
// How long to display humidity - seconds
int display_delay_humidity = 5;                                                                      
/************************************************************************************************/

/************************************************************************************************/
// Set the delay effect between flip discs. Recommended delay range: 0 - 100ms, max 255ms
int flip_disc_delay_time = 25;
/************************************************************************************************/

#define DHT_PIN A0            // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22         // Sensor type

DHT dht(DHT_PIN, DHTTYPE);    // Initialize DHT sensor

// Attention: do not change! Changing these settings may physical damage the flip-disc displays.
// Pin declaration for a dedicated controller
#define EN_PIN A7  // Start & End SPI transfer data
#define CH_PIN A2  // Charging PSPS module - turn ON/OFF
#define PL_PIN A3  // Release the current pulse - turn ON/OFF 


void setup() 
{
  // Attention: do not change! Changing these settings may physical damage the flip-disc displays.
  // Flip.Pin(...); it is the most important function and first to call before everything else. 
  // The function is used to declare pin functions.
  Flip.Pin(EN_PIN, CH_PIN, PL_PIN);
  
  // Attention: do not change! Changing these settings may physical damage the flip-disc displays. 
  // Flip.Init(...); it is the second most important function. Initialization function for a series 
  // of displays. The function also prepares SPI to control displays. Correct initialization requires 
  // code names of the serially connected displays:
  // - D7SEG - 7-segment display
  // - D3X1 - 3x1 display
  Flip.Init(D7SEG, D7SEG, D3X1, D7SEG, D7SEG);
  delay(3000);

  Flip.Delay(0);

  // This function allows you to display numbers and symbols
  // Flip.Matrix_7Seg(data1,data2,data3,data4); 
  Flip.Matrix_7Seg(T,E,M,P);

  // Function allows you to control one, two or three discs of the selected D3X1 display. 
  // The first argument is the relative number "module_number" of the display in the series 
  // of all displays. For example, if we have a combination of D3X1, D7SEG, D3X1, then 
  // the second D3X1 display will have a relative number of 2 even though there is a D7SEG display 
  // between the D3X1 displays. In the configuration of the D7SEG, D7SEG, D3X1, D7SEG, D7SEG displays, 
  // the D3X1 display has the relative number 1.
  // - Flip.Display_3x1(module_number, disc1, disc2, disc3);
  Flip.Display_3x1(1, 0,0,0); 

  Serial.begin(9600);
  
  // Initialize DHT22 sensor
  dht.begin();

  delay(3000);

  Flip.Display_3x1(1, 0,0,1);

  // The function is used to set the delay effect between flip discs. 
  // The default value without calling the function is 0. Can be called multiple times 
  // anywhere in the code. Recommended delay range: 0 - 100ms, max 255ms  
  Flip.Delay(flip_disc_delay_time);
}

void loop() 
{
  DisplayTemperatureHumidity();
  delay(1000 * measurement_frequency);
}

void DisplayTemperatureHumidity(void)
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

  // Rounding the temperature value ​​and preparing data for display
  float temperature = (heat_celsius + 0.05) * 10 ;
  humidity = humidity * 10;

  DisplayData(humidity, H);

  // How long to display humidity
  delay(1000 * display_delay_humidity);

  DisplayData(temperature, C);
}

void DisplayData(float data, uint8_t type)
{
  int data_value = int (data);
  
  uint8_t digit1 = (data_value / 100) % 10;
  uint8_t digit2 = (data_value / 10) % 10;
  uint8_t digit3 = (data_value / 1) % 10;

  Flip.Matrix_7Seg(digit1, digit2, digit3, type);
}
