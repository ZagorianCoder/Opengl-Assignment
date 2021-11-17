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


// Initial Field of View
float initialFoV = 30.0f;

float speed = 2.5f; // 3 units / second
//float mouseSpeed = 0.005f;

glm::mat4 Model = glm::mat4(1.0f);
float angle = glm::radians(5.0f);

void computeMatricesFromInputs(){

	// glfwGetTime is called only once, the first time this function is called

	static double lastTime = 0.0f;

	// Compute time difference between current and last frame
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);
	speed *= deltaTime;
	//Camera info
	glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);

	glm::mat4 transform = glm::mat4(1.0f);
	//transform = glm::translate(transform, glm::vec3(0.5f, -0.5f, 0.0f));
	//ProjectionMatrix *= transform;
	//ViewMatrix = transform;


	// Camera matrix
	ViewMatrix = glm::lookAt(
			
			glm::vec3(10.0f, 10.0f, 30.f), // Camera is at (4,3,-3), in World Space
			glm::vec3(0.0f,  0.0f, 0.0f), // and looks at the origin
			glm::vec3(0.0f, 0.0f, 1.0f)  // Head is up (set to 0,-1,0 to look upside-down)     

	);

	float angle = glm::degrees(1.0f);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{

		transform = glm::rotate(transform, angle + (float)glfwGetTime(), glm::vec3(1.0f, 0.0f, 0.0f));
		//ViewMatrix *= transform;

	}
	else if(glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
	{

		transform = glm::rotate(transform, angle - (float)glfwGetTime(), glm::vec3(1.0f, 0.0f, 0.0f));
		//ViewMatrix *= transform;

	}
	else if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{

		transform = glm::rotate(transform, angle + (float)glfwGetTime(), glm::vec3(1.0f, 1.0f, 0.0f));
		//ViewMatrix *= transform;

	}
	else if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{

		transform = glm::rotate(transform, angle - (float)glfwGetTime(), glm::vec3(1.0f, 1.0f, 0.0f));
		//ViewMatrix *= transform;

	}
	
	ViewMatrix *= transform;

	float FoV = initialFoV;// - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.

	ProjectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, 0.1f, 100.0f);
	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	
	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;
}