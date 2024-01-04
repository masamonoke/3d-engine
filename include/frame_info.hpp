#ifndef FRAME_INFO_HPP
#define FRAME_INFO_HPP

#include "camera.hpp"
#include <scene_object.hpp>

#include <vulkan/vulkan.h>

namespace engine {

	struct FrameInfo {
		int frameIdx;
		float frameTime;
		VkCommandBuffer cmdBuf;
		Camera& camera;
		VkDescriptorSet globalDescriptorSet;
		SceneObject::Map& sceneObjects;
	};

	const float INTENSITY = 0.02F;
	const int MAX_LIGHTS = 10;

	struct PointLight {
		glm::vec4 position {}; // ignore w
		glm::vec4 color {}; // w is intensity
	};

	struct GlobalUbo {
		glm::mat4 projection { 1.F };
		glm::mat4 view { 1.F };
		glm::mat4 inverseView { 1.0F };
		glm::vec4 ambientLightColor { 1.F, 1.F, 1.F, INTENSITY };
		std::array<PointLight, MAX_LIGHTS> pointLights;
		int lightsNum;
	};

}

#endif // FRAME_INFO_HPP
