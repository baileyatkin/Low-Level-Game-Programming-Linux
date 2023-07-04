// [header]
// A very basic raytracer example.
// [/header]
// [compile]
// c++ -o raytracer -O3 -Wall raytracer.cpp
// [/compile]
// [ignore]
// Copyright (C) 2012  www.scratchapixel.com
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
// [/ignore]
#include <stdlib.h>
#include <cstdio>
#include <cmath>
#include <fstream>
#include <vector>
#include <iostream>
#include <cassert>
#include <cctype>
// Windows only
#ifdef _WIN32
#include <algorithm>
#include <sstream>
#include <string.h>
#include <thread>
#include <mutex>
#endif

#include "Global.h"
#include "RayTracer.h"
#include "ThreadPool.h"

//[comment]
// In the main function, we will create the scene which is composed of 5 spheres
// and 1 light (which is also a sphere). Then, once the scene description is complete
// we render that scene, by calling the render() function.
//[/comment]
int main(int argc, char **argv)
{
	auto start = std::chrono::high_resolution_clock::now();
	srand(13);
	HeapDirector::CreateDefaultHeap();

	std::mutex* mainMutex = new std::mutex();
	ThreadPool* threadPool = new ThreadPool(20, mainMutex);
	Raytracer* r = new Raytracer("SphereJSON.json", threadPool);

	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	std::cout << "\nTime taken: " << duration.count() << "ms" << std::endl;

	delete(r);
	delete(threadPool);
	delete(mainMutex);

	string userInput = "";
	std::cout << "\nCreate video using the ffmpeg? Y/N: ";
	std::cin >> userInput;
	if (userInput == "Y" || userInput == "y")
	{
		system("ffmpeg -framerate 25 -i output/spheres%d.ppm -vcodec mpeg4 output.mp4 -y");
	}

	return 0;
}

