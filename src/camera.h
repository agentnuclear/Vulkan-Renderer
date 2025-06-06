
#include <vk_types.h>
#include <SDL_events.h>

class Camera {
public:
	glm::vec3 velocity; // cam speed
	glm::vec3 position; // cam location
	//vertical rotation
	float pitch{ 0.f };
	//horizontal rotation
	float yaw{ 0.f };

	glm::mat4 getViewMatrix();
	glm::mat4 getRotationMatrix();

	void processSDLEvent(SDL_Event& e); //input logic

	void update(); // position modification
};
