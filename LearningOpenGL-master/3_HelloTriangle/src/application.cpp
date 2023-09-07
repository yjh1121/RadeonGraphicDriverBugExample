#include <iostream>
#include <GL\glew.h>
#include <GLFW\glfw3.h>

#define isTestTexture1 1

using namespace std;


int initGLFW();
int initGLEW();
void CreateTriangleVertexArrayObject();
void CreateTriangleVertexBuffer();
void RenderLoop();
void terminate();
GLuint fbo, tex1, tex2;
GLuint gProgram(0);
GLuint gProgramForward(0);
const GLchar* gVertexShaderSource[] =
{
	"#version 430 core\n"

	"layout (location = 0 ) in vec3 position;\n"
	"layout (location = 1 ) in vec4 color;\n"

	"layout (location = 0 ) out vec4 f_color;\n"

	"void main(void)\n"
	"{\n"
	"  f_color = color;\n"
	"  gl_Position = vec4(position,1.0f);\n"
	"}\n"
};

const GLchar* gFragmentShaderSource[] =
{
	"#version 430 core\n"

	"layout (location = 0 ) in vec4 color;\n"

	"layout (location = 0) out vec4 o_color;\n"
	"layout (location = 1) out vec4 o_color2;\n"

	"void main(void)\n"
	"{\n"
	"  o_color = color;\n"
	"  o_color2 = color;\n"
	"}\n"
};

const GLchar* gForwardVertShaderSource[] =
{
	"#version 430 core\n"

	"layout (location = 0 ) in vec3 position;\n"

	"void main(void)\n"
	"{\n"
	"  gl_Position = vec4(position,1.0f);\n"
	"}\n"
};

const GLchar* gForwardFragShaderSource[] =
{
	"#version 430 core\n"

	"uniform sampler2DRect tex1;\n"
	"uniform sampler2DRect tex2;\n"
	"out vec4 outColor;\n"

	"void main(void)\n"
	"{\n"
#if isTestTexture1 == 1
	"outColor = texture(tex1, gl_FragCoord.xy); \n"
#else
	"outColor = texture(tex2, gl_FragCoord.xy); \n"
#endif
	"	outColor.a = 1.0f;\n"
	"}\n"
};

GLFWwindow* window;

//Id of the vertex array object
unsigned int triangleVertexArrayID;

//ID of the vertex buffer object;
unsigned int triangleVertexBufferID;

unsigned int forwardVertexArrayID;
unsigned int forwardVertexBufferID;

//Vertex buffer data - the coordinates of the points of the triangle
//Coordinates are in screen coordinates: floats from -1.0f to 1.0f in the X,Y and Z plane
const float triangleVertexBuffer[] = {
	//Coordinates	  X		 Y	  Z
				   -1.0f, -1.0f, -1.0f,	1.0f, 0.0f, 0.0f, 1.0f,//Vertex 0
					1.0f, -1.0f, -1.0f,	1.0f, 0.0f, 0.0f, 1.0f,//Vertex 1
					0.0f,  1.0f, -1.0f,	1.0f, 0.0f, 0.0f, 1.0f,//Vertex 2
					0.0f, -1.0f, 0.0f,	1.0f, 0.0f, 0.0f, 0.5f,//Vertex 0
					2.0f, -1.0f, 0.0f,	1.0f, 0.0f, 0.0f, 0.5f,//Vertex 1
					1.0f,  1.0f, 0.0f,	1.0f, 0.0f, 0.0f, 0.5f,//Vertex 2

};

const float forwardVertexBuffer[] = {
	//Coordinates	  X		 Y	  Z
					-1.0f,  -1.0f, 0.0f,	
					1.0f,  -1.0f, 0.0f,
					-1.0f,  1.0f, 0.0f,
					1.0f,  -1.0f, 0.0f,
					1.0f,  1.0f, 0.0f,	
					-1.0f,  1.0f, 0.0f,
};


GLuint CompileShaders(const GLchar** vertexShaderSource, const GLchar** fragmentShaderSource)
{
	//Compile vertex shader
	GLuint vertexShader(glCreateShader(GL_VERTEX_SHADER));
	glShaderSource(vertexShader, 1, vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	int success;
	char infoLog[1024];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 1024, NULL, infoLog);
		std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
	}

	//Compile fragment shader
	GLuint fragmentShader(glCreateShader(GL_FRAGMENT_SHADER));
	glShaderSource(fragmentShader, 1, fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 1024, NULL, infoLog);
		std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
	}

	//Link vertex and fragment shader together
	GLuint program(glCreateProgram());
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	//Delete shaders objects
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return program;
}

