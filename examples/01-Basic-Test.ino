/*-----------------------------------------------------------------------------------------------*
 * 7-Segment Flip-Disc Clock by Marcin Saj https://flipo.io                                      *
 * https://github.com/marcinsaj/Flipo-Clock-4x7-Segment-Flip-Disc-Display                        *
 *                                                                                               *
 * Basic Test                                                                                    *
 * This example is used to test clock displays                                                   *
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

#include <FlipDisc.h>   // https://github.com/marcinsaj/FlipDisc 

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
  Flip.Delay(50);
}

void loop() 
{
  /* The function is used to turn off (clear) all displays */
  Flip.Clear();
  delay(3000);

  /* The function is used to turn on (set) all discs of all displays */
  Flip.All();
  delay(3000);
}
