#version 120

// Interpolated values from the vertex shaders
varying vec2 UV;
varying vec4 particlecolor;

uniform sampler2D myTextureSampler;

uniform sampler2D uTexUnit;

uniform vec3 uViewerPos;
uniform vec3 uLightPos;

varying vec2 vTexCoord;
varying vec3 vColor;

varying vec3 vNorm;
varying vec3 vVert;

uniform int isLight;

void main(void) {
  vec4 texColor0 = vec4(vColor.x, vColor.y, vColor.z, 1);
  vec4 texColor1 = texture2D(uTexUnit, vTexCoord);

  vec3 uLightColor = vec3(1.0, 1.0, 1.0);
/*  
  if (isLight == 1) {  
    gl_FragColor = vec4(1.0, 1.0, 0.0, 1.0);
  }
  else {*/
    // Output color = color of the texture at the specified UV
//     gl_FragColor = texture2D( myTextureSampler, UV ) * particlecolor;
  gl_FragColor = particlecolor;
//   }  
}