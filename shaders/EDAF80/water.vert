#version 410

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 textcoord;

uniform mat4 vertex_model_to_world;
uniform mat4 normal_model_to_world;
uniform mat4 vertex_world_to_clip;

uniform vec3 camera_position;
uniform float t;
uniform vec3 camera_postion;

out VS_OUT {
    vec3 vertex;
    vec3 normal;
    vec3 textcoord;
    vec3 view;
    mat3 TBN;
    vec2 normalCoord0;
    vec2 normalCoord1;
    vec2 normalCoord2;
} vs_out;


vec3 waves(vec2 position, vec2 direction, float amplitude, float frequency, float phase, float sharpness, float time)
{
 float dp = position.x * direction.x + position.y * direction.y;
 float alpha = sin(dp * frequency + time * phase) * 0.5 + 0.5;

 float wave = amplitude * pow(alpha, sharpness);
 float d_wave = 0.5f * sharpness * frequency * amplitude * pow(alpha, sharpness-1) * cos(dp* frequency + time* phase);
 
 return vec3(wave, d_wave * direction.x, d_wave * direction.y);
}

void main()
{
    vec3 wave1 = waves(vertex.xz, vec2(-1.0, 0.0), 1.0, 0.2, 0.5, 2.0, t);
    vec3 wave2 = waves(vertex.xz, vec2(-0.7, 0.7), 0.5, 0.4, 1.3, 2.0, t);

    vec2 texScale = vec2(8,4);
    float normalTime = mod(t, 100);
    vec2 normalSpeed = vec2(-0.05,0);
    vs_out.normalCoord0.xy = textcoord.xy * texScale + normalTime * normalSpeed;
    vs_out.normalCoord1.xy = textcoord.xy * texScale * 2 + normalTime * normalSpeed * 4;
    vs_out.normalCoord2.xy = textcoord.xy * texScale * 4 + normalTime * normalSpeed * 8;

    vec3 normal = normalize(vec3(-(wave1.y+wave2.y), 1.0, -(wave1.z+wave2.z)));
    vec3 binormal = normalize(vec3(0.0, wave1.z + wave2.z, 1.0));
    vec3 tangent = normalize(vec3(1, wave1.y+wave2.y, 0));
    vs_out.TBN = mat3(tangent,binormal,normal);

    vec3 displaced_vertex = vertex;
    displaced_vertex.y += wave1.x;
    displaced_vertex.y += wave2.x;

    vec4 vertexPos = vertex_model_to_world * vec4(displaced_vertex, 1.0);

    vs_out.vertex = vertexPos.xyz;
    vs_out.normal = normal;
    vs_out.view = camera_postion - vertexPos.xyz;
    vs_out.textcoord = textcoord;

    gl_Position = vertex_world_to_clip * vertexPos;
}
