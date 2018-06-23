#include "CommonLights.h"
#include <cmath>


ColourScalar PointLight::Radiance(const Vec3& surfacePoint, Vec3& towardsLightDirection, IGeometry* geometry)
{
	// Check visibility first.
	if(geometry->IsInShadow(surfacePoint, position))
		return Vec3(0,0,0);

	towardsLightDirection = (this->position - surfacePoint);
	ColourScalar s = (this->intensity) / (towardsLightDirection.Length2());
	towardsLightDirection.Normalize();
	return s;
}

Vec3 PointLight::Sample(int indexOfPhoton, int sampleCount, RandomGenerator* random, Vec3& position, Vec3& direction)
{
	position = this->position;
	direction = random->NextDirection();
	return this->intensity / (Scalar)sampleCount;
}

ColourScalar DirectionalLight::Radiance(const Vec3& surfacePoint, Vec3& towardsLightDirection, IGeometry* geometry)
{
	const Scalar BIG = 100000;
	// Check visibility first.
	if(geometry->IsInShadow(surfacePoint, surfacePoint - direction * BIG))
		return Vec3(0,0,0);

	towardsLightDirection = -direction;
	return intensity;
}

Vec3 DirectionalLight::Sample(int indexOfPhoton, int sampleCount, RandomGenerator* random, Vec3& position, Vec3& direction)
{
	// Unsuported
	throw std::exception("Directional light doesnt support sampling.");
}


ColourScalar UniformSurfaceLight::Radiance(const Vec3& surfacePosition, const Vec3& direction, const Vec3& normal)
{
	return intensity;
}

Vec3 UniformSurfaceLight::Sample(int indexOfPhoton, int sampleCount, RandomGenerator* random, Vec3& position, Vec3& direction)
{
	if(!this->sampler)
		throw std::exception("Not supported, no sampler present");

	Vec3 normal;
	position = sampler->Sample(indexOfPhoton, sampleCount, random, normal);
	direction = random->NextHemisphereDirection(normal);

	// FIXME: not sure if scaling is correct
	return intensity / ((Scalar)sampleCount) * area;
}