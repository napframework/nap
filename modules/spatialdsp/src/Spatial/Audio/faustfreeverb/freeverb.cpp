#include "freeverb.h"

namespace nap
{

    namespace audio
    {

        void FreeverbModel::instanceConstants(int sample_rate)
        {
            fSampleRate = sample_rate;
            fConst0 = std::min<float>(192000.0f, std::max<float>(1.0f, float(fSampleRate)));
            fConst1 = float(int((0.0253061224f * fConst0)));
            fConst2 = float(int((0.0269387756f * fConst0)));
            fConst3 = float(int((0.0289569162f * fConst0)));
            fConst4 = float(int((0.0307482984f * fConst0)));
            fConst5 = float(int((0.0322448984f * fConst0)));
            fConst6 = float(int((0.033809524f * fConst0)));
            fConst7 = float(int((0.0353061222f * fConst0)));
            fConst8 = float(int((0.0366666652f * fConst0)));
            fConst9 = float(int((0.0126077095f * fConst0)));
            fConst10 = float(int((0.00999999978f * fConst0)));
            fConst11 = float(int((0.00773242628f * fConst0)));
            fConst12 = float(int((0.00510204071f * fConst0)));
        }


        void FreeverbModel::instanceClear()
        {
            for (int l0 = 0; (l0 < 2); l0 = (l0 + 1))
            {
                fRec10[l0] = 0.0f;
            }
            for (int l1 = 0; (l1 < 2); l1 = (l1 + 1))
            {
                fRec9[l1] = 0.0f;
            }
            for (int l2 = 0; (l2 < 2); l2 = (l2 + 1))
            {
                fRec11[l2] = 0.0f;
            }
            IOTA = 0;
            for (int l3 = 0; (l3 < 8192); l3 = (l3 + 1))
            {
                fVec0[l3] = 0.0f;
            }
            for (int l4 = 0; (l4 < 2); l4 = (l4 + 1))
            {
                fRec8[l4] = 0.0f;
            }
            for (int l5 = 0; (l5 < 2); l5 = (l5 + 1))
            {
                fRec13[l5] = 0.0f;
            }
            for (int l6 = 0; (l6 < 8192); l6 = (l6 + 1))
            {
                fVec1[l6] = 0.0f;
            }
            for (int l7 = 0; (l7 < 2); l7 = (l7 + 1))
            {
                fRec12[l7] = 0.0f;
            }
            for (int l8 = 0; (l8 < 2); l8 = (l8 + 1))
            {
                fRec15[l8] = 0.0f;
            }
            for (int l9 = 0; (l9 < 8192); l9 = (l9 + 1))
            {
                fVec2[l9] = 0.0f;
            }
            for (int l10 = 0; (l10 < 2); l10 = (l10 + 1))
            {
                fRec14[l10] = 0.0f;
            }
            for (int l11 = 0; (l11 < 2); l11 = (l11 + 1))
            {
                fRec17[l11] = 0.0f;
            }
            for (int l12 = 0; (l12 < 8192); l12 = (l12 + 1))
            {
                fVec3[l12] = 0.0f;
            }
            for (int l13 = 0; (l13 < 2); l13 = (l13 + 1))
            {
                fRec16[l13] = 0.0f;
            }
            for (int l14 = 0; (l14 < 2); l14 = (l14 + 1))
            {
                fRec19[l14] = 0.0f;
            }
            for (int l15 = 0; (l15 < 8192); l15 = (l15 + 1))
            {
                fVec4[l15] = 0.0f;
            }
            for (int l16 = 0; (l16 < 2); l16 = (l16 + 1))
            {
                fRec18[l16] = 0.0f;
            }
            for (int l17 = 0; (l17 < 2); l17 = (l17 + 1))
            {
                fRec21[l17] = 0.0f;
            }
            for (int l18 = 0; (l18 < 8192); l18 = (l18 + 1))
            {
                fVec5[l18] = 0.0f;
            }
            for (int l19 = 0; (l19 < 2); l19 = (l19 + 1))
            {
                fRec20[l19] = 0.0f;
            }
            for (int l20 = 0; (l20 < 2); l20 = (l20 + 1))
            {
                fRec23[l20] = 0.0f;
            }
            for (int l21 = 0; (l21 < 8192); l21 = (l21 + 1))
            {
                fVec6[l21] = 0.0f;
            }
            for (int l22 = 0; (l22 < 2); l22 = (l22 + 1))
            {
                fRec22[l22] = 0.0f;
            }
            for (int l23 = 0; (l23 < 2); l23 = (l23 + 1))
            {
                fRec25[l23] = 0.0f;
            }
            for (int l24 = 0; (l24 < 8192); l24 = (l24 + 1))
            {
                fVec7[l24] = 0.0f;
            }
            for (int l25 = 0; (l25 < 2); l25 = (l25 + 1))
            {
                fRec24[l25] = 0.0f;
            }
            for (int l26 = 0; (l26 < 2); l26 = (l26 + 1))
            {
                fRec26[l26] = 0.0f;
            }
            for (int l27 = 0; (l27 < 2048); l27 = (l27 + 1))
            {
                fVec8[l27] = 0.0f;
            }
            for (int l28 = 0; (l28 < 2); l28 = (l28 + 1))
            {
                fRec6[l28] = 0.0f;
            }
            for (int l29 = 0; (l29 < 2048); l29 = (l29 + 1))
            {
                fVec9[l29] = 0.0f;
            }
            for (int l30 = 0; (l30 < 2); l30 = (l30 + 1))
            {
                fRec4[l30] = 0.0f;
            }
            for (int l31 = 0; (l31 < 2048); l31 = (l31 + 1))
            {
                fVec10[l31] = 0.0f;
            }
            for (int l32 = 0; (l32 < 2); l32 = (l32 + 1))
            {
                fRec2[l32] = 0.0f;
            }
            for (int l33 = 0; (l33 < 1024); l33 = (l33 + 1))
            {
                fVec11[l33] = 0.0f;
            }
            for (int l34 = 0; (l34 < 2); l34 = (l34 + 1))
            {
                fRec0[l34] = 0.0f;
            }
        }


