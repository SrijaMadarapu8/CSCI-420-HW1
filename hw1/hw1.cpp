/*
  CSCI 420 Computer Graphics, Computer Science, USC
  Assignment 1: Height Fields with Shaders.
  C/C++ starter code

  Student username: <SRIJA MADARAPU>
*/
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "openGLHeader.h"
#include "glutHeader.h"
#include "openGLMatrix.h"
#include "imageIO.h"
#include "pipelineProgram.h"
#include "vbo.h"
#include "vao.h"

#include <iostream>
#include <cstring>

#if defined(WIN32) || defined(_WIN32)
  #ifdef _DEBUG
    #pragma comment(lib, "glew32d.lib")
  #else
    #pragma comment(lib, "glew32.lib")
  #endif
#endif

#if defined(WIN32) || defined(_WIN32)
  char shaderBasePath[1024] = SHADER_BASE_PATH;
#else
  char shaderBasePath[1024] = "../openGLHelper";
#endif

using namespace std;

int mousePos[2]; // x,y screen coordinates of the current mouse position

int leftMouseButton = 0; // 1 if pressed, 0 if not 
int middleMouseButton = 0; // 1 if pressed, 0 if not
int rightMouseButton = 0; // 1 if pressed, 0 if not

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROL_STATE;
CONTROL_STATE controlState = ROTATE;

typedef enum { P, L, T, S,M ,TM} DISPLAY_STATE;
DISPLAY_STATE displayState = P; 

int a = 0,c=1,s=0; // for animation and screenshot count

// Transformations of the terrain.
float terrainRotate[3] = { 0.0f, 0.0f, 0.0f }; 
// terrainRotate[0] gives the rotation around x-axis (in degrees)
// terrainRotate[1] gives the rotation around y-axis (in degrees)
// terrainRotate[2] gives the rotation around z-axis (in degrees)
float terrainTranslate[3] = { 0.0f, 0.0f, 0.0f };
float terrainScale[3] = { 1.0f, 1.0f, 1.0f };

//values passed to the vertexshader mode=1(smoothtriangles) mode=0(points,lines,triangles)
//grayscale=1(grayimage is rendered) grayscale=0(colorimage is rendered)
int mode=0 ,grayscale = 1;
float scale = 1.0f, exponent = 1.0f;

// Width and height of the OpenGL window, in pixels.
int windowWidth = 1280;
int windowHeight = 720;
char windowTitle[512] = "CSCI 420 Homework 1";

// Stores the image loaded from disk.
ImageIO* heightmapImage, * colorImage;

// Number of vertices in the single triangle (starter code).
//int numVertices;

//Number of vertices in the image
int numVertices, numLineVertices, numTriangleVertices,numSmoothVertices;
//alpha value
float alpha = 1.0f;
//Postions and Color array
GLuint positions, colors;

// CSCI 420 helper classes.
OpenGLMatrix matrix;
PipelineProgram* pipelineProgram = nullptr;
//VBO and VAO for mode1
VBO* vboPointVertices = nullptr, * vboLineVertices = nullptr, * vboTriangleVertices = nullptr;
VBO* vboPointColors = nullptr, * vboLineColors = nullptr, * vboTriangleColors = nullptr;
VAO* vaoPoint = nullptr, * vaoLine = nullptr, * vaoTriangle = nullptr;
//VBO and VAO for mode2
VBO* vboSmoothCenter = nullptr,*vboSmoothLeft = nullptr, *vboSmoothRight = nullptr, *vboSmoothDown = nullptr, *vboSmoothUp = nullptr;
VBO* vboSmoothColors = nullptr;
VAO* vaoSmooth = nullptr;

//for storing the extracted rgb values from given colored image
float r, g, b;

//Values for texture
GLuint textureID;
int texture_width, texture_height, texture_Channels;


void texture(int argc, char* argv[])
{
    unsigned char* data = stbi_load(argv[1], &texture_width, &texture_height, &texture_Channels, 0);
    if (!data) {
        fprintf(stderr, "Failed to load texture\n");
        return;
    }
    // Create OpenGL texture
    
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load texture data into OpenGL
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_width, texture_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
}



//funtion to extract rgb values from given colored image location(i,j)
void rgbcolor(int i, int j)
{
    r = colorImage->getPixel(i, j, 0);
    r = (r / 255.0);
    g = colorImage->getPixel(i, j, 1);
    g = (g / 255.0);
    b = colorImage->getPixel(i, j, 2);
    b = (b / 255.0);
}

// Write a screenshot to the specified filename.
void saveScreenshot(const char * filename)
{
  unsigned char * screenshotData = new unsigned char[windowWidth * windowHeight * 3];
  glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, screenshotData);

  ImageIO screenshotImg(windowWidth, windowHeight, 3, screenshotData);

  if (screenshotImg.save(filename, ImageIO::FORMAT_JPEG) == ImageIO::OK)
    cout << "File " << filename << " saved successfully." << endl;
  else cout << "Failed to save file " << filename << '.' << endl;

  delete [] screenshotData;
}

