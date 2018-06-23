#include "CommonGeometry.h"

void Sphere::Intersect(const Ray& ray, IntersectResult& result)
{
	Scalar a = ray.direction * ray.direction;
	Scalar b = 2 * ray.direction * (ray.origin - this->center);
	Vec3 dummy1 = (ray.origin - this->center);
	Scalar c = dummy1*dummy1 - this->radius*this->radius;

	// We now calculate discriminant and reject if no intersection.
	Scalar D = b*b - 4*a*c;
	if(D <= 0)
		return;

	Scalar sD = std::sqrt(D);
	Scalar t = (-b-sD)/(2*a);
	// We check if best intersection is in range (minDistance,distance). We have two
	// possible values (two intersections).
	if(t < IL_MinimumNextIntersectionDistance || t > result.distance)
		t = (-b+sD)/(2*a);
	if(t < IL_MinimumNextIntersectionDistance || t > result.distance)
		return;

	result.distance = t;
	result.material = this->material;
	result.materialData = NULL;
	result.normal = ((ray.origin + t*ray.direction) - this->center).Normal();
}

Vec3 Sphere::Sample(int sampleIndex, int sampleCount, RandomGenerator* generator, Vec3& normal)
{
	Scalar theta = generator->NextUniform()*PI;
	Scalar phi = generator->NextUniform()*2*PI;

	Vec3 p = Vec3(std::sin(theta)* std::cos(phi), std::sin(theta)*std::sin(phi), std::cos(theta)); 
	normal = (p - this->center).Normal();
	return p;
}

/// ------------------------------------------------------------------------------------------------------------
/// Scene intersection
/// ------------------------------------------------------------------------------------------------------------


void Scene::Intersect(const Ray& ray, IntersectResult& result)
{
	for(std::vector<IGeometry*>::iterator i = geometry.begin(); i != geometry.end(); i++)
		(*i)->Intersect(ray, result);
}


/// ------------------------------------------------------------------------------------------------------------
/// Triangle intersection
/// ------------------------------------------------------------------------------------------------------------

bool TriangleMesh::Intersect(const Ray& ray, const Vec3& p1, const Vec3& p2, const Vec3& p3, 
		Vec3& point, Vec3& normal, Scalar minDistance, Scalar maxDistance)
{

	// Construct plane of triangle and distance from ray.
	normal = ((p2-p1)^(p3-p1)).Normal();
	Scalar d = -((ray.origin - p1) * normal) / (ray.direction * normal);
	if(d < minDistance || d > maxDistance)
		return false;
	point = ray.origin + d*ray.direction;

	// Check if point lies in triangle, against all three side
	if(((point-p1)^(p2-p1))*normal > 0) return false;
	if(((point-p2)^(p3-p2))*normal > 0) return false;
	if(((point-p3)^(p1-p3))*normal > 0) return false;

	return true;
}

void TriangleMesh::Intersect(const Ray& ray, IntersectResult& result)
{
	int N = indices.size()/3;

	// Foreach triangle
	Vec3 point, normal;
	for(int i = 0; i < N; i++)
	{
		// Find intersection
		Vec3 p1 = vertices[indices[i*3]], p2 = vertices[indices[i*3+1]], p3 = vertices[indices[i*3+2]];
		if(!Intersect(ray, p1, p2, p3, point, normal, IL_MinimumNextIntersectionDistance, result.distance))
			continue;
		
		// Check against current data, proceed only if this hit is closer.
		Scalar distance2 = (point - ray.origin).Length2();
		if(distance2 > result.distance*result.distance || 
			distance2 < IL_MinimumNextIntersectionDistance*IL_MinimumNextIntersectionDistance)
			continue;

		// Save current best hit.
		result.distance = std::sqrt(distance2);
		result.normal = normal;
		result.materialData = NULL;
		result.material = materials[i];
	}
}

/// ------------------------------------------------------------------------------------------------------------
/// Box intersection
/// ------------------------------------------------------------------------------------------------------------

bool Box::IsPointInBox(const Vec3& point)
{
	Vec3 pm = point - minDim;
	if(pm.x >= -IL_Epsilon && pm.y >= -IL_Epsilon && pm.z >= -IL_Epsilon &&
		point.x <= maxDim.x + IL_Epsilon && point.y <= maxDim.y + IL_Epsilon && point.z <= maxDim.z + IL_Epsilon)
		return true;
	return false;
}

inline bool RayPlane(const Vec3& rayDir, const Vec3& rayOrigin, const Vec3& normal, const Vec3& pointOnPlane, Vec3& point)
{
	Scalar denom = normal * rayDir;
	if(std::abs(denom) < IL_Epsilon)
		return false;

	Scalar t = normal * (pointOnPlane - rayOrigin) / denom;
	if(t < IL_MinimumNextIntersectionDistance)
		return false;

	point = rayOrigin + t * rayDir;
	return true;
}

void Box::Intersect(const Ray& ray, IntersectResult& result)
{
	// Fixme: rotate ray
	
	Vec3 point, normal, bestPoint;
	Scalar bestDistance = std::numeric_limits<Scalar>::max();
	// Bottom 3 rays
	if(RayPlane(ray.direction, ray.origin, Vec3(0,0,-1), minDim, point) && IsPointInBox(point))
	{
		Scalar distance = (point - ray.origin).Length();
		if(distance < bestDistance)
		{
			bestPoint = point;
			bestDistance = distance;
			normal = Vec3(0,0,-1);
		}

	}
	if(RayPlane(ray.direction, ray.origin, Vec3(-1,0,0), minDim, point) && IsPointInBox(point))
	{
		Scalar distance = (point - ray.origin).Length();
		if(distance < bestDistance)
		{
			bestPoint = point;
			bestDistance = distance;
			normal = Vec3(-1,0,0);
		}
	}
	if(RayPlane(ray.direction, ray.origin, Vec3(0,-1,0), minDim, point) && IsPointInBox(point))
	{
		Scalar distance = (point - ray.origin).Length();
		if(distance < bestDistance)
		{
			bestPoint = point;
			bestDistance = distance;
			normal = Vec3(0,-1, 0);
		}
	}
	if(RayPlane(ray.direction, ray.origin, Vec3(0,1,0), maxDim, point) && IsPointInBox(point))
	{
		Scalar distance = (point - ray.origin).Length();
		if(distance < bestDistance)
		{
			bestPoint = point;
			bestDistance = distance;
			normal = Vec3(0,1, 0);
		}
	}

	if(RayPlane(ray.direction, ray.origin, Vec3(1,0,0), maxDim, point) && IsPointInBox(point))
	{
		Scalar distance = (point - ray.origin).Length();
		if(distance < bestDistance)
		{
			bestPoint = point;
			bestDistance = distance;
			normal = Vec3(1, 0, 0);
		}
	}

	if(RayPlane(ray.direction, ray.origin, Vec3(0,0,1), maxDim, point) && IsPointInBox(point))
	{
		Scalar distance = (point - ray.origin).Length();
		if(distance < bestDistance)
		{
			bestPoint = point;
			bestDistance = distance;
			normal = Vec3(0,0,1);
		}
	}

	if(bestDistance == std::numeric_limits<Scalar>::max())
		return;

	result.normal = normal;
	result.materialData = 0;
	result.distance = bestDistance;
	result.material = this->material;
}