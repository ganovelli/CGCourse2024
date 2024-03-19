#pragma once
#include <vector>
#include "renderable.h"
#include <glm/ext.hpp>  
#include <glm/gtx/string_cast.hpp>

class shape {
public:
	std::vector<float> positions;
	std::vector<float> colors;
	std::vector<float> normals;
	std::vector<float> texcoords;
	std::vector<float> tangents;

	std::vector<unsigned int> indices_triangles;
	std::vector<unsigned int> indices_edges;

	unsigned int vn, fn;
	glm::vec3 get_pos(unsigned int i) const {
		return glm::vec3(positions[i * 3], positions[i * 3 + 1], positions[i * 3 + 2]);
	}

	void set_pos(unsigned int i, const glm::vec3 &  p) {
		positions[i * 3]	= p.x;
		positions[i * 3+1]	= p.y;
		positions[i * 3+2]	= p.z;
	}

	void set_norm(unsigned int i, const glm::vec3 &  p) {
		normals[i * 3] = p.x;
		normals[i * 3 + 1] = p.y;
		normals[i * 3 + 2] = p.z;
	}

	unsigned int * ind(unsigned int i)  { return &indices_triangles[3 * i]; }

	float cross(glm::vec2 a, glm::vec2 b) {
		return a.x * b.y - a.y * b.x;
	}

	glm::vec3 to_vec3(int i, std::vector<float>& v) {
		return glm::vec3(v[i * 3], v[i * 3 + 1], v[i * 3 + 2]);
	}
	glm::vec2 tcoord(int i) {
		return glm::vec2(texcoords[2 * i], texcoords[2 * i + 1]);
	}
	glm::vec2 compute_tangent_frame(glm::vec2 t0, glm::vec2 t1, glm::vec2 t2) {
		glm::vec2 t10 = t1 - t0;
		glm::vec2 t20 = t2 - t0;

		float area1 = cross(glm::vec2(1, 0), t20);
		float area2 = cross(t10, glm::vec2(1, 0));
		float area = cross(t10, t20);

		return glm::vec2(area1 / area, area2 / area);
	}

public:
	void compute_tangent_space() {
		tangents.resize(positions.size(), 0);
		std::vector<int> n_star;
		n_star.resize(vn, 0);
		glm::vec3	tangent;

		for (unsigned int it = 0; it < fn; ++it) {
			int x_pos = indices_triangles[it * 3] * 3;
			std::vector<glm::vec3> p;
			p.resize(3);
			p[0] = to_vec3(indices_triangles[it * 3], positions);
			p[1] = to_vec3(indices_triangles[it * 3 + 1], positions);
			p[2] = to_vec3(indices_triangles[it * 3 + 2], positions);

			std::vector<glm::vec2> t;
			t.resize(3);

			t[0] = tcoord(indices_triangles[it * 3]);
			t[1] = tcoord(indices_triangles[it * 3 + 1]);
			t[2] = tcoord(indices_triangles[it * 3 + 2]);

			for (int iv = 0; iv < 3; ++iv)
			{
				n_star[indices_triangles[it * 3 + iv]]++;

				glm::vec2 coords = compute_tangent_frame(t[iv], t[(iv + 1) % 3], t[(iv + 2) % 3]);

				glm::vec3 pos10 = (p[(iv + 1) % 3] - p[iv]);
				glm::vec3 pos20 = (p[(iv + 2) % 3] - p[iv]);
				tangent = normalize(coords[0] * pos10 + coords[1] * pos20);

				tangents[3 * indices_triangles[it * 3 + iv]] += tangent[0];
				tangents[3 * indices_triangles[it * 3 + iv] + 1] += tangent[1];
				tangents[3 * indices_triangles[it * 3 + iv] + 2] += tangent[2];
			}
		}

		for (unsigned int iv = 0; iv < vn; ++iv) {
			tangent = to_vec3(iv, tangents);
			tangent /= n_star[iv];
			tangent = glm::normalize(tangent);
			tangents[3 * iv] = tangent[0];
			tangents[3 * iv + 1] = tangent[1];
			tangents[3 * iv + 2] = tangent[2];
		}

	}

