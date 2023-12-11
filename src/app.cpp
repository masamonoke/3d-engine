#include "app.hpp"

#include <stdexcept>
#include <iostream>
#include <chrono>

#include <glm/gtc/constants.hpp>

#include "render_system.hpp"
#include "camera.hpp"
#include "keyboard_move_controller.hpp"

namespace engine {

	App::App() {
		loadSceneObjects();
	}

	App::~App() {
	}

	void App::run() {
		RenderSystem render { device_, renderer_.swapChainRenderPass() };
		Camera camera {};
		auto current_time = std::chrono::high_resolution_clock::now();
		auto viewer_obj = SceneObject::createObject();
		KeyboardMoveController camera_controller {};

		while (!window_.shouldClose()) {
			glfwPollEvents();

			auto new_time = std::chrono::high_resolution_clock::now();
			float frame_time = std::chrono::duration<float, std::chrono::seconds::period>(new_time - current_time).count();
			current_time = new_time;

			camera_controller.moveInPlayeXZ(window_.glfwWindow(), frame_time, viewer_obj);
			camera.viewYXZ(viewer_obj.transform.translation, viewer_obj.transform.rotation);

			float aspect = renderer_.aspectRatio();
			camera.perspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 10.0f);
			if (auto cmd_buf = renderer_.beginFrame()) {
				renderer_.beginSwapChainRenderPass(cmd_buf);
				render.renderSceneObjects(cmd_buf, sceneObjects_, camera);
				renderer_.endSwapChainRenderPass(cmd_buf);
				renderer_.endFrame();
			}
		}
		vkDeviceWaitIdle(device_.device());
	}

	void App::loadSceneObjects() {
		std::shared_ptr<Model> model = Model::createModelFromFile(device_, "../assets/models/smooth_vase.obj");
		auto obj = SceneObject::createObject();
		obj.model = model;
		obj.transform.translation = { 0.0f, 0.0f, 1.5f };
		obj.transform.scale = { 0.5f, 0.5f, 0.5f };
		sceneObjects_.push_back(std::move(obj));
	}

}
