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
  
  float trans = 1.0;
  if (isLight == 1) {          
    gl_FragColor = vec4(1.0, 1.0, 0.0, .5);    
    if (gl_FragCoord.x > 350) {        
      gl_FragColor = vec4(1.0, 1.0, 0.0, (700.0 - gl_FragCoord.x) / 350 + .1);
    }
    else if (gl_FragCoord.x <= 350) {
       gl_FragColor = vec4(1.0, 1.0, 0.0, gl_FragCoord.x / 350 + .1);
    }    
  }
  else {
    
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
    
    gl_FragColor = texColor1;
  }  
}