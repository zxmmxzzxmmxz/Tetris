#include "Angel.h"
#include <vector>
#include <algorithm> 
#include <new> 
#include <map>
#include <stdlib.h> 
#include <ctime>


using namespace std;

//It is nice to define and use a color pallete on your software. You can put this information on another utility file.
static const vec3  base_colors[] = {
		vec3( 1.0, 0.0, 0.0 ),
		vec3( 0.0, 1.0, 0.0 ),
		vec3( 0.0, 0.0, 1.0 ),
		vec3( 0.5, 0.5, 0.5 )
};

static const vec3 backGroundColor = vec3( 0, 0, 0 );

static const int NumberOfRectangles = 200;

static const int NumberOfTriangles = NumberOfRectangles * 2;

static const int NumPoints = NumberOfTriangles * 3;

static const int NumberOfHorizontalBricks = 10;

static const int NumberOfVerticalBricks = 20;

static const int NumberOfHorizontalLines = NumberOfVerticalBricks + 1;

static const int NumberOfVerticalLines = NumberOfHorizontalBricks + 1;

static const int NumberOfPointsForGridLines = (NumberOfVerticalLines + NumberOfHorizontalLines) * 2;

static const double GridLineStartX = -0.95;
static const double GridLineStartY = 0.95;
static const double GridLineEndX = 0.95;
static const double GridLineEndY = -0.95;
static const double GridLineIncrementX = 0.19;
static const double GridLineIncrementY = 0.095;
static const double Origin = 0.0;

enum BlockTypeCode{
    O,I,S,Z,L,J,T
};
enum BlockSubTypeCode{
    A,B,C,D
};
enum COLOR{
    red,blue,green,black
};

map<COLOR, vec3> colorToVec3;

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////Rectangle class represents a cube block on the grid////////
/////////////////////////////////////////////////////////////////////////////////////////////////
class Rectangle{
    public:
        int _position_x, _position_y;
        COLOR _color ;
    public:
        Rectangle();
        Rectangle(int position_x, int position_y, COLOR color);
        Rectangle(int position_x, int position_y, vec3 color);
        Rectangle(const Rectangle& other);
        void setColor(COLOR newColor);
        bool outsideCanvas();
        void draw(vec2 triangles[], vec3 colors[]);
        void draw(vec2 triangles[], vec3 colors[], bool skipBlack);
        void move(int offsetX, int offsetY);
};

Rectangle::Rectangle(){
    _position_x = Origin;
    _position_y = Origin;
    _color = black;
}

Rectangle::Rectangle(int position_x, int position_y, COLOR color){
    _position_x = position_x;
    _position_y = position_y;
    _color = color;
}

Rectangle::Rectangle(int position_x, int position_y, vec3 color){
    _position_x = position_x;
    _position_y = position_y;
    _color = black;
    if(base_colors[0][0] == color[0] && base_colors[0][1] == color[1] && base_colors[0][2] == color[2]){
        _color = red;
    }
    else if(base_colors[1][0] == color[0] && base_colors[1][1] == color[1] && base_colors[1][2] == color[2]){
        _color = green;
    }
    else if(base_colors[2][0] == color[0] && base_colors[2][1] == color[1] && base_colors[2][2] == color[2]){
        _color = blue;
    }
}

Rectangle::Rectangle(const Rectangle& other){
    _position_x = other._position_x;
    _position_y = other._position_y;
    _color = other._color;
}

void Rectangle::move(int offsetX, int offsetY){
    _position_x += offsetX;
    _position_y += offsetY;
}

void Rectangle::setColor(COLOR newColor){
    _color = newColor;
}



void Rectangle::draw(vec2 triangles[], vec3 colors[]){
    draw(triangles, colors, true);
}