void idleFunc()
{
  // Do some stuff... 
  //save the screenshots to disk (to make the animation).
    if (a == 1)
    {
        if (c <= 40)
        {
            
            if (c <=20)
            {
                terrainTranslate[2] += 0.01f;
                displayState = P;
                mode = 0;
                Sleep(500);
                string str ="animation/"+ to_string(c) + ".jpg";
                saveScreenshot(str.c_str());
                c++;
            }
            else
            {
                if (c <= 30)
                {
                    terrainScale[0] += 0.05f;
                    terrainScale[1] += 0.05f;
                    terrainScale[2] += 0.05f;
                    displayState = P;
                    mode = 2;
                    string str = "animation/" + to_string(c) + ".jpg";
                    saveScreenshot(str.c_str());
                    printf(" %d ", c);
                    c++;
                }
                else
                {
                    terrainScale[0] -= 0.05f;
                    terrainScale[1] -= 0.05f;
                    terrainScale[2] -= 0.05f;
                    displayState = P;
                    mode = 2;
                    string str = "animation/" + to_string(c) + ".jpg";
                    saveScreenshot(str.c_str());
                    c++;
                }
                
            }
        }  
        else if (c <= 80)
        {

            if (c <=60)
            {
                terrainTranslate[2] += 0.01f;
                displayState = L;
                mode = 0;
                Sleep(500);
                string str = "animation/" + to_string(c) + ".jpg";
                saveScreenshot(str.c_str());
                c++;
            }
            else
            {
                if (c <= 110)
                {
                    terrainScale[0] += 0.02f;
                    terrainScale[1] += 0.02f;
                    terrainScale[2] += 0.02f;
                    displayState = L;
                    mode = 2;
                    Sleep(500);
                    string str = "animation/" + to_string(c) + ".jpg";
                    saveScreenshot(str.c_str());
                    c++;
                }
                else
                {
                    terrainScale[0] -= 0.05f;
                    terrainScale[1] -= 0.05f;
                    terrainScale[2] -= 0.05f;
                    displayState = L;
                    mode = 2;
                    Sleep(500);
                    string str = "animation/" + to_string(c) + ".jpg";
                    saveScreenshot(str.c_str());
                    c++;
                }
                
            }
        }
        else if (c <= 120)
        {

            if (c <= 100)
            {
                terrainTranslate[2] += 0.01f;
                displayState = T;
                mode = 0;
                Sleep(500);
                string str = "animation/" + to_string(c) + ".jpg";
                saveScreenshot(str.c_str());
                c++;
            }
            else
            {
                if (c <= 110)
                {
                    terrainScale[0] += 0.02f;
                    terrainScale[1] += 0.02f;
                    terrainScale[2] += 0.02f;
                    displayState = T;
                    mode = 2;
                    Sleep(500);
                    string str = "animation/" + to_string(c) + ".jpg";
                    saveScreenshot(str.c_str());
                    c++;
                }
                else
                {
                    terrainScale[0] -= 0.08f;
                    terrainScale[1] -= 0.08f;
                    terrainScale[2] -= 0.08f;
                    displayState = T;
                    mode = 2;
                    Sleep(500);
                    string str = "animation/" + to_string(c) + ".jpg";
                    saveScreenshot(str.c_str());
                    c++;
                }
                
            }
        }
        else if (c <= 160)
        {

        if (c <= 140)
        {
            terrainTranslate[2] -= 0.01f;
            displayState = S;
            mode = 1;
            grayscale = 1;
            Sleep(500);
            string str = "animation/" + to_string(c) + ".jpg";
            saveScreenshot(str.c_str());
            c++;
        }
        else
        {
            if (c <= 150)
            {
                terrainRotate[0] += 0.6f;
                displayState = S;
                mode = 1;
                grayscale = 1;
                Sleep(500);
                string str = "animation/" + to_string(c) + ".jpg";
                saveScreenshot(str.c_str());
                c++;
            }
            else
            {
                terrainRotate[1] += 0.6f;
                displayState = S;
                mode = 1;
                grayscale = 1;
                Sleep(500);
                string str = "animation/" + to_string(c) + ".jpg";
                saveScreenshot(str.c_str());
                c++;
            }
            
        }
        }
        else if (c <= 260)
        {

        if (c <= 210)
        {
            if (c <= 185)
            {
                displayState = S;
                mode = 1;
                grayscale = 0;
                scale = scale * 2.0f;
                Sleep(500);
                string str = "animation/" + to_string(c) + ".jpg";
                saveScreenshot(str.c_str());
                c++;
            }
            else
            {
                displayState = S;
                mode = 1;
                grayscale = 0;
                scale = scale / 2.0f;
                Sleep(500);
                string str = "animation/" + to_string(c) + ".jpg";
                saveScreenshot(str.c_str());
                c++;
            }
        }
        else 
        {
            if (c <= 235)
            {
                displayState = S;
                mode = 1;
                grayscale = 0;
                exponent = exponent / 2.0f;
                Sleep(500);
                string str = "animation/" + to_string(c) + ".jpg";
                saveScreenshot(str.c_str());
                c++;
            }
            else
            {
                displayState = S;
                mode = 1;
                grayscale = 0;
                exponent = exponent * 2.0f;
                Sleep(500);
                exponent = exponent * 2.0f;
                string str = "animation/" + to_string(c) + ".jpg";
                saveScreenshot(str.c_str());
                c++;
            }
            
        }
        }
    }
  
    else if (s == 1)
    {

        if (c <=5)
        {
            
            terrainTranslate[2] += 0.01f;
            displayState = P;
            mode = 0;
            Sleep(500);
            string str = "animation/" + to_string(c) + ".jpg";
            saveScreenshot(str.c_str());
            c++;
        }
        else if (c <= 10)
        {
            terrainTranslate[2] += 0.01f;
            displayState = L;
            mode = 0;
            Sleep(500);
            string str = "animation/" + to_string(c) + ".jpg";
            saveScreenshot(str.c_str());
            c++;
        }
        else if (c <= 15)
        {
            terrainTranslate[2] += 0.01f;
            displayState = T;
            mode = 0;
            Sleep(500);
            string str = "animation/" + to_string(c) + ".jpg";
            saveScreenshot(str.c_str());
            c++;
        }
    }

  // Notify GLUT that it should call displayFunc.
  glutPostRedisplay();
}

void reshapeFunc(int w, int h)
{
  glViewport(0, 0, w, h);

  // When the window has been resized, we need to re-set our projection matrix.
  matrix.SetMatrixMode(OpenGLMatrix::Projection);
  matrix.LoadIdentity();
  // You need to be careful about setting the zNear and zFar. 
  // Anything closer than zNear, or further than zFar, will be culled.
  const float zNear = 0.1f;
  const float zFar = 10000.0f;
  const float humanFieldOfView = 60.0f;
  matrix.Perspective(humanFieldOfView, 1.0f * w / h, zNear, zFar);
}

