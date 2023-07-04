#include "JSONReader.h"

ReadSphere::ReadSphere(int count, int frames)
{
	sphereAmount = count;
	frameCount = frames;
	spheres = new Sphere[sphereAmount];
	endPos = new Vec3f[sphereAmount];
	movement = new Vec3f[sphereAmount];
	endColours = new Vec3f[sphereAmount];
	colourChange = new Vec3f[sphereAmount];
	endRad = new float[sphereAmount];
	radChange = new float[sphereAmount];

}

ReadSphere::~ReadSphere()
{
	delete(spheres);
	delete(endPos);
	delete(movement);
	delete(endColours);
	delete(colourChange);
	delete(endRad);
	delete(radChange);
}

void ReadSphere::CalcMovement()
{
	float multi = 1 / (float)frameCount;
	//iterate through all spheres
	for (int i = 0; i < sphereAmount; i++)
	{
		//get the vector for the current sphere's current position up to the end position then divide it by the number of frames
		Vec3f dif = endPos[i] - spheres[i].center;
		dif = dif.operator*(multi);
		movement[i] = dif;
	}
}

void ReadSphere::CalcColourChange()
{
	float multi = 1 / (float)frameCount;
	for (int i = 0; i < sphereAmount; i++)
	{
		Vec3f dif = endColours[i] - spheres[i].surfaceColor;
		dif = dif.operator*(multi);
		colourChange[i] = dif;
	}
}

void ReadSphere::CalcRadiusChange()
{
	float multi = 1 / (float)frameCount;
	for (int i = 0; i < sphereAmount; i++)
	{
		float dif = endRad[i] - spheres[i].radius;
		dif = dif * multi;
		radChange[i] = dif;
	}
}

ReadSphere* JSONReader::LoadJSON(const char* path)
{
	//attempt to read the file, if the file does not exist then output saying so and return null
	std::fstream file(path);
	if (!file.good())
	{
		std::cout << "Unable to load file: " << path << std::endl;
		return nullptr;
	}
	json f;
	file >> f;
	int sphereCount = 0;
	int frameCount = 0;
	//Check if the json file specified in path contains a sphere amount and the frame count, if it doesn't find either then null is returned and an output is given
	if (f.contains("sphereAmount"))
	{
		sphereCount = f["sphereAmount"];
	}
	else
	{
		std::cout << "JSONReader could not find 'sphereAmount' in the JSON File." << std::endl;
		return nullptr;
	}

	if (f.contains("frameCount"))
	{
		frameCount = f["frameCount"];
	}
	else
	{
		std::cout << "JSONReader could not find 'frameCount' in the JSON File." << std::endl;
		return nullptr;
	}

	ReadSphere* sphereInfo = new ReadSphere(sphereCount, frameCount);
	json spheres = f["spheres"];
	for (int i = 0; i < sphereCount; i++)
	{
		json sphere = spheres[i];
		bool failed = false;
		if (sphere.contains("startPos"))
		{
			std::vector<float> start = sphere["startPos"];
			sphereInfo->spheres[i].center = Vec3f(start[0], start[1], start[2]);
		}
		else
			failed = true;

		if (sphere.contains("endPos"))
		{
			std::vector<float> end = sphere["endPos"];
			sphereInfo->endPos[i] = Vec3f(end[0], end[1], end[2]);
		}
		else
			failed = true;

		if (sphere.contains("startRadius"))
		{
			float radius = sphere["startRadius"];
			sphereInfo->spheres[i].radius = radius;
			sphereInfo->spheres[i].radius2 = radius * radius;
		}
		else
			failed = true;

		if (sphere.contains("endRadius"))
		{
			float radius = sphere["endRadius"];
			sphereInfo->endRad[i] = radius;
		}
		else
		{
			sphereInfo->endRad[i] = sphereInfo->spheres[i].radius;
		}

		if (sphere.contains("surfaceColor"))
		{
			std::vector<float> surfaceCol = sphere["surfaceColor"];
			sphereInfo->spheres[i].surfaceColor = Vec3f(surfaceCol[0], surfaceCol[1], surfaceCol[2]);
		}
		else
			failed = true;

		if (sphere.contains("reflection"))
		{
			float reflection = sphere["reflection"];
			sphereInfo->spheres[i].reflection = reflection;
		}
		else
			failed = true;

		if (sphere.contains("transparency"))
		{
			float transparency = sphere["transparency"];
			sphereInfo->spheres[i].transparency = transparency;
		}
		else
			failed = true;

		if (sphere.contains("surfaceColor"))
		{
			std::vector<float> surfaceCol = sphere["surfaceColor"];
			sphereInfo->spheres[i].surfaceColor = Vec3f(surfaceCol[0], surfaceCol[1], surfaceCol[2]);
		}
		else
			failed = true;

		if (sphere.contains("endColour"))
		{
			std::vector<float> endColour = sphere["endColour"];
			sphereInfo->endColours[i] = Vec3f(endColour[0], endColour[1], endColour[2]);
		}
		else
			failed = true;

		if (failed)
		{
			std::cout << "JSONReader: Values missing from file!" << std::endl;
			return nullptr;
		}
	}

	sphereInfo->CalcMovement();
	sphereInfo->CalcColourChange();
	sphereInfo->CalcRadiusChange();
	return sphereInfo;
}