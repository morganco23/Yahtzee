//
//  Display a rotating cube with lighting
//
//  Light and material properties are sent to the shader as uniform
//    variables.  Vertex positions and normals are attribute variables.
//  Shininess comes into play, so the side of a cube will not necessarily
//    appear as a uniform color.

#include "cs432.h"
#include "vec.h"
#include "mat.h"
#include "picking.h"
#include "matStack.h"
#include "characters.h"
#include <string>

// tick: every 50 milliseconds
#define TICK_INTERVAL 50

// typedefs to make code more readable
typedef vec4  color4;
typedef vec4  point4;

// for now, we know that we only need 72 vertices for our cube
const int NumVertices = 8000000; //(6 faces)(2 triangles/face)(3 vertices/triangle)(2 cubes)

// the properties of our objects
static point4 points[NumVertices];
static vec3   normals[NumVertices];
static color4   colorsDiffuse[NumVertices];
static color4   colorsSpecular[NumVertices];
static color4   colorsAmbient[NumVertices];
static GLfloat  objShininess[NumVertices];

// defining some colors
static color4 RED(1.0,0.0,0.0,1.0);
static color4 GRAY(0.5,0.5,0.5,1.0);
static color4 BLUE(0,0,1.0,1.0);
static color4 GREEN(0.0,1.0,0.0,1.0);
static color4 YELLOW(1.0,1.0,0.0,1.0);
static color4 WHITE(1.0,1.0,1.0,1.0);
static color4 CYAN(0.0,1.0,1.0,1.0);
static color4 BLACK(0.0,0.0,0.0,1.0);
static color4 BROWN(0.7,0.7,0.7,1.0);

// Vertices of a unit cube centered at origin, sides aligned with axes

// Array of rotation angles (in degrees) for each coordinate axis;
// These are used in rotating the cube.
static int Axis = 0; // 0 => x-rotation, 1=>y, 2=>z, 3=>none
static GLfloat Theta[] = { 0.0, 0.0, 0.0, 0.0};
static float spinSpeed = 1;

// Model-view, model-view-start and projection matrices uniform location
static GLuint  ModelView, ModelViewStart, Projection;


static int charInfo[256][2];
// The matrix that defines where the camera is. This starts out +3 in the
// z-direction, but can change based on the user moving the camera with
// keyboard input
mat4 model_view_start = LookAt(0,10,10,0,0,0,0,1,0);

static MatrixStack mvstack;
mat4 model_view;
// Variables used to control the moving of the light
static bool lightSpin = false; // whether the light is moving
static GLfloat lightAngle = 0.0; // the current lighting angle

// Variables used to control the moving of the dice
static float gravity = -.05;
static float resistance = 2;
static bool die1Moving = true;
static bool dieMoving[5] = { true, true, true, true, true };
static float diePositions[5][3] = { {3,0,0} ,{1.5,0,0} ,{0,0,0} ,{-1.5,0,0} ,{-3,0,0} };

static mat4 rotmat = RotateX(0);
static float dieVelocities[5][3] = { {0,0,0} ,{0,0,0} ,{0,0,0} ,{0,0,0} ,{0,0,0} };

static float dieRotaions[5][3] = { {0,0,0} ,{0,0,0} ,{0,0,0} ,{0,0,0} ,{0,0,0} };
static float dieOmegas[5][3] = { {0,0,0} ,{0,0,0} ,{0,0,0} ,{0,0,0} ,{0,0,0} };

static float diebounces[5] = { 0,0,0,0,0 };


// Variables used to control the game
static int scores[2] = { 0,0 };
static int playerNum;
static int runningSum = 0;
static bool playerWon = false;
static int whowon = 0;
//----------------------------------------------------------------------------

