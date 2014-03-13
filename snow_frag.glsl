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
 
  if  (isLight == 1) { //REUSING VARIABLE CAUSE I'M LAZY
    gl_FragColor = particlecolor;
  }
  else {  
    vec3 fog_calc = vec3(1.0, 1.0, 1.0);
    
    vec4 vPosition;
    vec4 light;
    vec3 Refl, Viewer;
    vec3 Spec, Diffuse;
    float vLength, rLength, nLength;   
    vec3 fColor;

    //Calculating Diffuse
    Diffuse = uLightColor * max(dot(normalize(vNorm), normalize(uLightPos)), 0);
    
    //Calculating Specular
    Viewer = normalize(uViewerPos - vVert);
    Refl = (-uLightPos) + 2.0 * (dot(normalize(uLightPos), normalize(vNorm))) * vNorm;  
    Refl = normalize(Refl);
    Spec = uLightColor * pow(dot(Viewer, Refl), 2.0) ; 
    
    //Calculating Color
    float attenuation = length(uLightPos);
    attenuation *= attenuation;
    Diffuse.x /= attenuation;
    Diffuse.y /= attenuation;
    Diffuse.z /= attenuation;
    Spec.x /= attenuation;
    Spec.y /= attenuation;
    Spec.z /= attenuation;
    fColor = Diffuse + Spec + uLightColor;
    
    gl_FragColor = vec4(fColor.r, fColor.g, fColor.b, 1.0);  
    gl_FragColor = texColor1;
  }

}