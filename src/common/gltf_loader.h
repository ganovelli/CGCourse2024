#pragma once
#include <tinygltf/tiny_gltf.h>


#include "texture.h"
#include "renderable.h"
#include "debugging.h"

struct gltf_loader {

	tinygltf::Model model;
	tinygltf::TinyGLTF loader;

	std::string err;
	std::string warn;

	std::vector<renderable> rs;
	std::vector<GLuint> id_textures;
	int n_vert, n_tri;

	static std::string GetFilePathExtension(const std::string& FileName) {
		if (FileName.find_last_of(".") != std::string::npos)
			return FileName.substr(FileName.find_last_of(".") + 1);
		return "";
	}

	void load(std::string input_filename) {
		std::string ext = GetFilePathExtension(input_filename);

		bool ret = false;
		if (ext.compare("glb") == 0) {
			// assume binary glTF.
			ret =
				loader.LoadBinaryFromFile(&model, &err, &warn, input_filename.c_str());
		}
		else {
			// assume ascii glTF.
			ret = loader.LoadASCIIFromFile(&model, &err, &warn, input_filename.c_str());
		}

		if (!warn.empty()) {
			printf("Warn: %s\n", warn.c_str());
		}

		if (!err.empty()) {
			printf("ERR: %s\n", err.c_str());
		}
		if (!ret) {
			printf("Failed to load .glTF : %s\n", input_filename.c_str());
			exit(-1);
		}
	}

