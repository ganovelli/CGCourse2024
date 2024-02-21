#pragma once
#include <glm/glm.hpp>  
#include <glm/ext.hpp>
#include <vector>
#include <iostream>


struct matrix_stack {
	matrix_stack() { 
		_stack.push_back(glm::mat4(1.0)); 
	}
	void push() {
		_stack.push_back(_stack.back());
	}
	template <bool exit_on_error=true>
	void pop() {
		if (_stack.empty()) {
			std::cout << "Error: pop on a empty stack. Push/pop are unbalanced" << std::endl;
			if(exit_on_error)
				exit(0);
		}
		_stack.pop_back();
	}
	void mult(glm::mat4 m) {
		_stack.back() = _stack.back()*m;
	}
	void load(glm::mat4 m) {
		_stack.back() = m;
	}
	void load_identity() {
		load(glm::mat4(1.0));
	}

	const glm::mat4 & m() { return _stack.back(); }
private:
	std::vector < glm::mat4 > _stack;
};