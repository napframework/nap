// Local Includes
#include "nofUtils.h"

// std includes
#include <assert.h>

static float kOverrideFramerate = 0;

/**
@brief Extern Implementations
**/
const static int sMidiMinValue(0);
const static int sMidiMaxValue(127);



/**
@brief Default background color
**/
const ofColor gDefaultBackColor(0,0,0,255);



/**
@brief Default transparent background color
**/
const ofColor gDefaultTransparentBackColor(0,0,0,0);



/**
@brief Min midi value
**/
const int gMinMidiValue(0);



/**
@brief Default background color
**/
const ofColor gDefaultWhiteColor(255,255,255,255);



/**
@brief Default max midi value
**/
const int gMaxMidiValue(SCHAR_MAX);



/**
@brief Max byte value provided by system (max uchar)
**/
const int gByteMax(UCHAR_MAX);



/**
@brief Returns closest float value to 0
**/
const float gEpsilon(std::numeric_limits<float>::epsilon());



/**
@brief Default origin value (0,0,0)
**/
const ofPoint gOrigin(ofPoint(0.0f, 0.0f, 0.0f));



/**
@brief Empty string
**/
const std::string gEmptyString;



/**
@brief Returns the GPU texture target for inImage
**/
int gGetTextureTarget(ofImage& inImage)
{
	if(!inImage.isUsingTexture())
		return -1;

	return inImage.getTexture().getTextureData().textureTarget;
}



/**
@brief Returns the texture target for inTexture
**/
int gGetTextureTarget(ofTexture& inTexture)
{
	if(!inTexture.isAllocated())
		return -1;

	return inTexture.getTextureData().textureTarget;
}



/**
@brief Sets the texture interpolation method for inImage

The inMin and inMax interpolation values are of type GLint
**/
void gSetTextureInterpolation(ofImage& inImage, int inInterpolationMin, int inInterpolationMag)
{
	int texture_target = gGetTextureTarget(inImage);
	assert(texture_target >= 0);
	inImage.getTexture().bind();
	glTexParameteri(gGetTextureTarget(inImage), GL_TEXTURE_MIN_FILTER, inInterpolationMin);
	glTexParameteri(gGetTextureTarget(inImage), GL_TEXTURE_MAG_FILTER, inInterpolationMag);
	inImage.getTexture().unbind();
}



/**
@brief Sets the texture interpolation method for inTexture

The inMin and inMax interpolation values are of type GLint
**/
void gSetTextureInterpolation(ofTexture& inTexture, int inInterpolationMin, int inInterpolationMag)
{
	int texture_target = gGetTextureTarget(inTexture);
	assert(texture_target >= 0);
	inTexture.bind();
	glTexParameteri(gGetTextureTarget(inTexture), GL_TEXTURE_MIN_FILTER, inInterpolationMin);
	glTexParameteri(gGetTextureTarget(inTexture), GL_TEXTURE_MAG_FILTER, inInterpolationMag);
	inTexture.unbind();
}



/**
@brief Normalizes the incoming midi value
**/
float gNormalizeMidiValue(int inValue)
{
	return ofMap((float)inValue, sMidiMinValue, sMidiMaxValue, 0.0f, 1.0f, true);
}



// Creates a midi value
int gCreateMidiValue(float inValue)
{
	return (int)ofMap(inValue, 0.0f, 1.0f, sMidiMinValue, sMidiMaxValue);
}


// Mixes two color values
 void gMixColor(const ofVec4f& inColorOne, const ofVec4f& inColorTwo, float inMixValue, ofVec4f& outMixColor)
{
	outMixColor.x = ofLerp(inColorOne.x, inColorTwo.x, inMixValue);
	outMixColor.y = ofLerp(inColorOne.y, inColorTwo.y, inMixValue);
	outMixColor.z = ofLerp(inColorOne.z, inColorTwo.z, inMixValue);
	outMixColor.w = ofLerp(inColorOne.w, inColorTwo.w, inMixValue);
}


 // Mixes two float colors
 void gMixFloatColor(const ofFloatColor& inColorOne, const ofFloatColor& inColorTwo, float inMixValue, ofFloatColor& outMixColor)
 {
	 outMixColor.r = ofLerp(inColorOne.r, inColorTwo.r, inMixValue);
	 outMixColor.g = ofLerp(inColorOne.g, inColorTwo.g, inMixValue);
	 outMixColor.b = ofLerp(inColorOne.b, inColorTwo.b, inMixValue);
	 outMixColor.a = ofLerp(inColorOne.a, inColorTwo.a, inMixValue);
 }



