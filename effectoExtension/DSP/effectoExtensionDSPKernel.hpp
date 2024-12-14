//
//  effectoExtensionDSPKernel.hpp
//  effectoExtension
//
//  Created by Andreas Mueller on 10/12/2024.
//

#pragma once

#import <AudioToolbox/AudioToolbox.h>
#import <algorithm>
#import <span>
#import <vector>

#import "effectoExtensionParameterAddresses.h"

#import "gen_exported.h"

#include <iostream>
using namespace std;

/*
 effectoExtensionDSPKernel
 As a non-ObjC class, this is safe to use from render thread.
 */
class effectoExtensionDSPKernel {

  public:
    void initialize(int inputChannelCount, int outputChannelCount,
                    double inSampleRate) {

        mSampleRate = inSampleRate;

        cout << "-- fn initialize..." << endl;
        cout << "Samplerate: " << inSampleRate << endl;
        cout << "outChannels: " << outputChannelCount << endl;
        cout << "inChannels: " << inputChannelCount << "\n" << endl;

        gState = (CommonState *)gen_exported::create(mSampleRate,
                                                     mMaxFramesToRender);

        cout << "gState ins: " << gen_exported::num_inputs() << endl;
        cout << "gState outs: " << gen_exported::num_outputs() << endl;
        cout << "num_params: " << gen_exported::num_params() << "\n" << endl;

        for (int i = 0; i < inputChannelCount; ++i) {
            inputGenBuffers.push_back(new t_sample[mMaxFramesToRender]);
        }
        for (int i = 0; i < outputChannelCount; ++i) {
            outputGenBuffers.push_back(new t_sample[mMaxFramesToRender]);
        }
    }

    void deInitialize() {

        cout << "-- fn deInitialize...\n" << endl;
        runOnce = false;

        if (gState) {
            gen_exported::destroy((CommonState *)gState);
            gState = NULL;

            for (int i = 0; i < inputGenBuffers.size(); ++i) {
                delete[] inputGenBuffers[i];
            }
            inputGenBuffers.clear();
           

            for (int i = 0; i < outputGenBuffers.size(); ++i) {
                delete[] outputGenBuffers[i];
            }
            outputGenBuffers.clear();
           
        }
    }

    // MARK: - Bypass
    bool isBypassed() { return mBypassed; }

    void setBypass(bool shouldBypass) { mBypassed = shouldBypass; }

    // MARK: - Parameter Getter / Setter
    void setParameter(AUParameterAddress address, AUValue value) {
        switch (address) {
        case effectoExtensionParameterAddress::gain:
            mGain = value;
            if (gState) {
                //                cout << "newValue: " << value << endl;
                gen_exported::setparameter(gState, 0, mGain, nullptr);

                //                t_param v;
                //                gen_exported::getparameter(gState, 0, &v);
                //                cout << "set to " << v << endl;
            }
            break;
            // Add a case for each parameter in
            // effectoExtensionParameterAddresses.h
        }
    }

    AUValue getParameter(AUParameterAddress address) {
        // Return the goal. It is not thread safe to return the ramping value.

        switch (address) {
        case effectoExtensionParameterAddress::gain:
            return (AUValue)mGain;

        default:
            return 0.f;
        }
    }

    // MARK: - Max Frames
    AUAudioFrameCount maximumFramesToRender() const {
        return mMaxFramesToRender;
    }

    void setMaximumFramesToRender(const AUAudioFrameCount &maxFrames) {
        mMaxFramesToRender = maxFrames;
    }

    // MARK: - Musical Context
    void setMusicalContextBlock(AUHostMusicalContextBlock contextBlock) {
        mMusicalContextBlock = contextBlock;
    }

