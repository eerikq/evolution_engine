#include <glm/glm.hpp>

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;

    glm::vec4 sun_direction;
    glm::vec4 sun_color;
};
