#include "model.h"
#include "cglm/vec3.h"
#include "cglm/cglm.h"

model model_default() {
	model m;
	glm_vec3_zero(m.pos);
	glm_vec3_zero(m.rot);
	glm_vec3_one(m.scale);
	return m;
}

void model_matrix(model* m, mat4 dest) {

	glm_mat4_identity(dest);
	glm_translate(dest, m->pos);
	glm_rotate_x(dest, m->rot[0], dest);
	glm_rotate_y(dest, m->rot[1], dest);
	glm_rotate_z(dest, m->rot[2], dest);
	glm_scale(dest, m->scale);
}