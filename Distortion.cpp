//
//  Distortion.cpp
//  FMOD Distortion
//
//  Created by Max Walley on 13/05/2020.
//  Copyright © 2020 Max Walley. All rights reserved.
//

#include "fmod.hpp"
#include "fmod_errors.h"
#include <stdio.h>
#include <cstring>
#include <math.h>

void errorCheck(FMOD_RESULT result);

extern "C"
{
    F_EXPORT FMOD_DSP_DESCRIPTION* F_CALL FMODGetDSPDescription();
}

FMOD_RESULT F_CALLBACK create(FMOD_DSP_STATE* state);
FMOD_RESULT F_CALLBACK release(FMOD_DSP_STATE* state);
FMOD_RESULT F_CALLBACK reset(FMOD_DSP_STATE* state);
FMOD_RESULT F_CALLBACK read(FMOD_DSP_STATE *dsp_state, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int *outchannels);
FMOD_RESULT F_CALLBACK setFloat(FMOD_DSP_STATE* state, int index, float value);
FMOD_RESULT F_CALLBACK setBool(FMOD_DSP_STATE* state, int index, FMOD_BOOL value);
FMOD_RESULT F_CALLBACK getFloat(FMOD_DSP_STATE* state, int index, float* value, char* valuestr);
FMOD_RESULT F_CALLBACK getBool(FMOD_DSP_STATE* state, int index, FMOD_BOOL* value, char* valuestr);
FMOD_RESULT F_CALLBACK shouldIProcess(FMOD_DSP_STATE *dsp_state, FMOD_BOOL inputsidle, unsigned int length, FMOD_CHANNELMASK inmask, int inchannels, FMOD_SPEAKERMODE speakermode);

//Parameter definitions here
FMOD_DSP_PARAMETER_DESC gain;
FMOD_DSP_PARAMETER_DESC hardClipLevel;
FMOD_DSP_PARAMETER_DESC shapeOn;

//List of pointers to parameters
FMOD_DSP_PARAMETER_DESC* params[3]
{
    &gain,
    &hardClipLevel,
    &shapeOn
};

FMOD_DSP_DESCRIPTION pluginDesc =
{
    FMOD_PLUGIN_SDK_VERSION,
    "MaxWalley Distortion",
    1,
    1,
    1,
    create,
    release,
    reset,
    read,
    0,
    0,
    3,
    params,
    setFloat,
    0,
    setBool,
    0,
    getFloat,
    0,
    getBool,
    0,
    shouldIProcess,
    0,
    0,
    0,
    0
};

void errorCheck(FMOD_RESULT result)
{
    if(result != FMOD_OK)
    {
        printf("FMOD Error %d: %s\n", result, FMOD_ErrorString(result));
    }
}

extern "C"
{
    F_EXPORT FMOD_DSP_DESCRIPTION* F_CALL FMODGetDSPDescription()
    {
        FMOD_DSP_INIT_PARAMDESC_FLOAT(gain, "Gain", "dB", "Level Gain", -80.0, 10.0, 0.0);
        FMOD_DSP_INIT_PARAMDESC_FLOAT(hardClipLevel, "Clip Level", "dB", "The level to clip audio at", -40.0, 0, 0);
        FMOD_DSP_INIT_PARAMDESC_BOOL(shapeOn, "Shape?", "", "Whether to apply soft clipping to audio", 0, 0);
        return &pluginDesc;
    }
}

class pluginData
{
public:
    pluginData();
    
    void setGain(float newGain);
    float getGain() const;
    
    void setClipLevel(float newLevel);
    float getClipLevel() const;
    
    void setShapeOn(FMOD_BOOL on);
    FMOD_BOOL getShapeOn() const;
    
private:
    float pGain;
    float clipLevel;
    FMOD_BOOL shapeSetting;
};

pluginData::pluginData()
{
    pGain = 1.0;
    clipLevel = 1;
}