	void compute_edges() {
		for (unsigned int i = 0; i < indices_triangles.size() / 3; ++i) {
			indices_edges.push_back(indices_triangles[i * 3]);
			indices_edges.push_back(indices_triangles[i * 3 + 1]);

			indices_edges.push_back(indices_triangles[i * 3 + 1]);
			indices_edges.push_back(indices_triangles[i * 3 + 2]);

			indices_edges.push_back(indices_triangles[i * 3 + 2]);
			indices_edges.push_back(indices_triangles[i * 3]);
		}
	}

	void to_renderable(renderable & r) {
		r.create();
		r.add_vertex_attribute<float>(&positions[0], 3*vn, 0, 3);

		if(!colors.empty())
			r.add_vertex_attribute<float>(&colors[0], 3 * vn, 1, 3);

		if (!normals.empty())
			r.add_vertex_attribute<float>(&normals[0], 3 * vn, 2, 3);

		if (!tangents.empty())
			r.add_vertex_attribute<float>(&tangents[0], 3 * vn, 3, 3);

		if (!texcoords.empty())
			r.add_vertex_attribute<float>(&texcoords[0], 2 * vn, 4, 2);

		if(!indices_triangles.empty())
			r.add_indices<GLuint>(&indices_triangles[0], (unsigned int) indices_triangles.size(), GL_TRIANGLES);

		if (!indices_edges.empty())
			r.add_indices<GLuint>(&indices_edges[0], (unsigned int)indices_edges.size(), GL_LINES);
	}


};

struct shape_maker {


	static void cube(shape & s, float r = 0.5, float g = 0.5, float b = 0.5) {
		// vertices definition
		////////////////////////////////////////////////////////////
		s.positions = {
					   -1.0, -1.0, 1.0,
					   1.0, -1.0, 1.0,
					   -1.0, 1.0, 1.0,
					   1.0, 1.0, 1.0,
					   -1.0, -1.0, -1.0,
					   1.0, -1.0, -1.0,
					   -1.0, 1.0, -1.0,
					   1.0, 1.0, -1.0
		};
		s.colors = {
			r,g,b,
			r,g,b,
			r,g,b,
			r,g,b,
			r,g,b,
			r,g,b,
			r,g,b,
			r,g,b };
		// triangles definition
		////////////////////////////////////////////////////////////

		s.indices_triangles = {
					   0, 1, 2, 2, 1, 3,  // front
					   5, 4, 7, 7, 4, 6,  // back
					   4, 0, 6, 6, 0, 2,  // left
					   1, 5, 3, 3, 5, 7,  // right
					   2, 3, 6, 6, 3, 7,  // top
					   4, 5, 0, 0, 5, 1   // bottom
		};
		s.vn = 8;
		s.fn = 12;
	}

	static renderable cube(float r = 0.5, float g = 0.5, float b = 0.5) {
		shape s;
		cube(s, r, g, b);
		renderable res;
		s.to_renderable(res);
		return res;
	}

	static renderable frame(float scale = 1.f) {
		shape  s;
		// vertices definition
		////////////////////////////////////////////////////////////
		s.positions = {
			0.0,0.0,0.0,
			1.0,0.0,0.0,
			0.0,0.0,0.0,
			0.0,1.0,0.0,
			0.0,0.0,0.0,
			0.0,0.0,1.0
		};
		for (int i = 0; i < 18; ++i)
			s.positions[i] *= scale;

		s.colors = {
			1.0,0.0,0.0,
			1.0,0.0,0.0,
			0.0,1.0,0.0,
			0.0,1.0,0.0,
			0.0,0.0,1.0,
			0.0,0.0,1.0
		};

		// LINES definition
		// non indexed mode, to be drawn by glDrawArras
		s.vn = 6;
		renderable res;
		s.to_renderable(res);
		return res;
	}