    /**
     MARK: - Internal Process

     This function does the core siginal processing.
     Do your custom DSP here.
     */
    void process(std::span<float const *> inputBuffers,
                 std::span<float *> outputBuffers,
                 AUEventSampleTime bufferStartTime,
                 AUAudioFrameCount frameCount) {

        if (!runOnce) {
            cout << "-- fn process..." << endl;
            cout << "inputBuffers: " << inputBuffers.size() << endl;
            cout << "outputBuffers: " << outputBuffers.size() << endl;
            cout << "bufferStartTime: " << bufferStartTime << endl;
            cout << "frameCount: " << frameCount << "\n" << endl;
            runOnce = true;
        }

        /*
         Note: For an Audio Unit with 'n' input channels to 'n' output
         channels, remove the assert below and modify the check in
         [effectoExtensionAudioUnit allocateRenderResourcesAndReturnError]
         */
        assert(inputBuffers.size() == outputBuffers.size());

        if (mBypassed) {
            // Pass the samples through
            for (UInt32 channel = 0; channel < inputBuffers.size(); ++channel) {
                std::copy_n(inputBuffers[channel], frameCount,
                            outputBuffers[channel]);
            }
            return;
        }

        // Use this to get Musical context info from the Plugin Host,
        // Replace nullptr with &memberVariable according to the
        // AUHostMusicalContextBlock function signature
        /*
         if (mMusicalContextBlock) {
         mMusicalContextBlock(nullptr, 	// currentTempo
         nullptr, 	// timeSignatureNumerator
         nullptr, 	// timeSignatureDenominator
         nullptr, 	// currentBeatPosition
         nullptr, 	// sampleOffsetToNextBeat
         nullptr);	// currentMeasureDownbeatPosition
         }
         */

        // Perform per sample dsp on the incoming float in before assigning
        // it to out
        //        for (UInt32 channel = 0; channel < inputBuffers.size();
        //        ++channel) {
        //            for (UInt32 frameIndex = 0; frameIndex < frameCount;
        //            ++frameIndex) {
        //
        //                // Do your sample by sample dsp here:
        //
        //                // GAIN
        //                outputBuffers[channel][frameIndex] =
        //                    inputBuffers[channel][frameIndex] * mGain;
        //            }
        //        }

        // GEN~ code

        //        convert input channels into a Gen -ready format
        for (int i = 0; i < inputBuffers.size(); ++i) {
            FloatToSample(inputGenBuffers[i], inputBuffers[i], frameCount);
        }

        //                silence any open Gen inputs-- what does this do
        //                    ? ?
        //                for (int i = ; i < inputGenBuffers.size(); ++i) {
        //                            zeroBuffer(inputGenBuffers[i],
        //                            frameCount);
        //                        }

        // apply the Gen filter
        if (gState) {
            gen_exported::perform((CommonState *)gState, &inputGenBuffers[0],
                                  inputGenBuffers.size(), &outputGenBuffers[0],
                                  outputGenBuffers.size(), frameCount);

            //            gen_exported::perform(
            //                (CommonState *)gState, (double
            //                **)&inputBuffers[0], (int)inputBuffers.size(),
            //                (double **)&outputBuffers[0],
            //                (int)outputBuffers.size(), (int)frameCount);
        }

        for (int i = 0; i < outputBuffers.size(); ++i) {
            // wrap channels where more inputs are handed in
            // than the gen filter outputs
            //            int gen_idx = i % outputGenBuffers.size();
            FloatFromSample(outputBuffers[i], outputGenBuffers[i], frameCount);
        }
    }

    void handleOneEvent(AUEventSampleTime now, AURenderEvent const *event) {
        switch (event->head.eventType) {
        case AURenderEventParameter: {
            handleParameterEvent(now, event->parameter);
            break;
        }

        default:
            break;
        }
    }

    void handleParameterEvent(AUEventSampleTime now,
                              AUParameterEvent const &parameterEvent) {
        // Implement handling incoming Parameter events as needed
    }

    // HELPER FUNCTIONS:
    void FloatToSample(t_sample *dst, const float *src, long n) {
        while (n--)
            *dst++ = *src++;
    }

    void FloatFromSample(float *dst, const t_sample *src, long n) {
        while (n--)
            *dst++ = *src++;
    }

    void zeroBuffer(t_sample *dst, int n) {
        while (n--)
            *dst++ = 0;
    }

    // MARK: Member Variables
    AUHostMusicalContextBlock mMusicalContextBlock;

    double mSampleRate = 44100.0;
    double mGain = 0.25;
    bool mBypassed = false;
    AUAudioFrameCount mMaxFramesToRender = 1024;

    // gen stuff:
    CommonState *gState = NULL;

    // for debug logs only:
    bool runOnce = false;

    std::vector<t_sample *> inputGenBuffers;
    std::vector<t_sample *> outputGenBuffers;
};
