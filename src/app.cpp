#include "app.hpp"

#include <stdexcept>
#include <iostream>
#include <chrono>

#include <glm/gtc/constants.hpp>

#include "render_system.hpp"
#include "camera.hpp"
#include "keyboard_move_controller.hpp"
#include "buffer.hpp"

namespace engine {

	struct GlobalUbo {
		glm::mat4 projectionView { 1.0F };
		glm::vec3 lightDirection = glm::normalize(glm::vec3 { 1.0F, -3.0F, -1.0F });
	};

	App::App() {
		globalPool_ = DescriptorPool::Builder(device_)
			.maxSets(SwapChain::MAX_FRAMES)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES)
			.build();
		loadSceneObjects();
	}

	App::~App() = default;

	void App::run() {
		std::vector<std::unique_ptr<Buffer>> ubo_buffers { SwapChain::MAX_FRAMES };
		for (size_t i = 0; i < ubo_buffers.size(); i++) {
			ubo_buffers[i] = std::make_unique<Buffer>(
				device_,
				sizeof(GlobalUbo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);
			ubo_buffers[i]->map();
		}

		auto global_set_layout = DescriptorSetLayout::Builder(device_)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.build();
		std::vector<VkDescriptorSet> global_descriptors_sets { SwapChain::MAX_FRAMES };
		for (size_t i = 0; i < global_descriptors_sets.size(); i++) {
			auto buffer_info = ubo_buffers[i]->decriptorInfo();
			DescriptorWriter(*global_set_layout, *globalPool_)
				.writeBuffer(0, &buffer_info)
				.build(global_descriptors_sets[i]);
		}

		RenderSystem render { device_, renderer_.swapChainRenderPass(), global_set_layout->descriptorSetLayout() };
		Camera camera {};
		auto current_time = std::chrono::high_resolution_clock::now();
		auto viewer_obj = SceneObject::createObject();
		KeyboardMoveController camera_controller {};

		while (!window_.shouldClose()) {
			glfwPollEvents();

			auto new_time = std::chrono::high_resolution_clock::now();
			const float frame_time = std::chrono::duration<float, std::chrono::seconds::period>(new_time - current_time).count();
			current_time = new_time;

			camera_controller.moveInPlayeXZ(window_.glfwWindow(), frame_time, viewer_obj);
			camera.viewYXZ(viewer_obj.transform.translation, viewer_obj.transform.rotation);

			const float aspect = renderer_.aspectRatio();
			camera.perspectiveProjection(glm::radians(50.0F), aspect, 0.1F, 10.0F); // NOLINT
			if (auto* cmd_buf = renderer_.beginFrame()) {

				const int frame_idx = renderer_.frameIdx();
				GlobalUbo ubo {};
				ubo.projectionView = camera.projection() * camera.view();
				ubo_buffers[frame_idx]->writeToBuffer(&ubo);
				ubo_buffers[frame_idx]->flush();

				FrameInfo frame_info {
					frame_idx,
					frame_time,
					cmd_buf,
					camera,
					global_descriptors_sets[frame_idx]
				};
				renderer_.beginSwapChainRenderPass(cmd_buf);
				render.renderSceneObjects(frame_info, sceneObjects_);
				renderer_.endSwapChainRenderPass(cmd_buf);
				renderer_.endFrame();
			}
		}
		vkDeviceWaitIdle(device_.device());
	}

	void App::loadSceneObjects() {
		const std::shared_ptr<Model> model = Model::createModelFromFile(device_, "../assets/models/flat_vase.obj");
		auto obj = SceneObject::createObject();
		obj.model = model;
		obj.transform.translation = { 0.0F, 0.5F, 1.5F }; // NOLINT
		obj.transform.scale = { 2.5F, 2.5F, 2.5F }; // NOLINT
		sceneObjects_.push_back(std::move(obj));
	}

}
