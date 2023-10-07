#version 410

layout (location = 0) in vec3 skybox;

uniform mat4 vertex_model_to_world;
uniform mat4 vertex_world_to_clip;

out VS_OUT {
    vec3 skybox;
} vs_out;

void main()
{
    vs_out.skybox = skybox;

    gl_Position = vertex_world_to_clip * vertex_model_to_world * vec4(skybox, 1.0);
}
