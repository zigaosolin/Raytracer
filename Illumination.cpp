#include "Illumination.h"


/// -------------------------------------------------------------------------------------------------------
/// BSDF 
/// -------------------------------------------------------------------------------------------------------

void IBSDF::GenerateTangentBinormal(const Vec3& normal, Vec3& tangent, Vec3& binormal)
{
	Vec3 scale = normal.CMultiply(normal);
	if(scale.x > scale.y)
	{
		if(scale.x > scale.z)
		{
			tangent.z = 0; tangent.y = 1;
			tangent.x = -normal.y/normal.x;
		} else {
			tangent.x = 0; tangent.y = 1;
			tangent.z = -normal.y/normal.z;
		}
	} else {
		if(scale.y > scale.z)
		{
			tangent.x = 0; tangent.z = 1;
			tangent.y = -normal.z/normal.y;
		} else {
			tangent.x = 0; tangent.y = 1;
			tangent.z = -normal.y/normal.z;
		}
	}
	tangent.Normalize();
	binormal = (normal ^ tangent).Normal(); //< FIXME: Normal not needed, probably.
}
	
ColourScalar IBSDF::Sample(int rayIndex, int numberOfSamples, const Vec3& worldPosition, const Vec3& normal, 
		const Vec3& cameraDirection, RandomGenerator* random, void* materialData,
		IMedium* insideMedium, IMedium* outsideMedium, Vec3& genDirection)
{
	// We create normal hemisphere sampling.
	genDirection = random->NextHemisphereDirection(normal);

	// Scaling factor for each ray is 2PI/n, since this is the part of it's "differential" solid angle.
	return this->BSDF(worldPosition, normal, genDirection, cameraDirection, materialData, insideMedium, outsideMedium)
		* ((genDirection * normal) * 2*PI / (Scalar)numberOfSamples);
}

/// -------------------------------------------------------------------------------------------------------
/// Camera
/// -------------------------------------------------------------------------------------------------------
Vec3 Camera::GetPixelDirection(int xx, int yy, RandomGenerator* random)
{
	Scalar x = (Scalar)xx, y = (Scalar)yy;
	if(random)
	{
		x += random->NextUniform()-(Scalar)0.5;
		y += random->NextUniform()-(Scalar)0.5;
	} 
	Scalar dangle = FOV / image.GetWidth();

	//FIXME: add view direction transformation
    return Vec3(-FOV / 2 + x * dangle,
                -FOV / (2 * (Scalar)image.GetWidth()/image.GetHeight()) + y * dangle, -1.0f).Normal();
}


bool Camera::GetPixelFromDirection(const Vec3& direction, int& x, int& y)
{
	return false;
}

/// -------------------------------------------------------------------------------------------------------
/// Random
/// -------------------------------------------------------------------------------------------------------

// Generation with rejection - may change in future.
Vec3 RandomGenerator::NextDirection()
{
	for(;;)
	{
		Vec3 v(NextUniform()*2-1, NextUniform()*2-1, NextUniform()*2-1);
		Scalar l2 = v*v;
		if(l2 <= 1.0)
			return v / (std::sqrt(l2));
	}
}

// Generation with rejection - may change in future.
Vec3 RandomGenerator::NextHemisphereDirection(const Vec3& normal)
{
	for(;;)
	{
		Vec3 v(NextUniform()*2-1, NextUniform()*2-1, NextUniform()*2-1);
		Scalar l2 = v*v;
		if(l2 <= 1.0 && normal*v >= 0)
			return v / (std::sqrt(l2));
	}
}

/// -------------------------------------------------------------------------------------------------------
/// Geometry
/// -------------------------------------------------------------------------------------------------------

bool IGeometry::IsInShadow(const Vec3& p1, const Vec3& p2)
{
	// We calculate in range [minDistance, maxDistance].
	IntersectResult result;
	Scalar maxDistance = (p2-p1).Length() - IL_Epsilon;
	result.distance = maxDistance;

	// Generate ray and check for intersection.
	Ray ray(p1, (p2-p1)/result.distance);
	this->Intersect(ray, result);
	if(result.distance < maxDistance)
		return true;
	return false;
}


/// ----------------------------------------------------------------------------------------------------------
/// Constants
/// ----------------------------------------------------------------------------------------------------------

Scalar IL_Epsilon = (Scalar)1e-5;
Scalar IL_MinimumNextIntersectionDistance = (Scalar)1e-3;
