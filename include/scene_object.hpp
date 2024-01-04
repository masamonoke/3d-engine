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

	struct PointLightComponent {
		float lightIntensity = 1.0F;
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

			static SceneObject createPointLight(float intensity = 10.F, float radius = 0.1F, glm::vec3 color = glm::vec3(1.0F));

			[[nodiscard]] id_t id() const {
				return id_;
			}

			std::shared_ptr<Model> model {};
			glm::vec3 color {};
			TransformComponent transform {};

			std::unique_ptr<PointLightComponent> pointLight = nullptr;

		private:
			id_t id_;

			explicit SceneObject(id_t id) : id_(id) {} // NOLINT
	};

}

#endif // SCENE_OBJECT_HPP
