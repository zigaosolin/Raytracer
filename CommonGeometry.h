#pragma once

#include "Illumination.h"
#include <vector>

// A scene is collection of geometry. If you check against it, you check against all geometry.
class Scene : public IGeometry
{
	std::vector<IGeometry*> geometry;
public:
	Scene() {}
	void AddGeometry(IGeometry* geom) { geometry.push_back(geom); }
	virtual void Intersect(const Ray& ray, IntersectResult& result);
};

// TODO: add scene that organizes geometry in bounding boxes or sth (kd trees etc). For example octree for
// all objects and k-d trees for enviorment (terrain, rooms etc).

// Allowed geometry to be translated/rotated by applying inverse transformation in intersect ray.
class TransformedGeometry : public IGeometry
{
	Vec3 invTranslate;
	IGeometry* transformedGeometry;
	//Mat3x3 invRotate;
public:
};

/// A sphere geometry.
class Sphere : public IGeometry, public ISurfaceSampler
{
	Vec3 center;
	Scalar radius;
	Material* material;
public:
	Sphere(const Vec3& c, Scalar r, Material* material) 
		: center(c), radius(r) { this->material = material; }
	virtual void Intersect(const Ray& ray, IntersectResult& result);

	virtual Vec3 Sample(int sampleIndex, int sampleCount, RandomGenerator* generator, Vec3& normal);

};

// A triangle mesh.
// Data is saved in indexed format. 3 indices define triangle, each triangle has it's own material (can be shared). Size
// of material array is always 1/3 of size of indices array.
class TriangleMesh : public IGeometry
{
	std::vector<Vec3> vertices;
	std::vector<int> indices;
	std::vector<Material*> materials;
public:
	TriangleMesh(int capacity = 0) { if(capacity) 
	{ materials.reserve(capacity); indices.reserve(capacity*3); } }
	virtual void Intersect(const Ray& ray, IntersectResult& result);

	// Mesh constructing methods.
	int AddVertex(const Vec3& p) { vertices.push_back(p); return vertices.size()-1; }
	void AddIndexedTriangle(int id1, int id2, int id3, Material* material) 
	{ 
		indices.push_back(id1); indices.push_back(id2); indices.push_back(id3); 
		materials.push_back(material);
	}
	int AddTriangle(const Vec3& p1, const Vec3& p2, const Vec3& p3, Material* material)
	{
		int idx = AddVertex(p1); AddVertex(p2); AddVertex(p3);
		AddIndexedTriangle(idx, idx+1, idx+2, material);
		return materials.size() - 1;
	}
	void GetTriangle(int idx, Vec3& p1, Vec3& p2, Vec3& p3, Material* &material)
	{
		p1 = vertices[indices[idx*3]];
		p2 = vertices[indices[idx*3+1]];
		p3 = vertices[indices[idx*3+2]];
		material = materials[indices[idx]];
	}

	// A static helper triangle-ray intersection
	static bool Intersect(const Ray& ray, const Vec3& p1, const Vec3& p2, const Vec3& p3, 
		Vec3& point, Vec3& normal, Scalar minDistance, Scalar maxDistance);
	
};

class Box : public IGeometry
{
	Vec3 minDim, maxDim;
	Material* material;
public:
	Box(const Vec3& min, const Vec3& max, Material* mat) : minDim(min), maxDim(max), material(mat) {}
	bool IsPointInBox(const Vec3& point);
	virtual void Intersect(const Ray& ray, IntersectResult& result);
};