static point4 vertices[178] = {

    //for dice cube
    point4(-0.5, -0.5,  0.5, 1.0), //bottom left corner
    point4(-0.5,  0.5,  0.5, 1.0), //top left corner
    point4(0.5,  0.5,  0.5, 1.0), //top right corner
    point4(0.5, -0.5,  0.5, 1.0), //bottom right corner
    point4(-0.5, -0.5, -0.5, 1.0),
    point4(-0.5,  0.5, -0.5, 1.0),
    point4(0.5,  0.5, -0.5, 1.0),
    point4(0.5, -0.5, -0.5, 1.0),

    //for face 1 (facing front of cube) (vertexes 8-15)
    point4(-0.1, -0.1, 0.501, 1.0),
    point4(-0.1, 0.1, 0.501, 1.0),
    point4(0.1, 0.1, 0.501, 1.0),
    point4(0.1, -0.1, 0.501, 1.0),
    point4(-0.1, -0.1, 0.5, 1.0),
    point4(-0.1, 0.1, 0.5, 1.0),
    point4(0.1, 0.1, 0.5, 1.0),
    point4(0.1, -0.1, 0.5, 1.0),

    //for face 2 (face back of cube) (vertexes 16-31)
    point4(-0.4, -0.4, -0.501, 1.0),//bottom left cube
    point4(-0.4, -0.2, -0.501, 1.0),
    point4(-0.2, -0.2, -0.501, 1.0),
    point4(-0.2, -0.4, -0.501, 1.0),
    point4(-0.4, -0.4, -0.5, 1.0),
    point4(-0.4, -0.2, -0.5, 1.0),
    point4(-0.2, -0.2, -0.5, 1.0),
    point4(-0.2, -0.4, -0.5, 1.0),
    point4(0.2, 0.2, -0.501, 1.0), //top right cube
    point4(0.2, 0.4, -0.501, 1.0),
    point4(0.4, 0.4, -0.501, 1.0),
    point4(0.4, 0.2, -0.501, 1.0),
    point4(0.2, 0.2, -0.5, 1.0),
    point4(0.2, 0.4, -0.5, 1.0),
    point4(0.4, 0.4, -0.5, 1.0),
    point4(0.4, 0.2, -0.5, 1.0),

    //for face 3 (face bottom of cube) (vertexes 32-55)
    point4(-0.4, -0.501, -0.4, 1.0),//bottom left cube
    point4(-0.4, -0.501, -0.2, 1.0),
    point4(-0.2, -0.501, -0.2, 1.0),
    point4(-0.2, -0.501, -0.4, 1.0),
    point4(-0.4, -0.5, -0.4, 1.0),
    point4(-0.4, -0.5, -0.2, 1.0),
    point4(-0.2, -0.5, -0.2, 1.0),
    point4(-0.2, -0.5, -0.4, 1.0),
    point4(0.2, -0.501, 0.2, 1.0), //top right cube
    point4(0.2, -0.501, 0.4, 1.0),
    point4(0.4, -0.501, 0.4, 1.0),
    point4(0.4, -0.501, 0.2, 1.0),
    point4(0.2, -0.5, 0.2, 1.0),
    point4(0.2, -0.5, 0.4, 1.0),
    point4(0.4, -0.5, 0.4, 1.0),
    point4(0.4, -0.5, 0.2, 1.0),
    point4(-0.1, -0.501, -0.1, 1.0), //middle cube
    point4(-0.1, -0.501, 0.1, 1.0),
    point4(0.1, -0.501, 0.1, 1.0),
    point4(0.1, -0.501, -0.1, 1.0),
    point4(-0.1, -0.5, -0.1, 1.0),
    point4(-0.1, -0.5, 0.1, 1.0),
    point4(0.1, -0.5, 0.1, 1.0),
    point4(0.1, -0.5, -0.1, 1.0),

    //for face 4 (face top of cube) (vertexes 56-87)
    point4(-0.4, 0.501, -0.4, 1.0),//bottom left cube
    point4(-0.4, 0.501, -0.2, 1.0),
    point4(-0.2, 0.501, -0.2, 1.0),
    point4(-0.2, 0.501, -0.4, 1.0),
    point4(-0.4, 0.5, -0.4, 1.0),
    point4(-0.4, 0.5, -0.2, 1.0),
    point4(-0.2, 0.5, -0.2, 1.0),
    point4(-0.2, 0.5, -0.4, 1.0),
    point4(-0.4, 0.501, 0.2, 1.0),//top left cube
    point4(-0.4, 0.501, 0.4, 1.0),
    point4(-0.2, 0.501, 0.4, 1.0),
    point4(-0.2, 0.501, 0.2, 1.0),
    point4(-0.4, 0.5, 0.2, 1.0),
    point4(-0.4, 0.5, 0.4, 1.0),
    point4(-0.2, 0.5, 0.4, 1.0),
    point4(-0.2, 0.5, 0.2, 1.0),
    point4(0.2, 0.501, 0.2, 1.0), //top right cube
    point4(0.2, 0.501, 0.4, 1.0),
    point4(0.4, 0.501, 0.4, 1.0),
    point4(0.4, 0.501, 0.2, 1.0),
    point4(0.2, 0.5, 0.2, 1.0),
    point4(0.2, 0.5, 0.4, 1.0),
    point4(0.4, 0.5, 0.4, 1.0),
    point4(0.4, 0.5, 0.2, 1.0),
    point4(0.2, 0.501, -0.4, 1.0), //bottom right cube
    point4(0.2, 0.501, -0.2, 1.0),
    point4(0.4, 0.501, -0.2, 1.0),
    point4(0.4, 0.501, -0.4, 1.0),
    point4(0.2, 0.5, -0.4, 1.0),
    point4(0.2, 0.5, -0.2, 1.0),
    point4(0.4, 0.5, -0.2, 1.0),
    point4(0.4, 0.5, -0.4, 1.0),

    //for face 5 (face left of cube) (vertexes 88-127)
    point4(0.501, -0.1, -0.1, 1.0), //middle cube
    point4(0.501, -0.1, 0.1, 1.0),
    point4(0.501, 0.1, 0.1, 1.0),
    point4(0.501, 0.1, -0.1, 1.0),
    point4(0.5, -0.1, -0.1, 1.0),
    point4(0.5, -0.1, 0.1, 1.0),
    point4(0.5, 0.1, 0.1, 1.0),
    point4(0.5, 0.1, -0.1, 1.0),
    point4(0.501, -0.4, -0.4, 1.0),//bottom left cube
    point4(0.501, -0.4, -0.2, 1.0),
    point4(0.501, -0.2, -0.2, 1.0),
    point4(0.501, -0.2, -0.4, 1.0),
    point4(0.5, -0.4, -0.4, 1.0),
    point4(0.5, -0.4, -0.2, 1.0),
    point4(0.5, -0.2, -0.2, 1.0),
    point4(0.5, -0.2, -0.4, 1.0),
    point4(0.501, -0.4, 0.2, 1.0),//top left cube
    point4(0.501, -0.4, 0.4, 1.0),
    point4(0.501, -0.2, 0.4, 1.0),
    point4(0.501, -0.2, 0.2, 1.0),
    point4(0.5, -0.4, 0.2, 1.0),
    point4(0.5, -0.4, 0.4, 1.0),
    point4(0.5, -0.2, 0.4, 1.0),
    point4(0.5, -0.2, 0.2, 1.0),
    point4(0.501, 0.2, 0.2, 1.0), //top right cube
    point4(0.501, 0.2, 0.4, 1.0),
    point4(0.501, 0.4, 0.4, 1.0),
    point4(0.501, 0.4, 0.2, 1.0),
    point4(0.5, 0.2, 0.2, 1.0),
    point4(0.5, 0.2, 0.4, 1.0),
    point4(0.5, 0.4, 0.4, 1.0),
    point4(0.5, 0.4, 0.2, 1.0),
    point4(0.501, 0.2, -0.4, 1.0), //bottom right cube
    point4(0.501, 0.2, -0.2, 1.0),
    point4(0.501, 0.4, -0.2, 1.0),
    point4(0.501, 0.4, -0.4, 1.0),
    point4(0.5, 0.2, -0.4, 1.0),
    point4(0.5, 0.2, -0.2, 1.0),
    point4(0.5, 0.4, -0.2, 1.0),
    point4(0.5, 0.4, -0.4, 1.0),

    //for face 6 (face right of cube) (vertexes 128-175)
    point4(-0.501, -0.4, -0.4, 1.0),//bottom left cube
    point4(-0.501, -0.4, -0.2, 1.0),
    point4(-0.501, -0.2, -0.2, 1.0),
    point4(-0.501, -0.2, -0.4, 1.0),
    point4(-0.5, -0.4, -0.4, 1.0),
    point4(-0.5, -0.4, -0.2, 1.0),
    point4(-0.5, -0.2, -0.2, 1.0),
    point4(-0.5, -0.2, -0.4, 1.0),
    point4(-0.501, -0.4, -0.1, 1.0),//mid left cube
    point4(-0.501, -0.4, 0.1, 1.0),
    point4(-0.501, -0.2, 0.1, 1.0),
    point4(-0.501, -0.2, -0.1, 1.0),
    point4(-0.5, -0.4, -0.1, 1.0),
    point4(-0.5, -0.4, 0.1, 1.0),
    point4(-0.5, -0.2, 0.1, 1.0),
    point4(-0.5, -0.2, -0.1, 1.0),
    point4(-0.501, -0.4, 0.2, 1.0),//top left cube
    point4(-0.501, -0.4, 0.4, 1.0),
    point4(-0.501, -0.2, 0.4, 1.0),
    point4(-0.501, -0.2, 0.2, 1.0),
    point4(-0.5, -0.4, 0.2, 1.0),
    point4(-0.5, -0.4, 0.4, 1.0),
    point4(-0.5, -0.2, 0.4, 1.0),
    point4(-0.5, -0.2, 0.2, 1.0),
    point4(-0.501, 0.2, 0.2, 1.0),//top right cube
    point4(-0.501, 0.2, 0.4, 1.0),
    point4(-0.501, 0.4, 0.4, 1.0),
    point4(-0.501, 0.2, 0.2, 1.0),
    point4(-0.5, 0.2, 0.2, 1.0),
    point4(-0.5, 0.2, 0.4, 1.0),
    point4(-0.5, 0.4, 0.4, 1.0),
    point4(-0.5, 0.4, 0.2, 1.0),
    point4(-0.501, 0.2, -0.1, 1.0), //mid right cube
    point4(-0.501, 0.2, 0.1, 1.0),
    point4(-0.501, 0.4, 0.1, 1.0),
    point4(-0.501, 0.4, -0.1, 1.0),
    point4(-0.5, 0.2, -0.1, 1.0),
    point4(-0.5, 0.2, 0.1, 1.0),
    point4(-0.5, 0.4, 0.1, 1.0),
    point4(-0.5, 0.4, -0.1, 1.0),
    point4(-0.501, 0.2, -0.4, 1.0), //bottom right cube
    point4(-0.501, 0.2, -0.2, 1.0),
    point4(-0.501, 0.4, -0.2, 1.0),
    point4(-0.501, 0.4, -0.4, 1.0),
    point4(-0.5, 0.2, -0.4, 1.0),
    point4(-0.5, 0.2, -0.2, 1.0),
    point4(-0.5, 0.4, -0.2, 1.0),
    point4(-0.5, 0.4, -0.4, 1.0),

};

