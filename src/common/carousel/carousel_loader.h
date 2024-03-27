#pragma once
#include "carousel.h"
#include "..\path.h"

struct carousel_loader {
	carousel_loader() {}

	static race* & r() {   static race * r; return r; }

	static void push_stick_object(NSVGpath* npath,float h, std::vector<stick_object> & vso) {
		stick_object  so; 
		so.pos = glm::vec3(npath->pts[0], r()->ter().y(npath->pts[0], npath->pts[1]), npath->pts[1]);
		so.height = 2.0;
		vso.push_back(so);
	}

	static void push_cameraman(NSVGpath* npath, float rd, std::vector<cameraman>& vc) {
		cameraman  so(rd);
		so.frame = glm::mat4(1.f);
		so.frame[3] = glm::vec4(npath->pts[0], r()->ter().y(npath->pts[0], npath->pts[1]), npath->pts[1], 1.0);
		vc.push_back(so);
	}

	static void regular_sampling(const NSVGpath * path, double delta, std::vector<glm::vec3>& samples_pos, std::vector<glm::vec3>& samples_tan, float* tot = 0) {
		int ip = 0;
		std::vector<glm::vec3> controlPoints;
		for (int i = 0; i < path->npts; i += 1)
			controlPoints.push_back(glm::vec3(path->pts[i * 2], 0, path->pts[i * 2 + 1]));
		bezier_path::regular_sampling(controlPoints, 0.1f, samples_pos, samples_tan);
	}

	static int load(const char * svgFile, const char* terrain_image,race & r) {
		carousel_loader::r() = &r;

		int sx, sy,comp;
		unsigned char* data = stbi_load( terrain_image, &sx, &sy, &comp, 1);

		r._ter.size_pix[0] = sx;
		r._ter.size_pix[1] = sy;

		r._ter.height_field.resize(sx*sy);
		memcpy_s(&r._ter.height_field[0], sx * sy , data, sx * sy );

		struct NSVGimage* image;
		struct ::NSVGrasterizer* rast = ::nsvgCreateRasterizer();
		image = nsvgParseFromFile(svgFile, "px", 96);
		printf("size: %f x %f\n", image->width, image->height);
		
		r._bbox.add(glm::vec3(0.f, 0.f, 0.f));
		r._bbox.add(glm::vec3(image->width, 0.f, image->height));
		r._ter.rect_xz = glm::vec4(0, 0, image->width, image->height);

		for (NSVGshape* shape = image->shapes; shape != NULL; shape = shape->next) {
			printf("id %s\n", shape->id);

			if (std::string(shape->id).find("tree") != std::string::npos)  
				push_stick_object(shape->paths, 2.f, r._trees);
			else
			if (std::string(shape->id).find("lamp") != std::string::npos) 
				push_stick_object(shape->paths, 2.f, r._lamps);
			else
				if (std::string(shape->id).find("cameraman") != std::string::npos) {
					size_t pos1 = std::string(shape->id).find_first_of("_")+1;
					size_t pos2 = std::string(shape->id).find_last_of("_");
					std::string rad  = std::string(shape->id).substr(pos1 , pos2 - pos1);
					float radius = (float) atof(rad.c_str());
					push_cameraman(shape->paths, 15.f, r._cameramen);
				}
			else
				if (std::string(shape->id).find("track") != std::string::npos) {
					int ip = 0;
					std::vector<glm::vec3> samples_pos, samples_tan;
					regular_sampling(shape->paths, 0.1f, samples_pos, samples_tan);

					for (unsigned int i = 0;i < samples_pos.size();++i) {
						glm::vec3 d =glm::vec3 (-samples_tan[i].z, 0, samples_tan[i].x);
						d = glm::normalize(d);
						r._t.curbs[0].push_back(r._ter.p(samples_pos[i] + d * 2.f));
						r._t.curbs[1].push_back(r._ter.p(samples_pos[i] - d * 2.f));
					}
					
				}
				else
					if (std::string(shape->id).find("carpath") != std::string::npos) {
						int ip = 0;
						std::vector<glm::vec3> samples_pos, samples_tan;
						for (NSVGpath* path = shape->paths; path != NULL; path = path->next) {
							int ip = 0;
							std::vector<glm::vec3> samples_pos, samples_tan;

							float tot_length;
							regular_sampling(shape->paths, 1.f, samples_pos, samples_tan, &tot_length);
							samples_pos.clear();
							samples_tan.clear();

							float delta = tot_length / 60000 * 33.f;
							regular_sampling(shape->paths, delta, samples_pos, samples_tan, &tot_length);

							r.carpaths.push_back(::path());
							for (unsigned int i = 0; i < samples_pos.size(); ++i) {
								r.carpaths.back().frames.push_back(glm::mat4(1.f));
								r.carpaths.back().frames.back()[3] = glm::vec4(r._ter.p(samples_pos[i]), 1.0);
								glm::vec3 tn = glm::normalize(samples_tan[i]);
								glm::vec3  z = glm::normalize(r._ter.p(samples_pos[i] - tn) -  r._ter.p(samples_pos[i]));

								glm::vec3 d = glm::vec3(-samples_tan[i].z, 0, samples_tan[i].x);
								d = glm::normalize(d);
								glm::vec3  x = glm::normalize(r._ter.p(samples_pos[i] + d) - r._ter.p(samples_pos[i]));
								glm::vec3 y = glm::cross(z, x);
								r.carpaths.back().frames.back()[0] = glm::vec4(x, 0); 
								r.carpaths.back().frames.back()[1] = glm::vec4(y, 0);
								r.carpaths.back().frames.back()[2] = glm::vec4(z, 0);

							}

							//
						}

					}
		}

		nsvgDelete(image);
		return 1;
	}
};