void CreateFB()
{
	glGenFramebuffers(1, &fbo);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glGenTextures(1, &tex1);
	glBindTexture(GL_TEXTURE_RECTANGLE, tex1);
	glTexStorage2D(GL_TEXTURE_RECTANGLE, 1, GL_RGBA16, 640, 480);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, tex1, 0);
	glBindTexture(GL_TEXTURE_RECTANGLE, 0);

	glGenTextures(1, &tex2);
	glBindTexture(GL_TEXTURE_RECTANGLE, tex2);
	glTexStorage2D(GL_TEXTURE_RECTANGLE, 1, GL_RGBA16, 640, 480);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_RECTANGLE, tex2, 0);
	glBindTexture(GL_TEXTURE_RECTANGLE, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int main() {

	cout << "Hello Triangles" << endl;

	if (!initGLFW()) {
		return -1;
	}

	if (!initGLEW()) {
		terminate();
		return -1;
	}

	gProgram = CompileShaders(gVertexShaderSource, gFragmentShaderSource);
	gProgramForward = CompileShaders(gForwardVertShaderSource, gForwardFragShaderSource);

	CreateFB();
	CreateTriangleVertexArrayObject();
	CreateTriangleVertexBuffer();

	RenderLoop();

	glDeleteProgram(gProgram);
	glDeleteProgram(gProgramForward);
	glDeleteTextures(1, &tex1);
	glDeleteFramebuffers(1, &fbo);

	terminate();
	return 0;
}


int initGLFW() {
	cout << "Initializing GLFW" << endl;
	if (!glfwInit()) {
		cout << "Failed to init GLFW" << endl;
		return 0;
	}

	window = glfwCreateWindow(640, 480, "Hello Triangle", NULL, NULL);
	if (!window) {
		cout << "Failed to create GLFW context" << endl;
		glfwTerminate();
		return 0;
	}

	glfwMakeContextCurrent(window);

	return 1;
}

int initGLEW() {

	cout << "Initializing GLEW" << endl;

	if (glewInit() != GLEW_OK) {
		cout << "Glew init failed" << endl;
		return 0;
	}

	cout << "Using glew version: " << glewGetString(GLEW_VERSION) << endl;
	return 1;
}

void terminate() {
	cout << "Terminating press enter to exit" << endl;
	cin.get();
	glfwTerminate();
}

void CreateTriangleVertexArrayObject() {
	

	
}

void CreateTriangleVertexBuffer() {
	//Generate a vertex array object and get it's ID in triangleVertexArrayID
	glGenVertexArrays(1, &triangleVertexArrayID);
	//Bind the newly created vertex array object
	glBindVertexArray(triangleVertexArrayID);

	//Generate a vertex buffer object and get a reference to it
	glGenBuffers(1, &triangleVertexBufferID);
	//Bind the Vertex Buffer object
	glBindBuffer(GL_ARRAY_BUFFER, triangleVertexBufferID);
	//Load the vertex buffer data into the bound vertex buffer object
	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(triangleVertexBuffer), 
		triangleVertexBuffer,
		GL_STATIC_DRAW);


	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(float)*7,
		(void*)0
	);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		1,
		4,
		GL_FLOAT,
		GL_FALSE,
		sizeof(float) * 7,
		(void*)(3*sizeof(float))
	);
	glEnableVertexAttribArray(1);


	//Generate a vertex array object and get it's ID in triangleVertexArrayID
	glGenVertexArrays(1, &forwardVertexArrayID);
	//Bind the newly created vertex array object
	glBindVertexArray(forwardVertexArrayID);

	glGenBuffers(1, &forwardVertexBufferID);
	//Bind the Vertex Buffer object
	glBindBuffer(GL_ARRAY_BUFFER, forwardVertexBufferID);
	//Load the vertex buffer data into the bound vertex buffer object
	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(forwardVertexBuffer),
		forwardVertexBuffer,
		GL_STATIC_DRAW);

	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(float) * 3,
		(void*)0
	);
	glEnableVertexAttribArray(0);
}

void RenderLoop() {
	
	//Set glfw to capture key's pressed
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	//Render frames until the escape key is pressed
	do {
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		const GLenum temp[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
		glDrawBuffers(2, temp);
		glUseProgram(gProgram);
		

		glClearColor(1, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		glEnable(GL_BLEND);
		glBlendFuncSeparatei(0,
			GL_SRC_ALPHA, GL_ZERO,
			GL_ZERO, GL_ZERO);
		glBlendFuncSeparatei(1,
			GL_SRC_ALPHA, GL_ZERO,
			GL_ZERO, GL_ZERO);

		glBindVertexArray(triangleVertexArrayID);
		glBindBuffer(GL_ARRAY_BUFFER, triangleVertexBufferID);
		glDrawArrays(GL_TRIANGLES, 0, 6);


		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glUseProgram(gProgramForward);

		glUniform1i(glGetUniformLocation(gProgramForward, "tex1"), 0);
		glUniform1i(glGetUniformLocation(gProgramForward, "tex2"), 1);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_RECTANGLE, tex1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_RECTANGLE, tex2);


		glBindVertexArray(forwardVertexArrayID);
		glBindBuffer(GL_ARRAY_BUFFER, forwardVertexBufferID);
		glDrawArrays(GL_TRIANGLES, 0, 6);



		glUseProgram(0);


		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);

}