void mouseMotionDragFunc(int x, int y)
{
  // Mouse has moved, and one of the mouse buttons is pressed (dragging).

  // the change in mouse position since the last invocation of this function
  int mousePosDelta[2] = { x - mousePos[0], y - mousePos[1] };

  switch (controlState)
  {
    // translate the terrain
    case TRANSLATE:
      if (leftMouseButton)
      {
        // control x,y translation via the left mouse button
        terrainTranslate[0] += mousePosDelta[0] * 0.01f;
        terrainTranslate[1] -= mousePosDelta[1] * 0.01f;
      }
      if (middleMouseButton)
      {
        // control z translation via the middle mouse button
        terrainTranslate[2] += mousePosDelta[1] * 0.01f;
      }
      break;

    // rotate the terrain
    case ROTATE:
      if (leftMouseButton)
      {
        // control x,y rotation via the left mouse button
        terrainRotate[0] += mousePosDelta[1];
        terrainRotate[1] += mousePosDelta[0];
      }
      if (middleMouseButton)
      {
        // control z rotation via the middle mouse button
        terrainRotate[2] += mousePosDelta[1];
      }
      break;

    // scale the terrain
    case SCALE:
      if (leftMouseButton)
      {
        // control x,y scaling via the left mouse button
        terrainScale[0] *= 1.0f + mousePosDelta[0] * 0.01f;
        terrainScale[1] *= 1.0f - mousePosDelta[1] * 0.01f;
      }
      if (middleMouseButton)
      {
        // control z scaling via the middle mouse button
        terrainScale[2] *= 1.0f - mousePosDelta[1] * 0.01f;
      }
      break;
  }

  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void mouseMotionFunc(int x, int y)
{
  // Mouse has moved.
  // Store the new mouse position.
  mousePos[0] = x;
  mousePos[1] = y;
}

void mouseButtonFunc(int button, int state, int x, int y)
{
  // A mouse button has has been pressed or depressed.

  // Keep track of the mouse button state, in leftMouseButton, middleMouseButton, rightMouseButton variables.
  switch (button)
  {
    case GLUT_LEFT_BUTTON:
      leftMouseButton = (state == GLUT_DOWN);
    break;

    case GLUT_MIDDLE_BUTTON:
      middleMouseButton = (state == GLUT_DOWN);
    break;

    case GLUT_RIGHT_BUTTON:
      rightMouseButton = (state == GLUT_DOWN);
    break;
  }

  // Keep track of whether CTRL and SHIFT keys are pressed.
  switch (glutGetModifiers())
  {
    case GLUT_ACTIVE_CTRL:
      controlState = TRANSLATE;
    break;

    case GLUT_ACTIVE_SHIFT:
      controlState = SCALE;
    break;

    // If CTRL and SHIFT are not pressed, we are in rotate mode.
    default:
      controlState = ROTATE;
    break;
  }

  // Store the new mouse position.
  mousePos[0] = x;
  mousePos[1] = y;
}

void keyboardFunc(unsigned char key, int x, int y)
{
  switch (key)
  {
    case 27: // ESC key
      exit(0); // exit the program
    break;

    case ' ':
      cout << "You pressed the spacebar." << endl;
    break;

    case 'x':
      // Take a screenshot.
      saveScreenshot("animation/screenshot.jpg");
    break;
    case '1':
        //points
        displayState = P;
        mode = 0;
        break;
    case '!':
        //points colored
        displayState = P;
        mode = 2;
        break;
    case '2':
        //Lines
        displayState = L;
        mode = 0;
        break;
    case '@':
        //Lines colored
        displayState = L;
        mode = 2;
        break;
    case '3':
        //Triangles
        displayState = T;
        mode = 0;
        break;
    case '#':
        //Triangle colored
        displayState = T;
        mode = 2;
        break;
    case '4':
        //Smooth triangle
        displayState = S;
        mode = 1;
        grayscale = 0;
        break;
    case '$':
        //Smooth triangle colored
        displayState = S;
        mode = 1;
        grayscale = 1;
        break;
    case '5':
        //wireframe on top of solid triangles
        displayState = M;
        mode = 0;
        break;
    case '6':
        //texture mapping
        displayState = TM;
        mode = 0;
        break;
    case '+':
        displayState = S;
        mode = 1;
        scale = scale * 2.0f;
        break;
    case '-':
        displayState = S;
        mode = 1;
        scale = scale/2.0f;
        break;
    case '0':
        displayState = S;
        mode = 1;
        exponent = exponent / 2.0f;
        break;
    case '9':
        displayState = S;
        mode = 1;
        exponent = exponent * 2.0f;
        break;
    case 'a':
        //for animation
        a = 1;
        break;
    case 's':
        //for animation where colors are taken from color image
        s = 1;
        break;
  }
}

void displayFunc()
{
  // This function performs the actual rendering.

  // First, clear the screen.
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Set up the camera position, focus point, and the up vector.
  matrix.SetMatrixMode(OpenGLMatrix::ModelView);
  matrix.LoadIdentity();
  matrix.LookAt(1.0, 1.0, 1.0,
                0.6, 0.3, 0.0,
                0.0, 1.0, 0.0);

  // In here, you can do additional modeling on the object, such as performing translations, rotations and scales.
  // ...
  matrix.Translate(terrainTranslate[0], terrainTranslate[1], terrainTranslate[2]);
  matrix.Rotate(terrainRotate[0], 1.0, 0.0, 0.0);
  matrix.Rotate(terrainRotate[1], 0.0, 1.0, 0.0);
  matrix.Rotate(terrainRotate[2], 0.0, 0.0, 1.0);
  matrix.Scale(terrainScale[0], terrainScale[1], terrainScale[2]);
  // Read the current modelview and projection matrices from our helper class.
  // The matrices are only read here; nothing is actually communicated to OpenGL yet.
  float modelViewMatrix[16];
  matrix.SetMatrixMode(OpenGLMatrix::ModelView);
  matrix.GetMatrix(modelViewMatrix);

  float projectionMatrix[16];
  matrix.SetMatrixMode(OpenGLMatrix::Projection);
  matrix.GetMatrix(projectionMatrix);

  // Upload the modelview and projection matrices to the GPU. Note that these are "uniform" variables.
  // Important: these matrices must be uploaded to *all* pipeline programs used.
  // In hw1, there is only one pipeline program, but in hw2 there will be several of them.
  // In such a case, you must separately upload to *each* pipeline program.
  // Important: do not make a typo in the variable name below; otherwise, the program will malfunction.
  pipelineProgram->SetUniformVariableMatrix4fv("modelViewMatrix", GL_FALSE, modelViewMatrix);
  pipelineProgram->SetUniformVariableMatrix4fv("projectionMatrix", GL_FALSE, projectionMatrix);

  //Set variables passed to the vertexshader
  pipelineProgram->SetUniformVariablei("mode", mode);
  pipelineProgram->SetUniformVariablef("scale", scale);
  pipelineProgram->SetUniformVariablef("exponent", exponent);
  pipelineProgram->SetUniformVariablei("grayscale", grayscale);
  // Execute the rendering.
    // Bind the VAO that we want to render. Remember, one object = one VAO.
    //vao->Bind();
    //glDrawArrays(GL_TRIANGLES, 0, numVertices); // Render the VAO, by rendering "numVertices", starting from vertex 0.

  switch (displayState)
  {
  case P:
      vaoPoint->Bind();
      glDrawArrays(GL_POINTS, 0, numVertices);
      break;
  case L:
      vaoLine->Bind();
      glDrawArrays(GL_LINES, 0, numLineVertices);
      break;
  case T:
      vaoTriangle->Bind();
      glDrawArrays(GL_TRIANGLES, 0, numTriangleVertices);
      break;
  case S:
      vaoSmooth->Bind();
      glDrawArrays(GL_TRIANGLES, 0, numTriangleVertices);
      break;
  case M:
      
      mode = 2;
      pipelineProgram->SetUniformVariablei("mode", mode);
      vaoTriangle->Bind();
      glDrawArrays(GL_TRIANGLES, 0, numTriangleVertices);
      glEnable(GL_POLYGON_OFFSET_FILL);
      glPolygonOffset(1.0f, 1.0f);
      mode = 0;
      pipelineProgram->SetUniformVariablei("mode", mode);
      vaoLine->Bind();
      glDrawArrays(GL_LINES, 0, numLineVertices);
      
      break;
  case  TM:
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, textureID);
      
      vaoTriangle->Bind();
      glDrawArrays(GL_TRIANGLES, 0, numTriangleVertices);
      break;
  default:
      break;
  }
  // Swap the double-buffers.
  glutSwapBuffers();
}