	static renderable line(float length = 1.f) {
		shape  s;
		// vertices definition
		////////////////////////////////////////////////////////////
		s.positions = {
			0.0,0.0,0.0,
			0.0,1.0,0.0
		};
		s.positions[4] *= length;

		// LINES definition
		// non indexed mode, to be drawn by glDrawArras
		s.vn = 2;
		renderable res;
		s.to_renderable(res);
		return res;
	}

	static void cylinder(shape & s, int resolution, float r = 0.5, float g = 0.5, float b = 0.5)
	{
		// vertices definition
		////////////////////////////////////////////////////////////

		s.positions.resize(3 * (2 * resolution + 2));

		float radius = 1.0;
		float angle;
		float step = float(6.283185307179586476925286766559) / resolution;

		// lower circle
		int vertexoffset = 0;
		for (int i = 0; i < resolution; i++) {

			angle = -step * i;

			s.positions[vertexoffset] = radius * std::cos(angle);
			s.positions[vertexoffset + 1] = 0.0;
			s.positions[vertexoffset + 2] = radius * std::sin(angle);
			vertexoffset += 3;
		}

		// upper circle
		for (int i = 0; i < resolution; i++) {

			angle = -step * i;

			s.positions[vertexoffset] = radius *  std::cos(angle);
			s.positions[vertexoffset + 1] = 2.0;
			s.positions[vertexoffset + 2] = radius *  std::sin(angle);
			vertexoffset += 3;
		}

		s.positions[vertexoffset] = 0.0;
		s.positions[vertexoffset + 1] = 0.0;
		s.positions[vertexoffset + 2] = 0.0;
		vertexoffset += 3;

		s.positions[vertexoffset] = 0.0;
		s.positions[vertexoffset + 1] = 2.0;
		s.positions[vertexoffset + 2] = 0.0;

		for (int i = 0; i < s.positions.size(); i += 3) {
			s.colors.push_back(r);
			s.colors.push_back(g);
			s.colors.push_back(b);
		}

		// triangles definition
		////////////////////////////////////////////////////////////

		s.indices_triangles.resize(3 * 4 * resolution);

		// lateral surface
		int triangleoffset = 0;
		for (int i = 0; i < resolution; i++)
		{
			s.indices_triangles[triangleoffset] = i;
			s.indices_triangles[triangleoffset + 1] = (i + 1) % resolution;
			s.indices_triangles[triangleoffset + 2] = (i % resolution) + resolution;
			triangleoffset += 3;

			s.indices_triangles[triangleoffset] = (i % resolution) + resolution;
			s.indices_triangles[triangleoffset + 1] = (i + 1) % resolution;
			s.indices_triangles[triangleoffset + 2] = ((i + 1) % resolution) + resolution;
			triangleoffset += 3;
		}

		// bottom of the cylinder
		for (int i = 0; i < resolution; i++)
		{
			s.indices_triangles[triangleoffset] = i;
			s.indices_triangles[triangleoffset + 1] = (i + 1) % resolution;
			s.indices_triangles[triangleoffset + 2] = 2 * resolution;
			triangleoffset += 3;
		}

		// top of the cylinder
		for (int i = 0; i < resolution; i++)
		{
			s.indices_triangles[triangleoffset] = resolution + i;
			s.indices_triangles[triangleoffset + 1] = ((i + 1) % resolution) + resolution;
			s.indices_triangles[triangleoffset + 2] = 2 * resolution + 1;
			triangleoffset += 3;
		}

		s.vn = static_cast<unsigned int> (s.positions.size() / 3);
		s.fn = static_cast<unsigned int> (s.indices_triangles.size() / 3);


	}

	static renderable cylinder(int resolution, float r = 0.5, float g = 0.5, float b = 0.5) {
		shape s;
		cylinder(s, resolution, r, g, b);
		renderable res;
		s.to_renderable(res);
		return res;
	}

