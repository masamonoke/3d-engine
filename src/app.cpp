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

	std::unique_ptr<Model> createCube(EngineDevice& device, glm::vec3 offset) {
		Model::Builder model_builder {};
		model_builder.vertices = {
			// left face (white)
			{{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
			{{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},

			// right face (yellow)
			{{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .8f, .1f}},
			{{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, -.5f}, {.8f, .8f, .1f}},

			// top face (orange, remember y axis points down)
			{{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
			{{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},

			// bottom face (red)
			{{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .1f, .1f}},
			{{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, -.5f}, {.8f, .1f, .1f}},

			// nose face (blue)
			{{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
			{{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},

			// tail face (green)
			{{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
			{{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
		};
		for (auto& v : model_builder.vertices) {
			v.position += offset;
		}

		model_builder.indices = {
			0,  1,  2,
			0,  3,  1,
			4,  5,  6,
			4,  7,  5,
			8,  9,  10,
			8,  11, 9,
            12, 13, 14,
			12, 15, 13,
			16, 17, 18,
			16, 19, 17,
			20, 21, 22,
			20, 23, 21
		};

		return std::make_unique<Model>(device, model_builder);
	}

	void App::loadSceneObjects() {
		std::shared_ptr<Model> model = createCube(device_, { 0.0f, 0.0f, 0.0f });
		auto cube = SceneObject::createObject();
		cube.model = model;
		cube.transform.translation = { 0.0f, 0.0f, 1.5f };
		cube.transform.scale = { 0.5f, 0.5f, 0.5f };
		sceneObjects_.push_back(std::move(cube));
	}

}
