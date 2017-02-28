#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <map>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

// #include <glad/glad.h>
#include <FTGL/ftgl.h>
#include <GLFW/glfw3.h>
#include <SOIL/SOIL.h>

using namespace std;

typedef struct COLOR{
	float r;
	float g;
	float b;
}COLOR;

typedef struct VAO {
	GLenum PrimitiveMode;
	GLenum FillMode;
	int NumVertices;

	GLuint VertexArrayID;
	GLuint VertexBuffer;
	GLuint ColorBuffer;
}VAO;
void changeview();
float eyearr[3],targetarr[3],uparr[3];
int choice,choices;

struct switch_struct{
	bool used;
	glm::vec3 place;
	vector<glm::vec3>locations;
};

struct cross_struct{
	glm::vec3 place;
	glm::vec3 other;
	bool used;
};
typedef struct switch_struct switch_struct;
typedef struct cross_struct cross_struct;
struct Level_struct{
	vector<cross_struct>crosses;
	vector<switch_struct>switches;
	glm::vec3 cube0_pos;
	glm::vec3 cube1_pos;
	int levelMatrix[10][10];
};
typedef struct Level_struct Level_struct;

struct Sprite{
	VAO* object;
	string name;
	VAO* line;
	COLOR color;
	glm::vec3 pos;
	int speed;
	glm::vec3 back;
	int dr;
	glm::vec3 theta;
	glm::vec3 scale;
	bool active;
	glm::vec3 ori;
	int status;
	bool choose;
	bool hover;
	glm::vec3 limit;
};
typedef struct Sprite Sprite;

struct GLMatrices {
	GLuint MatrixID;
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;
} Matrices;

