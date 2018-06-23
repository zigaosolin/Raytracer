#include "CommonBTDF.h"

/// ------------------------------------------------------------------------------------------------------
/// Refractive
/// ------------------------------------------------------------------------------------------------------

ColourScalar Refractive::BSDF(const Vec3& worldPosition, const Vec3& normal, 
            const Vec3& inDirection, const Vec3& outDirection, void* materialData, 
			IMedium* insideMedium, IMedium* outsideMedium) 
{
	// This is actually delta function, cant check for delta function with numerics.
	return ColourScalar(0,0,0);
}

ColourScalar Refractive::RefractRay(const Vec3& cameraDirection, const Vec3& normal2, Scalar n)
{
	Vec3 normal = normal2;
	// Only one ray will be generated. We need to refract ray.
	Scalar cosa = normal * cameraDirection;
	Scalar index; 
    
	// We check if we are from in-outside
	if(cosa < 0)
	{
		index = -n;
		cosa = -cosa;
		normal = -normal;
	} else {
		index = n;
	}

	// Calculate the other angle from sina = n* sinb with cosa=sqrt(1-sin^2a)
    Scalar cos2b = 1 - (1-cosa*cosa)/(index*index);
    if (cos2b < 0)
    {
        // Reflect ray.
        return -cameraDirection + 2*cosa*normal;
    }

	Scalar cosb = std::sqrt(cos2b);

	// Generate tangent direction.
	Vec3 tangent = (normal ^ cameraDirection) ^ normal;
	tangent.Normalize();

	// We have the other angle, direction is simple from it.
    return -cosb*normal - std::sqrt(1-cos2b)*tangent;
}

ColourScalar Refractive::Sample(int rayIndex, int numberOfSamples, const Vec3& worldPosition, const Vec3& normal, 
		const Vec3& cameraDirection, RandomGenerator* random, void* materialData,
		IMedium* insideMedium, IMedium* outsideMedium, Vec3& genDirection)
{
	genDirection = Refractive::RefractRay(cameraDirection, normal, 
		insideMedium->GetInvariantIndex() / outsideMedium->GetInvariantIndex());

	return transmisive;

}



/// ------------------------------------------------------------------------------------------------------
/// ReflectiveRefractive
/// ------------------------------------------------------------------------------------------------------


ColourScalar ReflectiveRefractive::BSDF(const Vec3& worldPosition, const Vec3& normal, 
            const Vec3& inDirection, const Vec3& outDirection, void* materialData, 
			IMedium* insideMedium, IMedium* outsideMedium)
{
	return ColourScalar(0,0,0);
}

ColourScalar ReflectiveRefractive::Sample(int rayIndex, int numberOfSamples, const Vec3& worldPosition, const Vec3& normal, 
		const Vec3& cameraDirection, RandomGenerator* random, void* materialData,
		IMedium* insideMedium, IMedium* outsideMedium, Vec3& genDirection)
{
	if(rayIndex == 0)
		return reflective.Sample(rayIndex, numberOfSamples, worldPosition, normal, 
			cameraDirection, random, materialData, insideMedium, outsideMedium, genDirection);
	else
		return refractive.Sample(rayIndex, numberOfSamples, worldPosition, normal, 
			cameraDirection, random, materialData, insideMedium, outsideMedium, genDirection);

}

/// ------------------------------------------------------------------------------------------------------
/// TransmitiveDiffuse
/// ------------------------------------------------------------------------------------------------------

ColourScalar TransmitiveDiffuse::BSDF(const Vec3& worldPosition, const Vec3& normal, 
            const Vec3& inDirection, const Vec3& outDirection, void* materialData, 
			IMedium* insideMedium, IMedium* outsideMedium)
{	
	if(inDirection*normal < 0 && outDirection*normal > 0)
	{
		Vec3 refractRay = Refractive::RefractRay(inDirection, normal, insideMedium->GetInvariantIndex() / outsideMedium->GetInvariantIndex());
		return this->diffuse.coefficients / (normal*outDirection) * (refractRay * outDirection);
	};
	return Vec3(0,0,0);
}

ColourScalar TransmitiveDiffuse::Sample(int rayIndex, int numberOfSamples, const Vec3& worldPosition, const Vec3& normal, 
		const Vec3& cameraDirection, RandomGenerator* random, void* materialData,
		IMedium* insideMedium, IMedium* outsideMedium, Vec3& genDirection)
{
	if(cameraDirection*normal < 0)
	{
		// Sample around "new" normal (fixme)
		return diffuse.Sample(rayIndex, numberOfSamples, worldPosition, normal, -cameraDirection, random,
			materialData, insideMedium, outsideMedium, genDirection);
	} else
	{
		return refr.Sample(rayIndex, numberOfSamples, worldPosition, normal, cameraDirection, random,
			materialData, insideMedium, outsideMedium, genDirection);
	}


}