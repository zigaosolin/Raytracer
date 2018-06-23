#pragma once

#include "../Illumination.h"
#include "kdtree.h"

// A photon structure.
// Remarks: may need to store photons in compressed form.
struct Photon
{
	// Position of photon
	Vec3 position;
	// Out direction (-ray direction of this photon)
	Vec3 outDirection;
	// Power per component.
	Vec3 power;
};

// A photon map is efficient data structure for retrieving photons.
class PhotonMap
{
	// Add Kd trees later
	std::vector<Photon*> photonMap;

	// Optimized K-d tree.
	kdtree* kdTree;
public:
	PhotonMap();
	~PhotonMap();

	int Count() { return photonMap.size(); }
	// Optimises the tree: must be called after all photons are added and before any searches are done.
	void Optimise();
	// Adds a photon to map; data will be freed by destructor.
	void AddPhoton(Photon* photon);
	// Finds N nearest photons.
	int FindNNearest(int n, const Vec3& position, Photon** list);
	// Finds photons in range.
	void FindInRange(const Vec3& position, Scalar range, std::vector<Photon*>& list);

};
