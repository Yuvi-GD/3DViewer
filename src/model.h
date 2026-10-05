#ifndef MODEL_H
#define MODEL_H

#include "cglm/types.h"

#include "third_party/sokol/sokol_gfx.h"
#include <string>

// A struct to hold the generated Sokol buffers for a loaded 3D model
typedef struct {
    sg_bindings bind;
    int index_count;
} mesh_data;

// We will implement this in model.cpp
mesh_data load_obj_mesh(const std::string& filepath);

typedef struct model{
	vec3 pos, rot, scale;
}model;

model model_default();

void model_matrix(model* m, mat4 dest);

#endif // !MODEL_H
