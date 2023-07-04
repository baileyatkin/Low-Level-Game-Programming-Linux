#include "Raytracer.h"
#include <sstream>
#include <chrono>

Raytracer::Raytracer(ThreadPool* threads)
{
	invWidth = 1 / float(width);
	invHeight = 1 / float(height);
	aspectratio = width / float(height);
	angle = tan(M_PI * 0.5 * fov / 180.0);
	threadPool = threads;

	json = JSONReader::LoadJSON("animation.json");
	if (json != nullptr)
		JSONRenderThreaded();
}

Raytracer::Raytracer(const char* jsonpath, ThreadPool* threads)
{
	invWidth = 1 / float(width);
	invHeight = 1 / float(height);
	aspectratio = width / float(height);
	angle = tan(M_PI * 0.5 * fov / 180.0);
	threadPool = threads;

	json = JSONReader::LoadJSON(jsonpath);
	if (json != nullptr)
		JSONRenderThreaded();
}

Raytracer::~Raytracer()
{
	delete(json);
}

float Raytracer::mix(const float& a, const float& b, const float& mix)
{
	return b * mix + a * (1 - mix);
}

// This is the main trace function. It takes a ray as argument (defined by its origin
// and direction). We test if this ray intersects any of the geometry in the scene.
// If the ray intersects an object, we compute the intersection point, the normal
// at the intersection point, and shade this point using this information.
// Shading depends on the surface property (is it transparent, reflective, diffuse).
// The function returns a color for the ray. If the ray intersects an object that
// is the color of the object at the intersection point, otherwise it returns
// the background color.
Vec3f Raytracer::Trace(const Vec3f& rayorig, const Vec3f& raydir, const std::vector<Sphere>& spheres, const int& depth)
{
	//if (raydir.length() != 1) std::cerr << "Error " << raydir << std::endl;
	float tnear = INFINITY;
	const Sphere* sphere = NULL;
	// find intersection of this ray with the sphere in the scene
	for (unsigned i = 0; i < spheres.size(); ++i) {
		float t0 = INFINITY, t1 = INFINITY;
		if (spheres[i].intersect(rayorig, raydir, t0, t1)) {
			if (t0 < 0) t0 = t1;
			if (t0 < tnear) {
				tnear = t0;
				sphere = &spheres[i];
			}
		}
	}
	// if there's no intersection return black or background color
	if (!sphere) return Vec3f(2);
	Vec3f surfaceColor = 0; // color of the ray/surfaceof the object intersected by the ray
	Vec3f phit = rayorig + raydir * tnear; // point of intersection
	Vec3f nhit = phit - sphere->center; // normal at the intersection point
	nhit.normalize(); // normalize normal direction
	// If the normal and the view direction are not opposite to each other
	// reverse the normal direction. That also means we are inside the sphere so set
	// the inside bool to true. Finally reverse the sign of IdotN which we want
	// positive.
	float bias = 1e-4; // add some bias to the point from which we will be tracing
	bool inside = false;
	if (raydir.dot(nhit) > 0) nhit = -nhit, inside = true;
	if ((sphere->transparency > 0 || sphere->reflection > 0) && depth < MAX_RAY_DEPTH)
	{
		float facingratio = -raydir.dot(nhit);
		// change the mix value to tweak the effect
		float fresneleffect = mix(pow(1 - facingratio, 3), 1, 0.1);
		// compute reflection direction (not need to normalize because all vectors
		// are already normalized)
		Vec3f refldir = raydir - nhit * 2 * raydir.dot(nhit);
		refldir.normalize();
		Vec3f reflection = Trace(phit + nhit * bias, refldir, spheres, depth + 1);
		Vec3f refraction = 0;
		// if the sphere is also transparent compute refraction ray (transmission)
		if (sphere->transparency) {
			float ior = 1.1, eta = (inside) ? ior : 1 / ior; // are we inside or outside the surface?
			float cosi = -nhit.dot(raydir);
			float k = 1 - eta * eta * (1 - cosi * cosi);
			Vec3f refrdir = raydir * eta + nhit * (eta * cosi - sqrt(k));
			refrdir.normalize();
			refraction = Trace(phit - nhit * bias, refrdir, spheres, depth + 1);
		}
		// the result is a mix of reflection and refraction (if the sphere is transparent)
		surfaceColor = (
			reflection * fresneleffect +
			refraction * (1 - fresneleffect) * sphere->transparency) * sphere->surfaceColor;
	}
	else {
		// it's a diffuse object, no need to raytrace any further
		for (unsigned i = 0; i < spheres.size(); ++i) {
			if (spheres[i].emissionColor.x > 0) {
				// this is a light
				Vec3f transmission = 1;
				Vec3f lightDirection = spheres[i].center - phit;
				lightDirection.normalize();
				for (unsigned j = 0; j < spheres.size(); ++j) {
					if (i != j) {
						float t0, t1;
						if (spheres[j].intersect(phit + nhit * bias, lightDirection, t0, t1)) {
							transmission = 0;
							break;
						}
					}
				}
				surfaceColor += sphere->surfaceColor * transmission *
					std::max(float(0), nhit.dot(lightDirection)) * spheres[i].emissionColor;
			}
		}
	}

	return surfaceColor + sphere->emissionColor;
}

