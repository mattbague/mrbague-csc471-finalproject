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

#include <iostream>
#include <math.h>
#include <assert.h>
#include <vector>

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

GLuint CubeBuffObj, CIndxBuffObj, TexBuffObj, CNormBuffObj;
GLuint GrndBuffObj, GIndxBuffObj, GTexBuffObj, GNormBuffObj;
GLuint HexBuffObj, HIndxBuffObj, HTexBuffObj, HNormBuffObj;
GLuint RoofBuffObj, RIndxBuffObj, RTexBuffObj, RNormBuffObj;
GLuint LightBuffObj, LIndxBuffObj;
GLuint h_isLight;
int g_CiboLen, g_GiboLen, g_HiboLen, g_RiboLen, g_LiboLen;
static float  g_width, g_height;
float g_angle = 0;
float g_trans = -10;
float g_transy = -3;
float lightRot = 0;

glm::vec3 lp = glm::vec3(0, 3.0, .81);

static const float g_groundY = -1.5;      // y coordinate of the ground
static const float g_groundSize = 20.0;   // half the ground length


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

    float rep = 8;
    
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
    1.5, -1.5, -1.5,
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
      1.0f, 0.f, 0.0f
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

  static GLfloat HexTex[] = {
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
    -base, 3.5, base,
    base, 3.5, base,
    base, 3.5, -base,
    -base, 3.5, -base,
    0.0, 4.75, 0.0    
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
   
   unsigned short idx[] = {0, 1, 2, 0, 2, 3, 7, 6, 4, 4, 6, 5, 1, 5, 6, 1, 6, 2, 0, 3, 7, 0, 7, 4};

    g_LiboLen = 24;
    glGenBuffers(1, &LightBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, LightBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CubePos), CubePos, GL_STATIC_DRAW);

    glGenBuffers(1, &LIndxBuffObj);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, LIndxBuffObj);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);
}

void InitGeom() {
  initCube();
  initGround();
  initHexPrism();
  initRoof();
  initLight();
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
       
       printf("sucessfully installed shader %d\n", ShadeProg);
       return 1;

}

/* Some OpenGL initialization */
void Initialize ()                  // Any GL Init Code
{
    // Start Of User Initialization
    glClearColor(0.5f,0.5f,0.5f,1.0f);          // We'll Clear To The Color Of The Fog ( Modified )
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

void drawLight() {   
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
    glUniform1i(h_isLight, 0);
    
    safe_glDisableVertexAttribArray(h_aPosition);   
}

void drawRoof() {
    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, 3);

    safe_glUniform1i(h_uTexUnit, 3);
    
    safe_glEnableVertexAttribArray(h_aPosition);
    glBindBuffer(GL_ARRAY_BUFFER, RoofBuffObj);
    safe_glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

    safe_glEnableVertexAttribArray(h_aTexCoord);
    glBindBuffer(GL_ARRAY_BUFFER, HTexBuffObj);
    safe_glVertexAttribPointer(h_aTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0); 
    
    // bind ibo
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RIndxBuffObj);
    glDrawElements(GL_TRIANGLES, g_RiboLen, GL_UNSIGNED_SHORT, 0);
    
    safe_glDisableVertexAttribArray(h_aPosition);
    safe_glDisableVertexAttribArray(h_aTexCoord);    
}

void drawHexPrism() {
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
//     safe_glDisableVertexAttribArray(h_aNormal);  
}

void drawGround() {
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
//     safe_glDisableVertexAttribArray(h_aNormal);    
}

/* Main display function */
void Draw (void)
{
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
  
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

void animate() {
  if (lightRot > 360.0) {
    lightRot = -1;
  }
  lightRot++;
  
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
  glutIdleFunc(animate);
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
  if (!InstallShader(textFileRead((char *)"tex_vert.glsl"), textFileRead((char *)"tex_frag.glsl"))) {
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




