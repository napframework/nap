#ifndef NAIVI_OF_UTILS
#define NAIVI_OF_UTILS

// Std includes
#include <string>
#include <assert.h>

// OF Includes
#include <ofTypes.h>
#include <ofTexture.h>
#include <ofImage.h>
#include <ofAppRunner.h>

/**
@brief Holds a variety of utility functions that can be used in combination with OpenFrameworks
Note that all functions are external, meaning they will point to the same object in the entire program.
**/

// Default background color (0,0,0,255)
extern const ofColor	gDefaultBackColor;

// Default background color (0,0,0,0)
extern const ofColor	gDefaultTransparentBackColor;

// Default white color	(255, 255, 255, 255)
extern const ofColor	gDefaultWhiteColor;

// Default max midi value 127
extern const int		gMaxMidiValue;

// Default min midi value 0
extern const int		gMinMidiValue;

// Default origin value (0,0,0)
extern const ofPoint	gOrigin;

// Max byte value
extern const int		gByteMax;

// Min float value
extern const float		gEpsilon;

// Empty string
extern const std::string gEmptyString;

// Returns the texture target for the ofImage specified, -1 if not bound
int						gGetTextureTarget(ofImage& inImage);

// Returns the texture target for the ofTexture specified, -1 if not bound
int						gGetTextureTarget(ofTexture& inTexture);

// Sets the interpolation type for the image specified
void					gSetTextureInterpolation(ofImage& inImage, int inInterpolationMin, int inInterpolationMag);

// Sets the interpolation type for the texture specified
void					gSetTextureInterpolation(ofTexture& inTexture, int inInterpolationMin, int inInterpolationMag);

// Maps default min max midi range to 0 - 1
float					gNormalizeMidiValue(int inValue);

// Creates a midi value
int						gCreateMidiValue(float inValue);

// Mixes two color values
void					gMixColor(const ofVec4f& inColorOne, const ofVec4f& inColorTwo, float inMixValue, ofVec4f& outMixColor);

// Mixes two color values
void					gMixFloatColor(const ofFloatColor& inColorOne, const ofFloatColor& inColorTwo, float inMixValue, ofFloatColor& outMixColor);

// Returns a normalized color based on an nofColor
ofVec4f					gGetNormalizedColor(const ofColor& inColor);

// Mixes two point values
void					gMixPoint(const ofPoint& inPointOne, const ofPoint& inPointTwo, float inMixValue, ofPoint& outMixPoint);

// Mixes a 2d vector
void					gMixVector2f(const ofVec2f& vec1, const ofVec2f& vec2, float mixValue, ofVec2f& outVec);

// Returns a bell curve value
float					gGetBellValue(float inCurveValue, float inStrength);

// Returns the closest power of 2
int						gGetClosestPowerOfTwo(int inValue);

// Smoothdamp
float					gSmoothDamp(float inCurrent, float inTarget, float& ioCurrentVel, float inSmoothTime, float inMaxSpeed, float inDeltatime);

// Sign
template <typename T>	
T gGetSign(T inValue);

// Returns a normalized sine wave
float					gGetSineWave(float inTime, float inFrequency);

// Returns a normalized square wave
float					gGetSquareWave(float inTime, float inFrequency);

// Returns a normalized sawtooth
float					gGetSawWave(float inTime, float inFrequency);

// Returns a normalized triangle wave
float					gGetTriangleWave(float inTime, float inFrequency);


// Clamp
template<typename TDataType>
inline TDataType		gClamp(TDataType inValue, TDataType inMin, TDataType inMax);

// Fit
inline float			gFit(float inValue, float inMin, float inMax, float outMin, float outMax);

// Random
void					gSetRandomSeed(unsigned int inValue);
float					gGetNormRandomValue();
float					gGetRandomValue(float inMin, float inMax);

// Round
int						gRound(float inValue);


