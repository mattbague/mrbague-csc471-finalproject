//for CSC 471 - texture mapping lab
//ZJ WOOD

#ifdef __APPLE__
#include "GLUT/glut.h"
#include <OPENGL/gl.h>
#endif
#ifdef __unix__
#include <GL/glut.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include "GLSL_helper.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp" //perspective, trans etc
#include "glm/gtc/type_ptr.hpp" //value_ptr
#include "MStackHelp.h"
#include "glm/gtx/norm.hpp"

#include <iostream>
#include <math.h>
#include <assert.h>
#include <vector>
#include <algorithm>

using namespace std;
using namespace glm;

/*data structure for the image used for  texture mapping */
typedef struct Image {
  unsigned long sizeX;
  unsigned long sizeY;
  char *data;
} Image;

Image *TextureImage;

typedef struct RGB {
  GLubyte r;
  GLubyte g; 
  GLubyte b;
} RGB;

RGB myimage[64][64];
RGB* g_pixel;

//forward declaration of image loading and texture set-up code
int ImageLoad(char *filename, Image *image);
GLvoid LoadTexture(char* image_file, int tex_id);

//mode to toggle drawing
int cube;

//flag and ID to toggle on and off the shader
int ShadeProg, Shade2;

//Handles to the shader data
GLint h_uTexUnit;
GLint h_uTexUnit2;
GLint h_aPosition;
GLint h_aTexCoord;
GLint h_uModelMatrix;
GLint h_uViewMatrix;
GLint h_uProjMatrix;
GLint h_uModelMatrix2;
GLint h_uViewMatrix2;
GLint h_uProjMatrix2;
GLint h_uViewerPos;
GLint h_aNormal;
GLint h_uLightPos;
GLint h_uLightColor;

static int SHADER_MODE; // 1 = Nothing, 2 = Fog, 3 = Rain, 4 = Snow
static bool ROTATE = false;

GLuint CubeBuffObj, CIndxBuffObj, TexBuffObj, CNormBuffObj;
GLuint GrndBuffObj, GIndxBuffObj, GTexBuffObj, GNormBuffObj;
GLuint HexBuffObj, HIndxBuffObj, HTexBuffObj, HNormBuffObj;
GLuint RoofBuffObj, RIndxBuffObj, RTexBuffObj, RNormBuffObj;
GLuint LightBuffObj, LIndxBuffObj;
GLuint h_isLight;
int g_CiboLen, g_GiboLen, g_HiboLen, g_RiboLen, g_LiboLen;
static float  g_width, g_height;
float g_angle = 0;
float g_trans = -1;
float g_transy = -3;
float lightRot = 0;

glm::vec3 lp = glm::vec3(0, 3.0, .81);

static const float g_groundY = -1.5;      // y coordinate of the ground
static const float g_groundSize = 20.0;   // half the ground length

  // Vertex shader
  GLuint CameraRight_worldspace_ID;
  GLuint CameraUp_worldspace_ID;
  GLuint ViewProjMatrixID;

  // fragment shader
  GLuint TextureID;

  // Get a handle for our buffers
  GLuint squareVerticesID;
  GLuint xyzsID;
  GLuint colorID;   
  
  GLuint particles_color_buffer;
  GLuint particles_position_buffer;
  GLuint billboard_vertex_buffer;

  struct Particle{
    glm::vec3 pos, speed;
    unsigned char r,g,b,a; // Color
    float size, angle, weight;
    float life; // Remaining life of the particle. if <0 : dead and unused.
    float cameradistance; // *Squared* distance to the camera. if dead : -1.0f

    bool operator<(const Particle& that) const {
      // Sort in reverse order : far particles drawn first.
      return this->cameradistance > that.cameradistance;
    }
  };

  const int MaxParticles = 100000;
  Particle ParticlesContainer[MaxParticles];
  int LastUsedParticle = 0;    
  
  static GLfloat* g_particule_position_size_data = new GLfloat[MaxParticles * 4];
  static GLubyte* g_particule_color_data         = new GLubyte[MaxParticles * 4];
  
  static bool MAKE_IT_RAIN = false;
  
/* projection matrix */
void SetProjectionMatrix() {
  glm::mat4 Projection = glm::perspective(80.0f, (float)g_width/g_height, 0.1f, 100.f);
  safe_glUniformMatrix4fv(h_uProjMatrix, glm::value_ptr(Projection));
  safe_glUniformMatrix4fv(h_uProjMatrix2, glm::value_ptr(Projection));
}

/* camera controls - do not change */
void SetView() {
  glm::mat4 Trans = glm::translate( glm::mat4(1.0f), glm::vec3(0.0f, g_transy, g_trans));
  glm::mat4 RotateX = glm::rotate( Trans, g_angle, glm::vec3(0.0f, 1, 0));
  safe_glUniformMatrix4fv(h_uViewMatrix, glm::value_ptr(RotateX));
  safe_glUniformMatrix4fv(h_uViewMatrix2, glm::value_ptr(RotateX));
}

/* set the model transform to the identity */
void SetModelI() {
  glm::mat4 tmp = glm::mat4(1.0f);
  safe_glUniformMatrix4fv(h_uModelMatrix, glm::value_ptr(tmp));
  safe_glUniformMatrix4fv(h_uModelMatrix2, glm::value_ptr(tmp));
}