void Rectangle::draw(vec2 triangles[], vec3 colors[], bool skipBlack){
    //don't draw it if it's out of the bound
    if(_position_x < 0 || _position_x > 9 || _position_y < 0 || _position_y > 19){
        return;
    }
    if(skipBlack && _color == black){
        return;
    }
    double start_x = GridLineStartX + _position_x * GridLineIncrementX;
    double start_y = GridLineStartY - _position_y * GridLineIncrementY;
    int index = (_position_y * NumberOfHorizontalBricks + _position_x) * 6;
    colors[index] = colorToVec3[_color];
    triangles[index++] = vec2( start_x, start_y);
    colors[index] = colorToVec3[_color];
    triangles[index++] = vec2( start_x, start_y - GridLineIncrementY);
    colors[index] = colorToVec3[_color];
    triangles[index++] = vec2( start_x + GridLineIncrementX, start_y - GridLineIncrementY );

    colors[index] = colorToVec3[_color];
    triangles[index++] = vec2( start_x, start_y);
    colors[index] = colorToVec3[_color];
    triangles[index++] = vec2( start_x + GridLineIncrementX, start_y );
    colors[index] = colorToVec3[_color];
    triangles[index++] = vec2( start_x  + GridLineIncrementX, start_y - GridLineIncrementY );
}

bool Rectangle::outsideCanvas(){
    return _position_x > 9 || _position_y > 19 || _position_x < 0 || _position_y < 0;
}
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////Block class to represent a tile or shape in the grid screen///
/////////////////////////////////////////////////////////////////////////////////////////////////


class Block{
    public:
        BlockTypeCode _blockType;
        COLOR _color;
        int _position_x;
        int _position_y;
        Rectangle _rectangles[16];
        void initRectangles();
        void clearAll(vec2 triangles[], vec3 colors[]);
        BlockSubTypeCode _blockSubType;
        Block();
        Block(
            BlockTypeCode blockType,
            BlockSubTypeCode blockSubType,
            int position_x, 
            int position_y, 
            COLOR color);
        void move(int offsetX, int offsetY);
        void drawBlock(vec2 triangles[], vec3 colors[]);
        void setRectangles();
        bool shouldStop(bool filled[], int offsetX, int offsetY);
        void rotate();
        void addToFilled(bool filled[]);
};

Block::Block(){
    _color = black;
    _blockType = O;
    _blockSubType = A;
    _position_x = 0;
    _position_y = 0;
    initRectangles();
}

Block::Block(BlockTypeCode blockType, BlockSubTypeCode blockSubType, int position_x, int position_y, COLOR color){
    _blockType = blockType;
    _blockSubType = blockSubType;
    _position_y = position_y;
    _position_x = position_x;
    _color = color;
    initRectangles();
}

void Block::clearAll(vec2 triangles[], vec3 colors[]) {
    for(int i = 0;i < 16; i++){
        if(_rectangles[i]._color != black){
            _rectangles[i]._color = black;
            _rectangles[i].draw(triangles,colors, false);
        }
    }
}

void Block::initRectangles(){
    for(int x = 0; x < 4; x++){
        for(int y = 0; y < 4; y++){
            _rectangles[y * 4 + x] = Rectangle(_position_x + x, _position_y + y, black);
        }
    }
}

void Block::move(int offsetX, int offsetY){
    _position_x += offsetX;
    _position_y += offsetY;
    for(int y = 0; y < 4; y++){
        for(int x = 0; x < 4; x++){
            _rectangles[y * 4 + x].move(offsetX,offsetY);
        }
    }
}

bool Block::shouldStop(bool filled[],int offsetX, int offsetY) {
    for(int i = 0;i < 16; i++){
        _rectangles[i].setColor(black);
    }
    setRectangles();
    move(offsetX, offsetY);
    for(int i = 0;i < 16; i++){

        if(_rectangles[i]._color != black){ 
            if(_rectangles[i].outsideCanvas()){
                move(-offsetX,-offsetY);
                return true;
            }
            
            if(filled[_rectangles[i]._position_y * NumberOfHorizontalBricks + _rectangles[i]._position_x]){
                move(-offsetX, -offsetY);
                return true;
            }
        }
    }
    move(-offsetX,-offsetY);
    return false;
}