void initScene(int argc, char *argv[])
{
  // Load the image from a jpeg disk file into main memory.
  heightmapImage = new ImageIO();
  if (heightmapImage->loadJPEG(argv[1]) != ImageIO::OK)
  {
    cout << "Error reading image " << argv[1] << "." << endl;
    exit(EXIT_FAILURE);
  }
  //load color image if given
  if (argc == 3)
  {
      colorImage = new ImageIO();
      if (colorImage->loadJPEG(argv[2]) != ImageIO::OK)
      {
          cout << "Error reading image " << argv[2] << "." << endl;
          exit(EXIT_FAILURE);
      }
  }

  // Set the background color.
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black color.

  // Enable z-buffering (i.e., hidden surface removal using the z-buffer algorithm).
  glEnable(GL_DEPTH_TEST);

  // Create a pipeline program. This operation must be performed BEFORE we initialize any VAOs.
  // A pipeline program contains our shaders. Different pipeline programs may contain different shaders.
  // In this homework, we only have one set of shaders, and therefore, there is only one pipeline program.
  // In hw2, we will need to shade different objects with different shaders, and therefore, we will have
  // several pipeline programs (e.g., one for the rails, one for the ground/sky, etc.).
  pipelineProgram = new PipelineProgram(); // Load and set up the pipeline program, including its shaders.
  // Load and set up the pipeline program, including its shaders.
  

  if (pipelineProgram->BuildShadersFromFiles(shaderBasePath, "vertexShader.glsl", "fragmentShader.glsl") != 0)
  {
    cout << "Failed to build the pipeline program." << endl;
    throw 1;
  } 
  cout << "Successfully built the pipeline program." << endl;
    
  // Bind the pipeline program that we just created. 
  // The purpose of binding a pipeline program is to activate the shaders that it contains, i.e.,
  // any object rendered from that point on, will use those shaders.
  // When the application starts, no pipeline program is bound, which means that rendering is not set up.
  // So, at some point (such as below), we need to bind a pipeline program.
  // From that point on, exactly one pipeline program is bound at any moment of time.
  
  pipelineProgram->Bind();

  // Prepare the triangle position and color data for the VBO. 
  // The code below sets up a single triangle (3 vertices).
  // The triangle will be rendered using GL_TRIANGLES (in displayFunc()).

  //numVertices = 3; // This must be a global variable, so that we know how many vertices to render in glDrawArrays.

  // Vertex positions.
  //float * positions = (float*) malloc (numVertices * 3 * sizeof(float)); // 3 floats per vertex, i.e., x,y,z
  //positions[0] = 0.0; positions[1] = 0.0; positions[2] = 0.0; // (x,y,z) coordinates of the first vertex
  //positions[3] = 0.0; positions[4] = 1.0; positions[5] = 0.0; // (x,y,z) coordinates of the second vertex
  //positions[6] = 1.0; positions[7] = 0.0; positions[8] = 0.0; // (x,y,z) coordinates of the third vertex

  // Vertex colors.
  //float * colors = (float*) malloc (numVertices * 4 * sizeof(float)); // 4 floats per vertex, i.e., r,g,b,a
  //colors[0] = 0.0; colors[1] = 0.0;  colors[2] = 1.0;  colors[3] = 1.0; // (r,g,b,a) channels of the first vertex
  //colors[4] = 1.0; colors[5] = 0.0;  colors[6] = 0.0;  colors[7] = 1.0; // (r,g,b,a) channels of the second vertex
  //colors[8] = 0.0; colors[9] = 1.0; colors[10] = 0.0; colors[11] = 1.0; // (r,g,b,a) channels of the third vertex

  // Create the VBOs. 
  // We make a separate VBO for vertices and colors. 
  // This operation must be performed BEFORE we initialize any VAOs.
  //vboVertices = new VBO(numVertices, 3, positions, GL_STATIC_DRAW); // 3 values per position
  //vboColors = new VBO(numVertices, 4, colors, GL_STATIC_DRAW); // 4 values per color

  //VBO for Mode1 Case GL_POINTS
  int imgHeight = heightmapImage->getHeight();
  int imgWidth = heightmapImage->getWidth();

  numVertices = imgHeight * imgWidth;
  float* pointPositions = (float*)malloc(numVertices * 3 * sizeof(float));
  float* pointColors = (float*)malloc(numVertices * 4 * sizeof(float));

  for (int i = 0; i < imgWidth; i++)
  {
      for (int j = 0; j < imgHeight; j++)
      {
          
          pointPositions[(j * imgWidth + i) * 3] = 1.0 * i / (static_cast<int64_t>(imgWidth) - 1);
          pointPositions[(j * imgWidth + i) * 3 + 1] = 0.3 * heightmapImage->getPixel(i, j, 0) / 255.0f;
          pointPositions[(j * imgWidth + i) * 3 + 2] = 1.0 * -j / (static_cast<int64_t>(imgHeight) - 1);
          //if a 2nd image is given the rgb values from the image are considered
          if (argc == 3)
          {
              rgbcolor(i, j);
              pointColors[(j * imgWidth + i) * 4] = r;
              pointColors[(j * imgWidth + i) * 4 + 1] = g;
              pointColors[(j * imgWidth + i) * 4 + 2] = b;
              pointColors[(j * imgWidth + i) * 4 + 3] = alpha;
          }
          else
          {
              pointColors[(j * imgWidth + i) * 4] = pointPositions[(j * imgWidth + i) * 3 + 1];
              pointColors[(j * imgWidth + i) * 4 + 1] = pointPositions[(j * imgWidth + i) * 3 + 1];
              pointColors[(j * imgWidth + i) * 4 + 2] = pointPositions[(j * imgWidth + i) * 3 + 1];
              pointColors[(j * imgWidth + i) * 4 + 3] = alpha;
          }
      }
  }

  vboPointVertices = new VBO(numVertices, 3, pointPositions, GL_STATIC_DRAW);
  vboPointColors = new VBO(numVertices, 4, pointColors, GL_STATIC_DRAW);

  //VBO for Mode1 Case GL_LINES
  numLineVertices = (imgHeight - 1) * 2 * imgWidth + (imgWidth - 1) * 2 * imgHeight;

  float* linePositions = (float*)malloc(numLineVertices * 3 * sizeof(float));
  float* lineColors = (float*)malloc(numLineVertices * 4 * sizeof(float));

  int lineIndex = 0; 

  for (int j = 0; j < imgHeight; j++)
  {
      for (int i = 0; i < imgWidth - 1; i++)
      {
          
          linePositions[lineIndex * 3] = 1.0 * i / (static_cast<int64_t>(imgWidth) - 1);
          linePositions[lineIndex * 3 + 1] = 0.3 * heightmapImage->getPixel(i, j, 0) / 255.0f;
          linePositions[lineIndex * 3 + 2] = 1.0 * -j / (static_cast<int64_t>(imgHeight) - 1);
          //if a 2nd image is given the rgb values from the image are considered

          if (argc == 3)
          {
              rgbcolor(i, j);
              lineColors[lineIndex * 4] = r;
              lineColors[lineIndex * 4 + 1] =g;
              lineColors[lineIndex * 4 + 2] = b;
              lineColors[lineIndex * 4 + 3] = alpha;
              lineIndex++;
          }
          else
          {
              lineColors[lineIndex * 4] = linePositions[lineIndex * 3 + 1];
              lineColors[lineIndex * 4 + 1] = linePositions[lineIndex * 3 + 1];
              lineColors[lineIndex * 4 + 2] = linePositions[lineIndex * 3 + 1];
              lineColors[lineIndex * 4 + 3] = alpha;
              lineIndex++;
          }

          linePositions[lineIndex * 3] = 1.0 * (i + 1) / (static_cast<int64_t>(imgWidth) - 1);
          linePositions[lineIndex * 3 + 1] = 0.3 * heightmapImage->getPixel(i + 1, j, 0) / 255.0f;
          linePositions[lineIndex * 3 + 2] = 1.0 * -j / (static_cast<int64_t>(imgHeight) - 1);
          
          if (argc == 3)
          {
              rgbcolor(i+1, j);
              lineColors[lineIndex * 4] = r;
              lineColors[lineIndex * 4 + 1] = g;
              lineColors[lineIndex * 4 + 2] = b;
              lineColors[lineIndex * 4 + 3] = alpha;
              lineIndex++;
          }
          else
          {
              lineColors[lineIndex * 4] = linePositions[lineIndex * 3 + 1];
              lineColors[lineIndex * 4 + 1] = linePositions[lineIndex * 3 + 1];
              lineColors[lineIndex * 4 + 2] = linePositions[lineIndex * 3 + 1];
              lineColors[lineIndex * 4 + 3] = alpha;
              lineIndex++;
          }
      }
  }
  
  for (int i = 0; i < imgWidth; i++)
  {
      for (int j = 0; j < imgHeight - 1; j++)
      {
          linePositions[lineIndex * 3] = 1.0 * i / (static_cast<int64_t>(imgWidth) - 1);
          linePositions[lineIndex * 3 + 1] = 0.3 * heightmapImage->getPixel(i, j, 0) / 255.0f;
          linePositions[lineIndex * 3 + 2] = 1.0 * -j / (static_cast<int64_t>(imgHeight) - 1);
          
          if (argc == 3)
          {
              rgbcolor(i, j);
              lineColors[lineIndex * 4] = r;
              lineColors[lineIndex * 4 + 1] = g;
              lineColors[lineIndex * 4 + 2] = b;
              lineColors[lineIndex * 4 + 3] = alpha;
              lineIndex++;
          }
          else
          {
              lineColors[lineIndex * 4] = linePositions[lineIndex * 3 + 1];
              lineColors[lineIndex * 4 + 1] = linePositions[lineIndex * 3 + 1];
              lineColors[lineIndex * 4 + 2] = linePositions[lineIndex * 3 + 1];
              lineColors[lineIndex * 4 + 3] = alpha;
              lineIndex++;
          }

          linePositions[lineIndex * 3] = 1.0 * i / (static_cast<int64_t>(imgWidth) - 1);
          linePositions[lineIndex * 3 + 1] = 0.3 * heightmapImage->getPixel(i, j + 1, 0) / 255.0f;
          linePositions[lineIndex * 3 + 2] = 1.0 * -(j + 1) / (static_cast<int64_t>(imgHeight) - 1);
          
          if (argc == 3)
          {
              rgbcolor(i, j+1);
              lineColors[lineIndex * 4] = r;
              lineColors[lineIndex * 4 + 1] = g;
              lineColors[lineIndex * 4 + 2] = b;
              lineColors[lineIndex * 4 + 3] = alpha;
              lineIndex++;
          }
          else
          {
              lineColors[lineIndex * 4] = linePositions[lineIndex * 3 + 1];
              lineColors[lineIndex * 4 + 1] = linePositions[lineIndex * 3 + 1];
              lineColors[lineIndex * 4 + 2] = linePositions[lineIndex * 3 + 1];
              lineColors[lineIndex * 4 + 3] = alpha;
              lineIndex++;
          }
      }
  }

  vboLineVertices = new VBO(numLineVertices, 3, linePositions, GL_STATIC_DRAW);
  vboLineColors = new VBO(numLineVertices, 4, lineColors, GL_STATIC_DRAW);

  //VBO for Mode1 Case GL_TRIANGL
  numTriangleVertices = 2 * (imgWidth - 1) * (imgHeight - 1) * 3;

  int triangleIndex = 0;

  float* trianglePositions = (float*)malloc(numTriangleVertices * 3 * sizeof(float));
  float* triangleColors = (float*)malloc(numTriangleVertices * 4 * sizeof(float));

  for (int j = 0; j < imgHeight - 1; j++)
  {
      for (int i = 0; i < imgWidth - 1; i++)
      {
          //Vertex 1
          trianglePositions[triangleIndex * 3] = 1.0 * i / (static_cast<int64_t>(imgWidth) - 1);
          trianglePositions[triangleIndex * 3 + 1] = 0.3 * heightmapImage->getPixel(i, j, 0) / 255.0f;
          trianglePositions[triangleIndex * 3 + 2] = 1.0 * -j / (static_cast<int64_t>(imgHeight) - 1);
          //if a 2nd image is given the rgb values from the image are considered
          if (argc == 3)
          {
              rgbcolor(i, j);
              triangleColors[triangleIndex * 4] =r;
              triangleColors[triangleIndex * 4 + 1] = g ;
              triangleColors[triangleIndex * 4 + 2] = b;
              triangleColors[triangleIndex * 4 + 3] = alpha;
              triangleIndex++;
          }
          else {
              
              triangleColors[triangleIndex * 4] = trianglePositions[triangleIndex * 3 + 1];
              triangleColors[triangleIndex * 4 + 1] = trianglePositions[triangleIndex * 3 + 1];
              triangleColors[triangleIndex * 4 + 2] = trianglePositions[triangleIndex * 3 + 1];
              triangleColors[triangleIndex * 4 + 3] = alpha;
              triangleIndex++;
          }

          //Vertex 2
          trianglePositions[triangleIndex * 3] = 1.0 * (i + 1) / (static_cast<int64_t>(imgWidth) - 1);
          trianglePositions[triangleIndex * 3 + 1] = 0.3 * heightmapImage->getPixel(i + 1, j, 0) / 255.0f;
          trianglePositions[triangleIndex * 3 + 2] = 1.0 * -j / (static_cast<int64_t>(imgHeight) - 1);

          if (argc == 3)
          {
              rgbcolor(i + 1, j);
              triangleColors[triangleIndex * 4] = r;
              triangleColors[triangleIndex * 4 + 1] = g;
              triangleColors[triangleIndex * 4 + 2] = b;
              triangleColors[triangleIndex * 4 + 3] = alpha;
              triangleIndex++;
          }
          else {
              triangleColors[triangleIndex * 4] = trianglePositions[triangleIndex * 3 + 1];
              triangleColors[triangleIndex * 4 + 1] = trianglePositions[triangleIndex * 3 + 1];
              triangleColors[triangleIndex * 4 + 2] = trianglePositions[triangleIndex * 3 + 1];
              triangleColors[triangleIndex * 4 + 3] = alpha;
              triangleIndex++;
          }

          //Vertex 3
          trianglePositions[triangleIndex * 3] = 1.0 * i / (static_cast<int64_t>(imgWidth) - 1);
          trianglePositions[triangleIndex * 3 + 1] = 0.3 * heightmapImage->getPixel(i, j + 1, 0) / 255.0f;
          trianglePositions[triangleIndex * 3 + 2] = 1.0 * -(j + 1) / (static_cast<int64_t>(imgHeight) - 1);

          if (argc == 3)
          {
              rgbcolor(i, j + 1);
              triangleColors[triangleIndex * 4] = r;
              triangleColors[triangleIndex * 4 + 1] = g;
              triangleColors[triangleIndex * 4 + 2] = b;
              triangleColors[triangleIndex * 4 + 3] = alpha;
              triangleIndex++;
          }
          else {
              triangleColors[triangleIndex * 4] = trianglePositions[triangleIndex * 3 + 1];
              triangleColors[triangleIndex * 4 + 1] = trianglePositions[triangleIndex * 3 + 1];
              triangleColors[triangleIndex * 4 + 2] = trianglePositions[triangleIndex * 3 + 1];
              triangleColors[triangleIndex * 4 + 3] = alpha;
              triangleIndex++;
          }

          //Vertex 4
          trianglePositions[triangleIndex * 3] = 1.0 * i / (static_cast<int64_t>(imgWidth) - 1);
          trianglePositions[triangleIndex * 3 + 1] = 0.3 * heightmapImage->getPixel(i, j + 1, 0) / 255.0f;
          trianglePositions[triangleIndex * 3 + 2] = 1.0 * -(j + 1) / (static_cast<int64_t>(imgHeight) - 1);

          if (argc == 3)
          {
              rgbcolor(i, j + 1);
              triangleColors[triangleIndex * 4] = r;
              triangleColors[triangleIndex * 4 + 1] = g;
              triangleColors[triangleIndex * 4 + 2] = b;
              triangleColors[triangleIndex * 4 + 3] = alpha;
              triangleIndex++;
          }
          else {
              triangleColors[triangleIndex * 4] = trianglePositions[triangleIndex * 3 + 1];
              triangleColors[triangleIndex * 4 + 1] = trianglePositions[triangleIndex * 3 + 1];
              triangleColors[triangleIndex * 4 + 2] = trianglePositions[triangleIndex * 3 + 1];
              triangleColors[triangleIndex * 4 + 3] = alpha;
              triangleIndex++;
          }

          //Vertex 5
          trianglePositions[triangleIndex * 3] = 1.0 * (i + 1) / (static_cast<int64_t>(imgWidth) - 1);
          trianglePositions[triangleIndex * 3 + 1] = 0.3 * heightmapImage->getPixel(i + 1, j, 0) / 255.0f;
          trianglePositions[triangleIndex * 3 + 2] = 1.0 * -j / (static_cast<int64_t>(imgHeight) - 1);

          if (argc == 3)
          {
              rgbcolor(i + 1, j);
              triangleColors[triangleIndex * 4] = r;
              triangleColors[triangleIndex * 4 + 1] = g;
              triangleColors[triangleIndex * 4 + 2] = b;
              triangleColors[triangleIndex * 4 + 3] = alpha;
              triangleIndex++;
          }
          else {
              triangleColors[triangleIndex * 4] = trianglePositions[triangleIndex * 3 + 1];
              triangleColors[triangleIndex * 4 + 1] = trianglePositions[triangleIndex * 3 + 1];
              triangleColors[triangleIndex * 4 + 2] = trianglePositions[triangleIndex * 3 + 1];
              triangleColors[triangleIndex * 4 + 3] = alpha;
              triangleIndex++;
          }

          //Vertex 6
          trianglePositions[triangleIndex * 3] = 1.0 * (i + 1) / (static_cast<int64_t>(imgWidth) - 1);
          trianglePositions[triangleIndex * 3 + 1] = 0.3 * heightmapImage->getPixel(i + 1, j + 1, 0) / 255.0f;
          trianglePositions[triangleIndex * 3 + 2] = 1.0 * -(j + 1) / (static_cast<int64_t>(imgHeight) - 1);

          if (argc == 3)
          {
              rgbcolor(i + 1, j + 1);
              triangleColors[triangleIndex * 4] = r;
              triangleColors[triangleIndex * 4 + 1] = g;
              triangleColors[triangleIndex * 4 + 2] = b;
              triangleColors[triangleIndex * 4 + 3] = alpha;
              triangleIndex++;
          }
          else {
              triangleColors[triangleIndex * 4] = trianglePositions[triangleIndex * 3 + 1];
              triangleColors[triangleIndex * 4 + 1] = trianglePositions[triangleIndex * 3 + 1];
              triangleColors[triangleIndex * 4 + 2] = trianglePositions[triangleIndex * 3 + 1];
              triangleColors[triangleIndex * 4 + 3] = alpha;
              triangleIndex++;
          }
      }
  }
  
  vboTriangleVertices = new VBO(numTriangleVertices, 3, trianglePositions, GL_STATIC_DRAW);
  vboTriangleColors = new VBO(numTriangleVertices, 4, triangleColors, GL_STATIC_DRAW);

  //VBO for Mode2 

  float* smooth_centerPositions = (float*)malloc(numTriangleVertices * 3 * sizeof(float));
  float* smooth_centerColors = (float*)malloc(numTriangleVertices * 4 * sizeof(float));

  float* smooth_leftPositions = (float*)malloc(numTriangleVertices * 3 * sizeof(float));
  float* smooth_rightPositions = (float*)malloc(numTriangleVertices * 3 * sizeof(float));
  float* smooth_upPositions = (float*)malloc(numTriangleVertices * 3 * sizeof(float));
  float* smooth_downPositions = (float*)malloc(numTriangleVertices * 3 * sizeof(float));
  

  int smoothIndex = 0;

  for (int j = 0; j < imgHeight - 1; j++)
  {
      for (int i = 0; i < imgWidth - 1; i++)
      {
          //to make sure the values are in bound
          int i_minus_1 = std::max(0, i - 1);
          int j_minus_1 = std::max(0, j - 1);
          int i_plus_2 = std::min(imgWidth - 1, i + 2);
          int j_plus_2 = std::min(imgHeight - 1, j + 2);
          int i_plus_1 = std::min(imgWidth - 1, i + 1);
          int j_plus_1 = std::min(imgHeight - 1, j + 1);

          //Vertice 1 for (i,j)
          smooth_centerPositions[smoothIndex * 3] = 1.0 * i / (static_cast<int64_t>(imgWidth) - 1);
          smooth_centerPositions[smoothIndex * 3 + 1] = 1.0 * heightmapImage->getPixel(i, j, 0) / 255.0f;
          smooth_centerPositions[smoothIndex * 3 + 2] = 1.0 * -j / (static_cast<int64_t>(imgHeight) - 1);
         
          smooth_upPositions[smoothIndex * 3] = 1.0 * i_plus_1 / (static_cast<int64_t>(imgWidth) - 1);
          smooth_upPositions[smoothIndex * 3 + 1] = 1.0 * heightmapImage->getPixel(i_plus_1, j, 0) / 255.0f;
          smooth_upPositions[smoothIndex * 3 + 2] = 1.0 * -j / (static_cast<int64_t>(imgHeight) - 1);

          smooth_downPositions[smoothIndex * 3] = 1.0 * i_minus_1 / (static_cast<int64_t>(imgWidth) - 1);
          smooth_downPositions[smoothIndex * 3 + 1] = 1.0 * heightmapImage->getPixel(i_minus_1, j, 0) / 255.0f;
          smooth_downPositions[smoothIndex * 3 + 2] = 1.0 * -j / (static_cast<int64_t>(imgHeight) - 1);

          smooth_leftPositions[smoothIndex * 3] = 1.0 * (i) / (static_cast<int64_t>(imgWidth) - 1);
          smooth_leftPositions[smoothIndex * 3 + 1] = 1.0 * heightmapImage->getPixel(i, j_minus_1, 0) / 255.0f;
          smooth_leftPositions[smoothIndex * 3 + 2] = 1.0 * -j_minus_1 / (static_cast<int64_t>(imgHeight) - 1);

          smooth_rightPositions[smoothIndex * 3] = 1.0 * i / (static_cast<int64_t>(imgWidth) - 1);
          smooth_rightPositions[smoothIndex * 3 + 1] = 1.0 * heightmapImage->getPixel(i, j_plus_1, 0) / 255.0f;
          smooth_rightPositions[smoothIndex * 3 + 2] = 1.0 * -j_plus_1 / (static_cast<int64_t>(imgHeight) - 1);

          smooth_centerColors[smoothIndex * 4] = smooth_centerPositions[smoothIndex * 3 + 1];
          smooth_centerColors[smoothIndex * 4 + 1] = smooth_centerPositions[smoothIndex * 3 + 1];
          smooth_centerColors[smoothIndex * 4 + 2] = smooth_centerPositions[smoothIndex * 3 + 1];
          smooth_centerColors[smoothIndex * 4 + 3] = alpha;
          smoothIndex++;

          //Vertice 2 for (i+1,j)
          smooth_centerPositions[smoothIndex * 3] = 1.0 * i_plus_1 / (static_cast<int64_t>(imgWidth) - 1);
          smooth_centerPositions[smoothIndex * 3 + 1] = 1.0 * heightmapImage->getPixel(i_plus_1, j, 0) / 255.0f;
          smooth_centerPositions[smoothIndex * 3 + 2] = 1.0 * -j / (static_cast<int64_t>(imgHeight) - 1);

          smooth_upPositions[smoothIndex * 3] = 1.0 * i_plus_2 / (static_cast<int64_t>(imgWidth) - 1);
          smooth_upPositions[smoothIndex * 3 + 1] = 1.0 * heightmapImage->getPixel(i_plus_2, j, 0) / 255.0f;
          smooth_upPositions[smoothIndex * 3 + 2] = 1.0 * -j / (static_cast<int64_t>(imgHeight) - 1);

          smooth_downPositions[smoothIndex * 3] = 1.0 * i / (static_cast<int64_t>(imgWidth) - 1);
          smooth_downPositions[smoothIndex * 3 + 1] = 1.0 * heightmapImage->getPixel(i, j, 0) / 255.0f;
          smooth_downPositions[smoothIndex * 3 + 2] = 1.0 * -j / (static_cast<int64_t>(imgHeight) - 1);

          smooth_leftPositions[smoothIndex * 3] = 1.0 * i_plus_1 / (static_cast<int64_t>(imgWidth) - 1);
          smooth_leftPositions[smoothIndex * 3 + 1] = 1.0 * heightmapImage->getPixel(i_plus_1, j_minus_1, 0) / 255.0f;
          smooth_leftPositions[smoothIndex * 3 + 2] = 1.0 * -j_minus_1 / (static_cast<int64_t>(imgHeight) - 1);

          smooth_rightPositions[smoothIndex * 3] = 1.0 * i_plus_1 / (static_cast<int64_t>(imgWidth) - 1);
          smooth_rightPositions[smoothIndex * 3 + 1] = 1.0 * heightmapImage->getPixel(i_plus_1, j_plus_1, 0) / 255.0f;
          smooth_rightPositions[smoothIndex * 3 + 2] = 1.0 * -j_plus_1 / (static_cast<int64_t>(imgHeight) - 1);
          
          smooth_centerColors[smoothIndex * 4] = smooth_centerPositions[smoothIndex * 3 + 1];
          smooth_centerColors[smoothIndex * 4 + 1] = smooth_centerPositions[smoothIndex * 3 + 1];
          smooth_centerColors[smoothIndex * 4 + 2] = smooth_centerPositions[smoothIndex * 3 + 1];
          smooth_centerColors[smoothIndex * 4 + 3] = alpha;
          smoothIndex++;

          //Vertice 3 for (i,j+1)
          smooth_centerPositions[smoothIndex * 3] = 1.0 * (i) / (static_cast<int64_t>(imgWidth) - 1);
          smooth_centerPositions[smoothIndex * 3 + 1] = 1.0 * heightmapImage->getPixel(i, j_plus_1, 0) / 255.0f;
          smooth_centerPositions[smoothIndex * 3 + 2] = 1.0 * -j_plus_1 / (static_cast<int64_t>(imgHeight) - 1);

          smooth_upPositions[smoothIndex * 3] = 1.0 * i_plus_1 / (static_cast<int64_t>(imgWidth) - 1);
          smooth_upPositions[smoothIndex * 3 + 1] = 1.0 * heightmapImage->getPixel(i_plus_1, j_plus_1, 0) / 255.0f;
          smooth_upPositions[smoothIndex * 3 + 2] = 1.0 * -j_plus_1 / (static_cast<int64_t>(imgHeight) - 1);

          smooth_downPositions[smoothIndex * 3] = 1.0 * i_minus_1 / (static_cast<int64_t>(imgWidth) - 1);
          smooth_downPositions[smoothIndex * 3 + 1] = 1.0 * heightmapImage->getPixel(i_minus_1, j_plus_1, 0) / 255.0f;
          smooth_downPositions[smoothIndex * 3 + 2] = 1.0 * -j_plus_1 / (static_cast<int64_t>(imgHeight) - 1);

          smooth_leftPositions[smoothIndex * 3] = 1.0 * (i) / (static_cast<int64_t>(imgWidth) - 1);
          smooth_leftPositions[smoothIndex * 3 + 1] = 1.0 * heightmapImage->getPixel(i, j, 0) / 255.0f;
          smooth_leftPositions[smoothIndex * 3 + 2] = 1.0 * -(j) / (static_cast<int64_t>(imgHeight) - 1);

          smooth_rightPositions[smoothIndex * 3] = 1.0 * (i) / (static_cast<int64_t>(imgWidth) - 1);
          smooth_rightPositions[smoothIndex * 3 + 1] = 1.0 * heightmapImage->getPixel(i, j_plus_2, 0) / 255.0f;
          smooth_rightPositions[smoothIndex * 3 + 2] = 1.0 * -j_plus_2 / (static_cast<int64_t>(imgHeight) - 1);
          
          smooth_centerColors[smoothIndex * 4] = smooth_centerPositions[smoothIndex * 3 + 1];
          smooth_centerColors[smoothIndex * 4 + 1] = smooth_centerPositions[smoothIndex * 3 + 1];
          smooth_centerColors[smoothIndex * 4 + 2] = smooth_centerPositions[smoothIndex * 3 + 1];
          smooth_centerColors[smoothIndex * 4 + 3] = alpha;
          smoothIndex++;

          //vertice 4 for (i,j+1)
          smooth_centerPositions[smoothIndex * 3] = 1.0 * (i) / (static_cast<int64_t>(imgWidth) - 1);
          smooth_centerPositions[smoothIndex * 3 + 1] = 1.0 * heightmapImage->getPixel(i, j_plus_1, 0) / 255.0f;
          smooth_centerPositions[smoothIndex * 3 + 2] = 1.0 * -j_plus_1 / (static_cast<int64_t>(imgHeight) - 1);

          smooth_upPositions[smoothIndex * 3] = 1.0 * i_plus_1 / (static_cast<int64_t>(imgWidth) - 1);
          smooth_upPositions[smoothIndex * 3 + 1] = 1.0 * heightmapImage->getPixel(i_plus_1, j_plus_1, 0) / 255.0f;
          smooth_upPositions[smoothIndex * 3 + 2] = 1.0 * -j_plus_1 / (static_cast<int64_t>(imgHeight) - 1);

          smooth_downPositions[smoothIndex * 3] = 1.0 * i_minus_1 / (static_cast<int64_t>(imgWidth) - 1);
          smooth_downPositions[smoothIndex * 3 + 1] = 1.0 * heightmapImage->getPixel(i_minus_1, j_plus_1, 0) / 255.0f;
          smooth_downPositions[smoothIndex * 3 + 2] = 1.0 * -j_plus_1 / (static_cast<int64_t>(imgHeight) - 1);

          smooth_leftPositions[smoothIndex * 3] = 1.0 * (i) / (static_cast<int64_t>(imgWidth) - 1);
          smooth_leftPositions[smoothIndex * 3 + 1] = 1.0 * heightmapImage->getPixel(i, j, 0) / 255.0f;
          smooth_leftPositions[smoothIndex * 3 + 2] = 1.0 * -(j) / (static_cast<int64_t>(imgHeight) - 1);

          smooth_rightPositions[smoothIndex * 3] = 1.0 * (i) / (static_cast<int64_t>(imgWidth) - 1);
          smooth_rightPositions[smoothIndex * 3 + 1] = 1.0 * heightmapImage->getPixel(i, j_plus_2, 0) / 255.0f;
          smooth_rightPositions[smoothIndex * 3 + 2] = 1.0 * -j_plus_2 / (static_cast<int64_t>(imgHeight) - 1);

          smooth_centerColors[smoothIndex * 4] = smooth_centerPositions[smoothIndex * 3 + 1];
          smooth_centerColors[smoothIndex * 4 + 1] = smooth_centerPositions[smoothIndex * 3 + 1];
          smooth_centerColors[smoothIndex * 4 + 2] = smooth_centerPositions[smoothIndex * 3 + 1];
          smooth_centerColors[smoothIndex * 4 + 3] = alpha;
          smoothIndex++;

          //Vertice 5 for (i+1,j)
          smooth_centerPositions[smoothIndex * 3] = 1.0 * i_plus_1 / (static_cast<int64_t>(imgWidth) - 1);
          smooth_centerPositions[smoothIndex * 3 + 1] = 1.0 * heightmapImage->getPixel(i_plus_1, j, 0) / 255.0f;
          smooth_centerPositions[smoothIndex * 3 + 2] = 1.0 * -j / (static_cast<int64_t>(imgHeight) - 1);

          smooth_upPositions[smoothIndex * 3] = 1.0 * i_plus_2 / (static_cast<int64_t>(imgWidth) - 1);
          smooth_upPositions[smoothIndex * 3 + 1] = 1.0 * heightmapImage->getPixel(i_plus_2, j, 0) / 255.0f;
          smooth_upPositions[smoothIndex * 3 + 2] = 1.0 * -j / (static_cast<int64_t>(imgHeight) - 1);

          smooth_downPositions[smoothIndex * 3] = 1.0 * i / (static_cast<int64_t>(imgWidth) - 1);
          smooth_downPositions[smoothIndex * 3 + 1] = 1.0 * heightmapImage->getPixel(i, j, 0) / 255.0f;
          smooth_downPositions[smoothIndex * 3 + 2] = 1.0 * -j / (static_cast<int64_t>(imgHeight) - 1);

          smooth_leftPositions[smoothIndex * 3] = 1.0 * i_plus_1 / (static_cast<int64_t>(imgWidth) - 1);
          smooth_leftPositions[smoothIndex * 3 + 1] = 1.0 * heightmapImage->getPixel(i_plus_1, j_minus_1, 0) / 255.0f;
          smooth_leftPositions[smoothIndex * 3 + 2] = 1.0 * -j_minus_1 / (static_cast<int64_t>(imgHeight) - 1);

          smooth_rightPositions[smoothIndex * 3] = 1.0 * i_plus_1 / (static_cast<int64_t>(imgWidth) - 1);
          smooth_rightPositions[smoothIndex * 3 + 1] = 1.0 * heightmapImage->getPixel(i_plus_1, j_plus_1, 0) / 255.0f;
          smooth_rightPositions[smoothIndex * 3 + 2] = 1.0 * -j_plus_1 / (static_cast<int64_t>(imgHeight) - 1);

          smooth_centerColors[smoothIndex * 4] = smooth_centerPositions[smoothIndex * 3 + 1];
          smooth_centerColors[smoothIndex * 4 + 1] = smooth_centerPositions[smoothIndex * 3 + 1];
          smooth_centerColors[smoothIndex * 4 + 2] = smooth_centerPositions[smoothIndex * 3 + 1];
          smooth_centerColors[smoothIndex * 4 + 3] = alpha;
          smoothIndex++;

          //Vertice 6 for (i+1,j+1)
          smooth_centerPositions[smoothIndex * 3] = 1.0 * i_plus_1 / (static_cast<int64_t>(imgWidth) - 1);
          smooth_centerPositions[smoothIndex * 3 + 1] = 1.0 * heightmapImage->getPixel(i_plus_1, j_plus_1, 0) / 255.0f;
          smooth_centerPositions[smoothIndex * 3 + 2] = 1.0 * -j_plus_1/ (static_cast<int64_t>(imgHeight) - 1);

          smooth_upPositions[smoothIndex * 3] = 1.0 * i_plus_2 / (static_cast<int64_t>(imgWidth) - 1);
          smooth_upPositions[smoothIndex * 3 + 1] = 1.0 * heightmapImage->getPixel(i_plus_2, j_plus_1, 0) / 255.0f;
          smooth_upPositions[smoothIndex * 3 + 2] = 1.0 * -j_plus_1 / (static_cast<int64_t>(imgHeight) - 1);

          smooth_downPositions[smoothIndex * 3] = 1.0 * i / (static_cast<int64_t>(imgWidth) - 1);
          smooth_downPositions[smoothIndex * 3 + 1] = 1.0 * heightmapImage->getPixel(i, j_plus_1, 0) / 255.0f;
          smooth_downPositions[smoothIndex * 3 + 2] = 1.0 * -j_plus_1/ (static_cast<int64_t>(imgHeight) - 1);

          smooth_leftPositions[smoothIndex * 3] = 1.0 * i_plus_1/ (static_cast<int64_t>(imgWidth) - 1);
          smooth_leftPositions[smoothIndex * 3 + 1] = 1.0 * heightmapImage->getPixel(i_plus_1, j, 0) / 255.0f;
          smooth_leftPositions[smoothIndex * 3 + 2] = 1.0 * -(j) / (static_cast<int64_t>(imgHeight) - 1);

          smooth_rightPositions[smoothIndex * 3] = 1.0 * i_plus_1/ (static_cast<int64_t>(imgWidth) - 1);
          smooth_rightPositions[smoothIndex * 3 + 1] = 1.0 * heightmapImage->getPixel(i_plus_1, j_plus_2, 0) / 255.0f;
          smooth_rightPositions[smoothIndex * 3 + 2] = 1.0 * -j_plus_2 / (static_cast<int64_t>(imgHeight) - 1);
          
          smooth_centerColors[smoothIndex * 4] = smooth_centerPositions[smoothIndex * 3 + 1];
          smooth_centerColors[smoothIndex * 4 + 1] = smooth_centerPositions[smoothIndex * 3 + 1];
          smooth_centerColors[smoothIndex * 4 + 2] = smooth_centerPositions[smoothIndex * 3 + 1];
          smooth_centerColors[smoothIndex * 4 + 3] = alpha;
          smoothIndex++;
      }
  }
  
  vboSmoothCenter = new VBO(numTriangleVertices, 3, smooth_centerPositions, GL_STATIC_DRAW);
  vboSmoothColors = new VBO(numTriangleVertices, 4, smooth_centerColors, GL_STATIC_DRAW);

  vboSmoothUp = new VBO(numTriangleVertices, 3, smooth_upPositions, GL_STATIC_DRAW);
  vboSmoothDown = new VBO(numTriangleVertices, 3, smooth_downPositions, GL_STATIC_DRAW);
  vboSmoothLeft = new VBO(numTriangleVertices, 3, smooth_leftPositions, GL_STATIC_DRAW);
  vboSmoothRight = new VBO(numTriangleVertices, 3, smooth_rightPositions, GL_STATIC_DRAW);

  // Create the VAOs. There is a single VAO in this example.
  // Important: this code must be executed AFTER we created our pipeline program, and AFTER we set up our VBOs.
  // A VAO contains the geometry for a single object. There should be one VAO per object.
  // In this homework, "geometry" means vertex positions and colors. In homework 2, it will also include
  // vertex normal and vertex texture coordinates for texture mapping.
  //vao = new VAO();

  // Set up the relationship between the "position" shader variable and the VAO.
  // Important: any typo in the shader variable name will lead to malfunction.
  //vao->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboVertices, "position");

  // Set up the relationship between the "color" shader variable and the VAO.
  // Important: any typo in the shader variable name will lead to malfunction.
  //vao->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboColors, "color");
  // We don't need this data any more, as we have already uploaded it to the VBO. And so we can destroy it, to avoid a memory leak.
  //free(positions);
  //free(colors);

  //VAO for Mode1 CASE1:POINTS
  vaoPoint = new VAO();
  vaoPoint->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboPointVertices, "position");
  vaoPoint->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboPointColors, "color");

  //VAO for Mode1 CASE1:LINES
  vaoLine = new VAO();
  vaoLine->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboLineVertices, "position");
  vaoLine->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboLineColors, "color");

  //VAO for Mode1 CASE1:TRIANGLES
  vaoTriangle = new VAO();
  vaoTriangle->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboTriangleVertices, "position");
  vaoTriangle->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboTriangleColors, "color");

  //VAO for Mode2 SMOOTH
  vaoSmooth = new VAO();
  vaoSmooth->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboSmoothCenter, "smooth_center");
  vaoSmooth->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboSmoothColors, "color");

  vaoSmooth->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboSmoothUp, "smooth_up");
  vaoSmooth->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboSmoothDown, "smooth_down");
  vaoSmooth->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboSmoothLeft, "smooth_left");
  vaoSmooth->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboSmoothRight, "smooth_right");


  free(vboPointVertices);
  free(vboPointColors);
  free(vboLineVertices);
  free(vboLineColors);
  free(vboTriangleVertices);
  free(vboTriangleColors);
  free(vboSmoothCenter);
  free(vboSmoothColors);
  free(vboSmoothDown);
  free(vboSmoothLeft);
  free(vboSmoothRight);
  free(vboSmoothUp);

  // Check for any OpenGL errors.
  std::cout << "GL error status is: " << glGetError() << std::endl;
}

