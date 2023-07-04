#pragma once

#include "Global.h"
#include "json.hpp"
#include "Sphere.h"
#include "Vec3.h"
#include <fstream>

using nlohmann::json;

class ReadSphere
{
public:
	ReadSphere(int count, int frames);
	~ReadSphere();

	void CalcMovement();
	void CalcColourChange();
	void CalcRadiusChange();

	Sphere* spheres;
	int sphereAmount;
	Vec3f* endPos;
	Vec3f* movement;
	int frameCount;
	Vec3f* endColours;
	Vec3f* colourChange;
	float* endRad;
	float* radChange;
	
	Sphere* sphere;
};

class JSONReader
{
public:
	static ReadSphere* LoadJSON(const char* path);
};