static void initGround() {

  // A x-z plane at y = g_groundY of dimension [-g_groundSize, g_groundSize]^2
    float GrndPos[] = {
    -g_groundSize, g_groundY, -g_groundSize,
    -g_groundSize, g_groundY,  g_groundSize,
     g_groundSize, g_groundY,  g_groundSize,
     g_groundSize, g_groundY, -g_groundSize
    };

    float rep = 3;
    
    float GrndTex[] = {
      0, 0,
      rep, 0,
      rep, rep,
      0, rep
    };
    
    float GrndNorm[] = {
      0, 1, 0,
      0, 1, 0,
      0, 1, 0,
      0, 1, 0
    };

    unsigned short idx[] = {0, 1, 2, 0, 2, 3};

    g_GiboLen = 6;
    glGenBuffers(1, &GrndBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GrndPos), GrndPos, GL_STATIC_DRAW);

    glGenBuffers(1, &GIndxBuffObj);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);
  
    glGenBuffers(1, &GTexBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, GTexBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GrndTex), GrndTex, GL_STATIC_DRAW);

    glGenBuffers(1, &GNormBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, GNormBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GrndNorm), GrndNorm, GL_STATIC_DRAW);    
}

/* intialize the cube data */
static void initCube() {

  float CubePos[] = {
    -1.5, -1.5, -1.5, //back face 5 verts :0 
    -1.0, 2.5, -1.0,
    1.0, 2.5, -1.0,
    1.5, -1.5, -1.5
    ,
    1.5, -1.5, 1.5, //right face 5 verts :4
    1.0, 2.5, 1.0,
    1.0, 2.5, -1.0,
    1.5, -1.5, -1.5,
    
    -1.5, -1.5, 1.5, //front face 4 verts :8
    -1.0, 2.5, 1.0,
    1.0, 2.5, 1.0,
    1.5, -1.5, 1.5,
    
    -1.5, -1.5, -1.5, //left face 4 verts :12
    -1.0, 2.5, -1.0,
    -1.0, 2.5, 1.0,
    -1.5, -1.5, 1.5,
    
    -1, 2.5, -1, //top
    -1, 2.5, 1,
    1, 2.5, 1,
    1, 2.5, -1
  };

   static GLfloat CubeTex[] = {
      0.75, 0, // back 
      0.75, 1,
      0.5, 1,
      0.5, 0,
      
      0.25, 0, //right 
      0.25, 1,
      0.5, 1,
      0.5, 0,
      
      0, 0, //front 
      0, 1,
      0.25, 1,
      0.25, 0,
      
      0.75, 0, // left 
      0.75, 1,
      1, 1,
      1, 0,
      
      0, 1, //top
      1, 1,
      1, 0,
      0, 0
    }; 
    
   float CubeNorm[] = {
      0.0f, 0.0f, -1.0f,
      0.0f, 0.0f, -1.0f,
      0.0f, 0.0f, -1.0f,
      0.0f, 0.0f, -1.0f,
      0.0f, 0.0f, -1.0f,

      0.0f, 0.0f, 1.0f,
      0.0f, 0.0f, 1.0f,
      0.0f, 0.0f, 1.0f,
      0.0f, 0.0f, 1.0f,
      0.0f, 0.0f, 1.0f,

      -1.0f, 0.0f, 0.0f,
      -1.0f, 0.0f, 0.0f,
      -1.0f, 0.0f, 0.0f,
      -1.0f, 0.0f, 0.0f,

      1.0f, 0.f, 0.0f,
      1.0f, 0.f, 0.0f,
      1.0f, 0.f, 0.0f,
      1.0f, 0.f, 0.0f,
      
      0.0f, 1.f, 0.0f,
      0.0f, 1.f, 0.0f,
      0.0f, 1.f, 0.0f,
      0.0f, 1.f, 0.0f      
   };
   
    unsigned short idx[] = {0, 1, 2,  2, 3, 0,  4, 5, 6, 6, 7, 4,  8, 9, 10, 10, 11, 8,  12, 13, 14, 14, 15, 12,
      16, 17, 19, 19, 18, 17
    };

    g_CiboLen = 30;
    glGenBuffers(1, &CubeBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, CubeBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CubePos), CubePos, GL_STATIC_DRAW);

    glGenBuffers(1, &CIndxBuffObj);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CIndxBuffObj);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);

    glGenBuffers(1, &TexBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, TexBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CubeTex), CubeTex, GL_STATIC_DRAW);
    
    glGenBuffers(1, &CNormBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, CNormBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CubeNorm), CubeNorm, GL_STATIC_DRAW);
}

