// Include standard headers
#define _CRT_SECURE_NO_WARNINGS

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <filesystem>
using namespace std;

#include <stdlib.h>
#include <string.h>


// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "controls.hpp"
#include "objloader.hpp"

GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if (VertexShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if (FragmentShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;


	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const* VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}



	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const* FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}



	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}


	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}



// Very, VERY simple OBJ loader.
// Here is a short list of features a real function would provide : 
// - Binary files. Reading a model should be just a few memcpy's away, not parsing a file at runtime. In short : OBJ is not very great.
// - Animations & bones (includes bones weights)
// - Multiple UVs
// - All attributes should be optional, not "forced"
// - More stable. Change a line in the OBJ file and it crashes.
// - More secure. Change another line and you can inject code.
// - Loading from memory, stream, etc

bool loadOBJ(
	const char* path,
	std::vector<glm::vec3>& out_vertices,
	std::vector<glm::vec2>& out_uvs,
	std::vector<glm::vec3>& out_normals
) {
	printf("Loading OBJ file %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;


	FILE* file = fopen(path, "r");
	if (file == NULL) {
		printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
		getchar();
		return false;
	}

	while (1) {

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		// else : parse lineHeader

		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				printf("File can't be read by our simple parser :-( Try exporting with other options\n");
				fclose(file);
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
		else {
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

	}

	// For each vertex of each triangle
	for (unsigned int i = 0; i < vertexIndices.size(); i++) {

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];

		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		glm::vec2 uv = temp_uvs[uvIndex - 1];
		glm::vec3 normal = temp_normals[normalIndex - 1];

		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs.push_back(uv);
		out_normals.push_back(normal);

	}
	fclose(file);
	return true;
}

void keyCallback(GLFWwindow* window, int key, int scancodem, int action, int mods);

GLuint programID;
GLuint VertexArrayID;
GLuint textureID;
GLuint TextureID;
GLuint textureID2;
GLuint TextureID2;
GLuint textureID3;
GLuint TextureID3;
GLuint textureID4;
GLuint TextureID4;
GLuint vertexbuffer;
GLuint uvbuffer;
GLuint vertexbuffer2;
GLuint uvbuffer2;
GLuint vertexbuffer3;
GLuint uvbuffer3;
GLuint vertexbuffer4;
GLuint uvbuffer4;


int main(void)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(800, 800, "Ηλιακό Σύστημα", NULL, NULL);

	glfwSetKeyCallback(window, keyCallback);

	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the keys being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetInputMode(window, GLFW_LOCK_KEY_MODS, GL_TRUE);

	// Black background
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders("TransformVertexShader.vertexshader", "TextureFragmentShader.fragmentshader");

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");

	// Load the texture

	int width, height, nrChannels;
	unsigned char* data[3];
	data[0] = stbi_load("sun.jpg", &width, &height, &nrChannels, 0);
	data[1] = stbi_load("planet.jpg", &width, &height, &nrChannels, 0);
	data[2] = stbi_load("meteor.jpg", &width, &height, &nrChannels, 0);
	
	

	for (int i = 0; i < 2; i++)
	{

		if (data[i])
		{

		}
		else
		{
			std::cout << "Failed to load texture" << std::endl;
		}

	}

	glGenTextures(1, &textureID);
	

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data[0]);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Get a handle for our "myTextureSampler" uniform
	TextureID = glGetUniformLocation(programID, "myTextureSampler");
	
	glGenTextures(1, &textureID2);
	glBindTexture(GL_TEXTURE_2D, textureID2);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data[1]);
	glGenerateMipmap(GL_TEXTURE_2D);
	TextureID2 = glGetUniformLocation(programID, "myTextureSampler");

	glGenTextures(1, &textureID3);
	glBindTexture(GL_TEXTURE_2D, textureID3);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data[2]);
	glGenerateMipmap(GL_TEXTURE_2D);
	TextureID3 = glGetUniformLocation(programID, "myTextureSampler");

	// Read our .obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> uvs;
	bool res = loadOBJ("sun.obj", vertices, uvs, normals);
	
	std::vector<glm::vec3> vertices2;
	std::vector<glm::vec3> normals2;
	std::vector<glm::vec2> uvs2;
	res = loadOBJ("planet.obj", vertices2, uvs2, normals2);
	
	std::vector<glm::vec3> vertices3;
	std::vector<glm::vec3> normals3;
	std::vector<glm::vec2> uvs3;
	res = loadOBJ("meteor.obj", vertices3, uvs3, normals3);
	

	// Load it into a VBO

	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
	

	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
	
	glGenBuffers(1, &vertexbuffer2);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer2);
	glBufferData(GL_ARRAY_BUFFER, vertices2.size() * sizeof(glm::vec3), &vertices2[0], GL_STATIC_DRAW);

	glGenBuffers(1, &uvbuffer2);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer2);
	glBufferData(GL_ARRAY_BUFFER, uvs2.size() * sizeof(glm::vec2), &uvs2[0], GL_STATIC_DRAW);

	glGenBuffers(1, &vertexbuffer3);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer3);
	glBufferData(GL_ARRAY_BUFFER, vertices3.size() * sizeof(glm::vec3), &vertices3[0], GL_STATIC_DRAW);

	glGenBuffers(1, &uvbuffer3);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer3);
	glBufferData(GL_ARRAY_BUFFER, uvs3.size() * sizeof(glm::vec2), &uvs3[0], GL_STATIC_DRAW);

	glm::vec3 modelsPositions[] = { glm::vec3(0.0f,0.0f,0.0f),glm::vec3(25.0f,0.0f,0.0f) };
	float increase = 60.0f;
	bool spacebar_pressed = false;
	bool drawPlanet = true;
	bool showDestPlanet = false;
	glm::mat4 ModelMatrix2(1.0f);
	bool showExplosion = false;
	int photoNumber = 1;
	double activationTime = -1.0;
	float rotationSpeed = 0.5f;

	do {

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		// Bind our texture in Texture Unit 0

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID, 0);
		

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		ModelMatrix = glm::translate(glm::mat4(1.0f), modelsPositions[0]);
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, vertices.size());

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		if (drawPlanet)
		{

			if (!showExplosion)
			{

				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, textureID2);
				glUniform1i(TextureID2, 1);

			}
			else
			{
				string fName;
				if (photoNumber < 10)
				{
					
					fName = "000";

				}
				else if(photoNumber < 100)
				{

					fName = "00";

				}
				else
				{

					fName = "0";

				}
				fName.append(to_string(photoNumber));
				fName.append(".jpg");
				int width, height, nrChannels;
				unsigned char* data;
				data = stbi_load(fName.c_str(), &width, &height, &nrChannels, 0);
				if (data)
				{

				}
				else
				{
					std::cout << "Failed to load texture" << std::endl;
				}
				glGenTextures(1, &textureID4);
				glBindTexture(GL_TEXTURE_2D, textureID4);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
				glGenerateMipmap(GL_TEXTURE_2D);
				TextureID4 = glGetUniformLocation(programID, "myTextureSampler");
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, textureID4);
				glUniform1i(TextureID4, 1);

			}

			// 1rst attribute buffer : vertices
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer2);
			glVertexAttribPointer(
				0,                  // attribute
				3,                  // size
				GL_FLOAT,           // type
				GL_FALSE,           // normalized?
				0,                  // stride
				(void*)0            // array buffer offset
			);

			// 2nd attribute buffer : UVs
			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, uvbuffer2);
			glVertexAttribPointer(
				1,                                // attribute
				2,                                // size
				GL_FLOAT,                         // type
				GL_FALSE,                         // normalized?
				0,                                // stride
				(void*)0                          // array buffer offset
			);

			if (!showExplosion)
			{

				ModelMatrix2 = glm::translate(glm::mat4(1.0f), modelsPositions[0]);
				ModelMatrix2 = glm::rotate(ModelMatrix2, glm::radians(rotationSpeed), glm::vec3(0.0f, 1.0f, 0.0f));
				ModelMatrix2 = glm::translate(ModelMatrix2, modelsPositions[1]);
				rotationSpeed += 0.005f;
				

			}

			MVP = ProjectionMatrix * ViewMatrix * ModelMatrix2;
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
			// Draw the triangle !
			glDrawArrays(GL_TRIANGLES, 0, vertices2.size());

			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);

		}
		else if(activationTime > 0)
		{

			double current_time = glfwGetTime() - activationTime;
			if (current_time > 10)
			{

				//printf()
				drawPlanet = true;
				activationTime = -1.0;

			}

		}


		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS || spacebar_pressed)
		{

			spacebar_pressed = true;
			/**/if (!showExplosion)
			{
				
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, textureID3);
				glUniform1i(TextureID3, 1);

			}
			else
			{

				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, textureID4);
				glUniform1i(TextureID4, 1);

			}
			

			// 1rst attribute buffer : vertices
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer3);
			glVertexAttribPointer(
				0,                  // attribute
				3,                  // size
				GL_FLOAT,           // type
				GL_FALSE,           // normalized?
				0,                  // stride
				(void*)0            // array buffer offset
			);

			// 2nd attribute buffer : UVs
			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, uvbuffer3);
			glVertexAttribPointer(
				1,                                // attribute
				2,                                // size
				GL_FLOAT,                         // type
				GL_FALSE,                         // normalized?
				0,                                // stride
				(void*)0                          // array buffer offset
			);

			if (!showExplosion)
			{

				increase -= 0.005f;

			}
			glm::vec3 moveonz = glm::vec3(0.0f, 0.0f, increase);
			ModelMatrix = glm::translate(glm::mat4(1.0f), moveonz);
			glm::mat4 MVP3 = ProjectionMatrix * ViewMatrix * ModelMatrix;
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP3[0][0]);
			// Draw the triangle !
			glDrawArrays(GL_TRIANGLES, 0, vertices3.size());
			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);

			//d = ((x2 - x1)^2 + (y2 - y1)^2 + (z2 - z1)^2)^1/2;
			long double distance = 11.0;
			if (drawPlanet && showExplosion == false)
			{

				long double dx = ModelMatrix2[3].x;
				long double dz = ModelMatrix2[3].z - increase;
				distance = glm::pow((glm::pow(dx, 2) + glm::pow(dz, 2)), 0.5);

			}
			

			if (increase <= 0.0f)
			{

				spacebar_pressed = false;
				increase = 60.0f;

			}
			else if (distance <= 10.0)
			{

				showExplosion = true;
				activationTime = glfwGetTime();
				
			}

		}
		if (photoNumber < 120 && showExplosion)
		{

			photoNumber++;

		}
		else if(showExplosion)
		{

			spacebar_pressed = false;
			increase = 60.0f;
			drawPlanet = false;
			showExplosion = false;
			photoNumber = 1;
			
			
		}

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	}
	while (true);

}

void keyCallback(GLFWwindow* window, int key, int scancodem, int action, int mods)
{

	if (key == GLFW_KEY_Q && action == GLFW_PRESS && mods == 48)
	{

		// Cleanup VBO and shader
		glDeleteBuffers(1, &vertexbuffer);
		glDeleteBuffers(1, &uvbuffer);
		glDeleteBuffers(1, &vertexbuffer2);
		glDeleteBuffers(1, &uvbuffer2);
		glDeleteBuffers(1, &vertexbuffer3);
		glDeleteBuffers(1, &uvbuffer3);
		glDeleteProgram(programID);
		glDeleteTextures(1, &textureID);
		glDeleteTextures(1, &textureID2);
		glDeleteTextures(1, &textureID3);
		glDeleteTextures(1, &textureID4);
		glDeleteVertexArrays(1, &VertexArrayID);

		// Close OpenGL window and terminate GLFW
		glfwTerminate();

		exit(0);

	}

}

