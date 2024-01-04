#include "app.hpp"

#include <stdexcept>
#include <iostream>
#include <chrono>

#include <glm/gtc/constants.hpp>

#include "render_system.hpp"
#include "camera.hpp"
#include "keyboard_move_controller.hpp"
#include "buffer.hpp"
#include "point_light_system.hpp"

namespace engine {

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
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.build();

		std::vector<VkDescriptorSet> global_descriptors_sets { SwapChain::MAX_FRAMES };

		for (size_t i = 0; i < global_descriptors_sets.size(); i++) {
			auto buffer_info = ubo_buffers[i]->decriptorInfo();
			DescriptorWriter(*global_set_layout, *globalPool_)
				.writeBuffer(0, &buffer_info)
				.build(global_descriptors_sets[i]);
		}

		RenderSystem render { device_, renderer_.swapChainRenderPass(), global_set_layout->descriptorSetLayout() };
		PointLightSystem light_system { device_, renderer_.swapChainRenderPass(), global_set_layout->descriptorSetLayout() };
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
				FrameInfo frame_info {
					frame_idx,
					frame_time,
					cmd_buf,
					camera,
					global_descriptors_sets[frame_idx],
					sceneObjects_
				};
				GlobalUbo ubo {};

				ubo.projection = camera.projection();
				ubo.view = camera.view();
				light_system.update(frame_info, ubo);
				ubo_buffers[frame_idx]->writeToBuffer(&ubo);
				ubo_buffers[frame_idx]->flush();

				renderer_.beginSwapChainRenderPass(cmd_buf);
				render.renderSceneObjects(frame_info);
				light_system.render(frame_info);
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
		sceneObjects_.emplace(obj.id(), std::move(obj));

		const std::shared_ptr<Model> floor = Model::createModelFromFile(device_, "../assets/models/floor.obj");
		auto floor_obj = SceneObject::createObject();
		floor_obj.model = floor;
		floor_obj.transform.translation = { 0.0F, 0.5F, 0.0F }; // NOLINT
		floor_obj.transform.scale = { 2.5F, 2.5F, 2.5F }; // NOLINT
		sceneObjects_.emplace(floor_obj.id(), std::move(floor_obj));

		auto pointLight = SceneObject::createPointLight(0.2F);
		pointLight.transform.translation = glm::vec4(-1.F, -1.F, -1.F, 1.F);
		sceneObjects_.emplace(pointLight.id(), std::move(pointLight));
	}

}
