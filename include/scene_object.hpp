#ifndef SCENE_OBJECT_HPP
#define SCENE_OBJECT_HPP

#include <memory>

#include <glm/gtc/matrix_transform.hpp>

#include "model.hpp"

namespace engine {

	struct TransformComponent {
		glm::vec3 translation {};
		glm::vec3 scale { 1.0F, 1.0F, 1.0F };
		glm::vec3 rotation {};

		// https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
		// Tait-Bryan angles YXZ
		[[nodiscard]] glm::mat4 mat4() const;
		glm::mat3 normalMatrix();
	};

	class SceneObject {
		public:
			using id_t = unsigned int;
			using Map = std::unordered_map<id_t, SceneObject>;

			SceneObject(const SceneObject&) = delete;
			SceneObject &operator=(const SceneObject&) = delete;
			SceneObject(SceneObject&&) = default;
			SceneObject &operator=(SceneObject&&) = default;
			~SceneObject() = default;

			static SceneObject createObject() {
				static id_t current_id = 0;
				return SceneObject { current_id++ };
			}

			id_t id() const {
				return id_;
			}

			std::shared_ptr<Model> model {};
			glm::vec3 color {};
			TransformComponent transform {};

		private:
			id_t id_;

			SceneObject(id_t id) : id_(id) {}
	};

}

#endif // SCENE_OBJECT_HPP
