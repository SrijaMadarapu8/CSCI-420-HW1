#version 150
uniform float scale,exponent;
uniform int mode;
uniform int grayscale;
in vec3 smooth_center,smooth_left,smooth_right,smooth_up,smooth_down;

in vec3 position;
in vec4 color;
out vec4 col;

void JetColorMap(in float x,inout vec3 color1)
{
    float a; // alpha

    if (x < 0)
    {
        color1.r = 0;
        color1.g = 0;
        color1.b = 0;
        
    }
    else if (x < 0.125)
    {
        a = x / 0.125;
        color1.r = 0;
        color1.g = 0;
        color1.b = 0.5 + 0.5 * a;
      
    }
    else if (x < 0.375)
    {
        a = (x - 0.125) / 0.25;
        color1.r = 0;
        color1.g = a;
        color1.b = 1;
       
    }
    else if (x < 0.625)
    {
        a = (x - 0.375) / 0.25;
        color1.r = a;
        color1.g = 1;
        color1.b = 1 - a;
      
    }
    else if (x < 0.875)
    {
        a = (x - 0.625) / 0.25;
        color1.r = 1;
        color1.g = 1 - a;
        color1.b = 0;
       
    }
    else if (x <= 1.0)
    {
        a = (x - 0.875) / 0.125;
        color1.r = 1 - 0.5 * a;
        color1.g = 0;
        color1.b = 0;
        
    }
    else
    {
        color1.r = 1;
        color1.g = 1;
        color1.b = 1;
       
    }
}

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

void main()
{
  // compute the transformed and projected vertex position (into gl_Position) 
  // compute the vertex color (into col)
  if(mode==1)
  {
	vec3 smoothposition;
	vec4 c;
	smoothposition = (smooth_center+smooth_left+smooth_right+smooth_up+smooth_down)/5.0f;
	c.r=pow(smoothposition.y,exponent);
	c.g=pow(smoothposition.y,exponent);
	c.b=pow(smoothposition.y,exponent);
	c.a=1.0f;
	smoothposition.y = scale*pow(smoothposition.y,exponent);
	gl_Position = projectionMatrix * modelViewMatrix * vec4(smoothposition, 1.0f);
	
    if(grayscale==1)
    {
    col = c;
    }
    else if(grayscale==0)
    {
    float v = smoothposition.y;
    vec3 color1;
    color1.r = color.r;
    color1.g = color.g;
    color1.b = color.b;
    JetColorMap(v,color1);
    col.r = color1.r;
    col.g = color1.g;
    col.b = color1.b;
    col.a = color.a;
    }
  }
  else if (mode==0)
  {
	gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0f);
	col = color;
  }
  else if (mode==2)
  {
	gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0f);
    float v = position.y;
    vec3 color1;
    color1.r = color.r;
    color1.g = color.g;
    color1.b = color.b;
    JetColorMap(v,color1);
    col.r = color1.r;
    col.g = color1.g;
    col.b = color1.b;
    col.a = color.a;
  }
  
}

