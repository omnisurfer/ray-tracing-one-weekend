#include <iostream>
#include <fstream>
#include <stdlib.h>

#include "vec3.h"
#include "ray.h"

/* https://github.com/petershirley/raytracinginoneweekend
*/
//"screen" resolution
int nx = 1920, ny = 1080;

bool hit_sphere(const vec3& center, float radius, const ray& r) {
	vec3 oc = r.origin() - center;
	float a = dot(r.direction(), r.direction());
	float b = 2.0 * dot(oc, r.direction());
	float c = dot(oc, oc) - radius * radius;
	float discriminant = b * b - 4 * a*c;
	return (discriminant > 0.0);
}

vec3 color(const ray& r) {

	if (hit_sphere(vec3(0, 0, -1), 1.0, r))
		return vec3(1, 0, 0);

	vec3 unit_direction = unit_vector(r.direction());
	float t = 0.5*(unit_direction.y() + 1.0);
	return (1.0 - t)*vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
}

int main() {	

	std::ofstream outFile;
	
	outFile.open("output.ppm");

	if (outFile.fail()) {
		std::cout << "Failed to open file\n";
		return 1;
	}		

	outFile << "P3\n" << nx << " " << ny << "\n255\n";

	//simple camera setup
	vec3 lower_left_corner(-(nx / 100), -(ny / 100), -1.0);
	vec3 horizontal((2.0*(nx / 100)), 0.0, 0.0);
	vec3 vertical(0.0, (2.0*(ny / 100)), 0.0);
	vec3 origin(0.0, 0.0, 0.0);

	for (int j = ny - 1; j >= 0; j--) {
		for (int i = 0; i < nx; i++) {			
			float u = float(i) / float(nx);
			float v = float(j) / float(ny);

			ray r(origin, lower_left_corner + u * horizontal + v * vertical);
			vec3 col = color(r);

			int ir = int(255.99 * col[0]);
			int ig = int(255.99 * col[1]);
			int ib = int(255.99 * col[2]);
			outFile << ir << " " << ig << " " << ib << "\n";
		}
	}

	std::cout << "Image generation complete, pres any key to continue...\n";
		
	std::cin.get();

	return 0;
}