float rectangle_rotation = 0;
GLuint programID;
double current_time,last_update_time;
int floor_rel,do_rot;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {
	// Create the shaders
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	std::string VertexShaderCode;

	if(VertexShaderStream.is_open()){
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	//    printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	//    fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	//    printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	//    fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	//    fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	//    fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(FragmentShaderID);
	glDeleteShader(VertexShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}

void initGLEW(void){
	glewExperimental = GL_TRUE;
	if(glewInit()!=GLEW_OK){
		fprintf(stderr,"Glew failed to initialize : %s\n", glewGetErrorString(glewInit()));
	}
	if(!GLEW_VERSION_3_3)
		fprintf(stderr, "3.3 version not available\n");
}



/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
	struct VAO* vao = new struct VAO;
	vao->PrimitiveMode = primitive_mode;
	vao->NumVertices = numVertices;
	vao->FillMode = fill_mode;

	// Create Vertex Array Object
	// Should be done after CreateWindow and before any other GL calls
	glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
	glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
	glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

	glBindVertexArray (vao->VertexArrayID); // Bind the VAO
	glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
	glVertexAttribPointer(
			0,                  // attribute 0. Vertices
			3,                  // size (x,y,z)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
	glVertexAttribPointer(
			1,                  // attribute 1. Color
			3,                  // size (r,g,b)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
	GLfloat* color_buffer_data = new GLfloat [3*numVertices];
	for (int i=0; i<numVertices; i++) {
		color_buffer_data [3*i] = red;
		color_buffer_data [3*i + 1] = green;
		color_buffer_data [3*i + 2] = blue;
	}

	return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
	// Change the Fill Mode for this object
	glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

	// Bind the VAO to use
	glBindVertexArray (vao->VertexArrayID);

	// Enable Vertex Attribute 0 - 3d Vertices
	glEnableVertexAttribArray(0);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

	// Enable Vertex Attribute 1 - Color
	glEnableVertexAttribArray(1);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

	// Draw the geometry !
	glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/
int boardMatrix[10][10];
int moves[10]={0};
Sprite camera;
int current_level;
int merged;
Sprite floor_orange;
vector<Level_struct>levels;
int timer[10];
Sprite floor_green;
Sprite cube[2];
Sprite floor_red;
Sprite rectangle_line;
int rec;
int dom;
Sprite floor_black;
int score=0;
int mode;
Sprite floor_grey;
int toppling;
float rectangle_rot_dir = 1;
int dim=10;
int chosen;
bool paused;
float camera_rotation_angle_x = 90;
int falling=0;
float camera_rotation_angle_y = 90;

int a1,a2,a3,a4;

map <string, bool> buttons;
bool right_move;
void updateScore(){
	score+=(int)(100000/(moves[current_level]*timer[current_level]));
}

void Initialize(){
	changeview();
	if(right_move){
		updateScore();
		cout << score<<endl;
		current_level++;
		right_move=false;
	}
	toppling=0;
	merged=1;
	cube[0].pos = levels[current_level].cube0_pos;
	cube[1].pos = levels[current_level].cube1_pos;
	dom=0;
	falling=0;
	for(int i=0;i<dim;i++){
		for(int j=0;j<dim;j++){
			boardMatrix[j][i] = levels[current_level].levelMatrix[j][i];
		}
	}
	timer[current_level]=1;
	for(vector<switch_struct>::iterator it=levels[current_level].switches.begin();it<levels[current_level].switches.end();++it){
		boardMatrix[(int)it->place.x][(int)it->place.y]=8;
		it->used=false;
	}
	for(vector<cross_struct>::iterator it=levels[current_level].crosses.begin();it<levels[current_level].crosses.end();++it){
		boardMatrix[(int)it->place.x][(int)it->place.y]=7;
	}
	cube[0].theta.x=cube[0].ori.x=-45;
	cube[0].theta.y=cube[0].ori.y=-45;
	cube[1].theta.x=cube[1].ori.x=-45;
	cube[1].theta.y=cube[1].ori.y=-45;
	paused=false;
	camera.pos = glm::vec3(0,0,0);
}


/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */


int DominantNorth(){
	if(cube[0].pos.x==cube[1].pos.x){
	 	if(cube[0].pos.y != cube[1].pos.y && cube[0].pos.z == cube[1].pos.z){
		//case 1
		if(cube[0].pos.y > cube[1].pos.y)
			return 0; // fore most is dom
		else return 1;
		}
	}
	if(cube[0].pos.x != cube[1].pos.x && cube[0].pos.y == cube[1].pos.y && cube[0].pos.z == cube[1].pos.z){
		//case 2
		if(cube[0].pos.x > cube[1].pos.x)
			return 0; // both are dom
		else return 1;
	}
	if(cube[0].pos.x == cube[1].pos.x && cube[0].pos.y == cube[1].pos.y && cube[0].pos.z != cube[1].pos.z){
		//case 3
		if(cube[0].pos.z < cube[1].pos.z)
			return 0; // lower is dom
		else return 1;
	}
}
int changemodeNorth(){
	if(cube[0].pos.x != cube[1].pos.x && cube[0].pos.y == cube[1].pos.y && cube[0].pos.z == cube[1].pos.z){
		//case 1
		return 2;
	}
	if(cube[0].pos.x == cube[1].pos.x && cube[0].pos.y != cube[1].pos.y && cube[0].pos.z == cube[1].pos.z){
		//case 2
		return 1;
	}
	if(cube[0].pos.x == cube[1].pos.x && cube[0].pos.y == cube[1].pos.y && cube[0].pos.z != cube[1].pos.z){
		//case 3
		return 3;
	}
}
int DominantSouth(){
	if(cube[0].pos.x == cube[1].pos.x){
		//case 1
		if(cube[0].pos.y != cube[1].pos.y){
				if(cube[0].pos.z == cube[1].pos.z){
					if(cube[0].pos.y >= cube[1].pos.y)
						return 1;
					else
						return 0;
				}
		}
		else{

		}
	}
	if(cube[0].pos.x != cube[1].pos.x && cube[0].pos.y == cube[1].pos.y && cube[0].pos.z == cube[1].pos.z){
		//case 2
		if(cube[0].pos.x > cube[1].pos.x)
			return 0; // both are dom
		else return 1;
	}
	if(cube[0].pos.x == cube[1].pos.x && cube[0].pos.y == cube[1].pos.y && cube[0].pos.z != cube[1].pos.z){
		//case 3
		if(cube[0].pos.z < cube[1].pos.z)
			return 0; // lower is dom
		else return 1;
	}
}
int changemodeSouth(){
	if(cube[0].pos.x == cube[1].pos.x && cube[0].pos.y != cube[1].pos.y && cube[0].pos.z == cube[1].pos.z){
		//case 1
		return 3;
	}
	if(cube[0].pos.x != cube[1].pos.x && cube[0].pos.y == cube[1].pos.y && cube[0].pos.z == cube[1].pos.z){
		//case 2
		return 2;
	}
	if(cube[0].pos.x == cube[1].pos.x && cube[0].pos.y == cube[1].pos.y && cube[0].pos.z != cube[1].pos.z){
		//case 3
		return 1;
	}
}
int DominantEast(){
	if(cube[0].pos.x == cube[1].pos.x && cube[0].pos.y != cube[1].pos.y && cube[0].pos.z == cube[1].pos.z){
		//case 1

		if(cube[0].pos.y > cube[1].pos.y)
			return 0;
		else return 1;//both are dom
	}
	if(cube[0].pos.x != cube[1].pos.x && cube[0].pos.y == cube[1].pos.y && cube[0].pos.z == cube[1].pos.z){
		//case 2
		if(cube[0].pos.x > cube[1].pos.x)
			return 0; // fore x is dom
		else return 1;
	}
	if(cube[0].pos.x == cube[1].pos.x && cube[0].pos.y == cube[1].pos.y && cube[0].pos.z != cube[1].pos.z){
		//case 3
		if(cube[0].pos.z < cube[1].pos.z)
			return 0; // lower is dom
		else return 1;
	}
}
int EastMode(){
	if(cube[0].pos.x == cube[1].pos.x && cube[0].pos.y != cube[1].pos.y && cube[0].pos.z == cube[1].pos.z){
		//case 1
		return 2;
	}
	if(cube[0].pos.x != cube[1].pos.x && cube[0].pos.y == cube[1].pos.y && cube[0].pos.z == cube[1].pos.z){
		//case 2
		return 1;
	}
	if(cube[0].pos.x == cube[1].pos.x && cube[0].pos.y == cube[1].pos.y && cube[0].pos.z != cube[1].pos.z){
		//case 3
		return 3;
	}
}
int DominantWest(){
	if(cube[0].pos.x == cube[1].pos.x && cube[0].pos.y != cube[1].pos.y && cube[0].pos.z == cube[1].pos.z){
		//case 1

		if(cube[0].pos.y > cube[1].pos.y)
			return 0;
		else return 1;//both are dom
	}
	if(cube[0].pos.x != cube[1].pos.x && cube[0].pos.y == cube[1].pos.y && cube[0].pos.z == cube[1].pos.z){
		//case 2
		if(cube[0].pos.x < cube[1].pos.x)
			return 0; // less x is dom
		else return 1;
	}
	if(cube[0].pos.x == cube[1].pos.x && cube[0].pos.y == cube[1].pos.y && cube[0].pos.z != cube[1].pos.z){
		//case 3
		if(cube[0].pos.z < cube[1].pos.z)
			return 0; // lower is dom
		else return 1;
	}
}
int WestMode(){
	if(cube[0].pos.x == cube[1].pos.x && cube[0].pos.y != cube[1].pos.y && cube[0].pos.z == cube[1].pos.z){
		//case 1
		return 2;
	}
	if(cube[0].pos.x != cube[1].pos.x && cube[0].pos.y == cube[1].pos.y && cube[0].pos.z == cube[1].pos.z){
		//case 2
		return 3;
	}
	if(cube[0].pos.x == cube[1].pos.x && cube[0].pos.y == cube[1].pos.y && cube[0].pos.z != cube[1].pos.z){
		//case 3
		return 1;
	}
}
void CuboidToppleEast(){
	cube[dom].pos = glm::vec3(cube[dom].back.x+cube[dom].scale.y+cube[dom].scale.z*sin(cube[dom].theta.x*M_PI/180.0f)*sqrt(2),cube[dom].back.y,
			floor_grey.scale.z + cube[dom].scale.z*cos(cube[dom].theta.x*M_PI/180.0f)*sqrt(2)
			);
	switch(mode){
		case 1: cube[rec].pos = glm::vec3(cube[dom].pos.x - cos(45+cube[dom].theta.x*M_PI/180.0f),cube[dom].back.y,cube[dom].pos.z + sin(45+cube[dom].theta.x*M_PI/180.0f));
		break;
		case 2: cube[rec].pos = glm::vec3(cube[dom].pos.x,cube[rec].pos.y,cube[dom].pos.z);
		break;
		case 3: cube[rec].pos = glm::vec3(cube[dom].pos.x + sin(45+cube[dom].theta.x*M_PI/180.0f),cube[dom].back.y,cube[dom].pos.z + cos(45+cube[dom].theta.x*M_PI/180.0f));
		break;
	}
	cube[rec].theta.x += cube[rec].dr;
	cube[dom].theta.x += cube[dom].dr;
	if(cube[dom].theta.x >= cube[dom].limit.x){
		toppling=0;
		switch(mode){
			case 1: cube[rec].pos = glm::vec3(cube[rec].back.x+2,cube[rec].back.y,cube[rec].back.z+1);
			break;
			case 2: cube[rec].pos = glm::vec3(cube[rec].back.x+1,cube[rec].back.y,cube[rec].back.z);
			break;
			case 3: cube[rec].pos = glm::vec3(cube[rec].back.x+2,cube[rec].back.y,cube[rec].back.z-1);
			break;
		}
		cube[rec].theta.x=-cube[rec].limit.x;
		cube[dom].theta.x=-cube[dom].limit.x;
		cube[dom].pos = glm::vec3(cube[dom].back.x+1,cube[dom].back.y,cube[dom].back.z);
	}
}
void CubeToppleEast(){
	cube[dom].pos = glm::vec3(  cube[dom].back.x + cube[dom].scale.y + cube[dom].scale.z*sin(cube[dom].theta.x*M_PI/180.0f)*sqrt(2),
			cube[dom].back.y,floor_grey.scale.z+ cube[dom].scale.z*cos(cube[dom].theta.x*M_PI/180.0f)*sqrt(2));

	cube[dom].theta.x += cube[dom].dr;

	if(cube[dom].theta.x >= cube[dom].limit.x){
		toppling=0;
		cube[dom].theta.x=-cube[dom].limit.x;
		cube[dom].pos = glm::vec3(cube[dom].back.x+1,cube[dom].back.y,cube[dom].back.z);
	}
}

void CuboidToppleSouth(){
	cube[dom].pos = glm::vec3(  cube[dom].back.x,
			cube[dom].back.y - cube[dom].scale.y + cube[dom].scale.z*sin(cube[dom].theta.y*M_PI/180.0f)*sqrt(2),
			floor_grey.scale.z + cube[dom].scale.z*cos(cube[dom].theta.y*M_PI/180.0f)*sqrt(2)
			);
	switch(mode){
			case 1:	cube[rec].pos = glm::vec3(cube[dom].back.x,cube[dom].pos.y - cos(45+cube[dom].theta.y*M_PI/180.0f),cube[dom].pos.z + sin(45+cube[dom].theta.y*M_PI/180.0f));
			break;
			case 2: cube[rec].pos = glm::vec3(  cube[rec].back.x,cube[dom].pos.y,cube[dom].pos.z);
			break;
			case 3: cube[rec].pos = glm::vec3(  cube[dom].back.x,cube[dom].pos.y + sin(45+cube[dom].theta.y*M_PI/180.0f),cube[dom].pos.z + cos(45+cube[dom].theta.y*M_PI/180.0f));
			break;
	}
	cube[rec].theta.y += cube[rec].dr;
	cube[dom].theta.y += cube[dom].dr;

	if(cube[dom].theta.y <= cube[dom].limit.y){
		toppling=0;
		switch(mode){
				case 1:	cube[rec].pos = glm::vec3(cube[rec].back.x,cube[rec].back.y-2,cube[rec].back.z-1);
				break;
				case 2: cube[rec].pos = glm::vec3(cube[rec].back.x,cube[rec].back.y-1,cube[rec].back.z);
				break;
				case 3: cube[rec].pos = glm::vec3(cube[rec].back.x,cube[rec].back.y-2,cube[rec].back.z+1);
				break;
		}
		cube[rec].theta.y=-45;
		cube[dom].theta.y=-45;
		cube[dom].pos = glm::vec3(cube[dom].back.x,cube[dom].back.y-1,cube[dom].back.z);
	}
}
void CubeToppleSouth(){
	cube[dom].pos = glm::vec3(  cube[dom].back.x,cube[dom].back.y - cube[dom].scale.y + cube[dom].scale.z*sin(cube[dom].theta.y*M_PI/180.0f)*sqrt(2),
			floor_grey.scale.z + cube[dom].scale.z*cos(cube[dom].theta.y*M_PI/180.0f)*sqrt(2));

	cube[dom].theta.y += cube[dom].dr;

	if(cube[dom].theta.y <= cube[dom].limit.y){
		toppling=0;
		cube[dom].theta.y=-45;
		cube[dom].pos = glm::vec3(cube[dom].back.x,cube[dom].back.y-1,cube[dom].back.z);
	}
}

void CuboidToppleWest(){

	cube[dom].pos = glm::vec3(cube[dom].back.x-cube[dom].scale.y+cube[dom].scale.z*sin(cube[dom].theta.x*M_PI/180.0f)*sqrt(2),cube[dom].back.y,
	floor_grey.scale.z+ cube[dom].scale.z*cos(cube[dom].theta.x*M_PI/180.0f)*sqrt(2)
			);
	switch(mode){
		case 1:	cube[rec].pos = glm::vec3(cube[dom].pos.x - cos(45+cube[dom].theta.x*M_PI/180.0f),cube[dom].pos.y,cube[dom].pos.z + sin(45+cube[dom].theta.x*M_PI/180.0f));
		break;
		case 2: cube[rec].pos = glm::vec3(cube[dom].pos.x,cube[rec].pos.y,cube[dom].pos.z);
		break;
		case 3:	cube[rec].pos = glm::vec3(cube[dom].pos.x + sin(45+cube[dom].theta.x*M_PI/180.0f),cube[dom].pos.y,cube[dom].pos.z + cos(45+cube[dom].theta.x*M_PI/180.0f));
		break;
	}
	cube[rec].theta.x += cube[rec].dr;
	cube[dom].theta.x += cube[dom].dr;
	if(cube[dom].theta.x <= cube[dom].limit.x){
		toppling=0;
		switch(mode){
			case 1:cube[rec].pos = glm::vec3(cube[rec].back.x-2,cube[rec].back.y,cube[rec].back.z-1);
			break;
			case 2:cube[rec].pos = glm::vec3(cube[rec].back.x-1,cube[rec].back.y,cube[rec].back.z);
			break;
			case 3:cube[rec].pos = glm::vec3(cube[rec].back.x-2,cube[rec].back.y,cube[rec].back.z+1);
			break;
		}
		cube[rec].theta.x=-45;
		cube[dom].theta.x=-45;
		cube[dom].pos = glm::vec3(cube[dom].back.x-1,cube[dom].back.y,cube[dom].back.z);
	}
}
void CubeToppleWest(){
	cube[dom].pos = glm::vec3(  cube[dom].back.x - cube[dom].scale.y + cube[dom].scale.z*sin(cube[dom].theta.x*M_PI/180.0f)*sqrt(2),cube[dom].back.y,
	floor_grey.scale.z+cube[dom].scale.z*cos(cube[dom].theta.x*M_PI/180.0f)*sqrt(2));

	cube[dom].theta.x += cube[dom].dr;

	if(cube[dom].theta.x <= cube[dom].limit.x){
		toppling=0;
		cube[dom].theta.x=-45;
		cube[dom].pos = glm::vec3(cube[dom].back.x-1,cube[dom].back.y,cube[dom].back.z);
	}
}
void CuboidToppleNorth(){
	cube[dom].pos = glm::vec3(cube[dom].back.x,
		cube[dom].back.y + cube[dom].scale.y + cube[dom].scale.z*sin(cube[dom].theta.y*M_PI/180.0f)*sqrt(2),
	floor_grey.scale.z + cube[dom].scale.z*cos(cube[dom].theta.y*M_PI/180.0f)*sqrt(2));
	switch(mode){
		case 1: cube[rec].pos = glm::vec3(  cube[dom].back.x,cube[dom].pos.y - cos(45+cube[dom].theta.y*M_PI/180.0f),cube[dom].pos.z + sin(45+cube[dom].theta.y*M_PI/180.0f));
		break;
		case 2: cube[rec].pos = glm::vec3(  cube[rec].back.x,cube[dom].pos.y,cube[dom].pos.z);
		break;
		case 3: cube[rec].pos = glm::vec3(  cube[dom].back.x,cube[dom].pos.y + sin(45+cube[dom].theta.y*M_PI/180.0f),cube[dom].pos.z + cos(45+cube[dom].theta.y*M_PI/180.0f));
		break;
	}
	cube[rec].theta.y += cube[rec].dr;
	cube[dom].theta.y += cube[dom].dr;
	if(cube[dom].theta.y >= cube[dom].limit.y){
		toppling=0;
		switch(mode){
			case 1: cube[rec].pos = glm::vec3(cube[rec].back.x,cube[rec].back.y+2,cube[rec].back.z+1);
			break;
			case 2: cube[rec].pos = glm::vec3(cube[rec].back.x,cube[rec].back.y+1,cube[rec].back.z);
			break;
			case 3: cube[rec].pos = glm::vec3(cube[rec].back.x,cube[rec].back.y+2,cube[rec].back.z-1);
			break;
		}
		cube[rec].theta.y=cube[rec].limit.y;
		cube[dom].theta.y=cube[rec].limit.y;
		cube[dom].pos = glm::vec3(cube[dom].back.x,cube[dom].back.y+1,cube[dom].back.z);
	}
}
void CubeToppleNorth(){
	cube[dom].pos = glm::vec3(cube[dom].back.x,cube[dom].back.y + cube[dom].scale.y + cube[dom].scale.z*sin(cube[dom].theta.y*M_PI/180.0f)*sqrt(2),
			floor_grey.scale.z + cube[dom].scale.z*cos(cube[dom].theta.y*M_PI/180.0f)*sqrt(2));

	cube[dom].theta.y += cube[dom].dr;

	if(cube[dom].theta.y >= cube[dom].limit.y){
		toppling=0;
		cube[dom].theta.y=cube[dom].limit.y;
		cube[dom].pos = glm::vec3(cube[dom].back.x,cube[dom].back.y+1,cube[dom].back.z);
	}
}





void CubeActivateTopple(int dir){
	if(toppling !=0)
		return;

	if(dir==1){
		moves[current_level]++;
		if(merged!=1){
			cube[dom].back = cube[dom].pos;
			rec = 1-chosen;
			cube[dom].dr = cube[dom].speed;
			toppling = dir;
			cube[dom].limit.y  = 90 + cube[dom].theta.y ;
			dom = chosen;
			cube[dom].theta.y = cube[dom].ori.y = -45;
		}
		else{
			dom = DominantNorth();
			rec = 1 - dom;
			mode = changemodeNorth();
			toppling = dir;

			cube[dom].back = cube[dom].pos;
			cube[rec].back = cube[rec].pos;

			cube[dom].dr = cube[dom].speed;
			cube[rec].dr = cube[rec].speed;

			cube[dom].theta.y = cube[dom].ori.y = -45;
			cube[dom].limit.y  = 90 + cube[dom].theta.y ;
			cube[rec].limit.y  = 90 + cube[rec].theta.y ;

		}
	}
	else if(dir==2){
		moves[current_level]++;
		if(merged!=1){

						toppling = dir;
						dom = chosen;
						rec = 1 - dom;

						cube[dom].back = cube[dom].pos;

						cube[dom].dr = -cube[dom].speed;
						cube[dom].theta.y = cube[dom].ori.y = 45;
						cube[dom].limit.y  = -90 + cube[dom].theta.y ;
		}
		else{

						dom = DominantSouth();
						rec = 1 - dom;
						mode = changemodeSouth();
						toppling = dir;
						cube[dom].back = cube[dom].pos;
						cube[rec].back = cube[rec].pos;

						cube[dom].dr = -cube[dom].speed;
						cube[rec].dr = -cube[rec].speed;
						cube[dom].theta.y = cube[dom].ori.y = 45;
						cube[dom].limit.y  = -90 + cube[dom].theta.y ;
						cube[rec].limit.y  = -90 + cube[rec].theta.y ;


		}
	}
	else if(dir==3){
		moves[current_level]++;
		if(merged!=1){

						dom = chosen;
						rec = 1 - dom;

						toppling = dir;
						cube[dom].back = cube[dom].pos;
						cube[dom].dr = -cube[dom].speed;
						cube[dom].theta.x = cube[dom].ori.x = 45;
						cube[dom].limit.x  = -90 + cube[dom].theta.x ;
		}
		else{
			dom = DominantWest();
			rec = 1 - dom;
			mode = WestMode();
			toppling = dir;
			cube[dom].theta.x = cube[dom].ori.x = 45;
			cube[rec].back = cube[rec].pos;
			cube[dom].back = cube[dom].pos;
			cube[rec].dr = -cube[rec].speed;
			cube[dom].dr = -cube[dom].speed;
			cube[rec].limit.x  = -90 + cube[rec].theta.x ;
			cube[dom].limit.x  = -90 + cube[dom].theta.x ;
		}
	}
	else if(dir==4){
		moves[current_level]++;
		if(merged!=1){
			dom = chosen;
			rec = 1 - dom;
			toppling=dir;

			cube[dom].back = cube[dom].pos;

			cube[dom].dr = cube[dom].speed;

			cube[dom].theta.x = cube[dom].ori.x = -45;
			cube[dom].limit.x  = 90 + cube[dom].theta.x ;

		}
		else{
			dom = DominantEast();
			rec = 1 - dom;
			mode = EastMode();
			toppling=dir;
			cube[rec].back = cube[rec].pos;
			cube[dom].back = cube[dom].pos;

			cube[rec].dr = cube[rec].speed;
			cube[dom].dr = cube[dom].speed;

			cube[dom].theta.x = cube[dom].ori.x = -45;
			cube[rec].limit.x  = 90 + cube[rec].theta.x ;
			cube[dom].limit.x  = 90 + cube[dom].theta.x ;
			}
	}
}
/* Executed for character input (like in text boxes) */

/* Executed when a mouse button is pressed/released */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Function is called first on GLFW_PRESS.

	if (action == GLFW_RELEASE) {
		changeview();
		switch (key) {
			case GLFW_KEY_LEFT:
				buttons["LEFT"]=false;
				break;
			case GLFW_KEY_RIGHT:
				buttons["RIGHT"]=false;
				break;
			case GLFW_KEY_UP:
				buttons["UP"]=false;
				break;
			case GLFW_KEY_DOWN:
				buttons["DOWN"]=false;
				break;
			case GLFW_KEY_W:
				buttons["W"]=false;
				break;
			case GLFW_KEY_S:
				buttons["S"]=false;
				break;
			case GLFW_KEY_A:
				buttons["A"]=false;
				break;
			case GLFW_KEY_D:
				buttons["D"]=false;
				break;
			case GLFW_KEY_SPACE:
				chosen = 1-chosen;
				// do something ..
				break;
			default:
				break;
		}
	}
	else if (action == GLFW_PRESS) {
		switch (key) {
			case GLFW_KEY_ESCAPE:
				quit(window);
				break;
				if(buttons["LEFT"])
	// 		CubeActivateTopple(3);
	// if(buttons["RIGHT"])
	// 		CubeActivateTopple(4);
	// if(buttons["UP"])
	// 		CubeActivateTopple(1);
	// if(buttons["DOWN"])
	// 		CubeActivateTopple(2);
			case GLFW_KEY_UP:
			CubeActivateTopple(1);

			case GLFW_KEY_DOWN:
			CubeActivateTopple(2);
			case GLFW_KEY_LEFT:
			CubeActivateTopple(3);
				// buttons["LEFT"]=true;
			break;
			case GLFW_KEY_RIGHT:
			CubeActivateTopple(4);
				// buttons["RIGHT"]=true;
				break;
				case GLFW_KEY_W:
				buttons["W"]=true;
				break;
			case GLFW_KEY_S:
				buttons["S"]=true;
				break;
			case GLFW_KEY_A:
				buttons["A"]=true;
				break;
			case GLFW_KEY_D:
				buttons["D"]=true;
				break;
			default:
				break;
		}
	}
}

void keyboardChar (GLFWwindow* window, unsigned int key)
{
	system("aplay -q blip.wav &");
	switch (key) {
		case 'Q':
		case 'q':
			quit(window);
			break;

		case 'w':
			camera_rotation_angle_y+=10;

			break;
		case 's':
			camera_rotation_angle_y-=10;
			break;
		case 'a':
			camera_rotation_angle_x-=10;

			break;
		case 'd':
			camera_rotation_angle_x+=10;


			break;
		case 'j':
			a1-=1;
			changeview();
			break;
		case 'l':
			a1+=1;
			changeview();
			break;
		case 'i':
			a2+=1;
			changeview();
			break;
		case 'k':
			a2-=1;
			changeview();
			break;
		case 'p':
			paused=!paused;
			break;
		case 'r':
			Initialize();
			break;
		case 'v':
			choice=(choice+1)%4;
			changeview();
		case 'g':
			break;
		case ' ':
			break;
		default:
			break;
	}
}

void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
	switch (button) {
		case GLFW_MOUSE_BUTTON_RIGHT:
			if (action == GLFW_RELEASE) {
				rectangle_rot_dir *= -1;
			}
			break;
		default:
			break;
	}
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
	int fbwidth=width, fbheight=height;
	glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = M_PI/2;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// Store the projection matrix in a variable for future use
	// Perspective projection for 3D views
	Matrices.projection = glm::perspective(fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

	// Ortho projection for 2D views
	//Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}

VAO *rectangle, *cam, *floor_vao, *rectangle_grey, *rectangle_orange, *rectangle_grey_line;

// Creates the rectangle object used in this sample code

void createRectangle ()
{
	floor_grey.pos = floor_orange.pos = glm::vec3(0,0,0);
	floor_grey.scale = floor_orange.scale = glm::vec3(0.5,0.5,0.1);

	cube[0].scale = cube[0].scale = glm::vec3(0.5,0.5,0.5);
	cube[0].pos = cube[0].pos = glm::vec3(0,0,floor_grey.scale.z+cube[0].scale.z);
	cube[0].speed =	 10;

	cube[1].scale = cube[1].scale = glm::vec3(0.5,0.5,0.5);
	cube[1].pos = cube[1].pos = glm::vec3(1,0,floor_grey.scale.z+cube[1].scale.z);
	cube[1].speed =	 10;

	// GL3 accepts only Triangles. Quads are not supported
	static const GLfloat vertex_buffer_data[] = {
		-1.0f,-1.0f,-1.0f, // triangle 1 : begin
		-1.0f,-1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f, // triangle 1 : end
		1.0f, 1.0f,-1.0f, // triangle 2 : begin
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f,-1.0f, // triangle 2 : end
		1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f,-1.0f,
		1.0f,-1.0f,-1.0f,
		1.0f, 1.0f,-1.0f,
		1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f,-1.0f,
		1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f,-1.0f, 1.0f,
		1.0f,-1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f,-1.0f,-1.0f,
		1.0f, 1.0f,-1.0f,
		1.0f,-1.0f,-1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f,-1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f,-1.0f,
		-1.0f, 1.0f,-1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f,-1.0f, 1.0f
	};
	float grey=(float)112/255;
	float line=(float)50/255;
	float b=(float)112/255;
	COLOR orange,green, red;
	orange.r=1;
	orange.g=162.0f/255.0f;
	orange.b=0;
	green.r = (float)144/255;
	green.g = (float)238 /255;
	green.b = (float)144/255;
	red.r = (float)1;
	red.g = (float)0 /255;
	red.b = (float)0/255;
	static const GLfloat color_buffer_data_grey[] = {
		grey,  grey,  grey,
		grey,  grey,  grey,
		grey,  grey,  grey,
		grey,  grey,  grey,
		grey,  grey,  grey,
		grey,  grey,  grey,
		grey,  grey,  grey,
		grey,  grey,  grey,
		grey,  grey,  grey,
		grey,  grey,  grey,
		grey,  grey,  grey,
		grey,  grey,  grey,
		grey,  grey,  grey,
		grey,  grey,  grey,
		grey,  grey,  grey,
		grey,  grey,  grey,
		grey,  grey,  grey,
		grey,  grey,  grey,
		grey,  grey,  grey,
		grey,  grey,  grey,
		grey,  grey,  grey,
		grey,  grey,  grey,
		grey,  grey,  grey,
		grey,  grey,  grey,
		grey,  grey,  grey,
		grey,  grey,  grey,
		grey,  grey,  grey,
		grey,  grey,  grey,
		grey,  grey,  grey,
		grey,  grey,  grey,
		grey,  grey,  grey,
		grey,  grey,  grey,
		grey,  grey,  grey,
		grey,  grey,  grey,
		grey,  grey,  grey,
		grey,  grey,  grey
	};
	static const GLfloat color_buffer_data_green[] = {
		green.r,  green.g,  green.b,
		green.r,  green.g,  green.b,
		green.r,  green.g,  green.b,
		green.r,  green.g,  green.b,
		green.r,  green.g,  green.b,
		green.r,  green.g,  green.b,
		green.r,  green.g,  green.b,
		green.r,  green.g,  green.b,
		green.r,  green.g,  green.b,
		green.r,  green.g,  green.b,
		green.r,  green.g,  green.b,
		green.r,  green.g,  green.b,
		green.r,  green.g,  green.b,
		green.r,  green.g,  green.b,
		green.r,  green.g,  green.b,
		green.r,  green.g,  green.b,
		green.r,  green.g,  green.b,
		green.r,  green.g,  green.b,
		green.r,  green.g,  green.b,
		green.r,  green.g,  green.b,
		green.r,  green.g,  green.b,
		green.r,  green.g,  green.b,
		green.r,  green.g,  green.b,
		green.r,  green.g,  green.b,
		green.r,  green.g,  green.b,
		green.r,  green.g,  green.b,
		green.r,  green.g,  green.b,
		green.r,  green.g,  green.b,
		green.r,  green.g,  green.b,
		green.r,  green.g,  green.b,
		green.r,  green.g,  green.b,
		green.r,  green.g,  green.b,
		green.r,  green.g,  green.b,
		green.r,  green.g,  green.b,
		green.r,  green.g,  green.b,
		green.r,  green.g,  green.b
	};
	static const GLfloat color_buffer_data_red[] = {
		red.r,  red.g,  red.b,
		red.r,  red.g,  red.b,
		red.r,  red.g,  red.b,
		red.r,  red.g,  red.b,
		red.r,  red.g,  red.b,
		red.r,  red.g,  red.b,
		red.r,  red.g,  red.b,
		red.r,  red.g,  red.b,
		red.r,  red.g,  red.b,
		red.r,  red.g,  red.b,
		red.r,  red.g,  red.b,
		red.r,  red.g,  red.b,
		red.r,  red.g,  red.b,
		red.r,  red.g,  red.b,
		red.r,  red.g,  red.b,
		red.r,  red.g,  red.b,
		red.r,  red.g,  red.b,
		red.r,  red.g,  red.b,
		red.r,  red.g,  red.b,
		red.r,  red.g,  red.b,
		red.r,  red.g,  red.b,
		red.r,  red.g,  red.b,
		red.r,  red.g,  red.b,
		red.r,  red.g,  red.b,
		red.r,  red.g,  red.b,
		red.r,  red.g,  red.b,
		red.r,  red.g,  red.b,
		red.r,  red.g,  red.b,
		red.r,  red.g,  red.b,
		red.r,  red.g,  red.b,
		red.r,  red.g,  red.b,
		red.r,  red.g,  red.b,
		red.r,  red.g,  red.b,
		red.r,  red.g,  red.b,
		red.r,  red.g,  red.b,
		red.r,  red.g,  red.b
	};
	float black=(float)0/255;
	static const GLfloat color_buffer_data_black[] = {
		black,  black,  black,
		black,  black,  black,
		black,  black,  black,
		black,  black,  black,
		black,  black,  black,
		black,  black,  black,
		black,  black,  black,
		black,  black,  black,
		black,  black,  black,
		black,  black,  black,
		black,  black,  black,
		black,  black,  black,
		black,  black,  black,
		black,  black,  black,
		black,  black,  black,
		black,  black,  black,
		black,  black,  black,
		black,  black,  black,
		black,  black,  black,
		black,  black,  black,
		black,  black,  black,
		black,  black,  black,
		black,  black,  black,
		black,  black,  black,
		black,  black,  black,
		black,  black,  black,
		black,  black,  black,
		black,  black,  black,
		black,  black,  black,
		black,  black,  black,
		black,  black,  black,
		black,  black,  black,
		black,  black,  black,
		black,  black,  black,
		black,  black,  black,
		black,  black,  black
	};
	static const GLfloat color_buffer_data_orange[] = {
		orange.r,  orange.g,  orange.b,
		orange.r,  orange.g,  orange.b,
		orange.r,  orange.g,  orange.b,
		orange.r,  orange.g,  orange.b,
		orange.r,  orange.g,  orange.b,
		orange.r,  orange.g,  orange.b,
		orange.r,  orange.g,  orange.b,
		orange.r,  orange.g,  orange.b,
		orange.r,  orange.g,  orange.b,
		orange.r,  orange.g,  orange.b,
		orange.r,  orange.g,  orange.b,
		orange.r,  orange.g,  orange.b,
		orange.r,  orange.g,  orange.b,
		orange.r,  orange.g,  orange.b,
		orange.r,  orange.g,  orange.b,
		orange.r,  orange.g,  orange.b,
		orange.r,  orange.g,  orange.b,
		orange.r,  orange.g,  orange.b,
		orange.r,  orange.g,  orange.b,
		orange.r,  orange.g,  orange.b,
		orange.r,  orange.g,  orange.b,
		orange.r,  orange.g,  orange.b,
		orange.r,  orange.g,  orange.b,
		orange.r,  orange.g,  orange.b,
		orange.r,  orange.g,  orange.b,
		orange.r,  orange.g,  orange.b,
		orange.r,  orange.g,  orange.b,
		orange.r,  orange.g,  orange.b,
		orange.r,  orange.g,  orange.b,
		orange.r,  orange.g,  orange.b,
		orange.r,  orange.g,  orange.b,
		orange.r,  orange.g,  orange.b,
		orange.r,  orange.g,  orange.b,
		orange.r,  orange.g,  orange.b,
		orange.r,  orange.g,  orange.b,
		orange.r,  orange.g,  orange.b
	};
	cube[1].color.r = cube[0].color.r=1;
	cube[1].color.g = cube[0].color.g=162.0f/255.0f;
	cube[1].color.b = cube[0].color.b=200.0f/255.0f;
	static const GLfloat color_buffer_data_cube1[] = {
		cube[0].color.r,  cube[0].color.g,  cube[0].color.b,
		cube[0].color.r,  cube[0].color.g,  cube[0].color.b,
		cube[0].color.r,  cube[0].color.g,  cube[0].color.b,
		cube[0].color.r,  cube[0].color.g,  cube[0].color.b,
		cube[0].color.r,  cube[0].color.g,  cube[0].color.b,
		cube[0].color.r,  cube[0].color.g,  cube[0].color.b,
		cube[0].color.r,  cube[0].color.g,  cube[0].color.b,
		cube[0].color.r,  cube[0].color.g,  cube[0].color.b,
		cube[0].color.r,  cube[0].color.g,  cube[0].color.b,
		cube[0].color.r,  cube[0].color.g,  cube[0].color.b,
		cube[0].color.r,  cube[0].color.g,  cube[0].color.b,
		cube[0].color.r,  cube[0].color.g,  cube[0].color.b,
		cube[0].color.r,  cube[0].color.g,  cube[0].color.b,
		cube[0].color.r,  cube[0].color.g,  cube[0].color.b,
		cube[0].color.r,  cube[0].color.g,  cube[0].color.b,
		cube[0].color.r,  cube[0].color.g,  cube[0].color.b,
		cube[0].color.r,  cube[0].color.g,  cube[0].color.b,
		cube[0].color.r,  cube[0].color.g,  cube[0].color.b,
		cube[0].color.r,  cube[0].color.g,  cube[0].color.b,
		cube[0].color.r,  cube[0].color.g,  cube[0].color.b,
		cube[0].color.r,  cube[0].color.g,  cube[0].color.b,
		cube[0].color.r,  cube[0].color.g,  cube[0].color.b,
		cube[0].color.r,  cube[0].color.g,  cube[0].color.b,
		cube[0].color.r,  cube[0].color.g,  cube[0].color.b,
		cube[0].color.r,  cube[0].color.g,  cube[0].color.b,
		cube[0].color.r,  cube[0].color.g,  cube[0].color.b,
		cube[0].color.r,  cube[0].color.g,  cube[0].color.b,
		cube[0].color.r,  cube[0].color.g,  cube[0].color.b,
		cube[0].color.r,  cube[0].color.g,  cube[0].color.b,
		cube[0].color.r,  cube[0].color.g,  cube[0].color.b,
		cube[0].color.r,  cube[0].color.g,  cube[0].color.b,
		cube[0].color.r,  cube[0].color.g,  cube[0].color.b,
		cube[0].color.r,  cube[0].color.g,  cube[0].color.b,
		cube[0].color.r,  cube[0].color.g,  cube[0].color.b,
		cube[0].color.r,  cube[0].color.g,  cube[0].color.b,
		cube[0].color.r,  cube[0].color.g,  cube[0].color.b
	};
	static const GLfloat color_buffer_data_line[] = {
		line,  line,  line,
		line,  line,  line,
		line,  line,  line,
		line,  line,  line,
		line,  line,  line,
		line,  line,  line,
		line,  line,  line,
		line,  line,  line,
		line,  line,  line,
		line,  line,  line,
		line,  line,  line,
		line,  line,  line,
		line,  line,  line,
		line,  line,  line,
		line,  line,  line,
		line,  line,  line,
		line,  line,  line,
		line,  line,  line,
		line,  line,  line,
		line,  line,  line,
		line,  line,  line,
		line,  line,  line,
		line,  line,  line,
		line,  line,  line,
		line,  line,  line,
		line,  line,  line,
		line,  line,  line,
		line,  line,  line,
		line,  line,  line,
		line,  line,  line,
		line,  line,  line,
		line,  line,  line,
		line,  line,  line,
		line,  line,  line,
		line,  line,  line,
		line,  line,  line
	};


	// create3DObject creates and returns a handle to a VAO that can be used later
	floor_grey.object = create3DObject(GL_TRIANGLES, 13*3, vertex_buffer_data, color_buffer_data_grey, GL_FILL);

	floor_black.object = create3DObject(GL_TRIANGLES, 13*3, vertex_buffer_data, color_buffer_data_black, GL_FILL);

	floor_red.object = create3DObject(GL_TRIANGLES, 13*3, vertex_buffer_data, color_buffer_data_red, GL_FILL);

	floor_green.object = create3DObject(GL_TRIANGLES, 13*3, vertex_buffer_data, color_buffer_data_green, GL_FILL);

	floor_black.line = floor_grey.line = floor_orange.line = cube[1].line =cube[0].line = create3DObject(GL_TRIANGLES, 13*3, vertex_buffer_data, color_buffer_data_line, GL_LINE);

	floor_orange.object = create3DObject(GL_TRIANGLES, 13*3, vertex_buffer_data, color_buffer_data_orange, GL_FILL);

	cube[0].object = create3DObject(GL_TRIANGLES, 13*3, vertex_buffer_data, color_buffer_data_cube1, GL_FILL);

	cube[1].object = create3DObject(GL_TRIANGLES, 13*3, vertex_buffer_data, color_buffer_data_cube1, GL_FILL);


}
void createCam ()
{
	// GL3 accepts only Triangles. Quads are not supported
	static const GLfloat color_buffer_data [] = {
		1, 1, 1,
		1, 1, 1,
		1, 1, 1,
	};
	static const GLfloat vertex_buffer_data [] = {
		-0.1, 0, 0,
		0.1, 0, 0,
		0, 0.1, 0,
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	cam = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
}
void createFloor ()
{
	// GL3 accepts only Triangles. Quads are not supported


	static const GLfloat color_buffer_data [] = {
		0.65, 0.165, 0.165,
		0.65, 0.165, 0.165,
		0.65, 0.165, 0.165,
		0.65, 0.165, 0.165,
		0.65, 0.165, 0.165,
		0.65, 0.165, 0.165,
	};

	static const GLfloat vertex_buffer_data [] = {
		-2, -1, 2,
		2, -1, 2,
		-2, -1, -2,
		-2, -1, -2,
		2, -1, 2,
		2, -1, -2,
	};
	// create3DObject creates and returns a handle to a VAO that can be used later
	floor_vao = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw (GLFWwindow* window, float x, float y, float w, float h, int doM, int doV, int doP){
	int fbwidth, fbheight;
	glfwGetFramebufferSize(window, &fbwidth, &fbheight);
	glViewport((int)(x*fbwidth), (int)(y*fbheight), (int)(w*fbwidth), (int)(h*fbheight));


	// use the loaded shader program
	// Don't change unless you know what you are doing
	glUseProgram(programID);

	// Eye - Location of camera. Don't change unless you are sure!!
	glm::vec3 eye (eyearr[0],eyearr[1],eyearr[2] );
	// Target - Where is the camera looking at.  Don't change unless you are sure!!
	glm::vec3 target (targetarr[0], targetarr[1], targetarr[2]);
	// Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
	glm::vec3 up (uparr[0], uparr[1], uparr[2]);

	// Compute Camera matrix (view)
	if(!doV)
		Matrices.view = glm::mat4(1.0f);
	else
		Matrices.view = glm::lookAt(eye, target, up); // Fixed camera for 2D (ortho) in XY plane


	// Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
	glm::mat4 VP;
	if (!doP)
		VP = Matrices.view;
	else
		VP = Matrices.projection * Matrices.view;

	glm::mat4 MVP;	// MVP = Projection * View * Model

	// Send our transformation to the currently bound shader, in the "MVP" uniform
	// For each model you render, since the MVP will be different (at least the M part)
	for(int i=0;i<dim;i++){
		for(int j=0;j<dim;j++){

			// Load identity to model matrix
			Matrices.model = glm::mat4(1.0f);

			glm::mat4 translateRectangle = glm::translate (glm::vec3(floor_grey.pos.x+i, floor_grey.pos.y+j, floor_grey.pos.z));        // glTranslatef
			glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));
			glm::mat4 myScalingMatrix = glm::scale(glm::mat4(1.0f),floor_grey.scale);
			Matrices.model *= (translateRectangle * rotateRectangle * myScalingMatrix);
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

			// draw3DObject draws the VAO given to it using current MVP matrix
			if(boardMatrix[i][j]==1)
				draw3DObject(floor_grey.object);
			if(boardMatrix[i][j]==2)
				draw3DObject(floor_orange.object);
			if(boardMatrix[i][j]==7)
					draw3DObject(floor_red.object);
			if(boardMatrix[i][j]==8)
				draw3DObject(floor_green.object);
			if(boardMatrix[i][j]==9)
				draw3DObject(floor_black.object);

			draw3DObject(floor_grey.line);
		}
	}

	// Load identity to model matrix
	Matrices.model = glm::mat4(1.0f);

	glm::mat4 translateCam = glm::translate(eye);
	glm::mat4 rotateCamX = glm::rotate((float)((90 - camera_rotation_angle_x)*M_PI/180.0f), glm::vec3(0,1,0));
	glm::mat4 rotateCamY = glm::rotate((float)((90 - camera_rotation_angle_y)*M_PI/180.0f), glm::vec3(0,1,0));
	Matrices.model *= (translateCam * rotateCamX*rotateCamY);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

	// draw3DObject draws the VAO given to it using current MVP matrix
	draw3DObject(cam);

	Matrices.model = glm::mat4(1.0f);

	glm::mat4 translateCube1 = glm::translate (cube[0].pos);        // glTranslatef
	glm::mat4 rotateCube1X = glm::rotate((float)(-(cube[0].theta.x+45)*M_PI/180.0f), glm::vec3(0,-1,0));
	glm::mat4 rotateCube1Y = glm::rotate((float)(-(cube[0].theta.y+45)*M_PI/180.0f), glm::vec3(1,0,0));
	glm::mat4 myScalingMatrix2 = glm::scale(glm::mat4(1.0f),cube[0].scale);
	Matrices.model *= (translateCube1 * rotateCube1X * rotateCube1Y * myScalingMatrix2);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

	// draw3DObject draws the VAO given to it using current MVP matrix

	draw3DObject(cube[0].object);
	draw3DObject(cube[0].line);


	Matrices.model = glm::mat4(1.0f);

	glm::mat4 translateCube2 = glm::translate (cube[1].pos);        // glTranslatef
	glm::mat4 rotateCube2X = glm::rotate((float)(-(cube[1].theta.x+45)*M_PI/180.0f), glm::vec3(0,-1,0));
	glm::mat4 rotateCube2Y = glm::rotate((float)(-(cube[1].theta.y+45)*M_PI/180.0f), glm::vec3(1,0,0));
	glm::mat4 myScalingMatrix = glm::scale(glm::mat4(1.0f),cube[1].scale);
	Matrices.model *= (translateCube2 * rotateCube2X * rotateCube2Y * myScalingMatrix);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

	// draw3DObject draws the VAO given to it using current MVP matrix

	draw3DObject(cube[1].object);
	draw3DObject(cube[1].line);



}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height){
	GLFWwindow* window; // window desciptor/handle

	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	window = glfwCreateWindow(width, height, "3D Block Game", NULL, NULL);

	if (!window) {
		exit(EXIT_FAILURE);
		glfwTerminate();
	}

	glfwMakeContextCurrent(window);
	//    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	glfwSwapInterval( 1 );
	glfwSetFramebufferSizeCallback(window, reshapeWindow);
	glfwSetWindowSizeCallback(window, reshapeWindow);
	glfwSetWindowCloseCallback(window, quit);
	glfwSetKeyCallback(window, keyboard);      // general keyboard input
	glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling
	glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

	return window;
}

int oneofthetwo,other;
/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
	/* Objects should be created before any other gl function and shaders */
	// Create the models
	createRectangle ();
	createCam();
	createFloor();

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (window, width, height);

	// Background color of the scene
	glClearColor (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

	// cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
	// cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
	// cout << "VERSION: " << glGetString(GL_VERSION) << endl;
	// cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}
int fall_checker(){
	if((int)cube[0].pos.y>=dim ||(int)cube[0].pos.y<0){
		return 0;
	}
	if((int)cube[1].pos.x>=dim || (int)cube[1].pos.x<0)
		return 1;
	if(boardMatrix[(int)cube[0].pos.x][(int)cube[0].pos.y]==0){
		return 0;
	}
	if((int)cube[1].pos.y>=dim ||(int)cube[1].pos.y<0){
		return 1;
	}
	if((int)cube[0].pos.x>=dim || (int)cube[0].pos.x<0){
		return 0;
	}
	if(boardMatrix[(int)cube[1].pos.x][(int)cube[1].pos.y]==0)
	 	return 1;

	return -1;
}
void merge_checker(){
	if((abs((int)cube[0].pos.y - (int)cube[1].pos.y)==1&&abs((int)cube[0].pos.x - (int)cube[1].pos.x)==0) || (abs((int)cube[0].pos.y - (int)cube[1].pos.y)==0 &&abs((int)cube[0].pos.x - (int)cube[1].pos.x)==1) ){
		if(merged==0 && toppling==0 && falling==0){
			merged=1;
		}
	}
}
void black_checker(){
	if(cube[0].pos.y==cube[1].pos.y && cube[0].pos.x ==cube[1].pos.x && abs(cube[0].pos.z-cube[1].pos.z)==1 && merged==1 && falling ==0 && boardMatrix[(int)cube[0].pos.x][(int)cube[0].pos.y]==9){
		falling = 1;
		oneofthetwo=0;
		other=1;
		right_move=true;
	}
}
void orange_checker(){
	if(cube[0].pos.y==cube[1].pos.y && cube[0].pos.x ==cube[1].pos.x && abs(cube[0].pos.z-cube[1].pos.z)==1&& toppling ==0 && merged==1 && falling ==0 && boardMatrix[(int)cube[0].pos.x][(int)cube[0].pos.y]==2){
		falling = 1;
		oneofthetwo=0;
		other=1;
	}
}
void cross_checker(){
	for(vector<cross_struct>::iterator it=levels[current_level].crosses.begin();it<levels[current_level].crosses.end();++it){
		if(merged==1){
			if(falling ==0){
				if(toppling==0)
				if(cube[0].pos.y==cube[1].pos.y && cube[0].pos.x==cube[1].pos.x &&((cube[0].pos.x==it->place.x&&cube[0].pos.y==it->place.y))){
					cube[0].pos = it->place;
					cube[1].pos = it->other;
					merged=0;
					chosen=0;
					break;
				}
			}
		}
	}
}
void switch_checker(){
	for(vector<switch_struct>::iterator it=levels[current_level].switches.begin();it<levels[current_level].switches.end();it++){
		if(falling ==0){
			if(toppling ==0){
				if(((cube[0].pos.x==it->place.x&&cube[0].pos.y==it->place.y)||(cube[1].pos.x==it->place.x&&cube[1].pos.y==it->place.y))){
					if(it->used==false){
						it->used=true;
						for(vector<glm::vec3>::iterator it2 = it->locations.begin();it2< it->locations.end();it2++){
							boardMatrix[(int)it2->x][(int)it2->y]=1;
						}
					}
				}
			}
		}
	}
}

void gameEngine(){
	merge_checker();
	black_checker();
	orange_checker();
	cross_checker();
	switch_checker();

	//faller
	if(fall_checker()!=-1 && toppling==0 && falling == 0){
		oneofthetwo = fall_checker();
		other =1-oneofthetwo;
		if(merged==1){
			int temp;
			if(cube[other].pos.x == cube[oneofthetwo].pos.x && cube[other].pos.y != cube[oneofthetwo].pos.y){
				for(int i=0;i<10;i++){
						temp++;
				}
			}
			else if(cube[other].pos.y == cube[oneofthetwo].pos.y && cube[other].pos.x != cube[oneofthetwo].pos.x){
				for(int i=0;i<10;i++){
						temp--;
				}
			}
			else{
				cube[other].pos.x = cube[oneofthetwo].pos.x;
				cube[other].pos.y = cube[oneofthetwo].pos.y;
				cube[other].pos.z = floor_grey.scale.z + cube[other].scale.z;
				cube[ oneofthetwo].pos.z = floor_grey.scale.z + 3*cube[other].scale.z;
			}

		}
		falling  = 1;
		toppling = 0;
	}
	if(falling ==1){
		if(merged==1){
			cube[oneofthetwo].pos.z -=0.1;
			cube[other].pos.z -=0.1;
			if(cube[oneofthetwo].pos.z <=-10){
				falling = 0;
				Initialize();
			}
		}
		else{
			cube[dom].pos.z -=0.1;
			if(cube[dom].pos.z <=-10){
				falling = 0;
				Initialize();
			}
		}
	}
	if(toppling!=0 && falling == 0  ){
		if(toppling == 1){
			if(merged==1)
				CuboidToppleNorth();
			else
				CubeToppleNorth();
		}
		if(toppling == 2){
			if(merged==1)
				CuboidToppleSouth();
			else
				CubeToppleSouth();
		}
		if(toppling == 3){
			if(merged==1)
				CuboidToppleWest();
			else
				CubeToppleWest();
		}
		if(toppling == 4){
			if(merged==1)
				CuboidToppleEast();
			else
				CubeToppleEast();
		}
	}
	// if(buttons["LEFT"])
	// 		CubeActivateTopple(3);
	// if(buttons["RIGHT"])
	// 		CubeActivateTopple(4);
	// if(buttons["UP"])
	// 		CubeActivateTopple(1);
	// if(buttons["DOWN"])
	// 		CubeActivateTopple(2);
}
void Level_creator(){
	//Level1
	Level_struct curr;
	int m1[10][10] = {
		{1,1,2,0,0,0,0,0,0,0},
		{1,1,1,1,2,1,0,0,0,0},
		{1,1,1,1,1,1,1,1,1,0},
		{0,1,1,1,1,1,1,1,1,1},
		{0,0,0,0,0,1,1,9,1,1},
		{0,0,0,0,0,0,1,1,1,0},
		{0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0}
	};
	for(int i=0;i<dim;i++){
		for(int j=0;j<dim;j++){
			curr.levelMatrix[i][j] = m1[i][j];
		}
	}
	curr.cube0_pos = glm::vec3(0,0,floor_grey.scale.z+ cube[0].scale.z);
	curr.cube1_pos = glm::vec3(0,1,floor_grey.scale.z+ cube[0].scale.z);
	levels.push_back(curr);

	//Level2
	int m2[10][10] = {
		{1,1,1,1,0,0,0,1,1,1},
		{1,1,1,1,0,0,0,1,9,1},
		{1,1,1,1,0,0,0,1,1,1},
		{1,1,1,1,0,0,1,1,1,1},
		{0,0,0,0,1,1,1,0,0,0},
		{0,0,0,0,1,1,1,0,0,0},
		{0,0,0,0,1,1,1,0,0,0},
		{0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0}
	};
	for(int i=0;i<dim;i++){
		for(int j=0;j<dim;j++){
			curr.levelMatrix[i][j] = m2[i][j];
		}
	}

	switch_struct sw;
	sw.used=false;
	sw.place = glm::vec3(2,2,0);
	sw.locations.push_back(glm::vec3(3,4,0));
	curr.switches.push_back(sw);


	cross_struct cw;
	cw.place = glm::vec3(6,4,floor_grey.scale.z+ cube[0].scale.z);
	cw.other = glm::vec3(4,6,floor_grey.scale.z+ cube[0].scale.z);
	curr.crosses.push_back(cw);

	levels.push_back(curr);
	Level_struct curr2;

	int m3[10][10] = {
		{1,1,1,1,2,2,2,2,0,0},
		{1,1,1,1,2,2,2,2,0,0},
		{1,1,1,1,0,0,0,1,1,1},
		{1,1,1,1,0,0,0,0,1,1},
		{0,0,0,0,0,0,0,0,1,1},
		{0,0,0,0,0,0,2,2,2,2},
		{0,1,1,1,1,1,2,2,2,2},
		{0,1,1,1,1,1,2,1,2,2},
		{0,1,9,1,0,0,2,2,2,2},
		{0,1,1,1,0,0,0,0,0,0}
	};
	int quo=5,tt=0;
	for(int i=0;i<quo;i++){
			tt++;
	}
	for(int i=0;i<dim;i++){
		for(int j=0;j<dim;j++){
			curr2.levelMatrix[j][i] = m3[j][i];
		}
	}

	switch_struct sw1;
	sw1.used=false;
	sw1.place = glm::vec3(2,7,0);
	sw1.locations.push_back(glm::vec3(4,1,0));
	sw1.locations.push_back(glm::vec3(5,1,0));
	curr2.switches.push_back(sw1);
	curr2.cube0_pos = glm::vec3(0,0,floor_grey.scale.z+ cube[0].scale.z);
	curr2.cube1_pos = glm::vec3(0,1,floor_grey.scale.z+ cube[0].scale.z);

	levels.push_back(curr2);
}
void changeview(){
	if(choice==0){
		//Tower View
		eyearr[0]=0;eyearr[1]=-2;eyearr[2]=10;
		targetarr[0]=0,targetarr[1]=0,targetarr[2]=0;
		uparr[0]=0,uparr[1]=0,uparr[2]=1;
	}
	if(choice==1){
		//Top View
		eyearr[0]=0;eyearr[1]=0;eyearr[2]=10;
		targetarr[0]=0,targetarr[1]=0,targetarr[2]=0;
		uparr[0]=0,uparr[1]=1,uparr[2]=0;
	}
	if(choice==2){
		//Followcam view
		float follow_cam_rotation=270.0f;
		eyearr[0]=cube[1].pos.x+5.0*cos(follow_cam_rotation*M_PI/180.0f);eyearr[1]=cube[1].pos.y+5.0*sin(follow_cam_rotation*M_PI/180.0f);eyearr[2]=5.0;
		targetarr[0]=cube[1].pos.x+2.0*cos(follow_cam_rotation*M_PI/180.0f),targetarr[1]=cube[1].pos.y+2.0*sin(follow_cam_rotation*M_PI/180.0f),targetarr[2]=3.0;
		uparr[0]=0,uparr[1]=0,uparr[2]=0.1;
	}
	if(choice==3){
		//Helicopter View
		eyearr[0]=5*cos(camera_rotation_angle_x*M_PI/180.0f);
		eyearr[1]=5*cos(camera_rotation_angle_y*M_PI/180.0f);
		eyearr[2]=5*sin(camera_rotation_angle_y*M_PI/180.0f)+5*sin(camera_rotation_angle_x*M_PI/180.0f);
		targetarr[0]=a1;
		targetarr[1]=a2;
		targetarr[2]=0;
		uparr[0]=0,uparr[1]=1,uparr[2]=0;
	}
}
int main (int argc, char** argv)
{
	a1=0,a2=0,a3=0,a4=0;
	choice=0;
	int width = 600,height = 600;

	do_rot = 0;
	int current_level=0;
	Level_creator();

	GLFWwindow* window = initGLFW(width, height);
	initGLEW();
	initGL (window, width, height);
	changeview();

	last_update_time = glfwGetTime();


	Initialize();
	score=0;

	/* Draw in loop */

	while (!glfwWindowShouldClose(window)) {

		// clear the color and depth in the frame buffer
		if(!paused){
			gameEngine();
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// OpenGL Draw commands
		current_time = glfwGetTime();
		if ((current_time - last_update_time) >= 1) { // atleast 0.5s elapsed since last frame
			timer[current_level]+=1;
			// do something every 0.5 seconds ..

		}

		last_update_time = current_time;
		draw(window, 0, 0, 1, 1, 0, 1, 1);
		// draw(window, 0.5, 0, 0.5, 0.5, 0, 1, 1);
		// draw(window, 0, 0.5, 0.5, 0.5, 1, 0, 1);
		// draw(window, 0.5, 0.5, 0.5, 0.5, 0, 0, 1);

		// Swap Frame Buffer in double buffering
		glfwSwapBuffers(window);

		// Poll for Keyboard and mouse events
		glfwPollEvents();
	}

	glfwTerminate();
	//    exit(EXIT_SUCCESS);
}