// Mixes two point values
void gMixPoint(const ofPoint& inPointOne, const ofPoint& inPointTwo, float inMixValue, ofPoint& outMixPoint)
{
	outMixPoint.x = ofLerp(inPointOne.x, inPointTwo.x, inMixValue);
	outMixPoint.y = ofLerp(inPointOne.y, inPointTwo.y, inMixValue);
	outMixPoint.z = ofLerp(inPointOne.z, inPointTwo.z, inMixValue);
}



// Returns a value from a bell curve
float gGetBellValue(float inCurveValue, float inStrength)
{
	return pow(4.0f, inStrength) * pow(inCurveValue*(1.0f-inCurveValue), inStrength); 
}


// Returns the closest power of two value (15 -> 16, 30 -> 32)
int gGetClosestPowerOfTwo(int inValue)
{
	return pow(2, ceil(log(inValue)/log(2)));
}



float gGetNormRandomValue()
{
	return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}



float gGetRandomValue(float inMin, float inMax)
{
	return inMin + static_cast<float>(rand()) /( static_cast<float>(RAND_MAX/(inMax-inMin)));
}



void gSetRandomSeed(unsigned int inValue)
{
	srand(inValue);
}



int gRound(float inValue)
{
	return inValue < 0.0 ? ceil(inValue - 0.5) : floor(inValue + 0.5);
}



/**
Unity Style Smooth Damp
**/
float gSmoothDamp(float inCurrent, float inTarget, float& ioCurrentVel, float inSmoothTime, float inMaxSpeed, float inDeltatime)
{
	inSmoothTime = max(0.0001f, inSmoothTime);
	float num = 2.0f / inSmoothTime;
	float num2 = num * inDeltatime;
	float num3 = 1.0f / (1.0f + num2 + 0.48f * num2 * num2 + 0.235f * num2 * num2 * num2);
	float num4 = inCurrent - inTarget;
	float num5 = inTarget;
	float num6 = inMaxSpeed * inSmoothTime;
	num4 = ofClamp(num4, -num6, num6);
	inTarget = inCurrent - num4;
	float num7 = (ioCurrentVel + num * num4) * inDeltatime;
	ioCurrentVel = (ioCurrentVel - num * num7) * num3;
	float num8 = inTarget + (num4 + num7) * num3;
	if (num5 - inCurrent > 0.0f == num8 > num5)
	{
		num8 = num5;
		ioCurrentVel = (num8 - num5) / inDeltatime;
	}
	return num8;
}



float gGetSineWave(float inTime, float inFrequency)
{
	return (sin(inTime * inFrequency*PI * 2) / 2.0) + 0.5;
}



float gGetSquareWave(float inTime, float inFrequency)
{
	return ((gGetSign(sin(inTime * inFrequency * PI * 2)) / 2.0) + 0.5);
}



float gGetSawWave(float inTime, float inFrequency)
{
	return fmod(inTime, (1.0 / inFrequency)) * (1.0f / (1.0f / inFrequency));
}



float gGetTriangleWave(float inTime, float inFrequency)
{
	return abs(fmod((inTime *  inFrequency * 2), 2) - 1);
}




/**
@brief Returns a normalized color based on an of Color
**/
ofVec4f gGetNormalizedColor(const ofColor& inColor)
{
	ofVec4f return_vec;
	return_vec.x = (float)inColor.r / (float)gByteMax;
	return_vec.y = (float)inColor.g / (float)gByteMax;
	return_vec.z = (float)inColor.b / (float)gByteMax;
	return_vec.w = (float)inColor.a / (float)gByteMax;
	return return_vec;
}


/**
@brief Lerps a 2d vector
**/
void gMixVector2f(const ofVec2f& vec1, const ofVec2f& vec2, float mixValue, ofVec2f& outVec)
{
	outVec.x = ofLerp(vec1.x, vec2.x, mixValue);
	outVec.y = ofLerp(vec1.y, vec2.y, mixValue);
}