	static renderable quad() {
		shape s;
		renderable r;
		s.positions = { -1,-1,0,  1,-1,0, 1,1,0, -1,1,0 };
		s.normals = {0,0,1, 0,0,1, 0,0,1, 0,0,1 };
		s.indices_triangles = { 0,1,2, 0,2,3 };
		s.vn = 4;
		s.fn = 2;
		s.to_renderable(r);
		return r;
	}

	static void rectangle(shape & s, unsigned int nX, unsigned int nY) {

		s.positions.resize(3 * (nX + 1) * (nY + 1));
		s.texcoords.resize(2 * (nX + 1) * (nY + 1));

		for (unsigned int i = 0; i < nX + 1; ++i)
			for (unsigned int j = 0; j < nY + 1; ++j) {
				s.positions[3 * (j * (nX + 1) + i) + 0] = -1.f + 2 * j / float(nY);
				s.positions[3 * (j * (nX + 1) + i) + 1] = 0.f;
				s.positions[3 * (j * (nX + 1) + i) + 2] = -1.f + 2 * i / float(nX);

				s.texcoords[2 * (j * (nX + 1) + i) + 0] = (-1.f + 2 * j / float(nY) / 2.f + 1.f);
				s.texcoords[2 * (j * (nX + 1) + i) + 1] = (float)(1.0 - (-1.f + 2 * i / float(nX) / 2.f + 1.f));
			}
		for (unsigned int i = 0; i < s.positions.size() / 3; ++i) {
			s.normals.push_back(0.0);
			s.normals.push_back(1.0);
			s.normals.push_back(0.0);
		}


		for (unsigned int i = 0; i < nX; ++i)
			for (unsigned int j = 0; j < nY; ++j) {
				s.indices_triangles.push_back(j * (nX + 1) + i);
				s.indices_triangles.push_back(j * (nX + 1) + i + 1);
				s.indices_triangles.push_back((j + 1) * (nX + 1) + i + 1);

				s.indices_triangles.push_back(j * (nX + 1) + i);
				s.indices_triangles.push_back((j + 1) * (nX + 1) + i + 1);
				s.indices_triangles.push_back((j + 1) * (nX + 1) + i);
			}

		s.vn = static_cast<unsigned int> (s.positions.size() / 3);
		s.fn = static_cast<unsigned int> (s.indices_triangles.size() / 3);
	}

	static renderable rectangle(unsigned int nX, unsigned int nY) {
		renderable res;
		shape s;
		rectangle(s, nX, nY);
		s.to_renderable(res);
		return res;
	}


	static int pos(int i, int j, int stacks) {
		return j*(stacks + 1) + i;
	}
	static void torus(shape & s, float in_radius, float out_radius, unsigned int stacks, unsigned int slices) {
		// vertices definition
	////////////////////////////////////////////////////////////
		s.texcoords.resize(2 * ((stacks + 1) * (slices + 1)));
		s.positions.resize(3 * ((stacks + 1) * (slices + 1)));
		s.normals.resize(3 * ((stacks + 1) * (slices + 1)));

		float step_slices = (float)6.283185307179586476925286766559 / slices;
		float step_stacks = (float)6.283185307179586476925286766559 / stacks;

		glm::mat4 R(1.0);
		glm::vec4 p(0.0), nm(0.0);
		for (unsigned int i = 0; i < stacks + 1; ++i) {
			R = glm::rotate(glm::mat4(1.f), step_stacks * i, glm::vec3(0, 1, 0));

			for (unsigned int j = 0; j < slices + 1; ++j) {
				float x = in_radius * cos(j * step_slices);
				float y = in_radius * sin(j * step_slices);
				float z = 0.0;
				float nx = x;

				x += out_radius;

				p = R * glm::vec4(x, y, z, 1.0);

				s.positions[3 * pos(i, j, stacks)] = p[0];
				s.positions[3 * pos(i, j, stacks) + 1] = p[1];
				s.positions[3 * pos(i, j, stacks) + 2] = p[2];

				nm = R * glm::vec4(nx, y, z, 0.0);
				nm = normalize(nm);
				s.normals[3 * pos(i, j, stacks)] = nm[0];
				s.normals[3 * pos(i, j, stacks) + 1] = nm[1];
				s.normals[3 * pos(i, j, stacks) + 2] = nm[2];

				s.texcoords[2 * pos(i, j, stacks)] = i / (1.f * stacks);
				s.texcoords[2 * pos(i, j, stacks) + 1] = j / (1.f * slices);
			}
		}

		// triangles defition
		////////////////////////////////////////////////////////////



		s.indices_triangles.resize((stacks) * (slices) * 2 * 3);
		int n = 0;
		for (unsigned int i = 0; i < stacks; ++i)
			for (unsigned int j = 0; j < slices; ++j) {
				int i1 = (i + 1);//%stacks;
				int j1 = (j + 1);//%slices;

				s.indices_triangles[3 * n] = pos(i, j, stacks);
				s.indices_triangles[3 * n + 1] = pos(i1, j, stacks);
				s.indices_triangles[3 * n + 2] = pos(i1, j1, stacks);
				n++;
				s.indices_triangles[3 * n] = pos(i, j, stacks);
				s.indices_triangles[3 * n + 1] = pos(i1, j1, stacks);
				s.indices_triangles[3 * n + 2] = pos(i, j1, stacks);
				n++;
			}


		s.vn = (unsigned  int)s.positions.size() / 3;
		s.fn = (unsigned  int)s.indices_triangles.size() / 3;
	}

