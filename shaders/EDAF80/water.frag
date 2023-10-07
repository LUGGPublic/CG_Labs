#version 410

uniform vec3 light_position;
uniform samplerCube cube_map;
uniform mat4 normal_model_to_world;
uniform sampler2D normal_map;

in VS_OUT {
    vec3 vertex;
    vec3 normal;
    vec3 textcoord;
    vec3 view;
    mat3 TBN;
    vec2 normalCoord0;
    vec2 normalCoord1;
    vec2 normalCoord2;
} fs_in;

out vec4 frag_color;

void main() {
    vec4 deep = vec4(0.0, 0.0, 0.1, 1.0);
    vec4 shallow = vec4(0.0, 0.5, 0.5, 1.0);

    vec3 n = normalize(texture(normal_map, fs_in.normalCoord0).xyz*2.0-1.0
        + texture(normal_map, fs_in.normalCoord1).xyz*2.0-1.0
        + texture(normal_map, fs_in.normalCoord2).xyz*2.0-1.0);

    vec3 N = normalize(fs_in.TBN * n);

    float Rn = 1.0/1.33;
    if(!gl_FrontFacing)
    {
        N = -N;
    }

    vec3 V = normalize(fs_in.view);
    vec3 R = normalize(reflect(-V,N));
    vec3 Refract = normalize(refract(-V, N, Rn));

    float R0 = 0.02037;
    float fresnel = R0 + (1.0 - R0) * pow((1.0 - dot(V,N)),5.0);

    float facing = 1.0 - max(dot(V,N), 0.0);


    frag_color = mix(deep, shallow, facing) + texture(cube_map, R) * fresnel + texture(cube_map, Refract) * (1.0 - fresnel);
}