static void initHexPrism() {
  float t_height = 3.5, b_height = 2.5, x = .58, z = 1;
  
  float HexPos[] = {
    // Top hexagon: 6 verts going CW
    -x, t_height, -z,                   //0
    -1, t_height, 0,                //1
    -x, t_height, z,                    //2
    x, t_height, z,                     //3
    1, t_height, 0,               //4
    x, t_height, -z,                    //5
    // Bottom hexagon: 6 verts going CW
    -x, b_height, -z,              //6
    -1, b_height, 0,            //7
    -x, b_height, z,                //8
    x, b_height, z,                 //9
    1, b_height, 0,           //10
    x, b_height, -z                 //11
  };  

  /*static GLfloat HexTex[] = {
    -x, t_height, -z,                   //0
    -1, t_height, 0,                //1
    -x, t_height, z,                    //2
    x, t_height, z,                     //3
    1, t_height, 0,               //4
    x, t_height, -z,                    //5
    // Bottom hexagon: 6 verts going CW
    -x, b_height, -z,              //6
    -1, b_height, 0,            //7
    -x, b_height, z,                //8
    x, b_height, z,                 //9
    1, b_height, 0,           //10
    x, b_height, -z                 //11  
    };   */
    
    //TESTING
  static GLfloat HexTex[] = {
      0, 1, //top
      1, 1,
      1, 0,
      0, 0,
    
      0, 1, //top
      1, 1,
      1, 0,
      0, 0,
    
      0, 1, //top
      1, 1,
      1, 0,
      0, 0,    
  };  
  
  unsigned short idx[] = {0, 6, 1, 1, 7, 6,
    1, 7, 2, 2, 8, 7,
    2, 8, 3, 3, 9, 8,
    3, 9, 4, 4, 10, 9,
    4, 10, 5, 5, 11, 10, 
    5, 11, 0, 0, 6, 11
  };  
  
    g_HiboLen = 36;
    glGenBuffers(1, &HexBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, HexBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(HexPos), HexPos, GL_STATIC_DRAW);

    glGenBuffers(1, &HIndxBuffObj);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, HIndxBuffObj);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);

    glGenBuffers(1, &HTexBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, HTexBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(HexTex), HexTex, GL_STATIC_DRAW);  
}

static void initRoof() {
  float base = 1.2;
  float RoofPos[] = {
    -base, 3.5, base,
    base, 3.5, base,
    base, 3.5, -base,
    -base, 3.5, -base,
    0.0, 4.75, 0.0    
  };
  
  float RoofTex[] = {
    0.0, 0.0,
    .5, 1.0,
    1.0, 0.0,
    
    0.0, 0.5,
    1.0, .5,
  };  
   
   unsigned short idx[] = {0, 1, 4, 1, 2, 4, 2, 3, 4, 0, 3, 4, 0, 2, 3, 0, 1, 2};

    g_RiboLen = 18;
    glGenBuffers(1, &RoofBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, RoofBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(RoofPos), RoofPos, GL_STATIC_DRAW);

    glGenBuffers(1, &RIndxBuffObj);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RIndxBuffObj);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);  
    
    glGenBuffers(1, &RTexBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, RTexBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(RoofTex), RoofTex, GL_STATIC_DRAW);      
}

static void initLight() {
  float CubePos[] = {
    -0.35, -0.35, -0.35, 
    -0.35, 0.35, -0.35, 
    0.35, 0.35, -0.35, 
    0.35, -0.35, -0.35, 
    -0.35, -0.35, 0.35, 
    -0.35, 0.35, 0.35, 
    0.35, 0.35, 0.35, 
    0.35, -0.35, 0.35
  };
   
   unsigned short idx[] = {0, 1, 2, 0, 2, 3, 7, 6, 4, 4, 6, 5, 1, 5, 6, 1, 6, 2, 0, 3, 7, 0, 7, 4,
     2, 3, 6,
     3, 7, 6,
     0, 1, 5,
     0, 4, 5
  };

    g_LiboLen = 36;
    glGenBuffers(1, &LightBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, LightBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CubePos), CubePos, GL_STATIC_DRAW);

    glGenBuffers(1, &LIndxBuffObj);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, LIndxBuffObj);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);
}

// Finds a Particle in ParticlesContainer which isn't used yet.
// (i.e. life < 0);
int FindUnusedParticle(){

  for(int i=LastUsedParticle; i<MaxParticles; i++){
    if (ParticlesContainer[i].life < 0){
      LastUsedParticle = i;
      return i;
    }
  }

  for(int i=0; i<LastUsedParticle; i++){
    if (ParticlesContainer[i].life < 0){
      LastUsedParticle = i;
      return i;
    }
  }

  return 0; // All particles are taken, override the first one
}

void SortParticles(){
  std::sort(&ParticlesContainer[0], &ParticlesContainer[MaxParticles]);
}


