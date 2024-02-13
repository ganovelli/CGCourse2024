#pragma once
#include <vector>
#include "renderable.h"
#include <glm/ext.hpp>  
#include <glm/gtx/string_cast.hpp>

class shape {
public:
	std::vector<float> positions;
	std::vector<float> colors;
	std::vector<unsigned int> indices_triangles;

	unsigned int vn, fn;

	void to_renderable(renderable & r) {
		r.create();
		r.add_vertex_attribute<float>(&positions[0], 3*vn, 0, 3);

		if(!colors.empty())
			r.add_vertex_attribute<float>(&colors[0], 3 * vn, 1, 3);

		if(!indices_triangles.empty())
			r.add_indices<GLuint>(&indices_triangles[0], indices_triangles.size(), GL_TRIANGLES);

	}

};

struct shape_maker {
	 

	 static void cube(shape & s,float r = 0.5, float g = 0.5, float b = 0.5) {
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
			r,g,b		};
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

	 static renderable frame(float scale  = 1.f) {
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

		 static void cylinder(shape & s,int resolution, float r = 0.5, float g = 0.5, float b = 0.5)
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

		 s.vn = static_cast<unsigned int> (s.positions.size()  / 3);
		 s.fn = static_cast<unsigned int> (s.indices_triangles.size() / 3);

		 
	 }

	static renderable cylinder(int resolution, float r = 0.5, float g = 0.5, float b = 0.5) {
			shape s;
			cylinder(s, resolution, r,g,b);
			renderable res;
			s.to_renderable(res);
			return res;
	}

	 static renderable quad(){
		 shape s;
		 renderable r;
		 s.positions = {-1,-1,0,  1,-1,0, 1,1,0, -1,1,0 };
		 s.indices_triangles = { 0,1,2, 0,2,3 };
		 s.vn = 4;
		 s.fn = 2;
		 s.to_renderable(r);
		 return r;
	 }

	 static void rectangle(shape & s, unsigned int nX, unsigned int nY) {
		
		 s.positions.resize(3 * (nX + 1)*(nY + 1));

		 for (unsigned int i = 0; i < nX + 1; ++i)
			 for (unsigned int j = 0; j < nY + 1; ++j){
				 s.positions[3 * (j*(nX + 1) + i) + 0] =-1.f + 2 * j / float(nY);
				 s.positions[3 * (j*(nX + 1) + i) + 1] = 0.f;
				 s.positions[3 * (j*(nX + 1) + i) + 2] =  -1.f + 2 * i / float(nX);
			 }


		 for (unsigned int i = 0; i < nX ; ++i)
			 for (unsigned int j = 0; j < nY  ; ++j) {
					s.indices_triangles.push_back(j    *(nX + 1) + i    );
					s.indices_triangles.push_back(j    *(nX + 1) + i + 1);
					s.indices_triangles.push_back((j+1)*(nX + 1) + i + 1 );

					s.indices_triangles.push_back(j      *(nX + 1) + i);
					s.indices_triangles.push_back((j + 1)*(nX + 1) + i + 1);
					s.indices_triangles.push_back((j + 1)*(nX + 1) + i);
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
	 static void torus(shape & s, float in_radius, float out_radius,unsigned int stacks, unsigned int slices) {
		 // vertices definition
		 ////////////////////////////////////////////////////////////
		 s.positions.resize(3 * ((stacks + 1)*(slices + 1)));

		 float step_slices = (float)6.283185307179586476925286766559 / slices;
		 float step_stacks = (float)6.283185307179586476925286766559 / stacks;

		 glm::mat4 R(1.0);
		 glm::vec4 p(0.0),nm(0.0);
		 for (unsigned int i = 0; i < stacks + 1; ++i) {
			 R=  glm::rotate(glm::mat4(1.f), step_stacks*i, glm::vec3(0, 1, 0));

			 for (unsigned int j = 0; j < slices + 1; ++j) {
				 float x = in_radius*cos(j*step_slices);
				 float y = in_radius*sin(j*step_slices);
				 float z = 0.0;
				 float nx = x;

				 x += out_radius;

				 p = R*glm::vec4(x, y, z, 1.0);

				 s.positions[3 * pos(i, j, stacks)] = p[0];
				 s.positions[3 * pos(i, j, stacks) + 1] = p[1];
				 s.positions[3 * pos(i, j, stacks) + 2] = p[2];
				}
		 }

		 // triangles defition
		 ////////////////////////////////////////////////////////////



		 s.indices_triangles.resize((stacks)*(slices) * 2 * 3);
		 int n = 0;
		 for (unsigned int i = 0; i < stacks; ++i)
			 for (unsigned int j = 0; j < slices; ++j) {
				 int i1 = (i + 1);//%stacks;
				 int j1 = (j + 1);//%slices;

				 s.indices_triangles[3 * n] = pos(i, j,stacks);
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
	};