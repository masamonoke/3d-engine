#include "keyboard_move_controller.hpp"

namespace engine {

	void KeyboardMoveController::moveInPlayeXZ(GLFWwindow* window, float dt, SceneObject& scene_object) {
		glm::vec3 rotate {0};
		if (glfwGetKey(window, keys.lookRight) == GLFW_PRESS) {
			rotate.y += 1.0f;
		}
		if (glfwGetKey(window, keys.lookLeft) == GLFW_PRESS) {
			rotate.y -= 1.0f;
		}
		if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS) {
			rotate.x += 1.0f;
		}
		if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS) {
			rotate.x -= 1.0f;
		}

		if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
			scene_object.transform.rotation += lookSpeed * dt * glm::normalize(rotate);
		}

		scene_object.transform.rotation.x = glm::clamp(scene_object.transform.rotation.x, -1.5f, 1.5f);
		scene_object.transform.rotation.y = glm::mod(scene_object.transform.rotation.y, glm::two_pi<float>());
		float yaw = scene_object.transform.rotation.y;
		const glm::vec3 forward_dir { sin(yaw), 0.0f, cos(yaw) };
		const glm::vec3 right_dir { forward_dir.z, 0.0f, -forward_dir.x};
		const glm::vec3 up_dir { 0.0f, -1.0f, 0.0f };
		glm::vec3 move_dir { 0.0f };
		if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS) {
			move_dir += forward_dir;
		}
		if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) {
			move_dir -= forward_dir;
		}
		if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) {
			move_dir += right_dir;
		}
		if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) {
			move_dir -= right_dir;
		}
		if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) {
			move_dir += up_dir;
		}
		if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) {
			move_dir -= up_dir;
		}

		if (glm::dot(move_dir, move_dir) > std::numeric_limits<float>::epsilon()) {
			scene_object.transform.translation += moveSpeed * dt * glm::normalize(move_dir);
		}
	}

}