static void initRain() {
  for (int i=0; i<MaxParticles; i++){
    ParticlesContainer[i].life = -1.0f;
    ParticlesContainer[i].cameradistance = -1.0f;
  }    
  
  // The VBO containing the 4 vertices of the particles.
  // Thanks to instancing, they will be shared by all particles.
  static const GLfloat g_vertex_buffer_data[] = { 
  -0.5f, -0.5f, 0.0f,
  0.5f, -0.5f, 0.0f,
  -0.5f, 0.5f, 0.0f,
  0.5f, 0.5f, 0.0f,
  };
  
  glGenBuffers(1, &billboard_vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
  
  // The VBO containing the positions and sizes of the particles
  glGenBuffers(1, &particles_position_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
  // Initialize with empty (NULL) buffer : it will be updated later, each frame.
  glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
  
  // The VBO containing the colors of the particles
  glGenBuffers(1, &particles_color_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
  // Initialize with empty (NULL) buffer : it will be updated later, each frame.
  glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);  
}

void InitGeom() {
  initCube();
  initGround();
  initHexPrism();
  initRoof();
  initLight();
  initRain();  
}

/*function to help load the shaders (both vertex and fragment */
int InstallShader(const GLchar *vShaderName, const GLchar *fShaderName) {
        GLuint VS; //handles to shader object
        GLuint FS; //handles to frag shader object
        GLint vCompiled, fCompiled, linked; //status of shader

        VS = glCreateShader(GL_VERTEX_SHADER);
        FS = glCreateShader(GL_FRAGMENT_SHADER);

        //load the source
        glShaderSource(VS, 1, &vShaderName, NULL);
        glShaderSource(FS, 1, &fShaderName, NULL);

        //compile shader and print log
        glCompileShader(VS);
        /* check shader status requires helper functions */
        printOpenGLError();
        glGetShaderiv(VS, GL_COMPILE_STATUS, &vCompiled);
        printShaderInfoLog(VS);

        //compile shader and print log
        glCompileShader(FS);
        /* check shader status requires helper functions */
        printOpenGLError();
        glGetShaderiv(FS, GL_COMPILE_STATUS, &fCompiled);
        printShaderInfoLog(FS);

        if (!vCompiled || !fCompiled) {
                printf("Error compiling either shader %s or %s", vShaderName, fShaderName);
                return 0;
        }

        //create a program object and attach the compiled shader
        ShadeProg = glCreateProgram();
        glAttachShader(ShadeProg, VS);
        glAttachShader(ShadeProg, FS);

        glLinkProgram(ShadeProg);
        /* check shader status requires helper functions */
        printOpenGLError();
        glGetProgramiv(ShadeProg, GL_LINK_STATUS, &linked);
        printProgramInfoLog(ShadeProg);

        glUseProgram(ShadeProg);

        /* get handles to attribute data */
       h_aPosition = safe_glGetAttribLocation(ShadeProg, "aPosition");
       h_aNormal = safe_glGetAttribLocation(ShadeProg, "aNormal");
       h_aTexCoord = safe_glGetAttribLocation(ShadeProg,  "aTexCoord");
       h_uTexUnit = safe_glGetUniformLocation(ShadeProg, "uTexUnit");
       h_uProjMatrix = safe_glGetUniformLocation(ShadeProg, "uProjMatrix");
       h_uViewMatrix = safe_glGetUniformLocation(ShadeProg, "uViewMatrix");
       h_uModelMatrix = safe_glGetUniformLocation(ShadeProg, "uModelMatrix");       
       h_uViewerPos = safe_glGetUniformLocation(ShadeProg, "uViewerPos");
       h_uLightPos = safe_glGetUniformLocation(ShadeProg, "uLightPos");
       h_isLight = safe_glGetUniformLocation(ShadeProg, "isLight");

      // Vertex shader
      CameraRight_worldspace_ID  = glGetUniformLocation(ShadeProg, "CameraRight_worldspace");
      CameraUp_worldspace_ID  = glGetUniformLocation(ShadeProg, "CameraUp_worldspace");
      ViewProjMatrixID = glGetUniformLocation(ShadeProg, "VP");

      // fragment shader
      TextureID  = glGetUniformLocation(ShadeProg, "myTextureSampler");

      // Get a handle for our buffers
      squareVerticesID = glGetAttribLocation(ShadeProg, "squareVertices");
      xyzsID = glGetAttribLocation(ShadeProg, "xyzs");
      colorID = glGetAttribLocation(ShadeProg, "color");   
      
      static GLfloat* g_particule_position_size_data = new GLfloat[MaxParticles * 4];
      static GLubyte* g_particule_color_data         = new GLubyte[MaxParticles * 4];       
       
       printf("sucessfully installed shader %d\n", ShadeProg);
       return 1;

}

/* Some OpenGL initialization */
void Initialize ()                  // Any GL Init Code
{
    // Start Of User Initialization
    glClearColor(.529f,.808f,.980f,1.0f);
    // Black Background
    glClearDepth (1.0f);    // Depth Buffer Setup
    glDepthFunc (GL_LEQUAL);    // The Type Of Depth Testing
    glEnable (GL_DEPTH_TEST);// Enable Depth Testing
    /* texture specific settings */
    glEnable(GL_TEXTURE_2D);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);     
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);     
}

double lastTime = time(NULL);

