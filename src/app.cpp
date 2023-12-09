#include "app.hpp"

#include <stdexcept>
#include <iostream>

#include <glm/gtc/constants.hpp>

#include "render_system.hpp"
#include "camera.hpp"

namespace engine {

	App::App() {
		loadSceneObjects();
	}

	App::~App() {
	}

	void App::run() {
		RenderSystem render { device_, renderer_.swapChainRenderPass() };
		Camera camera {};
		camera.viewDirection(glm::vec3(0.0f), glm::vec3(0.5f, 0.0f, 1.0f));

		while (!window_.shouldClose()) {
			glfwPollEvents();
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
		std::vector<Model::Vertex> vertices {
			// left face (white)
			{{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
			{{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
			{{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, .5f}, {.9f, .9f, .9f}},

			// right face (yellow)
			{{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .8f, .1f}},
			{{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
			{{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .8f, .1f}},

			// top face (orange, remember y axis points down)
			{{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
			{{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
			{{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, .5f}, {.9f, .6f, .1f}},

			// bottom face (red)
			{{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .1f, .1f}},
			{{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
			{{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .1f, .1f}},

			// nose face (blue)
			{{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
			{{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
			{{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},

			// tail face (green)
			{{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
			{{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
			{{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
		};
		for (auto& v : vertices) {
			v.position += offset;
		}
		return std::make_unique<Model>(device, vertices);
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