// variable used in generating the vertices for our objects
static int Index = 0;
static int idxarr[2] = { 1000,NumVertices };

// quad generates a square (of our cube) using two triangles
//
// parameters:
// - a, b, c and d: the vertex-numbers for this square
// - col: the color of the square
//   - note: the ambient, specular and diffuse colors are all set to
//     be the same color
// - shininess: the shininess of the square
static void
quad( int a, int b, int c, int d, color4 col, GLfloat shininess)
{
    // Initialize temporary vectors along the quad's edge to
    //   compute its face normal 
    vec4 u = vertices[b] - vertices[a];
    vec4 v = vertices[c] - vertices[b];
	
    vec3 normal = normalize( cross(u, v) );
	
    // create the 6 faces, each with appropriate properties ...
    
    normals[Index] = normal; points[Index] = vertices[a]; colorsDiffuse[Index] = col;
    colorsSpecular[Index] = col; colorsAmbient[Index] = col; objShininess[Index] = shininess;
    Index++;
    
    normals[Index] = normal; points[Index] = vertices[b]; colorsDiffuse[Index] = col;
    colorsSpecular[Index] = col; colorsAmbient[Index] = col; objShininess[Index] = shininess;
    Index++;
    
    normals[Index] = normal; points[Index] = vertices[c]; colorsDiffuse[Index] = col;
    colorsSpecular[Index] = col; colorsAmbient[Index] = col; objShininess[Index] = shininess;
    Index++;
    
    normals[Index] = normal; points[Index] = vertices[a]; colorsDiffuse[Index] = col;
    colorsSpecular[Index] = col; colorsAmbient[Index] = col; objShininess[Index] = shininess;
    Index++;
    
    normals[Index] = normal; points[Index] = vertices[c]; colorsDiffuse[Index] = col;
    colorsSpecular[Index] = col; colorsAmbient[Index] = col; objShininess[Index] = shininess;
    Index++;
    
    normals[Index] = normal; points[Index] = vertices[d]; colorsDiffuse[Index] = col;
    colorsSpecular[Index] = col; colorsAmbient[Index] = col; objShininess[Index] = shininess;
    Index++;
}

