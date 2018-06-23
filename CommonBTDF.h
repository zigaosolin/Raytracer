#pragma once

#include "Illumination.h"
#include "CommonBRDF.h"

// Refraction of surface.
// Remarks: this will work only for mediums with invariant refraction index.
class Refractive : public IBSDF
{
	ColourScalar transmisive;
public:
	Refractive(const ColourScalar& t) : transmisive(t) {}

	virtual ColourScalar BSDF(const Vec3& worldPosition, const Vec3& normal, 
            const Vec3& inDirection, const Vec3& outDirection, void* materialData, 
			IMedium* insideMedium, IMedium* outsideMedium);
	virtual SamplingType GetSamplingType(const Vec3& cameraDir, const Vec3& normal) { return (SamplingType)(MultipleSample|Caustics); }
	virtual int GetMaxNumberOfSamples(const Vec3& cameraDir, const Vec3& normal) { return 1; }
	virtual ColourScalar Sample(int rayIndex, int numberOfSamples, const Vec3& worldPosition, const Vec3& normal, 
		const Vec3& cameraDirection, RandomGenerator* random, void* materialData,
		IMedium* insideMedium, IMedium* outsideMedium, Vec3& genDirection);

	static ColourScalar RefractRay(const Vec3& cameraDirection, const Vec3& normal, Scalar n);

};

// Combination of reflection and refraction.
class ReflectiveRefractive : public IBSDF
{
	Refractive refractive;
	Reflective reflective;
public:
	ReflectiveRefractive(const ColourScalar& refl, const ColourScalar& refr) 
		: reflective(refl), refractive(refr) {}

	virtual ColourScalar BSDF(const Vec3& worldPosition, const Vec3& normal, 
            const Vec3& inDirection, const Vec3& outDirection, void* materialData, 
			IMedium* insideMedium, IMedium* outsideMedium);
	virtual SamplingType GetSamplingType(const Vec3& cameraDir, const Vec3& normal) { return (SamplingType)(MultipleSample|Caustics); }
	virtual int GetMaxNumberOfSamples(const Vec3& cameraDir, const Vec3& normal) { return 2; }
	virtual ColourScalar Sample(int rayIndex, int numberOfSamples, const Vec3& worldPosition, const Vec3& normal, 
		const Vec3& cameraDirection, RandomGenerator* random, void* materialData,
		IMedium* insideMedium, IMedium* outsideMedium, Vec3& genDirection);

};

// A material that will acs as refractive on ray enter and as diffuse around  refracted exit ray  on exit.
// Remarks: this material can be used to model subsurface scatering.
class TransmitiveDiffuse : public IBSDF
{
	Diffuse diffuse;
	Refractive refr;
public:
	TransmitiveDiffuse(const Vec3& refl, const Vec3& coef)
		: diffuse(refl), refr(coef) {}

	virtual ColourScalar BSDF(const Vec3& worldPosition, const Vec3& normal, 
            const Vec3& inDirection, const Vec3& outDirection, void* materialData, 
			IMedium* insideMedium, IMedium* outsideMedium);
	virtual ColourScalar Sample(int rayIndex, int numberOfSamples, const Vec3& worldPosition, const Vec3& normal, 
		const Vec3& cameraDirection, RandomGenerator* random, void* materialData,
		IMedium* insideMedium, IMedium* outsideMedium, Vec3& genDirection);
	virtual SamplingType GetSamplingType(const Vec3& cameraDir, const Vec3& normal) 
	{ if(cameraDir*normal > 0) return (SamplingType)(MultipleSample|Caustics); return (SamplingType)(MultipleSample|Singular); }
	virtual int GetMaxNumberOfSamples(const Vec3& cameraDir, const Vec3& normal)
	{ if(cameraDir*normal > 0) return 1; return std::numeric_limits<int>::max(); }

};