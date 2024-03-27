#pragma once

#include "..\renderable.h"
#include "carousel.h"



struct game_to_renderable {
	
	static void ct(float* dst, glm::vec3 src) {
		dst[0] = src.x;
		dst[1] = src.y;
		dst[2] = src.z;
	}
	static void to_track(const race & r, renderable& r_t)  {
		
		std::vector<float> buffer_pos;
		buffer_pos.resize(r.t().curbs[0].size() * 2 * 3);
		for (unsigned int i = 0; i < r.t().curbs[0].size();++i) {
			ct(&buffer_pos[(2 * i  ) * 3], r.t().curbs[0][i % (r.t().curbs[0].size())]);
			ct(&buffer_pos[(2 * i+1) * 3], r.t().curbs[1][i % (r.t().curbs[1].size())]);
		}

		r_t.add_vertex_attribute<float>(&buffer_pos[0], static_cast<unsigned int>(buffer_pos.size()), 0, 3);
	}

	static void to_stick_object(const std::vector<stick_object>& vec, renderable& r_t) {

		std::vector<float> buffer_pos;
		buffer_pos.resize((vec.size()*2) * 3 );
		for (unsigned int i = 0; i < vec.size();++i) {
			ct(&buffer_pos[(2 * i) * 3], vec[i].pos);
			ct(&buffer_pos[(2 * i+1) * 3], vec[i].pos+glm::vec3(0, vec[i].height,0));
		}

		r_t.add_vertex_attribute<float>(&buffer_pos[0], static_cast<unsigned int>(buffer_pos.size()), 0, 3);
	}

	static void to_tree(const race& r, renderable& r_t) {
		to_stick_object(r.trees(), r_t);
	}
	static void to_lamps(const race& r, renderable& r_t) {
		to_stick_object(r.lamps(), r_t);
	}




	static void to_heightfield(const race& r, renderable& r_hf) {
		std::vector<unsigned int > buffer_id;
		const unsigned int& Z =static_cast<unsigned int>(r.ter().size_pix[1]);
		const unsigned int& X =static_cast<unsigned int>(r.ter().size_pix[0]);

		terrain ter = r.ter();

		std::vector<float>   hf3d;
		for (unsigned int iz = 0; iz < Z; ++iz)
			for (unsigned int ix = 0; ix < X; ++ix) {
				hf3d.push_back(ter.rect_xz[0] + (ix / float(X)) * ter.rect_xz[2]);
				hf3d.push_back(r.ter().hf(ix, iz));
				hf3d.push_back(ter.rect_xz[1] + (iz / float(Z)) * ter.rect_xz[3]);
			}

		for (unsigned int iz = 0; iz < Z-1; ++iz)
			for (unsigned int ix = 0; ix < X-1; ++ix) {
				
				buffer_id.push_back((iz * Z) + ix);
				buffer_id.push_back((iz * Z) + ix + 1);
				buffer_id.push_back((iz + 1) * Z + ix + 1);

				buffer_id.push_back((iz * Z) + ix);
				buffer_id.push_back((iz + 1) * Z + ix + 1);
				buffer_id.push_back((iz + 1) * Z + ix);
			}

		r_hf.add_vertex_attribute<float>(&hf3d[0], X * Z * 3, 0, 3);
		r_hf.add_indices<unsigned int>(&buffer_id[0], static_cast<unsigned int>(buffer_id.size()), GL_TRIANGLES);
	}

};