//----------------------------------------------------------------------------

static void
invquad(int a, int b, int c, int d, color4 col, GLfloat shininess)
{
    // Initialize temporary vectors along the quad's edge to
    //   compute its face normal 
    vec4 u = vertices[b] - vertices[a];
    vec4 v = vertices[c] - vertices[b];

    vec3 normal = -normalize(cross(u, v));

    // create the 6 faces, each with appropriate properties ...

    normals[Index] = normal; points[Index] = vertices[a]; colorsDiffuse[Index] = col;
    colorsSpecular[Index] = col; colorsAmbient[Index] = col; objShininess[Index] = shininess;
    Index++;

    normals[Index] = normal; points[Index] = vertices[b]; colorsDiffuse[Index] = col;
    colorsSpecular[Index] = col; colorsAmbient[Index] = col; objShininess[Index] = shininess;
    Index++;

    normals[Index] = normal; points[Index] = vertices[c]; colorsDiffuse[Index] = col;
    colorsSpecular[Index] = col; colorsAmbient[Index] = col; objShininess[Index] = shininess;
    Index++;

    normals[Index] = normal; points[Index] = vertices[a]; colorsDiffuse[Index] = col;
    colorsSpecular[Index] = col; colorsAmbient[Index] = col; objShininess[Index] = shininess;
    Index++;

    normals[Index] = normal; points[Index] = vertices[c]; colorsDiffuse[Index] = col;
    colorsSpecular[Index] = col; colorsAmbient[Index] = col; objShininess[Index] = shininess;
    Index++;

    normals[Index] = normal; points[Index] = vertices[d]; colorsDiffuse[Index] = col;
    colorsSpecular[Index] = col; colorsAmbient[Index] = col; objShininess[Index] = shininess;
    Index++;
}



// creates a cug with each face having difference color/shininess properties
static void colorcube()
{
    //draw cube
    quad(1, 0, 3, 2, GRAY, 100);
    quad(2, 3, 7, 6, GRAY, 100);
    quad(3, 0, 4, 7, GRAY, 100);
    quad(6, 5, 1, 2, GRAY, 100);
    quad(4, 5, 6, 7, GRAY, 100);
    quad(5, 4, 0, 1, GRAY, 100);

    for (int i = 8; i < 176; i += 8) { //draw the dots on the dice
        quad(i + 1, i, i + 3, i + 2, BLACK, 100);
        quad(i + 2, i + 3, i + 7, i + 6, BLACK, 100);
        quad(i + 3, i + 0, i + 4, i + 7, BLACK, 100);
        quad(i + 6, i + 5, i + 1, i + 2, BLACK, 100);
        quad(i + 4, i + 5, i + 6, i + 7, BLACK, 100);
        quad(i + 5, i + 4, i + 0, i + 1, BLACK, 100);
    }

}

//----------------------------------------------------------------------------
 
//creates the outside box for the die to bounce off of
static void
boundingbox() {
    invquad(1, 0, 3, 2, BROWN, 20);
    invquad(2, 3, 7, 6, BROWN, 20);
    invquad(3, 0, 4, 7, BROWN, 20);
    invquad(6, 5, 1, 2, BROWN, 20);
}
//----------------------------------------------------------------------------

// the GPU light ID, this allows us to change the position of the light during execution
static int lightId;

// send updated light-position information to the GPU
static void updateLightPosition() {
    GLfloat lightX = sin(lightAngle*0.023);
    GLfloat lightY = sin(lightAngle*0.031);
    GLfloat lightZ = sin(lightAngle*0.037);
    vec4 pos(lightX,lightY,lightZ,0.0);
    glUniform4fv( lightId, 1, pos );
    
}
//----------------------------------------------------------------------------

static void
generateRandomVelocities(int max,int dieNum) {
    for (int i = 0; i < 3; i++) {
        if (i != 2) {
            dieVelocities[dieNum][i] = static_cast <float> (rand() % max);
            //std::cout << "set random velocity " << i << " to " << die1Velocity[i] << std::endl;
        }
    }
}

static void generateRandomRotationV(int dieNum) {
    for (int i = 0; i < 3; i++) {
        dieOmegas[dieNum][i] = rand() % 10;
    }
}

static void drawBoundingBox() {
    mvstack.push(model_view);

    model_view *= Scale(-10, -10, -10);
    //model_view *= RotateX(180);
    model_view *= RotateZ(90);

    glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);

    glDrawArrays(GL_TRIANGLES, 792,24);
    model_view = mvstack.pop();
}

static void drawText(const char* text) {
    mvstack.push(model_view);
    for (int i = 0; i < strlen(text); i++) {
        model_view *= Translate(1.25, 0, 0);
        glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);
        glDrawArrays(GL_TRIANGLES, charInfo[text[i]][0], charInfo[text[i]][1]);
    }
    mvstack.pop();
};