void Block::setRectangles(){
    switch(_blockType){
        case O:{
            _rectangles[5].setColor(_color);
            _rectangles[6].setColor(_color);
            _rectangles[9].setColor(_color);
            _rectangles[10].setColor(_color);
            break;
        }
        case I:{
            switch(_blockSubType){
                case A:{
                    _rectangles[4].setColor(_color);
                    _rectangles[5].setColor(_color);
                    _rectangles[6].setColor(_color);
                    _rectangles[7].setColor(_color);
                    break;
                }
                case B:{
                    _rectangles[2].setColor(_color);
                    _rectangles[6].setColor(_color);
                    _rectangles[10].setColor(_color);
                    _rectangles[14].setColor(_color);
                    break;
                }
                default:break;

            }
            break;
        }
        case S:{
            switch(_blockSubType){
                case A:{
                    _rectangles[6].setColor(_color);
                    _rectangles[7].setColor(_color);
                    _rectangles[9].setColor(_color);
                    _rectangles[10].setColor(_color);
                    break;
                }
                case B:{
                    _rectangles[2].setColor(_color);
                    _rectangles[6].setColor(_color);
                    _rectangles[7].setColor(_color);
                    _rectangles[11].setColor(_color);
                    break;
                }
                default:break;

            }
            break;
        }
        case Z:{
            switch(_blockSubType){
                case A:{
                    _rectangles[5].setColor(_color);
                    _rectangles[6].setColor(_color);
                    _rectangles[10].setColor(_color);
                    _rectangles[11].setColor(_color);
                    break;
                }
                case B:{
                    _rectangles[3].setColor(_color);
                    _rectangles[6].setColor(_color);
                    _rectangles[7].setColor(_color);
                    _rectangles[10].setColor(_color);
                    break;
                }
                default:break;

            }
            break;
        }
        case L:{
            switch(_blockSubType){
                case A:{
                    _rectangles[5].setColor(_color);
                    _rectangles[6].setColor(_color);
                    _rectangles[7].setColor(_color);
                    _rectangles[9].setColor(_color);
                    break;
                }
                case B:{
                    _rectangles[2].setColor(_color);
                    _rectangles[6].setColor(_color);
                    _rectangles[10].setColor(_color);
                    _rectangles[11].setColor(_color);
                    break;
                }
                case C:{
                    _rectangles[3].setColor(_color);
                    _rectangles[5].setColor(_color);
                    _rectangles[6].setColor(_color);
                    _rectangles[7].setColor(_color);
                    break;
                }
                case D:{
                    _rectangles[1].setColor(_color);
                    _rectangles[2].setColor(_color);
                    _rectangles[6].setColor(_color);
                    _rectangles[10].setColor(_color);
                    break;
                }
                default:break;

            }
            break;
        }
        case J:{
            switch(_blockSubType){
                case A:{
                    _rectangles[5].setColor(_color);
                    _rectangles[6].setColor(_color);
                    _rectangles[7].setColor(_color);
                    _rectangles[11].setColor(_color);
                    break;
                }
                case B:{
                    _rectangles[2].setColor(_color);
                    _rectangles[3].setColor(_color);
                    _rectangles[6].setColor(_color);
                    _rectangles[10].setColor(_color);
                    break;
                }
                case C:{
                    _rectangles[1].setColor(_color);
                    _rectangles[5].setColor(_color);
                    _rectangles[6].setColor(_color);
                    _rectangles[7].setColor(_color);
                    break;
                }
                case D:{
                    _rectangles[2].setColor(_color);
                    _rectangles[6].setColor(_color);
                    _rectangles[9].setColor(_color);
                    _rectangles[10].setColor(_color);
                    break;
                }
                default:break;

            }
            break;
        }
        case T:{
            switch(_blockSubType){
                case A:{
                    _rectangles[5].setColor(_color);
                    _rectangles[6].setColor(_color);
                    _rectangles[7].setColor(_color);
                    _rectangles[10].setColor(_color);
                    break;
                }
                case B:{
                    _rectangles[2].setColor(_color);
                    _rectangles[6].setColor(_color);
                    _rectangles[7].setColor(_color);
                    _rectangles[10].setColor(_color);
                    break;
                }
                case C:{
                    _rectangles[2].setColor(_color);
                    _rectangles[5].setColor(_color);
                    _rectangles[6].setColor(_color);
                    _rectangles[7].setColor(_color);
                    break;
                }
                case D:{
                    _rectangles[2].setColor(_color);
                    _rectangles[5].setColor(_color);
                    _rectangles[6].setColor(_color);
                    _rectangles[10].setColor(_color);
                    break;
                }
                default:break;

            }
            break;
        }
    }
}

