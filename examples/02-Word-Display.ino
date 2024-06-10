/*-----------------------------------------------------------------------------------------------*
 * 7-Segment Flip-disc Clock by Marcin Saj https://flipo.io                                      *
 * https://github.com/marcinsaj/Flipo-Clock-4x7-Segment-Flip-Disc-Display                        *
 *                                                                                               *
 * Word Display                                                                                  *
 * This example shows how to display 4-letter words on displays                                  *
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
  /* This function allows you to display numbers and symbols
  Flip.Matrix_7Seg(data1,data2,data3,data4); */ 
  Flip.Matrix_7Seg(T,H,N,X);
  delay(3000);
  SetDots();
  Flip.Matrix_7Seg(G,O,A,L);
  SetDots();
  Flip.Matrix_7Seg(D,O,N,E);
  SetDots();
  Flip.Matrix_7Seg(B,O,L,D);
  SetDots();
  Flip.Matrix_7Seg(T,E,C,H);
  SetDots();
  Flip.Matrix_7Seg(A,D,D,S);
  SetDots();
  Flip.Matrix_7Seg(V,I,B,E);
  SetDots();
  Flip.Matrix_7Seg(T,I,M,E);
  SetDots();
  Flip.Matrix_7Seg(W,I,T,H);
  SetDots();
  Flip.Matrix_7Seg(N,I,C,E);
  SetDots();
  Flip.Matrix_7Seg(F,L,I,P);
  SetDots();
  Flip.Matrix_7Seg(D,I,S,C);
  SetDots();
  Flip.Matrix_7Seg(F,O,R,M);
  SetDots();
}

void SetDots(void)
{ 
  delay(1000);
  
  /* Function allows you to control one, two or three discs of the selected D3X1 display. 
  The first argument is the relative number "module_number" of the display in the series 
  of all displays. For example, if we have a combination of D3X1, D7SEG, D3X1, then 
  the second D3X1 display will have a relative number of 2 even though there is a D7SEG display 
  between the D3X1 displays. In the configuration of the D7SEG, D7SEG, D3X1, D7SEG, D7SEG displays, 
  the D3X1 display has the relative number 1.
  - Flip.Display_3x1(module_number, disc1, disc2, disc3); */
  Flip.Display_3x1(1, 1,0,0);
  delay(1000);
  Flip.Display_3x1(1, 1,1,0);
  delay(1000);
  Flip.Display_3x1(1, 1,1,1);
  delay(1000);
  Flip.Display_3x1(1, 0,0,0);
}
