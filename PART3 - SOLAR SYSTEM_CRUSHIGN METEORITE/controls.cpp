// Include GLFW
#include <GLFW/glfw3.h>
extern GLFWwindow* window; // The "extern" keyword here is to access the variable "window" declared in tutorialXXX.cpp. This is a hack to keep the tutorials simple. Please avoid this.

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "controls.hpp"

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

glm::mat4 getViewMatrix(){
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix(){
	return ProjectionMatrix;
}

glm::mat4 Model = glm::mat4(1.0f);
glm::vec3 camPos = glm::vec3(10.0f, 10.0f, 30.f);


void computeMatricesFromInputs(){

	glm::vec4 transformedVec(camPos,1.0f);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		glm::mat4 rm = glm::rotate(Model, glm::radians(0.02f), glm::vec3(1.0f, 0.0f, 0.0f));
		transformedVec = rm * transformedVec;
		camPos = transformedVec;

	}
	else if(glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
	{

		glm::mat4 rm = glm::rotate(Model, glm::radians(0.02f), glm::vec3(-1.0f, 0.0f, 0.0f));
		transformedVec = rm * transformedVec;
		camPos = transformedVec;
	

	}
	else if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{

		glm::mat4 rm = glm::rotate(Model, glm::radians(0.02f), glm::vec3(0.0f, 1.0f, 0.0f));
		transformedVec = rm * transformedVec;
		camPos = transformedVec;
		

	}
	else if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{

		glm::mat4 rm = glm::rotate(Model, glm::radians(0.02f), glm::vec3(0.0f,-1.0f, 0.0f));
		transformedVec = rm * transformedVec;
		camPos = transformedVec;
		

	}
	else if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{

		glm::mat4 transZ = glm::translate(Model, glm::vec3(0.0f, 0.0f, -0.005f));
		camPos = transZ * transformedVec;

	}
	else if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS)
	{

		glm::mat4 transZ = glm::translate(Model, glm::vec3(0.0f, 0.0f, 0.005f));
		camPos = transZ * transformedVec;

	}

	// Projection matrix : 30° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	ProjectionMatrix = glm::perspective(glm::radians(100.0f), 4.0f / 3.0f, 0.1f, 100.0f);
	// Camera matrix
	ViewMatrix = glm::lookAt(

		camPos, // Camera is at the calculated position, in World Space
		glm::vec3(0.0f, 0.0f, 0.0f), // and looks at the origin
		glm::vec3(0.0f, 1.0f, 0.0f)  // Up vector    

	);
	
}