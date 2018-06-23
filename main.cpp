#include "Raytracer.h"
#include "CommonBRDF.h"
#include "CommonGeometry.h"
#include "CommonLights.h"
#include "CommonBTDF.h"
#include "CommonMediums.h"
#include "Image.h"
#include "PhotonMapping\PhotonMap.h"
#include "PhotonMapping\PhotonTracer.h"


// Creates a cornell box (5 walls).
void CreateCornellBox(Scene* scene)
{
	Material* whiteDiffuse = new Material(new Diffuse(Vec3(1,1,1)));
	Material* redDiffuse = new Material(new Diffuse(Vec3(1,0,0)));
	Material* greenDiffuse = new Material(new Diffuse(Vec3(0,1,0)));

	
	TriangleMesh* mesh = new TriangleMesh;
	mesh->AddVertex(Vec3(-1,-1,1));  mesh->AddVertex(Vec3(1,-1,1));  mesh->AddVertex(Vec3(1,1,1));  mesh->AddVertex(Vec3(-1,1,1));
	mesh->AddVertex(Vec3(-1,-1,-1)); mesh->AddVertex(Vec3(1,-1,-1)); mesh->AddVertex(Vec3(1,1,-1)); mesh->AddVertex(Vec3(-1,1,-1));
	
	// RIGHT
	mesh->AddIndexedTriangle(1, 6, 5, greenDiffuse);
	mesh->AddIndexedTriangle(1, 2, 6, greenDiffuse);
	// LEFT
	mesh->AddIndexedTriangle(0, 7, 3, redDiffuse);
	mesh->AddIndexedTriangle(0, 4, 7, redDiffuse);
	// BOTTOM
	mesh->AddIndexedTriangle(0, 1, 5, whiteDiffuse);
	mesh->AddIndexedTriangle(0, 5, 4, whiteDiffuse);
	//TOP
	mesh->AddIndexedTriangle(3, 6, 2, whiteDiffuse);
	mesh->AddIndexedTriangle(3, 7, 6, whiteDiffuse);
	//BACK
	mesh->AddIndexedTriangle(5, 6, 7, whiteDiffuse);
	mesh->AddIndexedTriangle(5, 7, 4, whiteDiffuse);

	scene->AddGeometry(mesh);
}

// Tests surface light with distributed raytracing.
// only LR*DR*E paths (tweek maxGather for more D paths)
void Test_SurfaceLight(const char* filename)
{
	Scene scene;
	CreateCornellBox(&scene);

	// Create 2 spheres, one light one normal.
	Material mat1(new Diffuse(Vec3(1,0,0)));
	Material mat2(new Diffuse(Vec3(1,1,1)));
	mat2.surfaceLight = new UniformSurfaceLight(Vec3(1,1,1));
	Sphere sphere1(Vec3(0.5, 0.5, -0.5), 0.3, &mat1);
	Sphere sphere2(Vec3(-0.5, 0.5, -0.5), 0.1, &mat2);
	scene.AddGeometry(&sphere1);
	scene.AddGeometry(&sphere2);

	std::vector<ISingularLight*> lights;

	// Raytrace scene.
	Camera camera(300,300, PI/3);
	camera.position = Vec3(0,0,2.5);

	Raytracer raytracer;
	raytracer.maxGatherIterations = 1; // Change this to account for more diffuse reflections (this affects performance A LOT)
	raytracer.secondaryRays = 500;	   // Change to smaller value to raytrace faster (more noise)
	raytracer.raysPerPixel = 3;
	raytracer.Render(&camera, &scene, 0, lights, 0, 0);
	
	// Use eye response transform (sqrt function) from intensity to response transform.
	Image& image = camera.image;
	image.EyeResponseTransform1();
	image.Multiply(1/image.Max());
	image.SaveAsBmp(filename);

}

// Tests the correctness of reflections/refractions. No secondary reflections
void Test_ReflectRefract(const char* filename)
{
	Scene scene;
	CreateCornellBox(&scene);

	// We add 2 spheres.
	Material mat1(new Reflective(Vec3(1,1,1)));
	Material mat2(new Refractive(Vec3(1,1,1)));
	mat2.insideMedium = new NonInteractMedium(1.4); //< Glass index of refraction

	Sphere sphere1(Vec3(0.5, -0.6, -0.0), 0.35, &mat1);
	Sphere sphere2(Vec3(-0.5, -0.6, -0.0), 0.35, &mat2);
	scene.AddGeometry(&sphere1);
	scene.AddGeometry(&sphere2);

	// We add single light source.
	// FIXME: errors for Vec3(0.2, 0.3, -0.3), fix that!
	std::vector<ISingularLight*> lights;
	PointLight light(Vec3(0.2, 0.3, 1.1), Vec3(1,1,1));
	lights.push_back(&light);

		// Raytrace scene.
	Camera camera(300,300, PI/3);
	camera.position = Vec3(0,0,2.5);

	Raytracer raytracer;
	raytracer.maxGatherIterations = 1; 
	raytracer.secondaryRays = 100;	   
	raytracer.raysPerPixel = 10;
	raytracer.Render(&camera, &scene, 0, lights, 0, 0);
	
	// Use eye response transform (sqrt function) from intensity to response transform.
	Image& image = camera.image;
	image.EyeResponseTransform1();
	image.Multiply(1/image.Max());
	image.SaveAsBmp(filename);
}