void drawRain() {
  // Enable depth test
  glEnable(GL_DEPTH_TEST);
  // Accept fragment if it closer to the camera than the former one
  glDepthFunc(GL_LESS);
  
    double currentTime = time(NULL);
    double delta = currentTime - lastTime;
    lastTime = currentTime;  
  
    // Generate 10 new particule each millisecond,
    // but limit this to 16 ms (60 fps), or if you have 1 long frame (1sec),
    // newparticles will be huge and the next frame even longer.
    int newparticles = (int)(delta*10000.0);
    if (newparticles > (int)(0.016f*10000.0))
      newparticles = (int)(0.016f*10000.0);
    
    for(int i=0; i<newparticles; i++){
      int particleIndex = FindUnusedParticle();
      ParticlesContainer[particleIndex].life = 5.0f; // This particle will live 5 seconds.
      ParticlesContainer[particleIndex].pos = glm::vec3(0,0,-20.0f);

      float spread = 1.5f;
      glm::vec3 maindir = glm::vec3(0.0f, 10.0f, 0.0f);
      // Very bad way to generate a random direction; 
      // See for instance http://stackoverflow.com/questions/5408276/python-uniform-spherical-distribution instead,
      // combined with some user-controlled parameters (main direction, spread, etc)
      glm::vec3 randomdir = glm::vec3(
        (rand()%2000 - 1000.0f)/1000.0f,
        (rand()%2000 - 1000.0f)/1000.0f,
        (rand()%2000 - 1000.0f)/1000.0f
      );
      
      ParticlesContainer[particleIndex].speed = maindir + randomdir*spread;


      // Very bad way to generate a random color
      ParticlesContainer[particleIndex].r = rand() % 256;
      ParticlesContainer[particleIndex].g = rand() % 256;
      ParticlesContainer[particleIndex].b = rand() % 256;
      ParticlesContainer[particleIndex].a = (rand() % 256) / 3;

      ParticlesContainer[particleIndex].size = (rand()%1000)/2000.0f + 0.1f;
      
    }

    // Simulate all particles
    int ParticlesCount = 0;
    for(int i=0; i<MaxParticles; i++){

      Particle& p = ParticlesContainer[i]; // shortcut

      if(p.life > 0.0f){

        // Decrease life
        p.life -= delta;
        if (p.life > 0.0f){

          // Simulate simple physics : gravity only, no collisions
          p.speed += glm::vec3(0.0f,-9.81f, 0.0f) * (float)delta * 0.5f;
          p.pos += p.speed * (float)delta;
          p.cameradistance = glm::length2( p.pos - glm::vec3(0, -g_transy, -g_trans));
          //ParticlesContainer[i].pos += glm::vec3(0.0f,10.0f, 0.0f) * (float)delta;

          // Fill the GPU buffer
          g_particule_position_size_data[4*ParticlesCount+0] = p.pos.x;
          g_particule_position_size_data[4*ParticlesCount+1] = p.pos.y;
          g_particule_position_size_data[4*ParticlesCount+2] = p.pos.z;
                           
          g_particule_position_size_data[4*ParticlesCount+3] = p.size;
                           
          g_particule_color_data[4*ParticlesCount+0] = p.r;
          g_particule_color_data[4*ParticlesCount+1] = p.g;
          g_particule_color_data[4*ParticlesCount+2] = p.b;
          g_particule_color_data[4*ParticlesCount+3] = p.a;

        }else{
          // Particles that just died will be put at the end of the buffer in SortParticles();
          p.cameradistance = -1.0f;
        }

        ParticlesCount++;

      }
    }

    SortParticles();


//     printf("%d ",ParticlesCount);


    // Update the buffers that OpenGL uses for rendering.
    // There are much more sophisticated means to stream data from the CPU to the GPU, 
    // but this is outside the scope of this tutorial.
    // http://www.opengl.org/wiki/Buffer_Object_Streaming


    glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
    glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
    glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLfloat) * 4, g_particule_position_size_data);

    glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
    glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
    glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLubyte) * 4, g_particule_color_data);


    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    // Set our "myTextureSampler" sampler to user Texture Unit 0
    glUniform1i(TextureID, 0);

    // Same as the billboards tutorial
    glm::mat4 Trans = glm::translate( glm::mat4(1.0f), glm::vec3(0.0f, g_transy, g_trans));
    glm::mat4 ViewMatrix = glm::rotate( Trans, g_angle, glm::vec3(0.0f, 1, 0));    
    glUniform3f(CameraRight_worldspace_ID, ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]);
    glUniform3f(CameraUp_worldspace_ID   , ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]);

    glm::mat4 Projection = glm::perspective(80.0f, (float)g_width/g_height, 0.1f, 100.f);
    glm::mat4 ViewProjectionMatrix = Projection * ViewMatrix;
    glUniformMatrix4fv(ViewProjMatrixID, 1, GL_FALSE, &ViewProjectionMatrix[0][0]);

        
    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(squareVerticesID);
    glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
    glVertexAttribPointer(
      squareVerticesID,                  // attribute. No particular reason for 0, but must match the layout in the shader.
      3,                  // size
      GL_FLOAT,           // type
      GL_FALSE,           // normalized?
      0,                  // stride
      (void*)0            // array buffer offset
    );
    
    // 2nd attribute buffer : positions of particles' centers
    glEnableVertexAttribArray(xyzsID);
    glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
    glVertexAttribPointer(
      xyzsID,                                // attribute. No particular reason for 1, but must match the layout in the shader.
      4,                                // size : x + y + z + size => 4
      GL_FLOAT,                         // type
      GL_FALSE,                         // normalized?
      0,                                // stride
      (void*)0                          // array buffer offset
    );

    // 3rd attribute buffer : particles' colors
    glEnableVertexAttribArray(colorID);
    glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
    glVertexAttribPointer(
      colorID,                                // attribute. No particular reason for 1, but must match the layout in the shader.
      4,                                // size : r + g + b + a => 4
      GL_UNSIGNED_BYTE,                 // type
      GL_TRUE,                          // normalized?    *** YES, this means that the unsigned char[4] will be accessible with a vec4 (floats) in the shader ***
      0,                                // stride
      (void*)0                          // array buffer offset
    );

    // These functions are specific to glDrawArrays*Instanced*.
    // The first parameter is the attribute buffer we're talking about.
    // The second parameter is the "rate at which generic vertex attributes advance when rendering multiple instances"
    // http://www.opengl.org/sdk/docs/man/xhtml/glVertexAttribDivisor.xml
    glVertexAttribDivisorARB(squareVerticesID, 0); // particles vertices : always reuse the same 4 vertices -> 0
    glVertexAttribDivisorARB(xyzsID, 1); // positions : one per quad (its center)                 -> 1
    glVertexAttribDivisorARB(colorID, 1); // color : one per quad                                  -> 1

    // Draw the particules !
    // This draws many times a small triangle_strip (which looks like a quad).
    // This is equivalent to :
    // for(i in ParticlesCount) : glDrawArrays(GL_TRIANGLE_STRIP, 0, 4), 
    // but faster.
    glDrawArraysInstancedARB(GL_TRIANGLE_STRIP, 0, 4, ParticlesCount);
}

