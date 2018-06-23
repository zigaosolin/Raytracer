# Raytracer
A very simple C++ raytracer that I did for collega project (many years ago)

I allows any material definition (in code) and ray distribution. Photom mapping with Kd trees is used to speed up the
gathers. Caustics are also simulated.

![Reflect/Refract](https://raw.githubusercontent.com/zigaosolin/Raytracer/master/Result/reflectRefract.bmp)
![Caustic Rendering](https://raw.githubusercontent.com/zigaosolin/Raytracer/master/Result/caustics_scat.bmp)

Note that it does not use any GPU accelaration, nor other raytracing libraries so even though multiple threads are used, 
performance is not really good.
