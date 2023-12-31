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

}

#endif // FRAME_INFO_HPP
