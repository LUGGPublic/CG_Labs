#version 410

uniform sampler2D diffuse_texture;
uniform int has_textures;

in VS_OUT {
	vec2 texcoord;
} fs_in;

out vec4 frag_color;

void main()
{
	if (has_textures != 0)
		frag_color = texture(diffuse_texture, fs_in.texcoord);
	else
		frag_color = vec4(1.0);
}