void drawLight() {   
  glUniform1i(h_isLight, 1);
  safe_glEnableVertexAttribArray(h_aPosition);
  glBindBuffer(GL_ARRAY_BUFFER, LightBuffObj);
  safe_glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);    
  glUniform1i(h_isLight, 1);
    
  glm::mat4 Trans = glm::translate(glm::mat4(1.0f), lp);
  glm::mat4 Rot = glm::rotate(glm::mat4(1.0f), lightRot, glm::vec3(0, 1, 0));
  safe_glUniformMatrix4fv(h_uModelMatrix, glm::value_ptr(Rot*Trans));
    
  // bind ibo
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, LIndxBuffObj);
  glDrawElements(GL_TRIANGLES, g_LiboLen, GL_UNSIGNED_SHORT, 0);           
    
  safe_glDisableVertexAttribArray(h_aPosition);   
}

void drawRoof() {
  glUniform1i(h_isLight, 0);
  glEnable(GL_TEXTURE_2D);
  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D, 3);

  safe_glUniform1i(h_uTexUnit, 3);
    
  safe_glEnableVertexAttribArray(h_aPosition);
  glBindBuffer(GL_ARRAY_BUFFER, RoofBuffObj);
  safe_glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

  safe_glEnableVertexAttribArray(h_aTexCoord);
  glBindBuffer(GL_ARRAY_BUFFER, RTexBuffObj);
  safe_glVertexAttribPointer(h_aTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0); 
    
  // bind ibo
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RIndxBuffObj);
  glDrawElements(GL_TRIANGLES, g_RiboLen, GL_UNSIGNED_SHORT, 0);
   
  safe_glDisableVertexAttribArray(h_aPosition);
  safe_glDisableVertexAttribArray(h_aTexCoord);    
}

void drawHexPrism() {
  glUniform1i(h_isLight, 0);  
  glEnable(GL_TEXTURE_2D);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, 2);

  safe_glUniform1i(h_uTexUnit, 2);
    
  safe_glEnableVertexAttribArray(h_aPosition);
  glBindBuffer(GL_ARRAY_BUFFER, HexBuffObj);
  safe_glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

  safe_glEnableVertexAttribArray(h_aTexCoord);
  glBindBuffer(GL_ARRAY_BUFFER, HTexBuffObj);
  safe_glVertexAttribPointer(h_aTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0); 
    
  // bind ibo
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, HIndxBuffObj);
  glDrawElements(GL_TRIANGLES, g_HiboLen, GL_UNSIGNED_SHORT, 0);
    
  safe_glDisableVertexAttribArray(h_aPosition);
  safe_glDisableVertexAttribArray(h_aTexCoord);  
}

void drawCube() {
  glUniform1i(h_isLight, 0);  
  //set up the texture unit
  glEnable(GL_TEXTURE_2D);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, 0);

  safe_glUniform1i(h_uTexUnit, 0);
        
  safe_glEnableVertexAttribArray(h_aPosition);
  glBindBuffer(GL_ARRAY_BUFFER, CubeBuffObj);
  safe_glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

  safe_glEnableVertexAttribArray(h_aTexCoord);
  glBindBuffer(GL_ARRAY_BUFFER, TexBuffObj);
  safe_glVertexAttribPointer(h_aTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0); 
    
  safe_glEnableVertexAttribArray(h_aNormal);
  glBindBuffer(GL_ARRAY_BUFFER, CNormBuffObj);
  safe_glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);     
    
  // bind ibo
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CIndxBuffObj);
  glDrawElements(GL_TRIANGLES, g_CiboLen, GL_UNSIGNED_SHORT, 0);

  safe_glDisableVertexAttribArray(h_aPosition);
  safe_glDisableVertexAttribArray(h_aTexCoord);      
  safe_glDisableVertexAttribArray(h_aNormal);  
}

