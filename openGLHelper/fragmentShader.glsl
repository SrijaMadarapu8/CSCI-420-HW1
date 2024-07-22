#version 150


in vec4 col;
in vec2 texCord;
out vec4 c;

uniform sampler2D textureSampler;
void main()
{
  vec4 texColor = texture(textureSampler, texCoord);
  // compute the final pixel color
  c = texColor * col;
}

