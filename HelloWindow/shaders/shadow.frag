#version 410 core
out vec4 fColor;
uniform sampler2D diffuseTexture;
uniform bool isTransparent;
in vec2 fTexCoords;
void main()
{
	if(isTransparent) {
		if(texture(diffuseTexture, fTexCoords).a < 0.4f) {
			discard;
		}
	}
	fColor = vec4(0.3f);
}