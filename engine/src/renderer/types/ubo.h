#include <glm/glm.hpp>

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;

    glm::vec3 sun_direction = {0.0f, 0.0f, 0.0f};
    float ambient_intensity = 1.0f;
    glm::vec3 sun_color = {1.0f, 1.0f, 1.0f};
    float sun_intensity = 1.0f;
    glm::vec3 camera_position = {0.0f, 0.0f, 0.0f};
    float shininess = 0.5f;
};
