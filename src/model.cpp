#include "model.h"
#include "cglm/vec3.h"
#include "cglm/cglm.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "third_party/tiny_obj_loader/tiny_obj_loader.h"
#include <vector>

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

struct vertext {
	float x, y, z;
	float nx, ny, nz;
};

mesh_data load_obj_mesh(const std::string& filepath) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t>shapes;
	std::vector<tinyobj::material_t> material;
	std::string err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &material, &err, filepath.c_str()))
	{
		printf("Failed To Load OBJ %s", filepath.c_str());
		return {0};
	}

	std::vector<vertext> vertices;
	std::vector<uint16_t> indices;

	for (size_t s = 0; s < shapes.size(); s++) {
		for (size_t i = 0; i < shapes[s].mesh.indices.size(); i++)
		{
			tinyobj::index_t index = shapes[s].mesh.indices[i];

			vertext v{};
			v.x = attrib.vertices[3 * size_t(index.vertex_index) + 0];
			v.y = attrib.vertices[3 * size_t(index.vertex_index) + 1];
			v.z = attrib.vertices[3 * size_t(index.vertex_index) + 2];

			if (index.normal_index >= 0)
			{
				v.nx = attrib.normals[3 * size_t(index.normal_index) + 0];
				v.ny = attrib.normals[3 * size_t(index.normal_index) + 1];
				v.nz = attrib.normals[3 * size_t(index.normal_index) + 2];
			}
			else
			{
				v.nx = 0.0f, v.ny = 1.0f, v.nz = 0.0f;
			}

			vertices.push_back(v);
			indices.push_back((uint16_t)indices.size());
		}
	}

	mesh_data mesh = {};
	mesh.index_count = (int)indices.size();

	sg_buffer_desc vbuf_desc = {};
	vbuf_desc.data = { vertices.data(), vertices.size() * sizeof(vertext) };
	mesh.bind.vertex_buffers[0] = sg_make_buffer(&vbuf_desc);

	sg_buffer_desc ibuf_desc = {};
	ibuf_desc.data = { indices.data(), indices.size() * sizeof(uint16_t) };
	ibuf_desc.usage.index_buffer = true;
	mesh.bind.index_buffer = sg_make_buffer(&ibuf_desc);

	return mesh;
};