//+--------------------------------------------------------------------------
//
// NightDriver - (c) 2020 Dave Plummer.  All Rights Reserved.
//
// File:                  
//
// Description:
//
//   
//
// History:     Sep-28-2020     davepl      Created
//
//---------------------------------------------------------------------------

#include <Arduino.h>
#define FASTLED_INTERNAL
#include <FastLED.h>

void DrawComet()
{
    const byte fadeAmt = 128;
    const int cometSize = min (3, NUM_LEDS / 10);

    static int iDirection = 1;
    static int iPos = 0;


    iPos += iDirection;
    if (iPos >= (NUM_LEDS - cometSize)) {
        iDirection = -1;
    } else if (iPos <= 0) 
        iDirection = 1;
    
    for (int i = 0; i < cometSize; i++)
        FastLED.leds()[iPos + i] = g_color;
    
    // Randomly fade the LEDs
    for (int j = 0; j < NUM_LEDS; j++)
        if (random(10) > 5)
            FastLED.leds()[j] = FastLED.leds()[j].fadeToBlackBy(fadeAmt);  

}