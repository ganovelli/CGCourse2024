#pragma once
#include <GL/glew.h>
#include <vector>
#include "box3.h"

struct renderable {

	struct element_array {
		element_array():ind(0), mode(0), count(0), itype(GL_UNSIGNED_INT){}
		GLuint ind, mode, count, itype;
	};

	// vertex array object
	GLuint vao;

	// vertex buffer objects
	std::vector<GLuint> vbos;

	// vector of element array (indices)
	std::vector<element_array > elements;

	// primitive type
	unsigned int mode;

	// number of vertices
	unsigned int vn;

	// number of indices
	unsigned int in;

	// bounding box
	box3 bbox;

	// transformation matrix
	glm::mat4 transform;

	void create() {
		glGenVertexArrays(1, &vao);
		transform = glm::mat4(1.f);
	}

	void bind() {
		glBindVertexArray(vao);
	}

	GLuint assign_vertex_attribute(unsigned int va_id, unsigned int count,
		unsigned int attribute_index,
		unsigned int num_components,
		unsigned int TYPE,
		unsigned int stride = 0,
		unsigned int offset = 0) {

		vn = count;

		glBindVertexArray(vao);

		/* create a buffer for the render data in video RAM */
		vbos.push_back(va_id);
		
		glBindBuffer(GL_ARRAY_BUFFER, vbos.back());
		glEnableVertexAttribArray(attribute_index);

		/* specify the data format */
		glVertexAttribPointer(attribute_index, num_components, TYPE, false, stride, (void*)(size_t)offset);

		glBindVertexArray(NULL);
		return vbos.back();
	}


	template <class T>
	GLuint add_vertex_attribute(T* values, unsigned int count,
		unsigned int attribute_index,
		unsigned int num_components,
		unsigned int TYPE,
		unsigned int stride = 0,
		unsigned int offset = 0) {

		vn = count;

		glBindVertexArray(vao);

		/* create a buffer for the render data in video RAM */
		vbos.push_back(0);
		glGenBuffers(1, &vbos.back());

		glBindBuffer(GL_ARRAY_BUFFER, vbos.back());

		/* declare what data in RAM are filling the buffering video RAM */
		glBufferData(GL_ARRAY_BUFFER, sizeof(T) * count, values, GL_STATIC_DRAW);
		glEnableVertexAttribArray(attribute_index);

		/* specify the data format */
		glVertexAttribPointer(attribute_index, num_components, TYPE, false, stride, (void*)(size_t)offset);

		glBindVertexArray(NULL);
		return vbos.back();
	}

	template <class T>
	GLuint add_vertex_attribute(const T* values, unsigned int count,
		unsigned int attribute_index,
		unsigned int num_components,
		unsigned int stride = 0,
		unsigned int offset = 0) { }


	template <class C>
	int type_to_GL() { return -1; };

	template <class IND_TYPE>
	GLuint add_indices(void* indices, unsigned int count, unsigned int mode) {
		elements.push_back(element_array());
		glBindVertexArray(vao);
		glGenBuffers(1, &elements.back().ind);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elements.back().ind);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(IND_TYPE) * count, indices, GL_STATIC_DRAW);
		glBindVertexArray(NULL);
		elements.back().mode = mode;
		elements.back().count = count;
		elements.back().itype = type_to_GL<IND_TYPE>();

		return elements.back().ind;
	};

	/* this function return the first set of indices (if present), that is: elements[0]. 
	*  Often we have just a set of indices so it comes handy. Otherwise access elements[id]
	*  with the set of indices you need
	*/
	element_array operator()() {
		if (!elements.empty()) return elements[0]; else return element_array();}

	template <>
	int type_to_GL<unsigned int>() { return GL_UNSIGNED_INT; }

	template <>
	int type_to_GL<unsigned short>() { return GL_UNSIGNED_SHORT; }

	template <>
	int type_to_GL<unsigned char>() { return GL_UNSIGNED_BYTE; }

	template <>
	GLuint add_vertex_attribute(const float* values, unsigned int count,
		unsigned int attribute_index,
		unsigned int num_components,
		unsigned int stride,
		unsigned int offset) {
		return this->add_vertex_attribute(values, count, attribute_index, num_components, (unsigned int)GL_FLOAT, stride, offset);
	}
};






