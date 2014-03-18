uniform sampler2D uTexUnit;

uniform vec3 uViewerPos;
uniform vec3 uLightPos;

varying vec2 vTexCoord;
varying vec3 vColor;

varying vec3 vNorm;
varying vec3 vVert;

uniform int isLight;

uniform vec3 uSLColor;

void main(void) {
  vec4 texColor0 = vec4(vColor.x, vColor.y, vColor.z, 1);
  vec4 texColor1 = texture2D(uTexUnit, vTexCoord);

  vec3 uLightColor = vec3(1.0, 1.0, 1.0);
  
  const vec3 fog_colour = vec3 (0.5, 0.5, 0.5);
  const float min_fog_radius = 1.0;
  const float max_fog_radius = 15.0;
  
  float dist = length(-uViewerPos);
  // get a fog factor (thickness of fog) based on the distance
  float fog_fac = (dist - min_fog_radius) / (max_fog_radius - min_fog_radius);  
  
  //NOTE TO SELF: Function description is here: https://www.khronos.org/opengles/sdk/docs/man3/html/mix.xhtml
  vec3 fog_calc = mix(vec3(texColor1[0], texColor1[1], texColor1[2]), fog_colour, fog_fac);

  float trans = 1.0;
  if (isLight == 1) {  
    fog_calc = mix(vec3(uSLColor.x, uSLColor.y, uSLColor.z), vec3 (0.9, 0.9, 0.9), .5);
      if (gl_FragCoord.x > 350) {        
	trans = (700.0 - gl_FragCoord.x) / 350 + .1;
      }
      else if (gl_FragCoord.x <= 350) {
	trans = gl_FragCoord.x / 350 + .1;
      } 
  }
  
  vec4 vPosition;
  vec4 light;
  vec3 Refl, Viewer;
  vec3 Spec, Diffuse;
  float vLength, rLength, nLength;   
  vec3 fColor;

  //Calculating Diffuse
  Diffuse = uLightColor * max(dot(normalize(vNorm), normalize(uLightPos)), 0) * fog_calc;
  
  //Calculating Specular
  Viewer = normalize(uViewerPos - vVert);
  Refl = (-uLightPos) + 2.0 * (dot(normalize(uLightPos), normalize(vNorm))) * vNorm;  
  Refl = normalize(Refl);
  Spec = uLightColor * pow(dot(Viewer, Refl), 2.0) * fog_calc; 
  
  //Calculating Color
  float attenuation = length(uLightPos);
  attenuation *= attenuation;
  Diffuse.x /= attenuation;
  Diffuse.y /= attenuation;
  Diffuse.z /= attenuation;
  Spec.x /= attenuation;
  Spec.y /= attenuation;
  Spec.z /= attenuation;
  fColor = Diffuse + Spec + fog_calc * uLightColor;
  
  gl_FragColor = vec4(fColor.r, fColor.g, fColor.b, trans);  
}