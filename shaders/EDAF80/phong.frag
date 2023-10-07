#version 410

uniform mat4 normal_model_to_world;
uniform int use_normal_mapping;
uniform sampler2D normal_map;

uniform int has_diffuse_texture;
uniform sampler2D diffuse_texture;

uniform int has_opacity_texture;
uniform sampler2D specular_map;

in VS_OUT {
    vec2 texcoord;
    vec3 normal;
    vec3 light;
    vec3 view;
    mat3 TBN;
} fs_in;

uniform vec3 ambient;
uniform vec3 diffuse;
uniform vec3 specular;
uniform float shininess;

out vec4 frag_color;

void main()
{
    vec3 N = normalize(fs_in.normal);

    if(use_normal_mapping == 1){
        vec3 n = normalize(texture(normal_map, fs_in.texcoord).xyz*2.0-1.0);
        N = (normal_model_to_world * vec4(fs_in.TBN * n,0)).xyz;
    }
    
    vec3 L = normalize(fs_in.light);
    vec3 V = normalize(fs_in.view);

    vec3 R = normalize(reflect(-L,N));

    vec3 diff = diffuse*max(dot(N,L),0.0);
    vec3 spec = specular*pow(max(dot(R,V),0.0),shininess) * texture(specular_map, fs_in.texcoord).xyz;;

    if(has_diffuse_texture == 1){
        diff = diff * texture(diffuse_texture, fs_in.texcoord).xyz;
    }

    frag_color.xyz = ambient + diff + spec;

    frag_color.w = 1.0;
}