	static void pyramid(shape& s) {

		// Define the vertices of the pyramid
		s.positions = {
			0.0f, 1.0f, 0.0f,    // Top
		   -1.0f, -1.0f, 1.0f,   // Bottom left
			1.0f, -1.0f, 1.0f,   // Bottom right
			1.0f, -1.0f, -1.0f,  // Bottom right
		   -1.0f, -1.0f, -1.0f,  // Bottom left
		};

		// Define the indices for each face
		s.indices_triangles = {
			0, 1, 2,  // Front face
			0, 2, 3,  // Right face
			0, 3, 4,  // Back face
			0, 4, 1,  // Left face
			1, 2, 3,  // Bottom face
			4, 3, 2   // Bottom face
		};
		s.vn = (unsigned  int)s.positions.size() / 3;
		s.fn = (unsigned  int)s.indices_triangles.size() / 3;
	}

	static renderable pyramid() {
		renderable res;
		shape s;
		pyramid(s);
		s.to_renderable(res);
		return res;
	}

	static void icosahedron(shape &s)
	{
		float L = float((glm::sqrt(5.0) + 1.0) / 2.0);
		s.positions = { 0, L, 1,
						0, L,-1,
						0,-L, 1,
						0,-L,-1,
						L, 1, 0 ,
						L,-1, 0 ,
						-L, 1, 0,
						-L,-1, 0,
						1, 0, L ,
						-1, 0, L ,
						 1, 0,-L ,
						 -1, 0,-L };

		s.indices_triangles = {
			  1,0,4   ,  0,1,6  ,  2,3,5   ,  3,2,7  ,
			  4,5,10  ,  5,4,8  ,  6,7,9   ,  7,6,11 ,
			  8,9,2   ,  9,8,0  ,  10,11,1 ,  11,10,3,
			  0,8,4   ,  0,6,9  ,  1,4,10  ,  1,11,6 ,
			  2,5,8   ,  2,9,7  ,  3,10,5  ,  3,7,11
		};

		s.vn = (unsigned  int)s.positions.size() / 3;
		s.fn = (unsigned  int)s.indices_triangles.size() / 3;
	}

