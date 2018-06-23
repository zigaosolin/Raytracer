#pragma once
#include "Illumination.h"
#include "CommonMediums.h"
#include "PhotonMapping\PhotonMap.h"
#include <list>



// A raytracing renderer. 
class Raytracer : IRenderer
{
	Camera* camera;
	IGeometry* geometry;
	IGeometry* singularLightGeometry;
	std::vector<ISingularLight*> lights;
	PhotonMap* globalMap;
	PhotonMap* causticsMap;

	// Dummy medium.
	IMedium* vacuum;
	
	// Stat info (volatile so access is instant in MT applications)
	volatile int raysTraced;
	volatile int primaryRaysTraced;
	volatile bool isRunning;
public:
	// Maximum number of iterations in depth.
	int maxIterations;
	// Maximum number of "integral" iterations - iteration is considered integral if more than
	// gatherIterationThreeshold samples are needed.
	int maxGatherIterations;
	// Maximum number of ray spawns so it is not treated as secondary ray. Default is 3, so we can account
	// for perfect reflections/refractions that are different for each wavelength.
	int gatherIterationThreeshold;
	// Maximum number of integral (secondary) rays to spawn.
	int secondaryRays;
	// Number of rays per pixel (supersampling, antialiasing).
	int raysPerPixel;
	// The diminished number of secondary rays when, exp(-secondaryRayDecay*secondaryIterationDepth)*secondaryRays
	// are spawned on second/third/... iteration.
	Scalar secondaryRayDecay;
	// When surface is hit, this parameter controls the translatement along the ray (inwards or outwards,
	// depending on the kind of ray - in the surface or reflected) to get away with numerical errors with intersections.
	// Remarks: must be greater than IL_MinimumNextIntersectionDistance, a multiple of 3 usually suffices.
	Scalar hitTranslate;
	// Radiuses for gather operation in second pass from photon map, if maps are present. For caustics map, this
	// represents the scale of features visible.
	Scalar globalPhotonMapGatherRadius;
	Scalar causticsPhotonMapGatherRadius;


	Raytracer()
		: isRunning(false),
		  maxIterations(5), 
		  maxGatherIterations(1),
		  secondaryRays(1000),
		  raysPerPixel(1),
		  hitTranslate(3*IL_MinimumNextIntersectionDistance),
		  secondaryRayDecay(3),
		  gatherIterationThreeshold(3),
		  globalPhotonMapGatherRadius((Scalar)0.4),
		  causticsPhotonMapGatherRadius((Scalar)0.1)
	{
		this->vacuum = new NonInteractMedium(1);
	}

	virtual void Render(Camera* camera, IGeometry* geometry, IGeometry* singularLightGeom, 
		std::vector<ISingularLight*> lights, PhotonMap* globalMap, PhotonMap* causticsMap);
protected:
	// The trace function, with depth of recursion and secondary ray depth of recursion.
	Colour Trace(const Ray& ray, RandomGenerator* generator, int depth, int depth2, std::list<IMedium*>& mediumList);

	// Checks if points are shadowed.
	bool IsShadowed(const Vec3& p1, const Vec3& p2);
};