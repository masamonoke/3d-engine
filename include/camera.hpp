#ifndef CAMERA_HPP
#define CAMERA_HPP

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace engine {

	class Camera {
		public:
			const glm::mat4& projection() const { return projectionMatrix_; }
			const glm::mat4& view() const { return viewMatrix_; }

			void orhographicProjection(float left, float right, float top, float bottom, float near, float far);
			void perspectiveProjection(float fovy, float aspect, float near, float far);
			void viewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3 { 0.0f, -1.0f, 0.0f });
			void viewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3 { 0.0f, -1.0f, 0.0f });
			void viewYXZ(glm::vec3 position, glm::vec3 rotation);

			private:
			glm::mat4 projectionMatrix_ { 1.0f };
			glm::mat4 viewMatrix_ { 1.0f };
	};

}

#endif // CAMERA_HPP
