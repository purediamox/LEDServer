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

typedef enum _property_type { PropEnd, PropInteger, PropColor } PROPERTY_TYPE;

// Property array 
typedef struct _propinfo
{
    const char* _name;
    int _offset;
    int _range;
    PROPERTY_TYPE _type; 
    _propinfo (PROPERTY_TYPE type,  const char* name, int offset, int range = 0) {
        _name = name;
        _type = type;
        _offset = offset;
        _range = range;
    }
} PROPINFO; 

// Macros for property map
#define BEGIN_PROPERTY_MAP(EFFECT)    const PROPINFO EFFECT::_Props[] = {
#define END_PROPERTY_MAP()            _propinfo(PropEnd, NULL, 0 , 0)};
#define PROPERTY_INT(EFFECT, EFFECT_FIELD, EFFECT_NAME, EFFECT_MAX)   _propinfo(PropInteger, EFFECT_NAME, offsetof(EFFECT, EFFECT_FIELD), EFFECT_MAX),
#define PROPERTY_COLOR(EFFECT, EFFECT_FIELD, EFFECT_NAME          )   _propinfo(PropColor, EFFECT_NAME, offsetof(EFFECT, EFFECT_FIELD)),

#define DECLARE_PROPERTY_MAP(EFFECT)   static const PROPINFO _Props[];  virtual const PROPINFO* getPropinfo() { return EFFECT::_Props; };


class CEffectMgr 
{
private:
    int   g_numLEDs;     // total number of LEDs
    CRGB* g_pLEDs;
    const int g_Brightness = 10;         // 0-255 LED brightness scale
    const int g_PowerLimit = 900;         // 900mW Power Limit
    int g_active;
public:
    std::vector<CEffect*> _effects;

public:
    CEffectMgr();
    virtual ~CEffectMgr();

     int getNumLeds();
     CRGB color;
     void init(int NumLeds);
    inline  CRGB* LEDs()  { return g_pLEDs; };

    CEffect* getActiveEffect();
    bool setActiveEffect(int id);
};

extern CEffectMgr CFX;          // singleton

class CEffect
{
public:
    const char *name;
    CEffect(const char *name);
    virtual ~CEffect();
    virtual void Draw() = 0;
    virtual const PROPINFO* getPropinfo();

    bool setPropertyValue(int propid, const String& value);
    const String& getPropertyValue(int propid); 

    inline int size() { return CFX.getNumLeds();};
    inline CRGB* LEDs() { return CFX.LEDs();};

private:
    int _numPropInfo;
    int CountProperties(); 

};




class CSolidEffect : public CEffect
{
public:
    CSolidEffect() : CEffect("Solid") { _color = CRGB::Red;};

    DECLARE_PROPERTY_MAP(CSolidEffect);

    virtual void Draw();
//    virtual const PROPINFO* getPropinfo();
private:
    CRGB _color;
//    static const PROPINFO _Props[];
};



class CCometEffect : public CEffect
{
public:
    int _fade;
    CRGB _color;

    CCometEffect();
    virtual void Draw();

    DECLARE_PROPERTY_MAP(CCometEffect);

};
