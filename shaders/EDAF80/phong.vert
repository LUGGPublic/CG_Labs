#version 410

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 texcoord;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 binormal;

uniform mat4 vertex_model_to_world;
uniform mat4 normal_model_to_world;
uniform mat4 vertex_world_to_clip;

uniform vec3 light_position;
uniform vec3 camera_position;


out VS_OUT {
    vec2 texcoord;
    vec3 normal;
    vec3 light;
    vec3 view;
    mat3 TBN;
} vs_out;

void main()
{
    vs_out.normal = (normal_model_to_world*vec4(normal,0)).xyz;
    vs_out.texcoord = vec2(texcoord.x, texcoord.y);

    vec3 vertexPos = (vertex_model_to_world*vec4(vertex,1)).xyz;
    vs_out.view = camera_position - vertexPos;
    vs_out.light = light_position - vertexPos;

    vs_out.TBN = mat3(tangent,binormal,normal);

    gl_Position = vertex_world_to_clip * vertex_model_to_world * vec4(vertex, 1.0);
}