void drawGround() {
  glUniform1i(h_isLight, 0);
  glEnable(GL_TEXTURE_2D);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, 1);

  safe_glUniform1i(h_uTexUnit, 1);
   
  safe_glEnableVertexAttribArray(h_aPosition);
  glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
  safe_glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

  safe_glEnableVertexAttribArray(h_aTexCoord);
  glBindBuffer(GL_ARRAY_BUFFER, GTexBuffObj);
  safe_glVertexAttribPointer(h_aTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0); 

  safe_glEnableVertexAttribArray(h_aNormal);
  glBindBuffer(GL_ARRAY_BUFFER, GNormBuffObj);
  safe_glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);   
    
  // bind ibo
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
  glDrawElements(GL_TRIANGLES, g_GiboLen, GL_UNSIGNED_SHORT, 0);   
    
  safe_glDisableVertexAttribArray(h_aPosition);
  safe_glDisableVertexAttribArray(h_aTexCoord);
  safe_glDisableVertexAttribArray(h_aNormal);    
}

void determineClearColor() {
  switch(SHADER_MODE) {  
    case 1:
      glClearColor(.529f,.808f,.980f,1.0f);
      break;
    case 2:
      glClearColor(0.5f,0.5f,0.5f,1.0f);
      break;
    case 3:
      glClearColor(1.0f,1.0f,1.0f,1.0f);
//       glClearColor(0.8f,0.8f,0.8f,1.0f);
      break;
    case 4:
      glClearColor(0.0f,0.0f,0.0f,1.0f);
      break;      
  }
  
  glutPostRedisplay();
}

/* Main display function */
void Draw (void)
{
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
  determineClearColor();
    //Start our shader
    glUseProgram(ShadeProg);
    
    //Set up matrix transforms
    SetProjectionMatrix();
    SetView();
    SetModelI();

    //Set up Viewer Position
    glUniform3f(h_uViewerPos, g_trans, g_transy, 0);    
    
    //Set up light
    glUniform3f(h_uLightPos, lp.x, lp.y, lp.z);
    
    //Draw the objects
    drawGround();
    drawCube();
    drawHexPrism();
    drawRoof();    
    drawLight();
    if (MAKE_IT_RAIN == true) {
     drawRain();
    }
    glutSwapBuffers();    
    
    //Disable the shader
    glUseProgram(0);
    glDisable(GL_TEXTURE_2D);
}

void keyboard(unsigned char key, int x, int y ){
  switch( key ) {
    /* WASD keyes effect view/camera transform */
    case 'w':
      g_trans += 0.1;
      break;
    case 's':
      g_trans -= 0.1;
      break;
    case 'a':
      g_angle += 1;
      break;
    case 'd':
      g_angle -= 1;
      break;
    case 'r':
      g_transy -= .1;
      break;
    case 'f':
      g_transy += .1;
      break;      
    case 'u':
      lp.x +=.1;
      break;
    case 'j':
      lp.x -=.1;
      break;
    case 'i':
      lp.y +=.1;
      break;
    case 'k':
      lp.y -=.1;
      break;
    case 'o':
      lp.z +=.1;
      break;
    case 'l':
      lp.z -=.1;
      break;    
    case 'c':
      lp = glm::vec3(0, 5.0, 0);
      break;
    case '1':
      if (!InstallShader(textFileRead((char *)"none_vert.glsl"), textFileRead((char *)"none_frag.glsl"))) {
        printf("Error installing Rain shader!\n");
      } 
      SHADER_MODE = 1;
      break;      
    case '2':
      if (!InstallShader(textFileRead((char *)"fog_vert.glsl"), textFileRead((char *)"fog_frag.glsl"))) {
        printf("Error installing Fog shader!\n");
      }
      SHADER_MODE = 2;
      break;
    case '3':
      if (!InstallShader(textFileRead((char *)"rain_vert.glsl"), textFileRead((char *)"rain_frag.glsl"))) {
        printf("Error installing Rain shader!\n");
      } 
      SHADER_MODE = 3;      
      break;
    case '4':
      if (!InstallShader(textFileRead((char *)"snow_vert.glsl"), textFileRead((char *)"snow_frag.glsl"))) {
        printf("Error installing Rain shader!\n");
      } 
      SHADER_MODE = 4;
      break;      
    case 'z':
      ROTATE = !ROTATE;
      break;
    case 'p':
      MAKE_IT_RAIN = !MAKE_IT_RAIN;
      break;
    case 'q': case 'Q' :
      exit( EXIT_SUCCESS );
      break;
  }
  
  glutPostRedisplay();
}

//code to create a checker board texture..
void makeCheckerBoard ( int nRows, int nCols )
{
  g_pixel = new RGB[nRows * nCols];
  int c;
  
  long count = 0;
  for ( int i=0; i < nRows; i++ ) {
    for ( int j=0; j < nCols; j++ ) {
      count = j*nCols +i;
      c = (((i/8) + (j/8)) %2) * 255;
      g_pixel[count].r = c;
      g_pixel[count].g = c;
      g_pixel[count].b = c;
      
    }
  }
  /* set up the checker board texture as well */ 
  glBindTexture(GL_TEXTURE_2D, 2);
  
  (GL_TEXTURE_2D, 0, 3, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, g_pixel);
}

/* Reshape */
void ReshapeGL (int width, int height)
{
    g_width = (float)width;
    g_height = (float)height;
    glViewport (0, 0, (GLsizei)(width), (GLsizei)(height));

}

void animate(int value) {
  if (ROTATE == true) {
    if (lightRot > 360.0) {
      lightRot = -1;
    }
    lightRot++;
  }
  glutTimerFunc(25, animate, 0);
  glutPostRedisplay();  
}

