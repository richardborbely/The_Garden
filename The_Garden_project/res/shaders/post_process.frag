#version 440

// Texture captured from the scene
uniform sampler2D tex;

//PP Values
uniform float brightness;
uniform float saturation;
uniform float chromaticAbberation;
uniform int sepia;
uniform float frameTime;
uniform float noiseStrength;

// Incoming position
layout(location = 0) in vec3 vertex_position;
// Incoming normal
layout(location = 1) in vec3 normal;
// Incoming texture coordinate
layout(location = 2) in vec2 tex_coord;

// Outgoing colour
layout(location = 0) out vec4 colour;

void main()
{
  // Sample texture
	vec4 tex_colour = texture(tex, tex_coord);
  colour = tex_colour;

  // Brightness
  colour += vec4(vec3(brightness), 1.0f);

  // Saturation
  vec3 luminance =  vec3(0.299, 0.587, 0.114);
  vec3 grey = vec3(dot(luminance, vec3(colour)));
  colour = vec4(mix(grey, vec3(colour), saturation), 1.0f);

	// Chromatic Abberation
	if(chromaticAbberation > 0.000f){
		float Rcolour = texture(tex, tex_coord - vec2((chromaticAbberation + 0.001f), chromaticAbberation)).r;
		float Gcolour = texture(tex, tex_coord).g;
		float Bcolour = texture(tex, tex_coord - vec2(chromaticAbberation, (chromaticAbberation + 0.001f))).b;
		colour = mix(colour, vec4(Rcolour, Gcolour, Bcolour, 1.0f), 0.5);
	}

	// Sepia
	if(sepia > 0.0f){
		float out_Red = (colour.r * 0.393f) + (colour.g * 0.769f) + (colour.b * 0.189f);
		float out_Green = (colour.r * 0.349f) + (colour.g * 0.686f) + (colour.b * 0.168f);
		float out_Blue = (colour.r * 0.272f) + (colour.g * 0.534f) + (colour.b * 0.131f);
		colour = vec4(vec3(out_Red, out_Green, out_Blue), 1.0f);
	}

	// FilmGrain
	if(noiseStrength > 0.0f){
		float t = (fract(frameTime) + 1) * 10;
		float x = (tex_coord.x + 5) * (tex_coord.y + 5) * t;
		vec3 grain = vec3(mod((mod(x, 17) + 1) * (mod(x, 29) + 1), 0.01)) * noiseStrength;

		colour += vec4(grain, 1.0f);
	}

  colour.a = 1.0f;
}
