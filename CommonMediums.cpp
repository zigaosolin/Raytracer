#include "CommonMediums.h"

bool NonInteractMedium::SampleScattering(const Vec3& p1, const Vec3& n1, const Vec3& p2, RandomGenerator* random,
		ColourScalar& weight, Vec3& positionOfScattering, Vec3& newDirection)
{
	weight.x = 1; weight.y = 1; weight.z = 1;
	return false;
}



/// --------------------------------------------------------------------------------------------------------------------
/// Random scatter medium
/// --------------------------------------------------------------------------------------------------------------------

bool RandomScatterMedium::SampleScattering(const Vec3& p1, const Vec3& n1, const Vec3& p2, RandomGenerator* random,
		ColourScalar& weight, Vec3& positionOfScattering, Vec3& newDirection)
{
	// We create by rejection, generate apropriate x in [0, 5 / lambda] (ignore more).
	Scalar x = std::numeric_limits<Scalar>::max();
	if(scatterLambda != 0)
	{
		// Random scatter position based on exponential curve
		x = -std::log(random->NextUniform()) / scatterLambda;
	}

	// We calculate, if we get a scatter.
	Scalar length = (p2-p1).Length();
	if(x > length)
	{
		weight.x = std::exp(-absorptionCoef.x * length);
		weight.y = std::exp(-absorptionCoef.y * length);
		weight.z = std::exp(-absorptionCoef.z * length);
		return false;
	}

	// We have a scatter, create random new direction
	positionOfScattering = p1 + (p2-p1) * x/length;
	newDirection = random->NextDirection();
	weight.x = std::exp(-absorptionCoef.x * x);
	weight.y = std::exp(-absorptionCoef.y * x);
	weight.z = std::exp(-absorptionCoef.z * x);

	return true;
}

