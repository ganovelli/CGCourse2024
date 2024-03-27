#pragma once


class view_manipulator {

	/* a bool variable that indicates if we are currently rotating the trackball*/
	bool is_dragged;

	/* starting position of dragging */
	float start_xpos,start_ypos;

	/* euler angles */
	float d_alpha, d_beta;

	/* matrix to transform the scene according to the trackball: rotation only*/
	glm::mat4 rotation_matrix;

public:
	void reset() {
		rotation_matrix = glm::mat4(1.f);
		d_alpha =  d_beta = 0.0;
	}

	void mouse_move(double xpos, double ypos) {
		if (!is_dragged)
			return;

		d_alpha += (float)(xpos - start_xpos) / 1000.f;
		d_beta += (float)(ypos - start_ypos) / 800.f;
		start_xpos = (float)xpos;
		start_ypos = (float)ypos;
		rotation_matrix = glm::rotate(glm::rotate(glm::mat4(1.f), d_alpha, glm::vec3(0, 1, 0)), d_beta, glm::vec3(1, 0, 0));
	}

	void mouse_press(double xpos, double ypos) {
		start_xpos = (float) xpos;
		start_ypos = (float) ypos;
		is_dragged = true;
	}
	void mouse_release() {
		is_dragged = false;
	}

	glm::mat4 matrix() {
		return rotation_matrix;
	}
	glm::mat4 apply_to_view(glm::mat4 view_transformation) {
		glm::mat4 view_frame = inverse(view_transformation);
		glm::mat4 curr_view = view_frame;
		curr_view[3] = glm::vec4(0, 0, 0, 1);
		curr_view = rotation_matrix*curr_view;
		curr_view[3] = view_frame[3];
		curr_view = inverse(curr_view);
		return curr_view;
	}
};
