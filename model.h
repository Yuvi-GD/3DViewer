#ifndef MODEL_H
#define MODEL_H

#include "cglm/types.h"

typedef struct model{
	vec3 pos, rot, scale;
}model;

model model_default();

void model_matrix(model* m, mat4 dest);

#endif // !MODEL_H
