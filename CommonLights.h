#pragma once
#include "Illumination.h"

// Point light in space, attenuation as square of distance.
class PointLight : public ISingularLight
{
public:
	Vec3		 position;
	ColourScalar intensity;

	PointLight(const Vec3& p, const ColourScalar& inten) : position(p), intensity(inten) {}
	ColourScalar Radiance(const Vec3& surfacePoint, Vec3& towardsLightDirection, IGeometry* geometry);
	Vec3 Sample(int indexOfPhoton, int sampleCount, RandomGenerator* random, Vec3& position, Vec3& direction);
};

// Directional light.
class DirectionalLight : public ISingularLight
{
public:
	Vec3		 direction;
	ColourScalar intensity;

	DirectionalLight(const Vec3& dir, const ColourScalar& inten) : direction(dir), intensity(inten) {}
	ColourScalar Radiance(const Vec3& surfacePoint, Vec3& towardsLightDirection, IGeometry* geometry);
	Vec3 Sample(int indexOfPhoton, int sampleCount, RandomGenerator* random, Vec3& position, Vec3& direction);
};


// Uniform surface light.
class UniformSurfaceLight : public ISurfaceLight
{	
public:
	ISurfaceSampler* sampler;
	Scalar area;
	ColourScalar intensity;

	UniformSurfaceLight(const ColourScalar& inten) 
		: intensity(inten), sampler(0), area(0) {}
	virtual ColourScalar Radiance(const Vec3& surfacePosition, const Vec3& direction, const Vec3& normal);
	Vec3 Sample(int indexOfPhoton, int sampleCount, RandomGenerator* random, Vec3& position, Vec3& direction);
};