// Tests caustics of light
void Test_Caustics(const char* filename, Scalar index, Scalar scateringLen, const Vec3& absorption)
{
	Scene scene;
	CreateCornellBox(&scene);

	// We add 2 spheres.
	Material mat1(new Refractive(Vec3(1,1,1)));
	mat1.insideMedium = new RandomScatterMedium(index, scateringLen, absorption);


	Sphere sphere1(Vec3(-0.5, -0.3, 0), 0.35, &mat1);
	scene.AddGeometry(&sphere1);

	// We add single light source.
	std::vector<ISingularLight*> lights;
	PointLight light(Vec3(0.2, 0.3, -0.5), Vec3(1,1,1));
	lights.push_back(&light);

	// We add "singular light geometry"
	Material singMat(0);
	singMat.surfaceLight = new UniformSurfaceLight(Vec3(1,1,1)*PI); // Brightness scaled a bit
	Sphere lightGeom(light.position, 0.05, &singMat);

	// Calculate caustics map
	PhotonMap causticsMap;
	PhotonTracer tracer;
	tracer.rejectRatio = 0.05;

	// Trace is uniform, only caustics photons are stored.
	tracer.TraceCausticsPhotons(&scene, 5000000, &light, &causticsMap);
	causticsMap.Optimise();

		// Raytrace scene.
	Camera camera(300,300, PI/3);
	camera.position = Vec3(0,0,2.5);

	Raytracer raytracer;
	raytracer.maxGatherIterations = 1; 
	raytracer.secondaryRays = 100;	   
	raytracer.raysPerPixel = 10;
	raytracer.causticsPhotonMapGatherRadius = 0.01; //< Feature size
	raytracer.Render(&camera, &scene, &lightGeom, lights, 0, &causticsMap);
	
	// Use eye response transform (sqrt function) from intensity to response transform.
	Image& image = camera.image;
	image.EyeResponseTransform1();
	image.Multiply(1/image.Max());
	image.SaveAsBmp(filename);
}

// Tests photon mapping
void Test_PhotonMapping(const char* filename)
{
	Scene scene;
	CreateCornellBox(&scene);

	// We add 2 spheres.
	Material mat1(new Refractive(Vec3(1,1,1)));
	Material mat2(new Diffuse(Vec3(0,0,1)));
	mat1.insideMedium = new NonInteractMedium(1.4); //< Glass index of refraction


	Sphere sphere1(Vec3(-0.5, -0.3, 0), 0.35, &mat1);
	scene.AddGeometry(&sphere1);
	Sphere sphere2(Vec3(0.5, -0.6, -0.2), 0.2, &mat2);
	scene.AddGeometry(&sphere2);

	// We add single light source.
	std::vector<ISingularLight*> lights;
	PointLight light(Vec3(0.2, 0.3, -0.5), Vec3(1,1,1));
	lights.push_back(&light);

	// We add "singular light geometry" (so we know where light is)
	Material singMat(0);
	singMat.surfaceLight = new UniformSurfaceLight(Vec3(1,1,1)*PI); // Brightness scaled a bit
	Sphere lightGeom(light.position, 0.05, &singMat);

	// Calculate caustics and photon map. Since caustics map is used for direct rendering,
	// more samples of light source are needed.
	PhotonMap causticsMap;
	PhotonMap globalMap;
	PhotonTracer tracer;
	tracer.rejectRatio = 0.05;
	tracer.TraceCausticsPhotons(&scene, 1000000, &light, &causticsMap);
	tracer.TracePhotons(&scene, 3000, &light, &globalMap);
	causticsMap.Optimise();
	globalMap.Optimise();

		// Raytrace scene.
	Camera camera(300,300, PI/3);
	camera.position = Vec3(0,0,2.5);

	Raytracer raytracer;
	raytracer.maxGatherIterations = 1; 
	raytracer.secondaryRays = 50;	   
	raytracer.raysPerPixel = 10;
	raytracer.globalPhotonMapGatherRadius = 0.05;
	raytracer.causticsPhotonMapGatherRadius = 0.01; //< Feature size
	raytracer.Render(&camera, &scene, &lightGeom, lights, &globalMap, &causticsMap);
	
	// Use eye response transform (sqrt function) from intensity to response transform.
	Image& image = camera.image;
	//image.EyeResponseTransform1();
	image.Multiply(1/image.Max());
	image.SaveAsBmp(filename);
}

// Tests subsruface scatering; big sphere, point light behind.
// FIXME: not working
void Test_SubsurfaceScatering(const char* filename)
{
	Scene scene;
	CreateCornellBox(&scene);

	// We add one big sphere
	Material mat1(new TransmitiveDiffuse(Vec3(1,1,1), Vec3(1,1,1)));
	mat1.insideMedium = new RandomScatterMedium(1.0, 0, Vec3(1,1,1)*3); 


	Sphere sphere1(Vec3(0, -0.2, 0), 0.5, &mat1);
	scene.AddGeometry(&sphere1);


	// We add single light source.
	std::vector<ISingularLight*> lights;
	PointLight light(Vec3(0,0,-0.9), Vec3(1,1,1));
	lights.push_back(&light);

	// We add "singular light geometry" (so we know where light is)
	Material singMat(0);
	singMat.surfaceLight = new UniformSurfaceLight(Vec3(1,1,1)*PI); // Brightness scaled a bit
	Sphere lightGeom(light.position, 0.05, &singMat);


		// Raytrace scene.
	Camera camera(300,300, PI/3);
	camera.position = Vec3(0,0,2.5);

	Raytracer raytracer;
	raytracer.maxGatherIterations = 0; 
	raytracer.secondaryRays = 50;	   
	raytracer.raysPerPixel = 1;
	raytracer.Render(&camera, &scene, &lightGeom, lights, 0, 0);
	
	// Use eye response transform (sqrt function) from intensity to response transform.
	Image& image = camera.image;
	image.EyeResponseTransform1();
	image.Multiply(1/image.Max());
	image.SaveAsBmp(filename);
}






int main()
{
	Test_PhotonMapping("pm.bmp");
	
	return 0;
}