        void FreeverbModel::init(int sample_rate)
        {
            instanceInit(sample_rate);
        }


        void FreeverbModel::instanceInit(int sample_rate)
        {
            instanceConstants(sample_rate);
            instanceClear();
        }


        int FreeverbModel::getSampleRate()
        {
            return fSampleRate;
        }


        void FreeverbModel::compute(int count, FAUSTFLOAT **inputs, FAUSTFLOAT **outputs)
        {
            FAUSTFLOAT *input0 = inputs[0];
            FAUSTFLOAT *output0 = outputs[0];
            float fSlow0 = (0.00100000005f * float(mDamping));
            float fSlow1 = (0.00100000005f * float(mFeedback1));
            float fSlow2 = float(mSpread);
            int iSlow3 = int((fConst1 + fSlow2));
            int iSlow4 = int((fConst2 + fSlow2));
            int iSlow5 = int((fConst3 + fSlow2));
            int iSlow6 = int((fConst4 + fSlow2));
            int iSlow7 = int((fConst5 + fSlow2));
            int iSlow8 = int((fConst6 + fSlow2));
            int iSlow9 = int((fConst7 + fSlow2));
            int iSlow10 = int((fConst8 + fSlow2));
            float fSlow11 = (0.00100000005f * float(mFeedback2));
            float fSlow12 = (fSlow2 + -1.0f);
            int iSlow13 = int(std::min<float>(1024.0f, std::max<float>(0.0f, (fConst9 + fSlow12))));
            int iSlow14 = int(std::min<float>(1024.0f, std::max<float>(0.0f, (fConst10 + fSlow12))));
            int iSlow15 = int(std::min<float>(1024.0f, std::max<float>(0.0f, (fConst11 + fSlow12))));
            int iSlow16 = int(std::min<float>(1024.0f, std::max<float>(0.0f, (fConst12 + fSlow12))));
            for (int i = 0; (i < count); i = (i + 1))
            {
                float fTemp0 = float(input0[i]);
                fRec10[0] = (fSlow0 + (0.999000013f * fRec10[1]));
                float fTemp1 = (1.0f - fRec10[0]);
                fRec9[0] = ((fRec10[0] * fRec9[1]) + (fTemp1 * fRec8[1]));
                fRec11[0] = (fSlow1 + (0.999000013f * fRec11[1]));
                fVec0[(IOTA & 8191)] = (fTemp0 + (fRec9[0] * fRec11[0]));
                fRec8[0] = fVec0[((IOTA - iSlow3) & 8191)];
                fRec13[0] = ((fRec10[0] * fRec13[1]) + (fTemp1 * fRec12[1]));
                fVec1[(IOTA & 8191)] = (fTemp0 + (fRec11[0] * fRec13[0]));
                fRec12[0] = fVec1[((IOTA - iSlow4) & 8191)];
                fRec15[0] = ((fRec10[0] * fRec15[1]) + (fTemp1 * fRec14[1]));
                fVec2[(IOTA & 8191)] = (fTemp0 + (fRec11[0] * fRec15[0]));
                fRec14[0] = fVec2[((IOTA - iSlow5) & 8191)];
                fRec17[0] = ((fRec10[0] * fRec17[1]) + (fTemp1 * fRec16[1]));
                fVec3[(IOTA & 8191)] = (fTemp0 + (fRec11[0] * fRec17[0]));
                fRec16[0] = fVec3[((IOTA - iSlow6) & 8191)];
                fRec19[0] = ((fRec10[0] * fRec19[1]) + (fTemp1 * fRec18[1]));
                fVec4[(IOTA & 8191)] = (fTemp0 + (fRec11[0] * fRec19[0]));
                fRec18[0] = fVec4[((IOTA - iSlow7) & 8191)];
                fRec21[0] = ((fRec10[0] * fRec21[1]) + (fTemp1 * fRec20[1]));
                fVec5[(IOTA & 8191)] = (fTemp0 + (fRec11[0] * fRec21[0]));
                fRec20[0] = fVec5[((IOTA - iSlow8) & 8191)];
                fRec23[0] = ((fRec10[0] * fRec23[1]) + (fTemp1 * fRec22[1]));
                fVec6[(IOTA & 8191)] = (fTemp0 + (fRec11[0] * fRec23[0]));
                fRec22[0] = fVec6[((IOTA - iSlow9) & 8191)];
                fRec25[0] = ((fRec10[0] * fRec25[1]) + (fTemp1 * fRec24[1]));
                fVec7[(IOTA & 8191)] = (fTemp0 + (fRec11[0] * fRec25[0]));
                fRec24[0] = fVec7[((IOTA - iSlow10) & 8191)];
                fRec26[0] = (fSlow11 + (0.999000013f * fRec26[1]));
                float fTemp2 = ((((((((fRec8[0] + fRec12[0]) + fRec14[0]) + fRec16[0]) + fRec18[0]) + fRec20[0]) +
                                  fRec22[0]) + fRec24[0]) + (fRec26[0] * fRec6[1]));
                fVec8[(IOTA & 2047)] = fTemp2;
                fRec6[0] = fVec8[((IOTA - iSlow13) & 2047)];
                float fRec7 = (0.0f - (fRec26[0] * fTemp2));
                float fTemp3 = (fRec6[1] + (fRec7 + (fRec26[0] * fRec4[1])));
                fVec9[(IOTA & 2047)] = fTemp3;
                fRec4[0] = fVec9[((IOTA - iSlow14) & 2047)];
                float fRec5 = (0.0f - (fRec26[0] * fTemp3));
                float fTemp4 = (fRec4[1] + (fRec5 + (fRec26[0] * fRec2[1])));
                fVec10[(IOTA & 2047)] = fTemp4;
                fRec2[0] = fVec10[((IOTA - iSlow15) & 2047)];
                float fRec3 = (0.0f - (fRec26[0] * fTemp4));
                float fTemp5 = (fRec2[1] + (fRec3 + (fRec26[0] * fRec0[1])));
                fVec11[(IOTA & 1023)] = fTemp5;
                fRec0[0] = fVec11[((IOTA - iSlow16) & 1023)];
                float fRec1 = (0.0f - (fRec26[0] * fTemp5));
                output0[i] = FAUSTFLOAT((fRec1 + fRec0[1]));
                fRec10[1] = fRec10[0];
                fRec9[1] = fRec9[0];
                fRec11[1] = fRec11[0];
                IOTA = (IOTA + 1);
                fRec8[1] = fRec8[0];
                fRec13[1] = fRec13[0];
                fRec12[1] = fRec12[0];
                fRec15[1] = fRec15[0];
                fRec14[1] = fRec14[0];
                fRec17[1] = fRec17[0];
                fRec16[1] = fRec16[0];
                fRec19[1] = fRec19[0];
                fRec18[1] = fRec18[0];
                fRec21[1] = fRec21[0];
                fRec20[1] = fRec20[0];
                fRec23[1] = fRec23[0];
                fRec22[1] = fRec22[0];
                fRec25[1] = fRec25[0];
                fRec24[1] = fRec24[0];
                fRec26[1] = fRec26[0];
                fRec6[1] = fRec6[0];
                fRec4[1] = fRec4[0];
                fRec2[1] = fRec2[0];
                fRec0[1] = fRec0[0];
            }
        }


    }

}