	void visit_node(glm::mat4  currT, int i_node) {

 
			if (model.nodes[i_node].mesh != -1)
			{
			
			tinygltf::Mesh* mesh_ptr = mesh_ptr = &model.meshes[model.nodes[i_node].mesh];

			const std::vector<double> & m = model.nodes[i_node].matrix;
			glm::mat4 transform(1.f);
			if (!m.empty())
				transform = glm::mat4(m[0], m[1], m[2], m[3],
					m[4], m[5], m[6], m[7],
					m[8], m[9], m[10], m[11],
					m[12], m[13], m[14], m[15]);


			tinygltf::Mesh & mesh = *mesh_ptr;
			for (size_t i = 0; i < mesh.primitives.size(); i++) {
				const tinygltf::Primitive& primitive = mesh.primitives[i];

				if (primitive.indices < 0) return;

				rs.push_back(renderable());
				renderable & r = rs.back();
				r.create();
				r.transform = currT*transform;

				std::map<std::string, int>::const_iterator it(primitive.attributes.begin());
				std::map<std::string, int>::const_iterator itEnd(primitive.attributes.end());

				for (; it != itEnd; it++) {
					assert(it->second >= 0);
					const tinygltf::Accessor& accessor = model.accessors[it->second];
					int n_chan = 1;
					if (accessor.type == TINYGLTF_TYPE_SCALAR) {
						n_chan = 1;
					}
					else if (accessor.type == TINYGLTF_TYPE_VEC2) {
						n_chan = 2;
					}
					else if (accessor.type == TINYGLTF_TYPE_VEC3) {
						n_chan = 3;
					}
					else if (accessor.type == TINYGLTF_TYPE_VEC4) {
						n_chan = 4;
					}
					else {
						assert(0);
					}
					// it->first would be "POSITION", "NORMAL", "TEXCOORD_0", ...
					int attr_index = -1;
					if (it->first.compare("POSITION") == 0) attr_index = 0;
					if (it->first.compare("COLOR") == 0)    attr_index = 1;
					if (it->first.compare("NORMAL") == 0)   attr_index = 2;
					if (it->first.compare("TANGENT") == 0)   attr_index = 3;
					if (it->first.find("TEXCOORD_") != std::string::npos) {
						std::string n = it->first.substr(9, 3);
						attr_index = 4+ atoi(n.c_str());
					}

					if (attr_index != -1) {

						// Compute byteStride from Accessor + BufferView combination.
						int byteStride =
							accessor.ByteStride(model.bufferViews[accessor.bufferView]);
						assert(byteStride != -1);

						n_vert = (int) accessor.count;
						int n_comp = accessor.type; // only consider vec2, vec3 and vec4 (TINYGLTF_TYPE_VEC* ) 

						size_t buffer = model.bufferViews[accessor.bufferView].buffer;
						size_t bufferviewOffset = model.bufferViews[accessor.bufferView].byteOffset;

						switch (accessor.componentType) {
							case TINYGLTF_PARAMETER_TYPE_FLOAT: r.add_vertex_attribute<float>((float*)&model.buffers[buffer].data[bufferviewOffset + accessor.byteOffset], n_comp * n_vert, attr_index, n_comp);break;
							case TINYGLTF_PARAMETER_TYPE_BYTE: r.add_vertex_attribute<char>((char*)&model.buffers[buffer].data[bufferviewOffset + accessor.byteOffset], n_comp * n_vert, attr_index, n_comp);break;
							case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: r.add_vertex_attribute<unsigned char>((unsigned char*)&model.buffers[buffer].data[bufferviewOffset + accessor.byteOffset], n_comp * n_vert, attr_index, n_comp);break;
						}
						// if the are the position compute the object bounding box
						if (attr_index == 0) {
							float * v_ptr = (float*)& model.buffers[buffer].data[bufferviewOffset + accessor.byteOffset];
							for (  int iv = 0; iv < n_vert; ++iv)
								r.bbox.add(glm::vec3(*(v_ptr + iv * 3), *(v_ptr + iv * 3 + 1), *(v_ptr + iv * 3 + 2)));
						}
					}
				}

				const tinygltf::Accessor& indexAccessor =
					model.accessors[primitive.indices];

				int mode = -1;
				if (primitive.mode == TINYGLTF_MODE_TRIANGLES) {
					mode = GL_TRIANGLES;
				}
				//else if (primitive.mode == TINYGLTF_MODE_TRIANGLE_STRIP) {
				//    mode = GL_TRIANGLE_STRIP;
				//}
				//else if (primitive.mode == TINYGLTF_MODE_TRIANGLE_FAN) {
				//    mode = GL_TRIANGLE_FAN;
				//}
				//else if (primitive.mode == TINYGLTF_MODE_POINTS) {
				//    mode = GL_POINTS;
				//}
				//else if (primitive.mode == TINYGLTF_MODE_LINE) {
				//    mode = GL_LINES;
				//}
				//else if (primitive.mode == TINYGLTF_MODE_LINE_LOOP) {
				//    mode = GL_LINE_LOOP;
				//}
				else {
					assert(0);
				}

				// Compute byteStride from Accessor + BufferView combination.
				int byteStride =
					indexAccessor.ByteStride(model.bufferViews[indexAccessor.bufferView]);
				assert(byteStride != -1);

				// one long texture, just a stub implementation
				int buffer = model.bufferViews[indexAccessor.bufferView].buffer;
				size_t bufferviewOffset = model.bufferViews[indexAccessor.bufferView].byteOffset;

				size_t n_tri = indexAccessor.count / 3;

				check_gl_errors(__LINE__, __FILE__);
				switch (indexAccessor.componentType) {
				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:     r.add_indices<unsigned char>((unsigned char*)&model.buffers[buffer].data[bufferviewOffset + indexAccessor.byteOffset], (unsigned int)indexAccessor.count, GL_TRIANGLES); break;
				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:    r.add_indices<unsigned short>((unsigned short*)&model.buffers[buffer].data[bufferviewOffset + indexAccessor.byteOffset], (unsigned int) indexAccessor.count, GL_TRIANGLES); break;
				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:      r.add_indices<unsigned int>((unsigned int*)&model.buffers[buffer].data[bufferviewOffset + indexAccessor.byteOffset], (unsigned int) indexAccessor.count, GL_TRIANGLES); break;
				}

				check_gl_errors(__LINE__, __FILE__);

				// setup the material
				tinygltf::Material mat = model.materials[primitive.material];
				int index;
				
				index = mat.pbrMetallicRoughness.baseColorTexture.index;
				r.mater.base_color_texture = (index != -1)?this->id_textures[index]: this->id_textures[0];

				index = mat.normalTexture.index;
				r.mater.normal_texture = (index != -1) ? this->id_textures[index] : -1;

				index = mat.emissiveTexture.index;
				r.mater.emissive_texture = (index != -1) ? this->id_textures[index] : -1;
			}
		}
		if (!model.nodes[i_node].children.empty()) {
			const std::vector<double>& m = model.nodes[i_node].matrix;
			if (!m.empty())
				currT = currT * glm::mat4(m[0], m[1], m[2], m[3],
					m[4], m[5], m[6], m[7],
					m[8], m[9], m[10], m[11],
					m[12], m[13], m[14], m[15]);
			for (unsigned int ic = 0; ic < model.nodes[i_node].children.size(); ++ic)
				visit_node(currT, model.nodes[i_node].children[ic]);
		}
	}

