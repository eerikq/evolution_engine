#include <evo/assets/model.h>
#include <evo/core/math.h>
#include <evo/core/print.h>
#include <tiny_obj_loader.h>
#include <vector>

void assets::LoadObj(const char* path, Model* model) {
    if (path == nullptr || model == nullptr) {
        print::Error("Invalid LoadObj() input");
        return;
    }

    tinyobj::attrib_t attributes;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warnings, errors;

    if (!tinyobj::LoadObj(&attributes, &shapes, &materials, &warnings, &errors, path)) {
        print::Error("Failed to import .obj model. {}", errors);
        return;
    }

    usize total_indices = 0;
    for (usize i = 0; i < shapes.size(); i++) {
        total_indices += shapes[i].mesh.indices.size();
    }

    model->positions = new vec3[total_indices];
    model->normals = new vec3[total_indices];
    model->uvs = new vec2[total_indices];
    model->indices = new u32[total_indices];

    model->size += sizeof(model->positions[0]) * total_indices;
    model->size += sizeof(model->normals[0]) * total_indices;
    model->size += sizeof(model->uvs[0]) * total_indices;
    model->size += sizeof(model->indices[0]) * total_indices;

    for (usize i = 0; i < shapes.size(); i++) {
        tinyobj::shape_t* shape = &shapes[i];
        tinyobj::mesh_t* mesh = &shape->mesh;

        for (int j = 0; j < mesh->indices.size(); j++) {
            vec3 position = {
                attributes.vertices[mesh->indices[j].vertex_index * 3],
                attributes.vertices[mesh->indices[j].vertex_index * 3 + 1],
                attributes.vertices[mesh->indices[j].vertex_index * 3 + 2],
            };
            vec3 normal = {
                attributes.normals[mesh->indices[j].normal_index * 3],
                attributes.normals[mesh->indices[j].normal_index * 3 + 1],
                attributes.normals[mesh->indices[j].normal_index * 3 + 2],
            };
            vec2 tex_coord = {
                attributes.texcoords[mesh->indices[j].texcoord_index * 2],
                1.0f - attributes.texcoords[mesh->indices[j].texcoord_index * 2 + 1],
            };

            /* deduplicaiton check ↓
                if (unique_vertices.count(vertex) == 0) {
                    unique_vertices[vertex] = static_cast<uint32_t>(vertex_buffer->size());
                    vertex_buffer->push_back(vertex);
                }

                index_buffer->push_back(unique_vertices[vertex]);
            */

            model->positions[model->vertex_count] = position;
            model->normals[model->vertex_count] = normal;
            model->uvs[model->vertex_count] = tex_coord;
            model->indices[model->index_count] = model->vertex_count;

            model->vertex_count++;
            model->index_count++;
        }
    }
}
void assets::FreeModel(Model* model) {
    if (model == nullptr) return;

    delete[] model->positions;
    delete[] model->normals;
    delete[] model->uvs;
    delete[] model->indices;

    model->positions = nullptr;
    model->normals = nullptr;
    model->uvs = nullptr;
    model->indices = nullptr;
    model->vertex_count = 0;
    model->index_count = 0;
}