// Main rendering function. We compute a camera ray for each pixel of the image
// trace it and return a color. If the ray hits a sphere, we return the color of the
// sphere at the intersection point, else we return the background color.
void Raytracer::Render(const std::vector<Sphere>& spheres, int iteration)
{
	Vec3f* image = new Vec3f[size];
	Vec3f* pixel = image;


#ifdef _WIN32
	threadPool->ReleaseLock();
#else
	if (LINUX_POOLING)
		threadPool->ReleaseLock();
#endif // !_WIN32


	// Trace rays
	for (unsigned y = 0; y < height; ++y)
	{
		for (unsigned x = 0; x < width; ++x, ++pixel)
		{
			float xx = (2 * ((x + 0.5) * invWidth) - 1) * angle * aspectratio;
			float yy = (1 - 2 * ((y + 0.5) * invHeight)) * angle;
			Vec3f raydir(xx, yy, -1);
			raydir.normalize();
			*pixel = Trace(Vec3f(0), raydir, spheres, 0);
		}
	}

	auto start = std::chrono::high_resolution_clock::now();

	// Save result to a PPM image (keep these flags if you compile under Windows)
	string fileName = "output/spheres" + std::to_string(iteration) + ".ppm";
	std::ofstream ofs(fileName, std::ios::out | std::ios::binary);

	string line = "P6\n" + std::to_string(width) + " " + std::to_string(height) + "\n255\n";
	ofs.write(line.c_str(), line.length());
	//ofs << "P6\n" << width << " " << height << "\n255\n";
	char* charArr = new char[size * 3];
	int index = 0;
	for (unsigned i = 0; i < size; ++i, index += 3)
	{
		charArr[index] = (unsigned char)(std::min(1.0f, image[i].x) * 255);
		charArr[index + 1] = (unsigned char)(std::min(1.0f, image[i].y) * 255);
		charArr[index + 2] = (unsigned char)(std::min(1.0f, image[i].z) * 255);
	}
	ofs.write(charArr, size * 3);
	/*for (unsigned i = 0; i < size; ++i)
	{
		ofs << (unsigned char)(std::min(1.0f, image[i].x) * 255) <<
		(unsigned char)(std::min(1.0f, image[i].y) * 255) <<
		(unsigned char)(std::min(1.0f, image[i].z) * 255);
	}*/

	ofs.close();
	delete[] image;

	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	static long avgTime = 0;
	static int count = 0;
	avgTime += duration.count();
	count++;
	std::stringstream msg;
	msg << "Spheres" << iteration << ".ppm has been rendered and saved : \\Average time: " << avgTime / count << "ms\n";
	std::cout << msg.str();
}

void Raytracer::JSONRender(int iteration)
{
	std::vector<Sphere> spheresVec = std::vector<Sphere>();
	for (int j = 0; j < json->sphereAmount; j++)
	{
		spheresVec.push_back(json->spheres[j]);
		json->spheres[j].center += json->movement[j];
		json->spheres[j].surfaceColor += json->colourChange[j];
		json->spheres[j].radius += json->radChange[j];
		json->spheres[j].radius2 = json->spheres[j].radius * json->spheres[j].radius;
		/*if(j == 1)
		{
			std::stringstream m;
			m << "Centre: " << json->movement[j] << ", SurfaceColour: " << json->colourChange[j] << ", Radius: "<< json->radiusChange[j] << std::endl;
			std::cout << m.str();
		}*/
	}
	threadPool->Enqueue([this, iteration, spheresVec]
		{
			Render(spheresVec, iteration);
		});
	//Render(spheresVec, iteration);
	//spheresVec.clear();
	//std::stringstream msg;
	//msg << "Rendered and saved spheres" << iteration << ".ppm\n";
	//std::cout << msg.str();
}

void Raytracer::JSONRenderThreaded()
{
	for (int i = 0; i < json->frameCount; i++)
	{
		JSONRender(i);
		//threadPool->Enqueue([this, i] { JSONRender(i); });
	}
	threadPool->WaitUntilCompleted();
}