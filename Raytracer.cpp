#include "Raytracer.h"
#include <exception>
#include <iostream>
#include <list>
#include <algorithm>

// These are useful for debugging; for example you can filter just indirect lightning etc.

// Self lightning (default: always)
#define SELF_LIGHTNING_BIT(depth, depth2) (true)
#define SELF_LIGHTNING_MASK(depth,depth2,x) (x)
// Direct lightning from singular sources (default: always)
#define DIRECT_LIGHTNING_BIT(depth, depth2) (true)
#define DIRECT_LIGHTNING_MASK(depth, depth2, x) (x)
// Indirect lightning (hemishphere sampling) (default: always)
#define INDIRECT_LIGHTNING_BIT(depth, depth2) (true)
#define INDIRECT_LIGHTNING_MASK(depth, depth2, x) (x)
// Photonmap caustic map usage (default: always)
#define PHOTONMAP_CAUSTICS_BIT(depth, depth2) (true)
#define PHOTONMAP_CAUSTICS_MASK(depth, depth2, x) (x)
// Photonmap usage (default: for gathering steps, e.g. depth2>0)
#define PHOTONMAP_GLOBAL_BIT(depth, depth2) (depth2>0)
#define PHOTONMAP_GLOBAL_MASK(depth, depth2, x) (x)

void Raytracer::Render(Camera* camera, IGeometry* geometry, IGeometry* singularLightGeometry, 
	std::vector<ISingularLight*> lights, PhotonMap* globalMap, PhotonMap* causticsMap)
{
	if(isRunning)
		throw std::exception("Raytracer already running.");
	isRunning = true;
	
	this->camera = camera;
	this->geometry = geometry;
	this->lights = lights;
	this->causticsMap = causticsMap;
	this->globalMap = globalMap;
	this->singularLightGeometry = singularLightGeometry;
	if(camera == NULL || geometry == NULL)
		throw std::exception("Invalid parameters");

	// Stat data set to zero.
	primaryRaysTraced = 0;
	raysTraced = 0;

	int width = camera->image.GetWidth(), height = camera->image.GetHeight();
	// Cast ray(s) for each pixel (in parallel)
	#pragma omp parallel for
	for(int x = 0; x < width; x++)
	{
		for(int y = 0; y < height; y++)
		{
			// FIXME: better to create generator using time, here we use "deterministic"
			// generator based on pixel id. Each random generator is in it's own thread, so no thread-safety required.
			RandomGenerator random(x*height + y);
			for(int n = 0; n < raysPerPixel; n++)
			{
				Ray ray(camera->position, camera->GetPixelDirection(x, y, raysPerPixel==1?0:&random));
				ray.medium = camera->startingMedium == 0 ? this->vacuum : camera->startingMedium;
				Colour& pixelData = camera->image.GetData()[x + width * y];

				std::list<IMedium*> mediumList;
				pixelData = pixelData + (Trace(ray, &random, 0, 0, mediumList) / (Scalar)raysPerPixel);

				this->primaryRaysTraced++;
			}
			
		}

		#pragma omp critical
		std::cout << (primaryRaysTraced*100)/(width*height*raysPerPixel) << "% processed" << std::endl;
	}


	this->isRunning = false;
}