static void drawScore() {
    mvstack.push(model_view);
    const char* yourScoreCard = "Your Score Card:";
    const char* aiScoreCard = "AI Score Card:";

    const char* oneLabel = "One";
    const char* twoLabel = "Two";
    const char* threeLabel = "Three";
    const char* fourLabel = "Four";
    const char* fiveLabel = "Five";
    const char* sizLabel = "Six";

    const char* tokLabel = "3 of a kind";
    const char* fokLabel = "4 of a kind";
    const char* fhLabel = "Full house";
    const char* ssLabel = "Small straight";
    const char* lsLabel = "Large straight";
    const char* yzeLabel = "YAHTZEE";
    const char* bonus = "Bonus";

    drawText(yourScoreCard);
    


    model_view = mvstack.pop();

    mvstack.push(model_view);
    const char* txt = "Your score: ";
    const char* enemytxt = "AI score: ";
    model_view *= Translate(-4 , 4, -3);
    model_view *= Scale(.33, .33, .33);
    for (int i = 0; i < strlen(txt); i++) {
        model_view *= Translate(1.25, 0, 0);
        glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);
        glDrawArrays(GL_TRIANGLES, charInfo[txt[i]][0], charInfo[txt[i]][1]);
        
    }
    std::string myscore = std::to_string(scores[0]);
    for (int i = 0; i < myscore.length(); i++) {
        model_view *= Translate(1.25, 0, 0);
        glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);
        glDrawArrays(GL_TRIANGLES, charInfo[myscore[i]][0], charInfo[myscore[i]][1]);

    }
    model_view = mvstack.pop();
    mvstack.push(model_view);

    model_view *= Translate(-4, 2, -3);
    model_view *= Scale(.33, .33, .33);
    for (int i = 0; i < strlen(enemytxt); i++) {
        model_view *= Translate(1.25, 0, 0);
        glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);
        glDrawArrays(GL_TRIANGLES, charInfo[enemytxt[i]][0], charInfo[enemytxt[i]][1]);

    }
    std::string aiscore = std::to_string(scores[1]);
    for (int i = 0; i < aiscore.length(); i++) {
        model_view *= Translate(1.25, 0, 0);
        glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);
        glDrawArrays(GL_TRIANGLES, charInfo[aiscore[i]][0], charInfo[aiscore[i]][1]);

    }
    model_view = mvstack.pop();

}

// display the scene
static void
display( void )
{
    
    // set all to background color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
    // compute the initial model-view matrix based on camera position
    model_view = model_view_start;
    

    mvstack.push(model_view);

    // perform translations on the die
    for (int i = 0; i < 5; i++) {
        mvstack.push(model_view);
        model_view *= Translate(diePositions[i][0], diePositions[i][1], diePositions[i][2]);
        model_view *= RotateX(dieRotaions[i][0]);
        model_view *= RotateY(dieRotaions[i][1]);
        model_view *= RotateZ(dieRotaions[i][2]);

        // update the light position based on the light-rotation information
        updateLightPosition();

        // send uniform matrix variables to the GPU
        glUniformMatrix4fv(ModelViewStart, 1, GL_TRUE, model_view_start);
        glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);

        // emit the cube to the scene
        setPickId(1); // set pick-id, in case we're picking
        glDrawArrays(GL_TRIANGLES, 0, 800);
        clearPickId(); // clear pick-id
        model_view = mvstack.pop();
    }
    
    
    

    drawBoundingBox();

    drawScore();

    // swap buffers (so that just-drawn image is displayed) or perform picking,
    // depending on mode
    if (inPickingMode()) {
        endPicking();
    }
    else {
        glutSwapBuffers();
    }
    model_view = mvstack.pop();
}

// picking-finished callback: stop rotation if the cube has beenn selected
void scenePickingFcn(int code) {
    if (code == 1) { // the cube
        if (!dieMoving[code - 1]) {
            dieMoving[code - 1] = true;
            diePositions[code - 1][1] = 2;
            diebounces[code - 1] = 0;
            generateRandomVelocities(2,0);
            generateRandomRotationV(0);
        }
    }
    else {
        scores[0] += runningSum;
        runningSum = 0;
        scores[1] += rand() % 10;
    }
}

//----------------------------------------------------------------------------

// for now, stop the spinning if the cube is clicked
static void
mouse( int button, int state, int x, int y ) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        // perform a "pick", including any associated action
        startPicking(scenePickingFcn, x, y);
    }

}

//----------------------------------------------------------------------------

static float
dotProduct(vec4 v1, vec4 v2) {
    float runningTotal = 0;
    for (int i = 0; i < 4; i++) {
        runningTotal += v1[i] * v2[i];
    }
    return  runningTotal;
}

static void
calculatePoints() {
    vec4 leftnormal = vec4(-1,0,0,0);       // 1
    vec4 rightnormal = vec4(1, 0, 0, 0);    // 6
    vec4 frontnormal = vec4(0, 0, 1, 0);    // 3
    vec4 backnormal = vec4(0, 0, -1, 0);    // 4
    vec4 topnormal = vec4(0, 1, 0, 0);      // 2
    vec4 bottomnormal = vec4(0, -1, 0, 0);  // 5

    vec4 nLeft = rotmat * leftnormal;
    vec4 nright = rotmat * rightnormal;
    vec4 nfront = rotmat * frontnormal;
    vec4 nback = rotmat * backnormal;
    vec4 ntop = rotmat * topnormal;
    vec4 nbottom = rotmat * bottomnormal;

    vec4 normals[6] = { nLeft,nright,nfront,nback,ntop,nbottom };

    int largest = -1;
    float maxVal = 0;
    for (int i = 0; i < 6; i++) {
        std::cout << normals[i] << std::endl;
        if (dotProduct(normals[i], backnormal) > maxVal) {
            maxVal = dotProduct(normals[i], topnormal);
            largest = i;
        }
    }

    switch (largest)
    {
    case 0:
        std::cout << "1 was the largest" << std::endl;
        runningSum = 0;
        scores[1] += rand() % 10;
        break;
    case 1:
        std::cout << "6 was the largest" << std::endl;
        runningSum += 6;
        break;
    case 2:
        std::cout << "3 was the largest" << std::endl;
        runningSum += 3;
        break;
    case 3:
        std::cout << "4 was the largest" << std::endl;
        runningSum += 4;
        break;
    case 4:
        std::cout << "2 was the largest" << std::endl;
        runningSum += 2;
        break;
    case 5:
        std::cout << "5 was the largest" << std::endl;
        runningSum += 5;
        break;
    case -1:
        std::cout << "There was an error" << std::endl;
        break;
    }

}

