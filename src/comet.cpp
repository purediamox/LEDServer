#include <Arduino.h>
#define FASTLED_INTERNAL
#include <FastLED.h>
#include "comet.h"

#define LED_PIN     5

CEffectMgr CFX;          // singleton

CEffectMgr::CEffectMgr() 
{
    g_numLEDs = 0;
    g_pLEDs = NULL;
    color = CRGB::Red;
    g_active = 0;
}


int CEffectMgr::getNumLeds()
{
    return g_numLEDs;
}

void CEffectMgr::init(int numLEDs)
{
    g_numLEDs = numLEDs;
    
    g_pLEDs = new CRGB[g_numLEDs];
    memset(g_pLEDs, 0, sizeof(CRGB) * g_numLEDs);

    // add effects.
    _effects.push_back(new CCometEffect());
    _effects.push_back(new CSolidEffect());



    // FASTLED
    FastLED.addLeds<WS2812B, LED_PIN, GRB>(g_pLEDs, g_numLEDs);               // Add our LED strip to the FastLED library
    FastLED.setBrightness(g_Brightness);
    // set_max_power_indicator_LED(LED_BUILTIN);                               // Light the builtin LED if we power throttle
    FastLED.setMaxPowerInMilliWatts(g_PowerLimit);                          // Set the power limit, above which brightness will be throttled
}

CEffect* CEffectMgr::getActiveEffect() 
{
    return _effects[g_active];
}

bool CEffectMgr::setActiveEffect(int id)
{
    if (id >= 0 && id < _effects.size())
    {
        g_active = id;
        return true;

    }
    else
        return false;
}



CEffectMgr::~CEffectMgr() 
{
    delete[] g_pLEDs;
}


CEffect::CEffect(const char *name) : name(name)
{
    _numPropInfo = -1;
}


CEffect::~CEffect()
{
}


const PROPINFO* CEffect::getPropinfo()
{ 
    return NULL; 
}; 

int CEffect::CountProperties() 
{
    if (_numPropInfo == -1) 
    {
        const PROPINFO *pPropInfo = this->getPropinfo();
        if (pPropInfo != NULL)  // if no property map, this is num.
        {       
            int i = 0;
            while (pPropInfo[i]._type != PropEnd)  
            {
                i++;
            }
            _numPropInfo = i;
        }
        else 
            _numPropInfo = 0;
    }
    Serial.printf("%s has props=%d\n", name, _numPropInfo);
    return _numPropInfo;
}
    

    /**
 * Parses color value "#RRGGBB" into a CGRB value
 */
CRGB ParseRGBA(const char* rrggbb)
{
    int i_hex = std::strtol(rrggbb + 1, nullptr, 16);
    return CRGB(i_hex);
}


bool CEffect::setPropertyValue(int propid, const String& value)
{
    const PROPINFO *pPropInfo = this->getPropinfo();
    if (propid >= 0 && propid < CountProperties()) {
        const PROPINFO* pProp = pPropInfo + propid;
        Serial.printf("setting %s", pProp->_name);

        if (pProp->_type == PropInteger) {
            int valInt = value.toInt();
            if (valInt >= 0 && valInt < pProp->_range) {
                *((int*)(((byte*)this) + pProp->_offset)) = valInt; 
                return true;
            }
        } else if (pProp->_type == PropColor) {
            CRGB valClr = ParseRGBA(value.c_str());
            *((CRGB*)(((byte*)this) + pProp->_offset)) = valClr;
            return true;
        }
    }
    return false;  // property not set; 
}


const String& CEffect::getPropertyValue(int propid)
{
    const PROPINFO *pPropInfo = this->getPropinfo();
    if (propid >= 0 && propid < _numPropInfo) {
        const PROPINFO* pProp = pPropInfo + propid;

        if (pProp->_type == PropInteger) {
            int valInt = *((int*)(((byte*)this) + pProp->_offset)); // do pointer arithmetic in bytes, but cast pointer back to correct type before de-ref.
            return String(valInt);
        } else if (pProp->_type == PropColor) {
            CRGB valClr = *((CRGB*)(((byte*)this) + pProp->_offset));
            int intClr = (valClr.r <<16) | (valClr.g << 8) | valClr.b;
            char hex[10];
            sprintf(hex, "#%6.6X", intClr);
            return String(hex);
        }
    }
    return "";          // return an empty string (represents invalid)
}


PROPINFO CCometEffect::_Props[] =  { _propinfo(PropInteger, "fade", offsetof(CCometEffect, _fade), 255),
                                     _propinfo(PropColor, "color", offsetof(CCometEffect, _color)),
                                     _propinfo(PropEnd, NULL,0,  0)};   // end marker



CCometEffect::CCometEffect() : CEffect("Comet")
{
    _fade = 128;
    _color = CRGB::Blue;
}


void CCometEffect::Draw()
{
    const int cometSize = min(3, CFX.getNumLeds() / 10);

    static int iDirection = 1;
    static int iPos = 0;

    iPos += iDirection;
    if (iPos >= (CFX.getNumLeds() - cometSize))
    {
        iDirection = -1;
    }
    else if (iPos <= 0)
        iDirection = 1;

    for (int i = 0; i < cometSize; i++)
        FastLED.leds()[iPos + i] = _color;  

    // Randomly fade the LEDs
    for (int j = 0; j < CFX.getNumLeds(); j++)
        if (random(10) > 5)
            FastLED.leds()[j] = FastLED.leds()[j].fadeToBlackBy(_fade);
};





void CSolidEffect::Draw() 
{
    for (int i = 0; i < FastLED.size(); i++) 
        FastLED.leds()[i]= CFX.color;
}