#version 410

in VS_OUT {
	vec3 tangent;
} fs_in;

out vec4 frag_color;

void main()
{
	frag_color = vec4((normalize(fs_in.tangent) + 1.0) * 0.5, 1.0);
}