static bool isWithinRadius(float pos1[3], float pos2[3], float radius) {
    float sum = 0;
    for (int i = 0; i < 3; i++) {
        sum += (pos2[i] - pos1[i]) * (pos2[i] - pos1[i]);
    }
    return sqrt(sum) <= radius;
}

static void applyCollisions() {
    for (int i = 0; i < 5; i++) {
        for (int j = i + 1; j < 5; j++) {
            if (isWithinRadius(diePositions[i], diePositions[j], 1)) {
                std::cout << "colliding" << std::endl;
            }
        }
    }
}

// timer function, called when the timer has ticked
static void
tick(int n)
{
    srand(time(NULL));

    glutTimerFunc(n, tick, n); // schedule next tick

    // change the appropriate axis based on spin-speed
    //Theta[Axis] += spinSpeed;
    float speeds[5] = { 0,0,0,0,0 };
    for (int i = 0; i < 5; i++) {
        speeds[i] = dieVelocities[i][0] * dieVelocities[i][0] + dieVelocities[i][1] *
            dieVelocities[i][1] + dieVelocities[i][2] * dieVelocities[i][2];
    }


    if (scores[0] >= 100) {
        whowon = 0;
        playerWon = true;
    }
    else if (scores[1] >= 100) {
        whowon = 1;
        playerWon = true;
    }
    //std::cout << "X Rotation" << die1Rotation[0] << ", Y Rotation: " << die1Rotation[1] << std::endl;
    // update the position of the die by the different velocities
    for (int i = 0; i < 5; i++) {
        if (dieMoving[i]) {
            dieVelocities[i][1] += gravity;

            // all of the bouncing off of the wall conditions
            if (diePositions[i][0] > 3.75 || diePositions[i][0] < -3.75) {
                diePositions[i][0] = -dieVelocities[i][0];
                //give random rotaional velocity
                std::cout << "bounced of the x wall" << std::endl;
                generateRandomRotationV(i);
            }
            if (diePositions[i][2] > 3.75 || diePositions[i][2] < -3.75) {
                diePositions[i][2] = -dieVelocities[i][2];
                std::cout << "bounced of the z wall" << std::endl;
                //give random rotaional velocity
                generateRandomRotationV(i);
            }

            applyCollisions();

            //we hit the ground. lets bounce
            if (diePositions[i][1] <= -4.25) {
                if (speeds[i] < 0.02 || diebounces[i] > 3) {
                    dieMoving[i] = false;
                    // set the die on 1 face stead of on a point

                    int choose = rand() % 6; //randomly choose a side to land on
                    dieRotaions[i][1] = rand() % 366; //rotate y a random amount

                    //each axis rotation is specified and hard coded
                    if (choose == 0) { //for 1
                        dieRotaions[i][0] = -90;
                        dieRotaions[i][1] = 0;
                        dieRotaions[i][2] = rand() % 366;
                        //face = 1;
                    }
                    else if (choose == 1) { //for 2
                        dieRotaions[i][0] = 90;
                        dieRotaions[i][1] = 0;
                        dieRotaions[i][2] = rand() % 366;
                        //face = 2;
                    }
                    else if (choose == 2) { //for 3
                        dieRotaions[i][0] = 180;
                        dieRotaions[i][2] = 0;
                        //face = 3;
                    }
                    else if (choose == 3) { //for 4
                        dieRotaions[i][0] = 0;
                        dieRotaions[i][2] = 0;
                        //face = 4;
                    }
                    else if (choose == 4) { //for 5
                        dieRotaions[i][0] = 0;
                        dieRotaions[i][2] = 90;
                        //face = 5;
                    }
                    else if (choose == 5) { //for 6
                        dieRotaions[i][0] = 0;
                        dieRotaions[i][2] = -90;
                        //face = 6;
                    }

                    //currentScore += face;

                }
                else
                {
                    diebounces[i]++;
                    dieVelocities[i][1] *= -1;
                    dieVelocities[i][1] = dieVelocities[i][1] / (diebounces[i]);
                    diePositions[i][1] = -4.25 + dieVelocities[i][1];
                    diePositions[i][0] += dieVelocities[i][0] / (diebounces[i]);
                    diePositions[i][2] += dieVelocities[i][2] / (diebounces[i]);
                    // give the die random rotaional velocity
                    generateRandomRotationV(i);
                }

            }
            else {
                diePositions[i][0] += dieVelocities[i][0] / (diebounces[i] + 1);
                diePositions[i][1] += dieVelocities[i][1] / (diebounces[i] + 1);
                diePositions[i][2] += dieVelocities[i][2] / (diebounces[i] + 1);
                dieRotaions[i][0] += dieOmegas[i][0];
                dieRotaions[i][1] += dieOmegas[i][1];
                dieRotaions[i][2] += dieOmegas[i][2];
                /*rotmat *= RotateX(die1AngularVelocity[0]);
                rotmat *= RotateY(die1AngularVelocity[1]);
                rotmat *= RotateZ(die1AngularVelocity[2]);*/

            }

        }
    }
    

    // change the light angle
    if (lightSpin) {
        lightAngle += 5.0;
    }
    
    // tell GPU to display the frame
    glutPostRedisplay();
}
//----------------------------------------------------------------------------

