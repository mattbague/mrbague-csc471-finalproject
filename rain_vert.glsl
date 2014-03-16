#version 120

// Input vertex data, different for all executions of this shader.
attribute vec3 squareVertices;
attribute vec4 xyzs; // Position of the center of the particule and size of the square
attribute vec4 color; // Position of the center of the particule and size of the square

// Output data ; will be interpolated for each fragment.
varying vec2 UV;
varying vec4 particlecolor;

// Values that stay constant for the whole mesh.
uniform vec3 CameraRight_worldspace;
uniform vec3 CameraUp_worldspace;
uniform mat4 VP; // Model-View-Projection matrix, but without the Model (the position is in BillboardPos; the orientation depends on the camera)

attribute vec3 aPosition;
attribute vec2 aTexCoord;

uniform mat4 uProjMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uModelMatrix;

uniform vec3 uViewerPos;
uniform vec3 uLightPos;

varying vec3 vColor;
varying vec2 vTexCoord;

varying vec3 vNorm;
varying vec3 vVert;

attribute vec3 aNormal;
uniform int isLight;

void main() {
   if (isLight == 1) { //REUSING VARIABLE CAUSE I'M LAZY
    float particleSize = xyzs.w; // because we encoded it this way.
    vec3 particleCenter_wordspace = xyzs.xyz;
    
    vec3 vertexPosition_worldspace = 
      particleCenter_wordspace
      + CameraRight_worldspace * squareVertices.x * particleSize
      + CameraUp_worldspace * squareVertices.y * particleSize;

    // Output position of the vertex
    gl_Position = VP * vec4(vertexPosition_worldspace, 1.0f);

    // UV of the vertex. No special space for this one.
    UV = squareVertices.xy + vec2(0.5, 0.5);
    particlecolor = color;    
  }
  else {
    vec4 vPosition;

    /* First model transforms */
    vPosition = uModelMatrix* vec4(aPosition.x, aPosition.y, aPosition.z, 1);
    vPosition = uViewMatrix* vPosition;
    gl_Position = uProjMatrix*vPosition;

    vVert = vec3(uModelMatrix * vec4(aPosition.x, aPosition.y, aPosition.z, 1));
    vNorm = vec3(uModelMatrix * vec4(aNormal.x, aNormal.y, aNormal.z, 0));
    
    vColor = vec3(0.56, 0.3, 0.1);
    vTexCoord = aTexCoord;       
  }
}
