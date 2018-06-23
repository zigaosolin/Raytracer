#include "PhotonMap.h"
#include <algorithm>

PhotonMap::PhotonMap()
{
	kdTree = 0;
}

PhotonMap::~PhotonMap()
{
	kd_free(kdTree);
}

void PhotonMap::AddPhoton(Photon* photon)
{
	photonMap.push_back(photon);
}

void PhotonMap::Optimise()
{
	if(kdTree)
		kd_free(kdTree);
	kdTree = kd_create(3);

	for(std::vector<Photon*>::iterator i = this->photonMap.begin(); i != this->photonMap.end(); i++)
	{
		Photon* p = *i;
		kd_insert3(kdTree, p->position.x, p->position.y, p->position.z, p);
	}
	
}

int PhotonMap::FindNNearest(int n, const Vec3& position, Photon** list)
{
	// TODO:
	return 1;
}

void PhotonMap::FindInRange(const Vec3& position, Scalar range, std::vector<Photon*>& list)
{
	kdres* res = kd_nearest_range3(kdTree, position.x, position.y, position.z, range);
	int size = kd_res_size(res);
	list.reserve(kd_res_size(res));	//< Preallocates appropriate size
	for(int i = 0; i < size; i++)
	{
		list.push_back((Photon*)kd_res_item_data(res));
		kd_res_next(res);
	}

	kd_res_free(res);
}