// keyboard callback
static void
keyboard( unsigned char key, int x, int y )
{
    // Perform the appropriate action, based on the key that was pressed.
    // Default is to stop the cube-rotation
 	switch (key) {
        
		case 'q': case 'Q': case 033: // upper/lower Q or escape
            // Q: quit the program
			exit(0);
			break;
        case 'x': case 'X':
            // X: set rotation on X-axis
			Axis = 0;
			break;
		case 'y': case 'Y':
            // Y: set rotation on y-axis
			Axis = 1;
			break;
		case 'z': case 'Z':
            // Z: set rotation on z-axis
			Axis = 2;
			break;
        default:
            // default: stop spinning of cube
            Axis = 3;
            break;
		case '+': case '=':
            // + or =: increase spin-speed
			spinSpeed += 0.05;
			break;
		case '-': case '_':
            // - or _: decrease spin-speed
			spinSpeed -= 0.05;
			break;
        case 'l': case 'L':
            // L: toggle whether the light is spinning around scene
            lightSpin = !lightSpin;
            break;
        case 'w':
            // move forward
            model_view_start = Translate(0,0,0.1)*model_view_start;
            break;
        case 's':
            // move backward
            model_view_start = Translate(0,0,-0.1)*model_view_start;
            break;
        case 'a':
            // turn left
            model_view_start = RotateY(-1.5)*model_view_start;
            break;
        case 'd':
            // turn right
            model_view_start = RotateY(1.5)*model_view_start;
            break;
        case 'W':
            // turn up
            model_view_start = RotateX(-1.5)*model_view_start;
            break;
        case 'S':
            // turn down
            model_view_start = RotateX(1.5)*model_view_start;
            break;
        case 'A':
            // roll left
            model_view_start = RotateZ(-1.5)*model_view_start;
            break;
        case 'D':
            // roll right
            model_view_start = RotateZ(1.5)*model_view_start;
            break;
        case 'W'-64: // control-w
            // move up
            model_view_start = Translate(0,-0.1,0)*model_view_start;
            break;
        case 'S'-64: // control-s
            // move down
            model_view_start = Translate(0,0.1,0)*model_view_start;
            break;
        case 'A'-64: // control-a
            // move left
            model_view_start = Translate(0.1,0,0)*model_view_start;
            break;
        case 'D'-64: // control-d
            // move right
            model_view_start = Translate(-0.1,0,0)*model_view_start;
            break;
    }
}

//----------------------------------------------------------------------------

// window-reshape callback
void
reshape( int width, int height )
{
    glViewport( 0, 0, width, height );
	
    GLfloat aspect = GLfloat(width)/height;
    mat4  projection = Perspective( 65.0, aspect, 0.5, 100.0 );
	
    glUniformMatrix4fv( Projection, 1, GL_TRUE, projection );
}

