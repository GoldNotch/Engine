#version 450

//layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

/*layout(binding = 0) uniform UniformBufferObject {
    float t;
} ubo;*/

void main() {
    outColor = vec4(1.0, 0.0, 0.0, 1.0);
}