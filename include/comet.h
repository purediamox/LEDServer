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
    static int   g_numLEDs;     // total number of LEDs
    static CRGB* g_pLEDs;
    static const int g_Brightness = 10;         // 0-255 LED brightness scale
    static const int g_PowerLimit = 900;         // 900mW Power Limit
    static std::vector<CEffect*> g_Effects;
    static int g_active;

public:
    static int getNumLeds();
    static int getLEDs();
    static CRGB color;
    static void init(int NumLeds);
    inline static CRGB* LEDs()  { return g_pLEDs; };

    static CEffect* getActiveEffect();

    virtual ~CEffectMgr();
};


class CEffect
{
public:
    const char *name;
    CEffect(const char *name) : name(name)
    {
    }
    virtual ~CEffect() {};
    virtual void Draw();

    inline int size() { return CEffectMgr::getNumLeds();};
    inline CRGB* LEDs() { return CEffectMgr::LEDs();};
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
