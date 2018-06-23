#pragma once

#include "Illumination.h"

// Non scaterring, no absorb medium (vacuum, and where scatering/absorption can be neglected).
class NonInteractMedium : public IMedium
{
public:
	NonInteractMedium(Scalar n) : IMedium(Vec3(n,n,n)){}

	virtual bool SampleScattering(const Vec3& p1, const Vec3& n1, const Vec3& p2, RandomGenerator* random,
		ColourScalar& weight, Vec3& positionOfScattering, Vec3& newDirection);


};

// A medium where particles scatter in random direction.
class RandomScatterMedium : public IMedium
{
	Vec3 absorptionCoef;
	Scalar scatterLambda;
public:
	RandomScatterMedium(Scalar n, Scalar scatterFreq, Vec3 absorption) 
		: IMedium(Vec3(n,n,n)), scatterLambda(scatterFreq), absorptionCoef(absorption) {}

	virtual bool SampleScattering(const Vec3& p1, const Vec3& n1, const Vec3& p2, RandomGenerator* random,
		ColourScalar& weight, Vec3& positionOfScattering, Vec3& newDirection);

};