#pragma once

void _update_fps_counter(GLFWwindow *window)
{
	static double prev_sec = 0.0;
	static int frame_cnt = 0;
	double current_sec = glfwGetTime();
	double elapsed_sec = current_sec - prev_sec;

	++frame_cnt;

	if (elapsed_sec > 0.5) {
		prev_sec = current_sec;
		char tmp[128];
		double fps = (double)frame_cnt / elapsed_sec;
		sprintf(tmp, "opengl @ fps: %.2f", fps);
		glfwSetWindowTitle(window, tmp);
		frame_cnt = 0;
	}
}


bool prgmCompl(GLuint shader_index) {
	int params = -1;
	glGetShaderiv(shader_index, GL_COMPILE_STATUS, &params);
	if (GL_TRUE != params) {
		fprintf(stderr, "ERROR: GL shader index %i did not compile\n", shader_index);
		_print_shader_info_log(shader_index);
		return false;
	}
	return true;
}


bool prgmLink(GLuint program_index) {
	int params = -1;
	glGetProgramiv(program_index, GL_LINK_STATUS, &params);
	if (GL_TRUE != params) {
		fprintf(stderr, "ERROR: could not link shader program GL index %u\n", program_index);
		_print_program_info_log(program_index);
		return false;
	}
	return true;
}
