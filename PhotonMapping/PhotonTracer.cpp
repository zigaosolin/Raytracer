#include "PhotonTracer.h"
#include "../CommonMediums.h"
#include <iostream>

PhotonTracer::PhotonTracer()
: photonsTraced(0), primaryPhotonsTraced(0), rejectRatio(1/(Scalar)100), hitTranslate(3*IL_MinimumNextIntersectionDistance),
maxIterations(10)
{
	vacuum = new NonInteractMedium(1);
}

PhotonTracer::~PhotonTracer()
{
	delete vacuum;
}

void PhotonTracer::TraceCausticsPhotons(IGeometry* geometry, 
	int samples, ILight* light, PhotonMap* photonMap)
{
	TracePhotons(geometry, samples, light, photonMap, true);
}

void PhotonTracer::TracePhotons(IGeometry* geometry, 
	int sample, ILight* light, PhotonMap* photonMap, bool caustics)
{
	std::cout << "Calculating photon map for light ";
	if(caustics) std::cout << "(caustics map)";
	std::cout << "..." << std::endl;

	// FIXME: this is not really random, always the same sequence
	RandomGenerator* random = new RandomGenerator(123);

	// Store in tree.
	this->photonMap = photonMap;
	this->geometry = geometry;
	IMedium* startMedium = light->GetLightMedium();
	if(startMedium == 0) startMedium = vacuum;

	// FIXME: generate per index randoms and watch for AddPhoton to be synced
	// #pragma omp parallel for
	for(int i = 0; i < sample; i++)
	{
		Vec3 photonPos, photonDir;
		Vec3 photonE = light->Sample(i, sample, random, photonPos, photonDir);

		Ray ray(photonPos, photonDir);
		ray.medium = startMedium;

		std::list<IMedium*> mediumList;
		Trace(ray, photonE, photonE, random, 0, mediumList, caustics);
	}

	std::cout << "Done with " << photonMap->Count() << " samples" << std::endl;
}

void PhotonTracer::Trace(const Ray& ray, const Vec3& initialEnergy, Vec3 energy, RandomGenerator* random, 
		int depth, std::list<IMedium*>& mediumList, bool caustics)
{
	// Iteration depth check
	if(depth >= this->maxIterations)
		return;

	// Energy reject check.
	if(energy.x < initialEnergy.x * rejectRatio && energy.y < initialEnergy.y * rejectRatio
		&& energy.z < initialEnergy.z * rejectRatio)
		return;

	// Check for nearest hit; if no hit, photon went to infinity
	IntersectResult result;
	geometry->Intersect(ray, result);
	if(result.distance == std::numeric_limits<Scalar>::max())
		return;

	// Calculate intersect position.
	Vec3 position = ray.origin + ray.direction * result.distance;
	int samplingType = (int)result.material->bsdf->GetSamplingType(-ray.direction, result.normal);


	// 1) Check interraction with medium
	Vec3 scateringWeight;
	Vec3 scatteringPos, scatteringDir;
	if(ray.medium->SampleScattering(ray.origin, result.normal, position, random, scateringWeight,
		scatteringPos, scatteringDir))
	{
		Ray newRay(scatteringPos, scatteringDir);
		newRay.medium = ray.medium;
		Trace(newRay, initialEnergy, scateringWeight.CMultiply(energy), random, depth, mediumList, caustics);
		return;
	}

	energy = energy.CMultiply(scateringWeight);


	// 2) store the photon (if caustics, store it only if there were caustics hits before and we
	// came to non-caustics surface).
	if(!caustics || ((samplingType & Caustics) == 0 && depth > 0))
	{
		Photon* photon = new Photon;
		photon->position = position;
		photon->outDirection = -ray.direction;
		photon->power = energy;

		photonMap->AddPhoton(photon);
	}

	// 3) Photon reflection (only single), for caustics we need caustic surface to proceed.
	if(caustics && (samplingType & Caustics) == 0)
		return;

	// Now calculate mediums.
	bool isInsideMedium = ray.direction * result.normal > 0;
	IMedium* insideMedium, *outsideMedium;
	if(isInsideMedium)
	{
		// FIXME: in rare cases, this will not work since stack is incomplete. Ignore those cases
		if(mediumList.size() == 0)
			return;
		outsideMedium = mediumList.back();
		insideMedium = ray.medium;
	} else {
		outsideMedium = ray.medium;
		insideMedium = result.material->insideMedium;
	}

	// Sample using BRDF
	Vec3 lightDirection = -ray.direction, newDirection;
	ColourScalar scale = result.material->bsdf->Sample(0, 1, position, result.normal, lightDirection, random, result.materialData,
		insideMedium, outsideMedium, newDirection);

	// Create new ray and translate the position of intersection for numerical issues.
	Ray newRay(position, newDirection);

	Scalar translateInwards = -1;
	if(isInsideMedium)
	{
		// In-Out combination
		if(newDirection * result.normal > 0)
		{
			newRay.medium = mediumList.back();
			mediumList.pop_back();
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
			translateInwards = 1;
		}
	}

	// Translation of ray position so the whole solid angle can be sampled (also in corners)
	newRay.origin = newRay.origin + (translateInwards * this->hitTranslate) * ray.direction;
	newRay.direction = newDirection;

	Trace(newRay, initialEnergy, scale.CMultiply(energy), random, depth+1, mediumList, caustics);

}