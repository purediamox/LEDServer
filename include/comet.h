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
#include <vector>
#include <FastLED.h>        // for CRGB only... hmmm

class CEffect;



class CEffectMgr 
{
private:
    int   g_numLEDs;     // total number of LEDs
    CRGB* g_pLEDs;
    const int g_Brightness = 10;         // 0-255 LED brightness scale
    const int g_PowerLimit = 900;         // 900mW Power Limit
    std::vector<CEffect*> g_Effects;
    int g_active;

public:
    CEffectMgr();
    virtual ~CEffectMgr();

     int getNumLeds();
     int getLEDs();
     CRGB color;
     void init(int NumLeds);
    inline  CRGB* LEDs()  { return g_pLEDs; };

    CEffect* getActiveEffect();
};

extern CEffectMgr CFX;          // singleton

class CEffect
{
public:
    const char *name;
    CEffect(const char *name) : name(name)
    {
    }
    virtual ~CEffect() {};
    virtual void Draw();

    inline int size() { return CFX.getNumLeds();};
    inline CRGB* LEDs() { return CFX.LEDs();};
};




class CSolidEffect : public CEffect
{
public:
    CSolidEffect() : CEffect("Solid"){};
    virtual void Draw();
};



class CCometEffect : public CEffect
{
public:
    CCometEffect() : CEffect("Comet"){};
    virtual void Draw();
};
