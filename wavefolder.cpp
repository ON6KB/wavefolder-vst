#include "public.sdk/source/vst2.x/audioeffectx.h"
#include <cmath>

enum {
    kThreshold,
    kStages,
    kNumParams
};

class WavefolderDistortion : public AudioEffectX {
public:
    WavefolderDistortion(audioMasterCallback audioMaster)
    : AudioEffectX(audioMaster, kNumParams, 1) {
        setNumInputs(2);
        setNumOutputs(2);
        setUniqueID('WvFd');
        canProcessReplacing();
        canDoubleReplacing();

        threshold = 0.5f;
        stages = 1;
        setParameter(kThreshold, 0.5f);
        setParameter(kStages, 0.0f);
    }

    ~WavefolderDistortion() {}

    void setParameter(VstInt32 index, float value) {
        switch (index) {
            case kThreshold:
                threshold = value * 0.99f + 0.01f; // Range: 0.01 to 1.0
                break;
            case kStages:
                stages = std::floor(value * 7.99f) + 1; // Range: 1 to 8
                break;
        }
    }

    float getParameter(VstInt32 index) {
        switch (index) {
            case kThreshold:
                return (threshold - 0.01f) / 0.99f;
            case kStages:
                return (stages - 1) / 7.0f;
            default:
                return 0.0f;
        }
    }

    void getParameterName(VstInt32 index, char* label) {
        switch (index) {
            case kThreshold:
                vst_strncpy(label, "Threshold", kVstMaxParamStrLen);
                break;
            case kStages:
                vst_strncpy(label, "Stages", kVstMaxParamStrLen);
                break;
        }
    }

    void getParameterDisplay(VstInt32 index, char* text) {
        switch (index) {
            case kThreshold:
                float2string(threshold, text, kVstMaxParamStrLen);
                break;
            case kStages:
                int2string(stages, text, kVstMaxParamStrLen);
                break;
        }
    }

    void processReplacing(float** inputs, float** outputs, VstInt32 sampleFrames) {
        float* in1 = inputs[0];
        float* in2 = inputs[1];
        float* out1 = outputs[0];
        float* out2 = outputs[1];

        for (int i = 0; i < sampleFrames; i++) {
            float input = in1[i];
            float output = multiStageWavefolder(input);
            out1[i] = output;
            out2[i] = output;  // For stereo, process both channels
        }
    }

private:
    float threshold;
    int stages;

    float singleStageWavefolder(float input) {
        if (input > threshold) {
            return 2.0f * threshold - input;
        } else if (input < -threshold) {
            return -2.0f * threshold - input;
        }
        return input;
    }

    float multiStageWavefolder(float input) {
        float output = input;
        for (int i = 0; i < stages; i++) {
            output = singleStageWavefolder(output);
        }
        return output;
    }
};

AudioEffect* createEffectInstance(audioMasterCallback audioMaster) {
    return new WavefolderDistortion(audioMaster);
}