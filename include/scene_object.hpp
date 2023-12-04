#ifndef SCENE_OBJECT_HPP
#define SCENE_OBJECT_HPP

#include <memory>

#include "model.hpp"

namespace engine {

	struct Transform2dComponent {
		glm::vec2 translation {};
		glm::vec2 scale { 0.1f, 0.1f };
		float rotation;

		glm::mat2 mat2() {
			const auto s = glm::sin(rotation);
			const auto c = glm::cos(rotation);
			glm::mat2 rot_mat { {c, s}, {-s, c} };
			glm::mat2 scale_mat { {scale.x, 0.f}, {0.0f, scale.y} };
			return rot_mat * scale_mat;
		}
	};

	class SceneObject {
		public:
			using id_t = unsigned int;

			SceneObject(const SceneObject&) = delete;
			SceneObject &operator=(const SceneObject&) = delete;
			SceneObject(SceneObject&&) = default;
			SceneObject &operator=(SceneObject&&) = default;

			static SceneObject createObject() {
				static id_t current_id = 0;
				return SceneObject { current_id++ };
			}

			id_t id() const {
				return id_;
			}

			std::shared_ptr<Model> model {};
			glm::vec3 color {};
			Transform2dComponent transform2d {};

		private:
			id_t id_;

			SceneObject(id_t id) : id_(id) {}
	};

}

#endif // SCENE_OBJECT_HPP
