struct Material {
  vec3 aColor;
  vec3 dColor;
  vec3 sColor;
  float shine;
};

varying vec2 vTexCoord;

attribute vec3 aPosition;
attribute vec3 aNormal;

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
  vec4 vPosition;
  
  /* First model transforms */
  vPosition = uModelMatrix* vec4(aPosition.x, aPosition.y, aPosition.z, 1);
  vPosition = uViewMatrix* vPosition;
  
  vVert = vec3(uModelMatrix * vec4(aPosition.x, aPosition.y, aPosition.z, 1));
  vNorm = vec3(uModelMatrix * vec4(aNormal.x, aNormal.y, aNormal.z, 0));
  
  gl_Position = uProjMatrix*vPosition;  
  
  if (uSelf == 1) {
    vColor = uLColor;
    gl_FrontColor = vec4(uLColor.r, uLColor.g, uLColor.b, 1.0);
  }
  else {
    vColor = vec3(0.56, 0.3, 0.1);    
  }
  
  vTexCoord = aTexCoord;
}
