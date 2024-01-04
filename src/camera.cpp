#include "camera.hpp"

#include <cassert>
#include <limits>

namespace engine {

	// NOLINTBEGIN

	void Camera::orhographicProjection(float left, float right, float top, float bottom, float near, float far) {
		projectionMatrix_ = glm::mat4{1.0F};
		projectionMatrix_[0][0] = 2.F / (right - left);
		projectionMatrix_[1][1] = 2.F / (bottom - top);
		projectionMatrix_[2][2] = 1.F / (far - near);
		projectionMatrix_[3][0] = -(right + left) / (right - left);
		projectionMatrix_[3][1] = -(bottom + top) / (bottom - top);
		projectionMatrix_[3][2] = -near / (far - near);
	}

	void Camera::perspectiveProjection(float fovy, float aspect, float near, float far) {
		assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0F);
		const float tanHalfFovy = tan(fovy / 2.F);
		projectionMatrix_ = glm::mat4{0.0F};
		projectionMatrix_[0][0] = 1.F / (aspect * tanHalfFovy);
		projectionMatrix_[1][1] = 1.F / (tanHalfFovy);
		projectionMatrix_[2][2] = far / (far - near);
		projectionMatrix_[2][3] = 1.F;
		projectionMatrix_[3][2] = -(far * near) / (far - near);
	}

	void Camera::viewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up) {
		const glm::vec3 w{glm::normalize(direction)};
		const glm::vec3 u{glm::normalize(glm::cross(w, up))};
		const glm::vec3 v{glm::cross(w, u)};

		viewMatrix_ = glm::mat4{1.F};
		viewMatrix_[0][0] = u.x;
		viewMatrix_[1][0] = u.y;
		viewMatrix_[2][0] = u.z;
		viewMatrix_[0][1] = v.x;
		viewMatrix_[1][1] = v.y;
		viewMatrix_[2][1] = v.z;
		viewMatrix_[0][2] = w.x;
		viewMatrix_[1][2] = w.y;
		viewMatrix_[2][2] = w.z;
		viewMatrix_[3][0] = -glm::dot(u, position);
		viewMatrix_[3][1] = -glm::dot(v, position);
		viewMatrix_[3][2] = -glm::dot(w, position);

		inverseViewMatrix_ = glm::mat4{1.f};
		inverseViewMatrix_[0][0] = u.x;
		inverseViewMatrix_[0][1] = u.y;
		inverseViewMatrix_[0][2] = u.z;
		inverseViewMatrix_[1][0] = v.x;
		inverseViewMatrix_[1][1] = v.y;
		inverseViewMatrix_[1][2] = v.z;
		inverseViewMatrix_[2][0] = w.x;
		inverseViewMatrix_[2][1] = w.y;
		inverseViewMatrix_[2][2] = w.z;
		inverseViewMatrix_[3][0] = position.x;
		inverseViewMatrix_[3][1] = position.y;
		inverseViewMatrix_[3][2] = position.z;
	}

	void Camera::viewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up) {
		viewDirection(position, target - position, up);
	}

	void Camera::viewYXZ(glm::vec3 position, glm::vec3 rotation) {
		const float c3 = glm::cos(rotation.z);
		const float s3 = glm::sin(rotation.z);
		const float c2 = glm::cos(rotation.x);
		const float s2 = glm::sin(rotation.x);
		const float c1 = glm::cos(rotation.y);
		const float s1 = glm::sin(rotation.y);
		const glm::vec3 u{(c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1)};
		const glm::vec3 v{(c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3)};
		const glm::vec3 w{(c2 * s1), (-s2), (c1 * c2)};
		viewMatrix_ = glm::mat4{1.F};
		viewMatrix_[0][0] = u.x;
		viewMatrix_[1][0] = u.y;
		viewMatrix_[2][0] = u.z;
		viewMatrix_[0][1] = v.x;
		viewMatrix_[1][1] = v.y;
		viewMatrix_[2][1] = v.z;
		viewMatrix_[0][2] = w.x;
		viewMatrix_[1][2] = w.y;
		viewMatrix_[2][2] = w.z;
		viewMatrix_[3][0] = -glm::dot(u, position);
		viewMatrix_[3][1] = -glm::dot(v, position);
		viewMatrix_[3][2] = -glm::dot(w, position);

		inverseViewMatrix_ = glm::mat4{1.f};
		inverseViewMatrix_[0][0] = u.x;
		inverseViewMatrix_[0][1] = u.y;
		inverseViewMatrix_[0][2] = u.z;
		inverseViewMatrix_[1][0] = v.x;
		inverseViewMatrix_[1][1] = v.y;
		inverseViewMatrix_[1][2] = v.z;
		inverseViewMatrix_[2][0] = w.x;
		inverseViewMatrix_[2][1] = w.y;
		inverseViewMatrix_[2][2] = w.z;
		inverseViewMatrix_[3][0] = position.x;
		inverseViewMatrix_[3][1] = position.y;
		inverseViewMatrix_[3][2] = position.z;
	}

	// NOLINTEND

}
