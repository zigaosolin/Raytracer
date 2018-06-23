#include "CommonBRDF.h"


/// ------------------------------------------------------------------------------------------
/// Diffuse
/// ------------------------------------------------------------------------------------------

ColourScalar Diffuse::BSDF(const Vec3& worldPosition, const Vec3& normal, 
            const Vec3& inDirection, const Vec3& outDirection, void* materialData,
			IMedium* insideMedium, IMedium* outsideMedium)
{
	Scalar s1 = normal * inDirection, s2 = normal * outDirection;
	if(s1*s2 < 0)
		return Vec3(0,0,0);
	return this->coefficients;
}

ColourScalar Diffuse::Sample(int rayIndex, int numberOfSamples, const Vec3& worldPosition, const Vec3& normal, 
		const Vec3& cameraDirection, RandomGenerator* random, void* materialData,
		IMedium* insideMedium, IMedium* outsideMedium, Vec3& genDirection)
{
	// From MLT script, page 49. We generate phi and theta from uniform randoms.
	Scalar phi = 2*PI*random->NextUniform();
	Scalar theta = std::asin(std::sqrt(random->NextUniform()));

	// We must generate direction, easiest using rotated coordinate system [tangent,binormal,normal]
	Vec3 tangent, binormal;
	IBSDF::GenerateTangentBinormal(normal, tangent, binormal);
	genDirection = normal * std::cos(theta) + tangent * (std::sin(theta)*std::cos(phi)) 
		+ binormal * (std::sin(theta)*std::sin(phi));

	// Scale is PI / n (not 2PI/n, because of cos sampling).
	return this->coefficients * (PI / (Scalar)numberOfSamples);
}

/// ------------------------------------------------------------------------------------------
/// Reflection
/// ------------------------------------------------------------------------------------------

ColourScalar Reflective::BSDF(const Vec3& worldPosition, const Vec3& normal, 
            const Vec3& inDirection, const Vec3& outDirection, void* materialData,
			IMedium* insideMedium, IMedium* outsideMedium)
{
	// Actually a delta function, but cant numerically create delta function.
	return Vec3(0,0,0);
}


ColourScalar Reflective::Sample(int rayIndex, int numberOfSamples, const Vec3& worldPosition, const Vec3& normal, 
		const Vec3& cameraDirection, RandomGenerator* random, void* materialData,
		IMedium* insideMedium, IMedium* outsideMedium, Vec3& genDirection)
{
	genDirection = -cameraDirection + 2*(normal*cameraDirection)*normal;
	return this->reflectiness;
}

/// ------------------------------------------------------------------------------------------
/// Phong
/// ------------------------------------------------------------------------------------------
ColourScalar Phong::BSDF(const Vec3& worldPosition, const Vec3& normal, 
            const Vec3& inDirection, const Vec3& outDirection, void* materialData,
			IMedium* insideMedium, IMedium* outsideMedium)
{
	// We calculate reflected vector.
	Vec3 R = inDirection - 2 * (inDirection * normal) * normal;

	// Scaling factor is defined as cos(angle) between total reflection and actual, on exponent.
	Scalar f = std::pow(R * outDirection, exponent);
	return coefficients * f;
}


