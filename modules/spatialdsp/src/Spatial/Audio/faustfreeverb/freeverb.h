#pragma once

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif

// Std includes
#include <algorithm>

// Nap includes
#include <utility/dllexport.h>

// Spatial dsp includes
#include <Spatial/Audio/denormals.h>

#ifdef __APPLE__
#define exp10f __exp10f
#define exp10 __exp10
#endif


namespace nap
{

    namespace audio
    {

        class NAPAPI FreeverbModel
        {
        public:
            void setDamping(float value) { mDamping = value; }
            void setSpread(unsigned int value) { mSpread = value; }
            void setFeedback1(float value) { mFeedback1 = value; }
            void setFeedback2(float value) { mFeedback2 = value; }

        private:
            FAUSTFLOAT mDamping;
            float fRec10[2];
            float fRec9[2];
            FAUSTFLOAT mFeedback1;
            float fRec11[2];
            int IOTA;
            float fVec0[8192];
            int fSampleRate;
            float fConst0;
            float fConst1;
            FAUSTFLOAT mSpread;
            float fRec8[2];
            float fRec13[2];
            float fVec1[8192];
            float fConst2;
            float fRec12[2];
            float fRec15[2];
            float fVec2[8192];
            float fConst3;
            float fRec14[2];
            float fRec17[2];
            float fVec3[8192];
            float fConst4;
            float fRec16[2];
            float fRec19[2];
            float fVec4[8192];
            float fConst5;
            float fRec18[2];
            float fRec21[2];
            float fVec5[8192];
            float fConst6;
            float fRec20[2];
            float fRec23[2];
            float fVec6[8192];
            float fConst7;
            float fRec22[2];
            float fRec25[2];
            float fVec7[8192];
            float fConst8;
            float fRec24[2];
            FAUSTFLOAT mFeedback2;
            float fRec26[2];
            float fVec8[2048];
            float fConst9;
            float fRec6[2];
            float fVec9[2048];
            float fConst10;
            float fRec4[2];
            float fVec10[2048];
            float fConst11;
            float fRec2[2];
            float fVec11[1024];
            float fConst12;
            float fRec0[2];

        public:

            virtual void instanceConstants(int sample_rate);

            virtual void instanceClear();

            virtual void init(int sample_rate);
            virtual void instanceInit(int sample_rate);
            virtual int getSampleRate();
            virtual void compute(int count, FAUSTFLOAT **inputs, FAUSTFLOAT **outputs);

        };

    }

}