void Block::drawBlock(vec2 triangles[], vec3 colors[]){
    for(int i = 0;i < 16; i++){
        _rectangles[i].setColor(black);
        _rectangles[i].draw(triangles,colors);
    }
    setRectangles();
    for(int i = 0;i < 16; i++){
        _rectangles[i].draw(triangles,colors);
    }
}


void Block::rotate(){
    if(_blockType == O){
        return;
    }
    if(_blockType == I || _blockType == S || _blockType == Z){
        if(_blockSubType == A){
            _blockSubType = B;
        }
        else{
            _blockSubType = A;
        }
    }
    if(_blockType == L || _blockType == J || _blockType == T){
        if(_blockSubType == A){
            _blockSubType = B;
        }
        else if(_blockSubType == B){
            _blockSubType = C;
        }
        else if(_blockSubType == C){
            _blockSubType = D;
        }
        else if(_blockSubType == D){
            _blockSubType = A;
        }
    }
}

void Block::addToFilled(bool filled[]){
    for(int i = 0;i < 16; i++){
        if(_rectangles[i]._color != black){
            filled[_rectangles[i]._position_x + _rectangles[i]._position_y * NumberOfHorizontalBricks] = true;
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////Start of all the real things/////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

//vertex array for the triangles and for the lines, respectively
GLuint vao, vao1;

//triangle points and colors arrays
vec2 triangles[NumPoints];
vec3 triangleColors[NumPoints];

//lines points and colors arrays
vec2 backgroundGridPoints[(NumberOfVerticalLines + NumberOfHorizontalLines) * 2];
vec3 colors1[(NumberOfVerticalLines + NumberOfHorizontalLines) * 2];
//----------------------------------------------------------------------------
Block* currentBlock;
bool filled[NumberOfHorizontalBricks * NumberOfVerticalBricks]{false};
void
init( void )
{   
    colorToVec3[red] = base_colors[0];
    colorToVec3[green] = base_colors[1];
    colorToVec3[blue] = base_colors[2];
    colorToVec3[black] = backGroundColor;
    //generate a random tile
    srand((int)time(0));
    BlockTypeCode type = BlockTypeCode(rand()%7);
    BlockSubTypeCode subType = A;
    if(type == I || type == S || type == Z){
        subType = BlockSubTypeCode(rand()%2);
    }
    else if(type == L || type == J || type == T){
        subType = BlockSubTypeCode(rand()%4);
    }
    currentBlock = new Block(type,subType,5,0,red);
    currentBlock->drawBlock(triangles,triangleColors);
    
    //draw the grid which are 21 + 11 lines
    int gridPointIndex = 0;
    double gridLineX = GridLineStartX;
    double gridLineY = GridLineStartY;
    for(int indexOfHorizontalLines = 0; indexOfHorizontalLines < NumberOfHorizontalLines; indexOfHorizontalLines++){
        backgroundGridPoints[gridPointIndex] = vec2(GridLineStartX, gridLineY);
        colors1[gridPointIndex] = base_colors[3];
        gridPointIndex += 1;
        backgroundGridPoints[gridPointIndex] = vec2(GridLineEndX, gridLineY);
        colors1[gridPointIndex] = base_colors[3];
        gridPointIndex += 1;
        gridLineY -= GridLineIncrementY;
    }
    for(int indexOfVerticalLines = 0; indexOfVerticalLines < NumberOfVerticalLines; indexOfVerticalLines++){
        backgroundGridPoints[gridPointIndex] = vec2(gridLineX, GridLineStartY);
        colors1[gridPointIndex] = base_colors[3];
        gridPointIndex += 1;
        backgroundGridPoints[gridPointIndex] = vec2(gridLineX, GridLineEndY);
        colors1[gridPointIndex] = base_colors[3];
        gridPointIndex += 1;
        gridLineX += GridLineIncrementX;
    }


    //bin background grid to vao only once
    glGenVertexArrays( 1, &vao1 );
    glBindVertexArray( vao1 );

    GLuint buffer1;
    glGenBuffers( 1, &buffer1 );
    glBindBuffer( GL_ARRAY_BUFFER, buffer1 );
    
    glBufferData( GL_ARRAY_BUFFER, sizeof(backgroundGridPoints) + sizeof(colors1), backgroundGridPoints, GL_STATIC_DRAW );

    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(backgroundGridPoints), backgroundGridPoints );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(backgroundGridPoints), sizeof(colors1), colors1 );

    // Load shaders and use the resulting shader program
    GLuint program1 = InitShader( "vshader.glsl", "fshader.glsl" );
    glUseProgram( program1 );

    // Initialize the vertex position attribute from the vertex shader
    GLuint loc1 = glGetAttribLocation( program1, "vPosition" );
    glEnableVertexAttribArray( loc1 );
    glVertexAttribPointer( loc1, 2, GL_FLOAT, GL_FALSE, 0,
                           BUFFER_OFFSET(0) );

    GLuint vColor1 = glGetAttribLocation( program1, "vColor" );
    glEnableVertexAttribArray( vColor1 );
    glVertexAttribPointer( vColor1, 3, GL_FLOAT, GL_FALSE, 0,
                           BUFFER_OFFSET(sizeof(backgroundGridPoints)) );

    glClearColor( 0.0, 0.0, 0.0, 1.0 );
}

//----------------------------------------------------------------------------

void
display( void )
{

    // create and bind triagnels which represent all the rectangle cubes and their corresponding color
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    // Create and bind a buffer object
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
    
    glBufferData( GL_ARRAY_BUFFER, sizeof(triangles) + sizeof(triangleColors), NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(triangles), triangles );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(triangles), sizeof(triangleColors), triangleColors );
    

    // Load shaders and use the resulting shader program
    GLuint program = InitShader( "vshader.glsl", "fshader.glsl" );
    glUseProgram( program );
    GLuint loc = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( loc );
    glVertexAttribPointer( loc, 2, GL_FLOAT, GL_FALSE, 0,
                           BUFFER_OFFSET(0) );
    GLuint vColor = glGetAttribLocation( program, "vColor" );
    glEnableVertexAttribArray( vColor );
    glVertexAttribPointer( vColor, 3, GL_FLOAT, GL_FALSE, 0,
                           BUFFER_OFFSET(sizeof(triangles)) );


    glClear( GL_COLOR_BUFFER_BIT );     // clear the window
    

    //Draw triangles
    glBindVertexArray( vao );
    glDrawArrays( GL_TRIANGLES, 0, NumPoints );

    glBindVertexArray( vao1 );
    glDrawArrays( GL_LINES, 0, NumberOfPointsForGridLines );

    //Causes all issued commands to be executed as quickly as they are accepted by the actual rendering engine
    glFlush();
}

