#version 330

in float Col;
in float Frame;

void main()
{
	if (Frame > 0) gl_FragColor  = vec4(0, 0, 0, 1.0f);
	else {
		float r = (Col >= 0.75)? 1 : (Col - 0.5)*4;
		float g = (Col <= 0.5)? 2 * Col : (1 - Col)*2;
		float b = (Col <= 0.25)? 1 : (0.5 - Col)*4;
		gl_FragColor  = vec4(r, g, b, 1.0f);
	}
} 
