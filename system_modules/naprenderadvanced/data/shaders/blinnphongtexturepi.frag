#version 450 core

// Extensions
#extension GL_GOOGLE_include_directive : enable

// Total maximum supported number of lights
const uint MAX_LIGHTS = 8;

// Includes
#include "blinnphongtexturefragment.glslinc"
