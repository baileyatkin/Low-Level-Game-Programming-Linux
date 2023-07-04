#pragma once

#include "Global.h"
#include "Sphere.h"
#include "Vec3.h"
#include "JSONReader.h"
#include <string>
#include <chrono>
#include <mutex>
#include <thread>
#include "ThreadPool.h"

using std::string;

#if defined __linux__ || defined __APPLE__
// "Compiled for Linux
#else
// Windows doesn't define these values by default, Linux does
#define M_PI 3.141592653589793
#define INFINITY 1e8
#endif

// This variable controls the maximum recursion depth
#define MAX_RAY_DEPTH 5

class Raytracer
{
public:
	Raytracer(ThreadPool* threads);
	Raytracer(const char* jsonpath, ThreadPool* threads);
	~Raytracer();
	float mix(const float& a, const float& b, const float& mix);
	Vec3f Trace(const Vec3f& rayorig, const Vec3f& raydir, const std::vector<Sphere>& spheres, const int& depth);
	void Render(const std::vector<Sphere>& spheres, int iteration);
	void BasicRender();
	void SimpleShrinking();
	void SmoothScaling(int r);
	void SmoothScalingThreaded();
	void JSONRender(int iteration);
	void JSONRenderThreaded();

	ReadSphere* GetJSON() { return json; }
	void SetJSON(ReadSphere* j) { json = j; }
private:
	ReadSphere* json;

	// debug width/height: 640x480
	// release width/height: 1920x1080
	unsigned width = 640;
	unsigned height = 480;
	unsigned size = width * height;
	float invWidth;
	float invHeight;
	float fov = 30;
	float aspectratio;
	float angle;

	ThreadPool* threadPool;
};