Colour Raytracer::Trace(const Ray& ray, RandomGenerator* random, int depth, int depth2, std::list<IMedium*>& mediumList)
{
	if(depth >= this->maxIterations)
		return Vec3(0,0,0);

	// First find intersection with closes geometry.
	IntersectResult result;
	geometry->Intersect(ray, result);

	// First depth2=0, we also include "singular light geometry"
	if(depth2 == 0 && this->singularLightGeometry)
		singularLightGeometry->Intersect(ray, result);

	if(result.distance >= std::numeric_limits<Scalar>::max())
		return Vec3(0,0,0); //< Return "sky" radiance

	// Calculate position of intersection.
	
	Vec3 position = result.distance * ray.direction + ray.origin;
	Vec3 cameraDirection = -ray.direction;

	// Check for interaction with medium (scaterring)
	ColourScalar scateringWeight;
	Vec3 scatteringPos, scatteringDir;
	if(ray.medium->SampleScattering(ray.origin, result.normal, position, random, scateringWeight,
		scatteringPos, scatteringDir))
	{
		Ray newRay(scatteringPos, scatteringDir);
		newRay.medium = ray.medium;
		return scateringWeight.CMultiply(Trace(newRay, random, depth+1, depth2, mediumList));
	}


	// Now calculate mediums.
	bool isInsideMedium = cameraDirection * result.normal < 0;
	IMedium* insideMedium, *outsideMedium;
	if(isInsideMedium)
	{
		// FIXME: sometimes due to numerical errors, we ignore those hits (alternative - set to vacuum).
		if(mediumList.size() == 0)
			return Vec3(0,0,0);
		outsideMedium = mediumList.back();
		insideMedium = ray.medium;
	} else {
		outsideMedium = ray.medium;
		insideMedium = result.material->insideMedium;
	}

	// Compute radiance of point.
	Vec3 L(0,0,0);

	// 1) self radiance
	if(SELF_LIGHTNING_BIT(depth, depth2) && result.material->surfaceLight != 0)
		L = SELF_LIGHTNING_MASK(depth, depth2, result.material->surfaceLight->Radiance(position, cameraDirection, result.normal));
	
	// Early exit for non-reflective materials. This is useful for singular lights.
	if(result.material->bsdf == 0)
		return scateringWeight.CMultiply(L);
	SamplingType samplingType = result.material->bsdf->GetSamplingType(cameraDirection, result.normal);
	

	// 2) radiance from singular sources
	if(DIRECT_LIGHTNING_BIT(depth, depth2) && ((samplingType & Singular) != 0))
	{
		for(std::vector<ISingularLight*>::iterator i = lights.begin(); i != lights.end(); i++)
		{
			ISingularLight* light = *i;
			Vec3 towardsLightDirection;

			// We can use radiance at position for point lights (no translate).
			Vec3 Li = light->Radiance(position, towardsLightDirection, geometry);

			// Early exit for shadowed lights (no BRDF execution) && when backfacing lights.
			if(Li.x == 0 && Li.y == 0 && Li.z == 0)
				continue;
			 
			// Weights with cosine.
			Vec3 t = (result.normal * towardsLightDirection)*Li.CMultiply(result.material->bsdf->BSDF(position, result.normal,
				 cameraDirection, towardsLightDirection, result.materialData, insideMedium, outsideMedium));
			L += DIRECT_LIGHTNING_MASK(depth, depth2, t);
		}
	}
	// 3) caustics map lightning
	if(PHOTONMAP_CAUSTICS_BIT(depth, depth2) && this->causticsMap)
	{
		std::vector<Photon*> photons;
		causticsMap->FindInRange(position, this->causticsPhotonMapGatherRadius, photons);

		// We estimate radiance at the point.
		Vec3 Flux(0,0,0);
		for(std::vector<Photon*>::iterator i = photons.begin(); i != photons.end(); i++)
		{
			Photon* p = *i;

			// Cull backfacings.
			if(p->outDirection * result.normal < 0)
				continue;

			Flux += p->power.CMultiply((result.normal * p->outDirection) * result.material->bsdf->BSDF(position, result.normal, p->outDirection, cameraDirection, result.materialData,
				insideMedium, outsideMedium));
		}

		Vec3 t = Flux / (PI*this->causticsPhotonMapGatherRadius*this->causticsPhotonMapGatherRadius);

		L += PHOTONMAP_CAUSTICS_MASK(depth, depth2, t);
	}

	// Calculate number of samples
	int numberOfSamples = std::min(result.material->bsdf->GetMaxNumberOfSamples(cameraDirection, result.normal), 
		(int)(this->secondaryRays * exp(-secondaryRayDecay*depth2)));

	bool isPerfectReflection = numberOfSamples <= this->gatherIterationThreeshold;	//< This will only allow bigger iteration depth.

	// 4a) indirect photon map rendering (it is either this or hemisphere integration), reflection is still handled with
	// normal raytracing
	if(PHOTONMAP_GLOBAL_BIT(depth, depth2) && !isPerfectReflection && this->globalMap)
	{
		std::vector<Photon*> photons;
		globalMap->FindInRange(position, this->globalPhotonMapGatherRadius, photons);

		// We estimate radiance at the point.
		Vec3 Flux(0,0,0);
		for(std::vector<Photon*>::iterator i = photons.begin(); i != photons.end(); i++)
		{
			Photon* p = *i;

			// Cull backfacings.
			if(p->outDirection * result.normal < 0)
				continue;

			Flux += p->power.CMultiply((result.normal * p->outDirection) * result.material->bsdf->BSDF(position, result.normal, p->outDirection, cameraDirection, result.materialData,
				insideMedium, outsideMedium));
		}

		Vec3 t = Flux / (PI*this->globalPhotonMapGatherRadius*this->globalPhotonMapGatherRadius);

		L += PHOTONMAP_GLOBAL_MASK(depth, depth2, t);

		// We skip through
		return scateringWeight.CMultiply(L);
	}


	// 4b) integrated radiance over hemisphere
	if((samplingType & MultipleSample) == 0 || INDIRECT_LIGHTNING_BIT(depth, depth2) == false)
		return scateringWeight.CMultiply(L);

	
	if(depth2 < this->maxGatherIterations || isPerfectReflection)
	{
		// If perfect reflection, we need only one ray to approximate.
		for(int i = 0; i < numberOfSamples; i++)
		{
			Vec3 newDirection;
			ColourScalar S = result.material->bsdf->Sample(i, numberOfSamples, position, result.normal, 
				cameraDirection, random, result.materialData, insideMedium, outsideMedium, newDirection); 

			// Trace new ray.
			Ray newRay(position, newDirection);
			bool needsPop = false, needsPush = false;
			Scalar translateInwards = -1;
			if(isInsideMedium)
			{
				// In-Out combination
				if(newDirection * result.normal > 0)
				{
					newRay.medium = mediumList.back();
					mediumList.pop_back();
					needsPush = true;
					translateInwards = 1;
				}
				// In-In combination
				else
					newRay.medium = ray.medium;
			}
			else {
				// Out-out combination
				if(newDirection * result.normal > 0)
					newRay.medium = ray.medium;
				else
				{
					newRay.medium = result.material->insideMedium;
					mediumList.push_back(ray.medium);
					needsPop = true;
					translateInwards = 1;
				}
			}

			// Translation of ray position so the whole solid angle can be sampled (also in corners)
			newRay.origin = newRay.origin + (translateInwards * this->hitTranslate) * ray.direction;

			ColourScalar newL = Trace(newRay, random, depth+1, depth2 + (isPerfectReflection ? 0 : 1), mediumList);

			// Undo medium list to prev state
			if(needsPush)
				mediumList.push_back(newRay.medium);
			if(needsPop)
				mediumList.pop_back();

			// NaN check (FIXME: must get rid of it and find the source).
			if(newL.x != newL.x || newL.y != newL.y || newL.z != newL.z)
			{
				continue;
			}

			// Add already weighted result to intensity.
			Vec3 t = S.CMultiply(newL);
			L += INDIRECT_LIGHTNING_MASK(depth,depth2,t);
		}
	}

	return scateringWeight.CMultiply(L);

}