#pragma once

#include "../Illumination.h"
#include "PhotonMap.h"
#include "../CommonMediums.h"
#include <list>

// Traces photons from different lights and stores them into photon map.
class PhotonTracer 
{
	volatile int photonsTraced;
	volatile int primaryPhotonsTraced;

	PhotonMap* photonMap;
	IGeometry* geometry;
	IMedium* vacuum;

	void Trace(const Ray& ray, const Vec3& initialEnergy, Vec3 energy, RandomGenerator* random, 
		int depth, std::list<IMedium*>& mediumList, bool caustics);

	
public:
	// Maximum number of iterations in depth.
	int maxIterations;
	// Termination on energy criteria; when energy falls under initial_energy * rejectRatio for
	// all components of light.
	Scalar rejectRatio;
	// Numerical position translate to account for full sampling.
	Scalar hitTranslate;

	PhotonTracer();
	~PhotonTracer();

	// Traces only caustics photons. 
	// Remarks: this is not thread safe, my only call one at the time.
	void TraceCausticsPhotons(IGeometry* geometry, 
		int samples, ILight* light, PhotonMap* photonMap);
	void TracePhotons(IGeometry* geometry, 
		int sample, ILight* light, PhotonMap* photonMap, bool caustics=false);
};