//helper functions to remove a filled line of rectangle by moving all lines above one unit downward
vec3 getColorFrom(int x,int y){
    int index = (y * NumberOfHorizontalBricks + x) * 6;
    return triangleColors[index];
}
//helper functions to remove a filled line of rectangle by moving all lines above one unit downward
void copyLineFromAboveOne(int indexOfLineToBeReplaced){
    for(int i = 0; i < 10; i++){
        Rectangle(i,indexOfLineToBeReplaced,getColorFrom(i,indexOfLineToBeReplaced - 1)).draw(triangles, triangleColors, false);
    }
}
//helper functions to remove a filled line of rectangle by moving all lines above one unit downward
void setFirstLineBlack(){
    for(int i = 0; i < 10; i++){
        Rectangle(i,0,black).draw(triangles, triangleColors, false);
    }
}
//helper functions to remove a filled line of rectangle by moving all lines above one unit downward
void removeLineFromGrid(int indexOfLineToRemove){
    for(int i = indexOfLineToRemove; i > 0; i--){
        copyLineFromAboveOne(i);
    }
    setFirstLineBlack();
}

//every time the screen needs refresh, call this
// it does two things, remove filled lines of rectangles, and draw/create te active tile
// it will temrinate the game if no more tile can be added
void nextStep(){
    //remove filled lines
    for(int y = 0; y < 20; y++){
        bool lineFilled = true;
        for(int x = 0; x < 10; x++){
            if(!filled[y * NumberOfHorizontalBricks + x]){
                lineFilled = false;
            }
        }
        if(lineFilled){
            for(int yToMoveDown = y; yToMoveDown > 0; yToMoveDown--){
                for(int x = 0; x < 10; x++){
                    filled[yToMoveDown * NumberOfHorizontalBricks + x] = filled[(yToMoveDown-1) * NumberOfHorizontalBricks + x];
                }
            }
            for(int x = 0; x < 10; x++){
                    filled[x] = false;
            }
            removeLineFromGrid(y);
        }
    }

    //draw the current anywayz
    currentBlock->drawBlock(triangles,triangleColors);

    if(currentBlock->shouldStop(filled, 0, 1)){
        srand((int)time(0));
        //add to filled and generate next one, it stops here
        currentBlock->addToFilled(filled);

        //generate new block
        BlockTypeCode type = BlockTypeCode(rand()%7);
        BlockSubTypeCode subType = A;
        if(type == I || type == S || type == Z){
            subType = BlockSubTypeCode(rand()%2);
        }
        else if(type == L || type == J || type == T){
            subType = BlockSubTypeCode(rand()%4);
        }
        currentBlock = new Block(type,subType,rand()%7,0,COLOR(rand()%3));
        if(currentBlock->shouldStop(filled, 0, 1)){
            delete currentBlock;
            exit( EXIT_SUCCESS );
        }
    }
    else{
        currentBlock->clearAll(triangles,triangleColors);
        currentBlock->move(0,1);
        currentBlock->drawBlock(triangles,triangleColors);
    }
}