int main(int argc, char *argv[])
{
  if (argc != 2 && argc!=3)
  {
    cout << "The arguments are incorrect." << endl;
    cout << "usage: ./hw1 <heightmap file>" << endl;
    exit(EXIT_FAILURE);
  }

  cout << "Initializing GLUT..." << endl;
  glutInit(&argc,argv);

  cout << "Initializing OpenGL..." << endl;

  #ifdef __APPLE__
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
  #else
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
  #endif

  glutInitWindowSize(windowWidth, windowHeight);
  glutInitWindowPosition(0, 0);  
  glutCreateWindow(windowTitle);

  cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
  cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << endl;
  cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

  #ifdef __APPLE__
    // This is needed on recent Mac OS X versions to correctly display the window.
    glutReshapeWindow(windowWidth - 1, windowHeight - 1);
  #endif

  // Tells GLUT to use a particular display function to redraw.
  glutDisplayFunc(displayFunc);
  // Perform animation inside idleFunc.
  glutIdleFunc(idleFunc);
  // callback for mouse drags
  glutMotionFunc(mouseMotionDragFunc);
  // callback for idle mouse movement
  glutPassiveMotionFunc(mouseMotionFunc);
  // callback for mouse button changes
  glutMouseFunc(mouseButtonFunc);
  // callback for resizing the window
  glutReshapeFunc(reshapeFunc);
  // callback for pressing the keys on the keyboard
  glutKeyboardFunc(keyboardFunc);

  // init glew
  #ifdef __APPLE__
    // nothing is needed on Apple
  #else
    // Windows, Linux
    GLint result = glewInit();
    if (result != GLEW_OK)
    {
      cout << "error: " << glewGetErrorString(result) << endl;
      exit(EXIT_FAILURE);
    }
  #endif

  // Perform the initialization.
  initScene(argc, argv);

  //texture
  texture(argc, argv);

  // Sink forever into the GLUT loop.
  glutMainLoop();
}

