#include "Illumination.h"
#pragma once

#include <list>

// A diffuse (Lambertian) material.
class Diffuse : public IBSDF
{
public:
	ColourScalar coefficients;
	// Coefficents are normalized with 1/PI; this means function takes coefficients
	// in range [0,1] (they should be in range [0,1/PI] for energy conservation).
	Diffuse(const ColourScalar& coef) : coefficients(coef/PI) {}

	virtual ColourScalar BSDF(const Vec3& worldPosition, const Vec3& normal, 
            const Vec3& inDirection, const Vec3& outDirection, void* materialData,
			IMedium* insideMedium, IMedium* outsideMedium);

	// Cosine weighted gather implementation
	virtual ColourScalar Sample(int rayIndex, int numberOfSamples, const Vec3& worldPosition, const Vec3& normal, 
		const Vec3& cameraDirection, RandomGenerator* random, void* materialData,
		IMedium* insideMedium, IMedium* outsideMedium, Vec3& genDirection);
};

// A perfect reflection material.
class Reflective : public IBSDF
{
public:
	ColourScalar reflectiness;

	Reflective(const ColourScalar& refl) : reflectiness(refl) {}

	virtual SamplingType GetSamplingType(const Vec3& cameraDir, const Vec3& normal) { return (SamplingType)(MultipleSample|Caustics); }
	virtual int GetMaxNumberOfSamples(const Vec3& cameraDir, const Vec3& normal) { return 1; }
	virtual ColourScalar BSDF(const Vec3& worldPosition, const Vec3& normal, 
            const Vec3& inDirection, const Vec3& outDirection, void* materialData,
			IMedium* insideMedium, IMedium* outsideMedium);
	virtual ColourScalar Sample(int rayIndex, int numberOfSamples, const Vec3& worldPosition, const Vec3& normal, 
		const Vec3& cameraDirection, RandomGenerator* random, void* materialData,
		IMedium* insideMedium, IMedium* outsideMedium, Vec3& genDirection);
};

// A phong material.
class Phong : public IBSDF
{
	ColourScalar coefficients;
	Scalar exponent;
public:
	Phong(const ColourScalar& coef, Scalar exp) : coefficients(coef), exponent(exp) {}

	virtual ColourScalar BSDF(const Vec3& worldPosition, const Vec3& normal, 
            const Vec3& inDirection, const Vec3& outDirection, void* materialData,
			IMedium* insideMedium, IMedium* outsideMedium);
};