//timed function. We intended to execute this every one second so that it doesn't pause at all
void rotateDelay(int)
{
    nextStep();

    //Always remember to update your canvas
    glutPostRedisplay();

    //then we can set another identical event in 1000 miliseconds in the future, that is how we keep the game going on
    glutTimerFunc(1000.0, rotateDelay, 0);
}

//If you want to use arrows, take a look at this function
//It works similarly, but you have no acess to the ASCII keys using it. Please see the link below for details.
//https://www.opengl.org/resources/libraries/glut/spec3/node54.html
void keyboardSpecial( int key, int x, int y )
{
    switch ( key ) {
    case  GLUT_KEY_DOWN:{
        nextStep();
        glutPostRedisplay();
        break;
    }

    case GLUT_KEY_LEFT:{
        if(!currentBlock->shouldStop(filled, -1, 0)){
            currentBlock->clearAll(triangles,triangleColors);
            currentBlock->move(-1,0);
            currentBlock->drawBlock(triangles,triangleColors);
        }
        break;
    }

    case GLUT_KEY_RIGHT:{
        if(!currentBlock->shouldStop(filled, 1, 0)){
            currentBlock->clearAll(triangles,triangleColors);
            currentBlock->move(1,0);
            currentBlock->drawBlock(triangles,triangleColors);
        }
        break;
    }
    case GLUT_KEY_UP:{
        //try and revert then commit if successful process
        BlockSubTypeCode originalState = currentBlock->_blockSubType;
        currentBlock->rotate();
        bool goodToRotate = !currentBlock->shouldStop(filled, 0, 0);
        currentBlock->_blockSubType = originalState;
        currentBlock->initRectangles();
        currentBlock->setRectangles();
        currentBlock->clearAll(triangles,triangleColors);
        if(goodToRotate){
            currentBlock->rotate();
        }
        currentBlock->drawBlock(triangles,triangleColors);
        break;
    }

    case 033:
        delete currentBlock;
        exit( EXIT_SUCCESS );
        break;
    }
    glutPostRedisplay();
}