void pluginData::setGain(float newGain)
{
    pGain = newGain;
}

float pluginData::getGain() const
{
    return pGain;
}

void pluginData::setClipLevel(float newLevel)
{
    clipLevel = newLevel;
}

float pluginData::getClipLevel() const
{
    return clipLevel;
}

void pluginData::setShapeOn(FMOD_BOOL on)
{
    shapeSetting = on;
}

FMOD_BOOL pluginData::getShapeOn() const
{
    return shapeSetting;
}

//Referenced from FMOD gain example
FMOD_RESULT F_CALLBACK create(FMOD_DSP_STATE* state)
{
    //Allocating memory for the plugin
    state->plugindata = (pluginData*) FMOD_DSP_ALLOC(state, sizeof(pluginData));
    
    //Checking that memory has been correctly allocated
    if(state->plugindata == nullptr)
    {
        return FMOD_ERR_MEMORY;
    }
    
    return FMOD_OK;
}

//Referenced from FMOD gain example
FMOD_RESULT F_CALLBACK release(FMOD_DSP_STATE* state)
{
    //Deallocating the memory for the plugin
    pluginData* data = (pluginData*)state->plugindata;
    FMOD_DSP_FREE(state, data);
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK reset(FMOD_DSP_STATE* state)
{
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK read(FMOD_DSP_STATE *dsp_state, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int *outchannels)
{
    pluginData* data = (pluginData*)dsp_state->plugindata;
    
    
    for(int i = 0; i < length; i++)
    {
        for(int j = 0; j < inchannels; j++)
        {
            float sample = *inbuffer++;
            
            sample = sample * data->getGain();
            
            if(sample > data->getClipLevel())
            {
                sample = data->getClipLevel();
            }
            
            if(data->getShapeOn() == true)
            {
                sample = sin(0.5 * M_PI * sample);
            }
            
            *outbuffer++ = sample;
        }
    }
    
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK setFloat(FMOD_DSP_STATE* state, int index, float value)
{
    if(index == 0)
    {
        pluginData* data = (pluginData*)state->plugindata;
        data->setGain(pow(10, (value/20)));
        return FMOD_OK;
    }
    else if(index == 1)
    {
        pluginData* data = (pluginData*)state->plugindata;
        data->setClipLevel(pow(10, (value/20)));
        return FMOD_OK;
    }
    return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALLBACK setBool(FMOD_DSP_STATE* state, int index, FMOD_BOOL value)
{
    if(index == 2)
    {
        pluginData* data = (pluginData*)state->plugindata;
        data->setShapeOn(value);
        return FMOD_OK;
    }
    return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALLBACK getFloat(FMOD_DSP_STATE* state, int index, float* value, char* valuestr)
{
    if(index == 0)
    {
        pluginData* data = (pluginData*)state->plugindata;
        *value = data->getGain();
        sprintf(valuestr, "%f", data->getGain());
        return FMOD_OK;
    }
    else if(index == 1)
    {
        pluginData* data = (pluginData*)state->plugindata;
        *value = data->getClipLevel();
        sprintf(valuestr, "%f", data->getClipLevel());
        return FMOD_OK;
    }
    return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALLBACK getBool(FMOD_DSP_STATE* state, int index, FMOD_BOOL* value, char* valuestr)
{
    if(index == 2)
    {
        pluginData* data = (pluginData*)state->plugindata;
        *value = data->getShapeOn();
        sprintf(valuestr, "%d", data->getShapeOn());
        return FMOD_OK;
    }
    return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALLBACK shouldIProcess(FMOD_DSP_STATE *dsp_state, FMOD_BOOL inputsidle, unsigned int length, FMOD_CHANNELMASK inmask, int inchannels, FMOD_SPEAKERMODE speakermode)
{
    if(inputsidle == true)
    {
        return FMOD_ERR_DSP_DONTPROCESS;
    }
    return FMOD_OK;
}
