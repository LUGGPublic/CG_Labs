#version 410

in VS_OUT {
	vec3 binormal;
} fs_in;

out vec4 frag_color;

void main()
{
	frag_color = vec4((normalize(fs_in.binormal) + 1.0) / 2.0, 1.0);
}