template<typename T>
T gWrap(T value, T minValue, T maxValue) {
	T dif = maxValue - minValue;
	while (value < minValue)
		value += dif;
	while (value >= maxValue)
		value -= dif;
	return value;
}


// For x = [0-1] and p [1+] return a curve that is flat at the beginning and the end
inline float gPowerCurve(float x, float p) {
    if (x < 0.0f) return 0.0f;
	if (x > 1.0f) return 1.0f;
	if (x < 0.5f) return pow(x*2.0f, p) / 2.0f;
	else return (2.0f-(pow(2.0f-x*2.0f, p)))/2.0f;
}


// Min / Max
template<typename TDataType>
inline TDataType		gMin(TDataType inX, TDataType inY);

template<typename TDataType>
inline TDataType		gMax(TDataType inX, TDataType inY);

/**
@brief Locks a mutex on construction, releases the mutex on destruction
**/
class AutoLockMutex
{
public:
	AutoLockMutex(ofMutex* inMutex)				: mMutex(inMutex) { assert(mMutex); mMutex->lock(); }
	~AutoLockMutex()							{ mMutex->unlock(); }

private:
	ofMutex*					mMutex;
};

// Cubic bezier spline blend function.
// T is control point and result type, U is parameter type
template<typename T, typename U>
static T cubicBSplineBlend(const T &a, const T &b, const T &c, const T &d, U u)
{
	U it = 1 - u;
	U b0 = it * it * it / 6;
	U b1 = (3 * u * u * u - 6 * u * u + 4) / 6;
	U b2 = (-3 * u * u * u + 3 * u * u + 3 * u + 1) / 6;
	U b3 = u * u * u / 6;
	// Calculate point
	return (a * b0) + (b * b1) + (c * b2) + (d * b3);
}


// Used when clamping a bezier spline
inline int splineClampedIndex(int len, int i)
{
	if (i < 1) return 0;
	if (i > len - 1) return len - 1;
	return i - 1;
}

// Evaluate a cubic bezier spline.
// T is control point and result type, U is parameter type
template<typename T, typename U>
static T evalCubicBSplineClamped(const std::vector<T>& path, const U& t) {
	int pathSize = path.size();
	int pathLength = path.size() + 4;
	int numSections = pathLength - 3;
	int tSec = (int)floor(t * numSections);
	U u = t * numSections - tSec;

	T a = path[splineClampedIndex(pathSize, tSec - 1)];
	T b = path[splineClampedIndex(pathSize, tSec + 0)];
	T c = path[splineClampedIndex(pathSize, tSec + 1)];
	T d = path[splineClampedIndex(pathSize, tSec + 2)];

	return cubicBSplineBlend(a, b, c, d, u);
}

//////////////////////////////////////////////////////////////////////////
// template implementations
//////////////////////////////////////////////////////////////////////////
template<typename TDataType>
TDataType gClamp(TDataType inValue, TDataType inMin, TDataType inMax)
{
	return inValue > inMax ? inMax : inValue < inMin ? inMin : inValue;
}



template<typename TDataType>
TDataType gMin(TDataType inX, TDataType inY)
{
	return inX < inY ? inX : inY;
}



template<typename TDataType>
TDataType gMax(TDataType inX, TDataType inY)
{
	return inX > inY ? inX : inY;
}



template <typename T>
T gGetSign(T inValue)
{
	return inValue == 0 ? 0 : inValue > 0 ? 1 : -1;
}



//////////////////////////////////////////////////////////////////////////
// Inline definitions
//////////////////////////////////////////////////////////////////////////
float gFit(float inValue, float inMin, float inMax, float outMin, float outMax)
{
	float v = gClamp<float>(inValue, inMin, inMax);
	float m = inMax - inMin < gEpsilon ? gEpsilon : inMax - inMin;
	return (v - inMin) / (m) * (outMax - outMin) + outMin;
}







#endif // !NAIVI_OF_UTILS