	static void sphere(shape& ico, int subdiv) {
		icosahedron(ico);

		for (unsigned int i = 0; i < ico.vn; i++)
		{
			glm::vec3 p = ico.get_pos(i);
			p = glm::normalize(p);
			ico.set_pos(i, p);
		}

		for (int i = 0; i < subdiv; ++i)
		{
			unsigned int cfn = ico.fn;
			for (unsigned int fi = 0; fi < cfn; ++fi)
			{
				glm::vec3 me01 = (ico.get_pos(*ico.ind(fi)) + ico.get_pos(*(ico.ind(fi) + 1))) * 0.5f;
				glm::vec3 me12 = (ico.get_pos(*(ico.ind(fi) + 1)) + ico.get_pos(*(ico.ind(fi) + 2))) * 0.5f;
				glm::vec3 me20 = (ico.get_pos(*(ico.ind(fi) + 2)) + ico.get_pos(*(ico.ind(fi) + 0))) * 0.5f;

				ico.positions.resize(ico.positions.size() + 3 * 3 * 4);
				ico.set_pos(ico.vn + 0, me01);
				ico.set_pos(ico.vn + 1, me12);
				ico.set_pos(ico.vn + 2, me20);

				ico.set_pos(ico.vn + 3, ico.get_pos(*ico.ind(fi)));
				ico.set_pos(ico.vn + 4, me01);
				ico.set_pos(ico.vn + 5, me20);

				ico.set_pos(ico.vn + 6, ico.get_pos(*(ico.ind(fi) + 1)));
				ico.set_pos(ico.vn + 7, me12);
				ico.set_pos(ico.vn + 8, me01);

				ico.set_pos(ico.vn + 9, ico.get_pos(*(ico.ind(fi) + 2)));
				ico.set_pos(ico.vn + 10, me20);
				ico.set_pos(ico.vn + 11, me12);

				for (unsigned int fi = 0; fi < 12; ++fi)
					ico.indices_triangles.push_back(ico.vn + fi);

				ico.vn += 12;
				ico.fn += 4;
			}

			ico.normals.resize(ico.positions.size());
			for (unsigned int i = 0; i < ico.vn; i++)
			{
				glm::vec3 p = ico.get_pos(i);
				p = glm::normalize(p);
				ico.set_pos(i, p);
				ico.set_norm(i, p);
			}

		}


		ico.vn = (unsigned  int)ico.positions.size() / 3;
		ico.fn = (unsigned  int)ico.indices_triangles.size() / 3;
	}

	static renderable sphere(int rec) {
		renderable res;
		shape s;
		sphere(s, rec);
		s.to_renderable(res);
		return res;
	}


	static void cone(shape& s, float radius, float height, int segments) {
		s.positions.clear();
		s.indices_triangles.clear();

		// Create vertices for the base of the cone
		for (int i = 0; i < segments; ++i) {
			float theta = 2.0f * glm::pi<float>() * float(i) / float(segments);
			float x = radius * cos(theta);
			float z = radius * sin(theta);

			s.positions.push_back(x);
			s.positions.push_back(0.0f);
			s.positions.push_back(-z);
		}

		// Add the top vertex of the cone
		s.positions.push_back(0.0f); // x
		s.positions.push_back(height); // y
		s.positions.push_back(0.0f); // z

		// Create indices_triangles for the base of the cone
		for (int i = 0; i < segments - 1; ++i) {
			s.indices_triangles.push_back(i);
			s.indices_triangles.push_back(i + 1);
			s.indices_triangles.push_back(segments);
		}
		s.indices_triangles.push_back(segments - 1);
		s.indices_triangles.push_back(0);
		s.indices_triangles.push_back(segments);

		// Create indices_triangles for the side faces of the cone
		for (int i = 0; i < segments - 1; ++i) {
			s.indices_triangles.push_back(i);
			s.indices_triangles.push_back(i + 1);
			s.indices_triangles.push_back(segments);
		}
		s.indices_triangles.push_back(segments - 1);
		s.indices_triangles.push_back(0);
		s.indices_triangles.push_back(segments);

		s.vn = (unsigned  int)s.positions.size() / 3;
		s.fn = (unsigned  int)s.indices_triangles.size() / 3;
	}

	static renderable cone(float radius, float height, int segments) {
		renderable res;
		shape s;
		cone(s, radius,   height,   segments);
		s.to_renderable(res);
		return res;
	}

	};