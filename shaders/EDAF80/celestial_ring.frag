#version 410

uniform sampler2D diffuse_texture;
uniform sampler2D opacity_texture;
uniform int has_diffuse_texture;
uniform int has_opacity_texture;

in VS_OUT {
	vec2 texcoord;
} fs_in;

out vec4 frag_color;

void main()
{
	float alpha = 1.0f;
	if (has_opacity_texture != 0) {
		alpha = texture(opacity_texture,
		                vec2(1.0f - fs_in.texcoord.x, fs_in.texcoord.y)).r;
		if (alpha == 0.0f)
			discard;
	}

	if (has_diffuse_texture != 0)
		frag_color = texture(diffuse_texture, fs_in.texcoord);
	else
		frag_color = vec4(1.0);

	frag_color *= alpha;
}