int main(int argc, char** argv) {
  //initialize the window
  glutInit(&argc, argv);
  glutInitWindowPosition( 20, 20 );
  glutInitWindowSize(700, 700);
  glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutCreateWindow("My First texture maps");
  
  //set up the opengl call backs
  glutDisplayFunc(Draw);
  glutReshapeFunc(ReshapeGL);
  glutKeyboardFunc(keyboard);
  glutTimerFunc(25, animate, 0);
  cube = 0;

  Initialize();
  //load in the other image textures
  LoadTexture((char *)"stone.bmp", 0);
  LoadTexture((char *)"grass.bmp", 1);
  LoadTexture((char *)"window.bmp", 2);
  LoadTexture((char *)"roof.bmp", 3);
  //make the checker board image
  makeCheckerBoard(64, 64);

  //test the openGL version
  getGLversion();
  //install the shader
  if (!InstallShader(textFileRead((char *)"none_vert.glsl"), textFileRead((char *)"none_frag.glsl"))) {
        printf("Error installing shader!\n");
        return 0;
  }
  InitGeom();
 
  glutMainLoop();
  
}

//routines to load in a bmp files - must be 2^nx2^m and a 24bit bmp
GLvoid LoadTexture(char* image_file, int texID) { 
  
  TextureImage = (Image *) malloc(sizeof(Image));
  if (TextureImage == NULL) {
    printf("Error allocating space for image");
    exit(1);
  }
  cout << "trying to load " << image_file << endl;
  if (!ImageLoad(image_file, TextureImage)) {
    exit(1);
  }  
  /*  2d texture, level of detail 0 (normal), 3 components (red, green, blue),            */
  /*  x size from image, y size from image,                                              */    
  /*  border 0 (normal), rgb color data, unsigned byte data, data  */ 
  glBindTexture(GL_TEXTURE_2D, texID);
  glTexImage2D(GL_TEXTURE_2D, 0, 3,
    TextureImage->sizeX, TextureImage->sizeY,
    0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage->data);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST); /*  cheap scaling when image bigger than texture */    
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST); /*  cheap scaling when image smalled than texture*/
  
}


/* BMP file loader loads a 24-bit bmp file only */

/*
* getint and getshort are help functions to load the bitmap byte by byte
*/
static unsigned int getint(FILE *fp) {
  int c, c1, c2, c3;
  
  /*  get 4 bytes */ 
  c = getc(fp);  
  c1 = getc(fp);  
  c2 = getc(fp);  
  c3 = getc(fp);
  
  return ((unsigned int) c) +   
    (((unsigned int) c1) << 8) + 
    (((unsigned int) c2) << 16) +
    (((unsigned int) c3) << 24);
}

static unsigned int getshort(FILE *fp){
  int c, c1;
  
  /* get 2 bytes*/
  c = getc(fp);  
  c1 = getc(fp);
  
  return ((unsigned int) c) + (((unsigned int) c1) << 8);
}

/*  quick and dirty bitmap loader..for 24 bit bitmaps with 1 plane only.  */

int ImageLoad(char *filename, Image *image) {
  FILE *file;
  unsigned long size;                 /*  size of the image in bytes. */
  unsigned long i;                    /*  standard counter. */
  unsigned short int planes;          /*  number of planes in image (must be 1)  */
  unsigned short int bpp;             /*  number of bits per pixel (must be 24) */
  char temp;                          /*  used to convert bgr to rgb color. */
  
  /*  make sure the file is there. */
  if ((file = fopen(filename, "rb"))==NULL) {
    printf("File Not Found : %s\n",filename);
    return 0;
  }
  
  /*  seek through the bmp header, up to the width height: */
  fseek(file, 18, SEEK_CUR);
  
  /*  No 100% errorchecking anymore!!! */
  
  /*  read the width */    image->sizeX = getint (file);
  
  /*  read the height */ 
  image->sizeY = getint (file);
  
  /*  calculate the size (assuming 24 bits or 3 bytes per pixel). */
  size = image->sizeX * image->sizeY * 3;
  
  /*  read the planes */    
  planes = getshort(file);
  if (planes != 1) {
    printf("Planes from %s is not 1: %u\n", filename, planes);
    return 0;
  }
  
  /*  read the bpp */    
  bpp = getshort(file);
  if (bpp != 24) {
    printf("Bpp from %s is not 24: %u\n", filename, bpp);
    return 0;
  }
  
  /*  seek past the rest of the bitmap header. */
  fseek(file, 24, SEEK_CUR);
  
  /*  read the data.  */
  image->data = (char *) malloc(size);
  if (image->data == NULL) {
    printf("Error allocating memory for color-corrected image data");
    return 0; 
  }
  
  if ((i = fread(image->data, size, 1, file)) != 1) {
    printf("Error reading image data from %s.\n", filename);
    return 0;
  }
  
  for (i=0;i<size;i+=3) { /*  reverse all of the colors. (bgr -> rgb) */
    temp = image->data[i];
    image->data[i] = image->data[i+2];
    image->data[i+2] = temp;
  }
  
  fclose(file); /* Close the file and release the filedes */
  
  /*  we're done. */
  return 1;
}
