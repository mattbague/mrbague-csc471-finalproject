struct Material {
  vec3 aColor;
  vec3 dColor;
  vec3 sColor;
  float shine;
};

uniform sampler2D uTexUnit;

varying vec2 vTexCoord;

varying vec3 vNorm;
varying vec3 vVert;

uniform mat4 uProjMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uModelMatrix;

uniform vec3 uViewerPos;
uniform vec3 uLightPos;
uniform vec3 uLColor;
uniform Material uMat;
uniform int uSelf;

varying vec3 vColor;

void main() {
  vec4 texColor0 = vec4(vColor.x, vColor.y, vColor.z, 1);
  vec4 texColor1 = texture2D(uTexUnit, vTexCoord);
  vec3 texColor2 = vec3(texColor1.[0[, texColor1[1], texColor1[2]);
  
  vec4 vPosition;
  vec4 light;
  vec3 Refl, Viewer;
  vec3 Spec, Diffuse;
  float vLength, rLength, nLength;   
  vec3 fColor;

  //Calculating Diffuse
  Diffuse = uLColor * max(dot(normalize(vNorm), normalize(uLightPos)), 0) * uMat.dColor;
  
  //Calculating Specular
  Viewer = normalize(uViewerPos - vVert);
  Refl = (-uLightPos) + 2.0 * (dot(normalize(uLightPos), normalize(vNorm))) * vNorm;  
  Refl = normalize(Refl);
  Spec = uLColor * pow(dot(Viewer, Refl), uMat.shine) * uMat.sColor; 
  
  //Calculating Color
  float attenuation = length(uLightPos);
//   attenuation *= attenuation;
//   Diffuse.x /= attenuation;
//   Diffuse.y /= attenuation;
//   Diffuse.z /= attenuation;
//   Spec.x /= attenuation;
//   Spec.y /= attenuation;
//   Spec.z /= attenuation;
  fColor = Diffuse + Spec + uMat.aColor * uLColor;
  gl_FragColor = vec4(fColor.r, fColor.g, fColor.b, 1.0);
  
  if (uSelf == 1) {
    gl_FragColor = vec4(uLColor.r, uLColor.g, uLColor.b, 1.0);
  }
  
  gl_FragColor = vec4(texColor1[0], texColor1[1], texColor1[2], 1);
}