    // take a model and fill the buffers to be passed to the compute shader (for ray tracing)
	bool create_renderable(std::vector<renderable> & _renderable, box3 & bbox) {

		unsigned char* _data_vert[2] = { 0,0 };
		unsigned char * _data = 0;
		tinygltf::Mesh* mesh_ptr = 0;
		assert(model.scenes.size() > 0);

		check_gl_errors(__LINE__, __FILE__);


		// load texture
		for (unsigned int it = 0; it < model.textures.size(); ++it) {
			tinygltf::Texture& texture = model.textures[it];
			tinygltf::Sampler sampler = model.samplers[model.textures[it].sampler];
			tinygltf::Image  image = model.images[model.textures[it].source];



			int gl_format;
			switch (image.component) {
			case 1: gl_format = GL_ALPHA;	break;
			case 3: gl_format = GL_RGB;		break;
			case 4: gl_format = GL_RGBA;	break;
			default: assert(0);
			}

			// decode the image
			tinygltf::BufferView bufferview = model.bufferViews[image.bufferView];

			tinygltf::Buffer buffer = model.buffers[bufferview.buffer];
			unsigned char* v_ptr = &buffer.data[bufferview.byteOffset];

			int x, y;
			int  channels_in_file;
			stbi_uc* data = stbi_load_from_memory(v_ptr, bufferview.byteLength, &x, &y, &channels_in_file, image.component);

//			stbi_write_png("read_texture.png", x, y, 4, data, 0);

			id_textures.push_back(0);
			glGenTextures(1, &id_textures.back());

			glBindTexture(GL_TEXTURE_2D, id_textures.back());
			glTexImage2D(GL_TEXTURE_2D, 0, gl_format, image.width, image.height, 0, gl_format, image.pixel_type, data);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, sampler.wrapS);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, sampler.wrapT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, sampler.magFilter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, sampler.minFilter);

			if (sampler.minFilter == TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST ||
				sampler.minFilter == TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST ||
				sampler.minFilter == TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR ||
				sampler.minFilter == TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR
				)
				glGenerateMipmap(GL_TEXTURE_2D);
		}
		check_gl_errors(__LINE__, __FILE__);


		// just look for the first mesh 
		int scene_to_display = model.defaultScene > -1 ? model.defaultScene : 0;
		const tinygltf::Scene& scene = model.scenes[scene_to_display];

		glm::mat4 currT(1.f);
		visit_node(currT, 0);

		
		for (unsigned int ir = 0; ir < rs.size(); ++ir)
			for (unsigned int ic = 0; ic < 8; ++ic)
				bbox.add(rs[ir].transform*glm::vec4(rs[ir].bbox.p(ic),1.0));
		_renderable = rs;
 		return true;
	}

	void load_to_renderable(std::string input_filename, std::vector<renderable> & _renderable, box3 & bbox) {
		load(input_filename);
		create_renderable(_renderable, bbox);
	}

};