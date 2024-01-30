#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>

using namespace std;

/*
Simple class to implement an image saved in PPM format
https://netpbm.sourceforge.net/doc/ppm.html
Few applications load it.
One is IrfanView: https://www.irfanview.com/
*/
struct image {
	image(int _w, int _h) :w(_w), h(_h) { data.resize(w*h * 3); }
	int w, h;

	std::vector<int>  data;

	template <class S>
	void set_pixel(int i, int j, S  r, S  g, S  b) {
		j = h - 1 - j;
		data[(j*w + i) * 3]		= (unsigned char) r;
		data[(j*w + i) * 3 + 1] = (unsigned char) g;
		data[(j*w + i) * 3 + 2] = (unsigned char) b;
	}

	void save(char * filename) {
		ofstream f;
		f.open(filename);
		f << "P3\n";
		f << w << " " << h << std::endl;

		f << *(std::max_element(data.begin(), data.end())) << std::endl;

		for (unsigned int i = 0; i < data.size() / 3; ++i)
			f << data[i * 3] << " " << data[i * 3 + 1] << " " << data[i * 3 + 2] << std::endl;
		f.close();
	}
};

/*
simple struct implementing a 3D point or 3D vector
*/
struct p3 {
	p3() {}
	p3(float x, float y, float z):x(x),y(y),z(z){}
	float dot(p3 o) {
		return x*o.x + y*o.y + z*o.z;
	}
	p3 operator *(float s) { return p3(x*s, y*s, z*s); }
	p3 operator -(p3 o) { return p3(x-o.x, y-o.y, z-o.z); }
	p3 operator +(p3 o) { return p3(x + o.x, y + o.y, z + o.z); }

	float x, y, z;
};

/*
simple struct implementing a 3D ray
*/
struct ray {
	ray(p3 o, p3 d) :orig(o), dir(d) {}
	p3 orig, dir;
};

/*
simple struct implementing a 3D sphere
*/
struct sphere {
	sphere() {}
	sphere(p3 c, float R,p3 color):center(c), radius(R),color(color){}
	p3 center;
	float radius;
	p3 color;
};

/*
a hit_info contains all the useful information that can be computed when
an intersection test between a ray and a geometric entity (just spheres in this case)
*/
struct hit_info {
	hit_info():t(1000) {}
	hit_info(float t, p3 color, p3 normal) :t(t), color(color), normal(normal) {}
	float t;
	p3 color, normal;
};

/*
compute the intersection between a ray and a sphere
*/
hit_info hit_sphere(ray r, sphere s) {
	float A = r.dir.dot(r.dir);
	float B = r.dir.dot(r.orig - s.center)*2.f;
	float C = (r.orig - s.center).dot(r.orig - s.center) - s.radius*s.radius;

	float delta = B * B - 4 * A*C;


	if (delta < 0) // delta <0 means no real solutions -> no intersection
		return  hit_info();

	float t1 = (-B - sqrt(delta)) / (2 * A);
	float t2 = (-B + sqrt(delta)) / (2 * A);

	// compute the closest intersection point
	float t_min = min(t1, t2);

	// t_min<0 means that the first intersection is "before the ray starts"
	if (t_min < 0.f)
		return hit_info();

	// compute the intersection point
	p3 pos = r.orig + r.dir * t_min;

	// compute the normal at the intersection point
	p3 N = pos - s.center;
	N = N * (1.f / sqrt(N.dot(N))); 

	return hit_info(t_min, s.color, N);
}

void main(int args, char ** argv) {

	image a(800,800);

	// create the scene
	sphere s(p3(0, 0, -3), 1.f,p3(0,0,255));
	sphere s1(p3(1, 1, -1.5f), 0.3f,  p3(0, 255, 0));

	std::vector<sphere> scene;
	scene.push_back(s);
	scene.push_back(s1);

	// define the position of the light
	p3 light_pos(2, 2, 0);

	// for each pixel, trace the corresponding ray
	for (int i = 0; i < a.w; ++i)
		for (int j = 0; j < a.h; ++j) {

			/* build the ray 
			   Note: these ray do not actually go through the "center" of the pixel,
			   they go throught the bottome left corner! 
			   Could you fix it?
			*/
			ray r(p3(0, 0, 0), p3(-1 + (i / float(a.w)) * 2, -1 + (j / float(a.h)) * 2, -1));
		
			/*  set a black background color
				Note: try to replace the following line with 
				a.set_pixel(i, j, 0, 127, 127);
				You will see a ugly effect called "shadow acne". 
				We dont see it just because we picked a black blackground...
			*/
			a.set_pixel(i, j, 0, 0, 0);

			hit_info hit_closest;
			for (int is = 0; is < scene.size(); ++is) {
				hit_info hit = hit_sphere(r, scene[is]);
				if (hit.t < hit_closest.t) 
					hit_closest = hit;
			}

			if (hit_closest.t < 1000.f) {// something has been hit

				// compute the intersection point
				p3 p = r.orig + r.dir * hit_closest.t;

				// compute the direction of light
				p3 L = light_pos - p;
				L = L *(1.f/ (sqrt(L.dot(L))));

				// create the shadow ray
				ray shadow_ray(p, L);

				// look if the shadow ray intersect something in the scene
				int iss;
				for (iss = 0; iss < scene.size(); ++iss) {
					hit_info hit = hit_sphere(shadow_ray, scene[iss]);
					
					/*
					 this works only because we know there is no sphere
					 behind the light position. If light_pos was between the spheres
					 the result will be incorrect.
					 The correct test would be hit.t < distance between p and light_pos
					*/
				 	if (hit.t < 1000.f)
				 		break;
				 }

				if (iss == scene.size()) // the light is visible
				{
					float cos_alpha = hit_closest.normal.dot(L);

					p3 c = hit_closest.color * max(0.f,cos_alpha);
					a.set_pixel(i, j,  c.x ,   c.y ,   c.z);
				}
			}
		}

	a.save("rendering.ppm");
}