//bind most keys on your keyboard to this function
void
keyboard( unsigned char key, int x, int y )
{
    switch ( key ) {
    case 'q':{
        //quit game
        delete currentBlock;
        exit( EXIT_SUCCESS );
        break;
    }
    case 'r':{
        //refresh the game
        for(int y = 0; y < 20; y++){
            for(int x = 0; x < 10; x++){
                Rectangle(x,y,black).draw(triangles, triangleColors, false);
                filled[y * NumberOfHorizontalBricks + x] = false;
            }
        }
        delete currentBlock;
        //generate new block
        BlockTypeCode type = BlockTypeCode(rand()%7);
        BlockSubTypeCode subType = A;
        if(type == I || type == S || type == Z){
            subType = BlockSubTypeCode(rand()%2);
        }
        else if(type == L || type == J || type == T){
            subType = BlockSubTypeCode(rand()%4);
        }
        currentBlock = new Block(type,subType,rand()%7,0,COLOR(rand()%3));

        break;
    }
}
    
}

//----------------------------------------------------------------------------

int
main( int argc, char **argv )
{
    glutInit( &argc, argv );
    //We explicitly tell OpenGL that we are using the RGBA mode. When we start working with 3D resources there will be changes here.
    glutInitDisplayMode( GLUT_RGBA );

    //Change the size of the screen here. Keep in mind that the coordinates will also change accordingly
    //so, if you want to keep your points on the rendering canvas undisturbed you need a square screen.
    glutInitWindowSize( 400, 800 );

    // If you are using freeglut, the next two lines will check if 
    // the code is truly 3.2. Otherwise, comment them out
    glutInitContextVersion( 3, 2 );
    glutInitContextProfile( GLUT_CORE_PROFILE );

    //Title of your screen, change accordingly
    glutCreateWindow( "Tetris" );

    // Iff you get a segmentation error at line 34, please uncomment the line below
    glewExperimental = GL_TRUE; 
    glewInit();

    //this is the initialization function for your data.
    init();

    //Here we bind the function "display" to the OpenGL handler that will render your data
    glutDisplayFunc( display );

    //Similarly, we can also bind a function that will be executed 1000 miliseconds later. We bind this to the function rotateDelay.
    // We then bind this to the function "rotateDelay". The 0 in the end is the input of the rotateDelay class, it can only be an integer.
    glutTimerFunc(1000.0, rotateDelay, 0);

    //Finally, we bind the keyboard events inside the class "keyboard" using the line below
    glutKeyboardFunc( keyboard );

    //If you really want to use F1 ~ F9 keys, or the arrow keys, then use the line of code below.
    glutSpecialFunc( keyboardSpecial );

    glutMainLoop();
    return 0;
}