// OpenGL initialization
static void
init()
{

    

    // create the cube object
    colorcube();
    boundingbox();

    for (int i = '!'; i <= '~'; i++) {
        // set start position
        charInfo[i][0] = idxarr[0];
        // generate the vertices for the character
        genCharacter(i, // character
            RED, // color
            0.1, // width of stroke
            0.1, // depth (z-direction)
            0.3, // shininess
            idxarr, // current index
            // the arrays to fill
            points, normals, colorsDiffuse,
            colorsSpecular, colorsAmbient, objShininess);
        // set number of vertices (end position minus start position)
        charInfo[i][1] = idxarr[0] - charInfo[i][0];
    }

    // Create a vertex array object
    GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );
    
    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
    glBufferData( GL_ARRAY_BUFFER,
                 sizeof(points) + sizeof(normals) + sizeof(colorsDiffuse) +
                 sizeof(colorsSpecular) + sizeof(colorsAmbient),
                 NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(points),
                    sizeof(normals), normals );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(normals),
                    sizeof(colorsDiffuse), colorsDiffuse );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(normals) + sizeof(colorsDiffuse),
                    sizeof(colorsSpecular), colorsSpecular );
    glBufferSubData( GL_ARRAY_BUFFER,
                    sizeof(points) + sizeof(normals) + sizeof(colorsDiffuse) + sizeof(colorsSpecular),
                    sizeof(colorsAmbient), colorsAmbient );
    glBufferSubData( GL_ARRAY_BUFFER,
                    sizeof(points) + sizeof(normals) + sizeof(colorsDiffuse) + sizeof(colorsSpecular) + sizeof(colorsAmbient),
                    sizeof(objShininess), objShininess );
    
    // Load shaders and use the resulting shader program
    const GLchar* vShaderCode =
    // all of our attributes from the arrays uploaded to the GPU
    "attribute  vec4 vPosition; "
    "attribute  vec3 vNormal; "
    "attribute  vec4 vDiffCol; "
    "attribute  vec4 vSpecCol; "
    "attribute  vec4 vAmbCol; "
    "attribute  float vObjShininess; "
    
    // uniform variables
    "uniform mat4 ModelViewStart; "
    "uniform mat4 ModelView; "
    "uniform mat4 Projection; "
    "uniform vec4 LightPosition; "
    "uniform vec4 LightDiffuse; "
    "uniform vec4 LightSpecular; "
    "uniform vec4 LightAmbient; "
    "uniform vec4 PickColor; "
    
    // variables to send on to the fragment shader
    "varying vec3 N,L, E, H; "
    "varying vec4 colorAmbient, colorDiffuse, colorSpecular; "
    "varying float shininess; "
    
    // main vertex shader
    "void main() "
    "{ "
    
    // Transform vertex  position into eye coordinates
    "vec3 pos = (ModelView * vPosition).xyz; "
    " "
    // compute the lighting-vectors
    "N = normalize( ModelView*vec4(vNormal, 0.0) ).xyz; "
    "L = normalize( (ModelViewStart*LightPosition).xyz - pos ); "
    "E = normalize( -pos ); "
    "H = normalize( L + E ); "
    
    // pass on the material-related variables
    "colorAmbient = vAmbCol; "
    "colorDiffuse = vDiffCol; "
    "colorSpecular = vSpecCol; "
    "shininess = vObjShininess; "
    
    // convert the vertex to camera coordinates
    "gl_Position = Projection * ModelView * vPosition; "
    "} "
    ;
    const GLchar* fShaderCode =
    // variables passed from the vertex shader
    "varying vec3 N,L, E, H; "
    "varying float shininess; "
    "varying vec4 colorAmbient, colorDiffuse, colorSpecular; "
    
    // uniform variables
    "uniform vec4 light_ambient, light_diffuse, light_specular; "
    "uniform float Shininess; "
    "uniform vec4 PickColor; "
    
    // main fragment shader
    "void main()  "
    "{  "
    
    // if we are picking, use the pick color, ignoring everything else
    "if (PickColor.a >= 0.0) { "
    "   gl_FragColor = PickColor; "
    "   return;"
    "} "
    
    // compute color intensities
    "vec4 AmbientProduct = light_ambient * colorAmbient; "
    "vec4 DiffuseProduct = light_diffuse * colorDiffuse; "
    "vec4 SpecularProduct = light_specular * colorSpecular; "
    
    // Compute fragment colors based on illumination equations
    "vec4 ambient = AmbientProduct; "
    "float Kd = max( dot(L, N), 0.0 ); "
    "vec4  diffuse = Kd*DiffuseProduct; "
    "float Ks = pow( max(dot(N, H), 0.0), shininess ); "
    "vec4  specular = Ks * SpecularProduct; "
    "if( dot(L, N) < 0.0 ) { "
    "  specular = vec4(0.0, 0.0, 0.0, 1.0); } "
    
    // add the color components
    "  gl_FragColor = ambient + specular + diffuse; "
    "}  "
    ;
    
    // set up the GLSL shaders
    GLuint program = InitShader2(vShaderCode, fShaderCode);
    
    glUseProgram( program );
    
    // set up vertex arrays
    GLuint vPosition = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(0) );
    
    GLuint vNormal = glGetAttribLocation( program, "vNormal" );
    glEnableVertexAttribArray( vNormal );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(sizeof(points)) );
    
    GLuint vDiffCol = glGetAttribLocation( program, "vDiffCol" );
    glEnableVertexAttribArray( vDiffCol );
    glVertexAttribPointer( vDiffCol, 4, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(sizeof(points)+sizeof(normals)) );
    
    GLuint vSpecCol = glGetAttribLocation( program, "vSpecCol" );
    glEnableVertexAttribArray( vSpecCol );
    glVertexAttribPointer( vSpecCol, 4, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(sizeof(points)+sizeof(normals)+sizeof(colorsDiffuse)) );
    
    GLuint vAmbCol = glGetAttribLocation( program, "vAmbCol" );
    glEnableVertexAttribArray( vAmbCol );
    glVertexAttribPointer( vAmbCol, 4, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(sizeof(points)+sizeof(normals)+sizeof(colorsDiffuse)+sizeof(colorsSpecular)) );
    
    // Initialize lighting position and intensities
    point4 light_position( 1,1,1, 0 );
    color4 light_ambient( 0,0,0, 1.0 );
    color4 light_diffuse(1,1,1, 1.0);
    color4 light_specular( 0.4,0.4,0.4, 1.0 );
    
    glUniform4fv( glGetUniformLocation(program, "light_ambient"),
                 1, light_ambient );
    glUniform4fv( glGetUniformLocation(program, "light_diffuse"),
                 1, light_diffuse );
    glUniform4fv( glGetUniformLocation(program, "light_specular"),
                 1, light_specular );
    
    lightId = glGetUniformLocation(program, "LightPosition");
    
    // initialize picking
    setGpuPickColorId(glGetUniformLocation(program, "PickColor"));
    
    // Retrieve transformation uniform variable locations
    ModelViewStart = glGetUniformLocation( program, "ModelViewStart" );
    ModelView = glGetUniformLocation( program, "ModelView" );
    Projection = glGetUniformLocation( program, "Projection" );
    
    // enable z-buffer algorithm
    glEnable( GL_DEPTH_TEST );
    
    // set background color to be white
    glClearColor( 1.0, 1.0, 1.0, 1.0 );

    //// initialize game variables
    //playerNum = static_cast <int> (rand());
    //float r1 = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / .1));
    //float r2 = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / .1));
    //float r3 = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / .1));
    //dieVelocities[[0] = r1;
    //die1Velocity[1] = r2;
    //die1Velocity[2] = r3;



}

//----------------------------------------------------------------------------

int
main( int argc, char **argv )
{
    // perform OpenGL initialization
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
    glutInitWindowSize( 512, 512 );
    glutCreateWindow( "Pig" );
	glewInit();
    init();
	
    // set up callback functions
    glutDisplayFunc( display );
    glutKeyboardFunc( keyboard );
    glutReshapeFunc( reshape );
    glutMouseFunc( mouse );
    glutTimerFunc(TICK_INTERVAL, tick, TICK_INTERVAL); // timer callback

    // start executing the main loop, waiting for a callback to occur
    glutMainLoop();
    return 0;
}
