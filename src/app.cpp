#include "app.hpp"

#include <stdexcept>
#include <iostream>

#include <glm/gtc/constants.hpp>

#include "render_system.hpp"

namespace engine {

	App::App() {
		loadSceneObjects();
	}

	App::~App() {
	}

	void App::run() {
		RenderSystem render { device_, renderer_.swapChainRenderPass() };
		while (!window_.shouldClose()) {
			glfwPollEvents();
			if (auto cmd_buf = renderer_.beginFrame()) {
				renderer_.beginSwapChainRenderPass(cmd_buf);
				render.renderSceneObjects(cmd_buf, sceneObjects_);
				renderer_.endSwapChainRenderPass(cmd_buf);
				renderer_.endFrame();
			}
		}
		vkDeviceWaitIdle(device_.device());
	}

	void sierpinsky(std::vector<Model::Vertex>& vertices, int depth, glm::vec2 left, glm::vec2 right, glm::vec2 top) {
		if (depth <= 0) {
			vertices.push_back({{top}, {1.0f, 0.0f, 0.0f}});
			vertices.push_back({{right}, {0.0f, 1.0f, 0.0f}});
			vertices.push_back({{left}, {0.0f, 0.0f, 1.0f}});
		} else {
			auto left_top = 0.5f * (left + top);
			auto right_top = 0.5f * (right + top);
			auto left_right = 0.5f * (left + right);
			sierpinsky(vertices, depth - 1, left, left_right, left_top);
			sierpinsky(vertices, depth - 1, left_right, right, right_top);
			sierpinsky(vertices, depth - 1, left_top, right_top, top);
		}
	}

	void App::loadSceneObjects() {
		std::vector<Model::Vertex> vertices = {
			{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
		};
		/* std::vector<Model::Vertex> vertices = {}; */
		/* sierpinsky(vertices, 5, {-0.5f, 0.5f}, {0.5f, 0.5f}, {0.0f, -0.5f}); */
		auto model = std::make_shared<Model>(device_, vertices);
		auto triangle = SceneObject::createObject();
		triangle.model = model;
		triangle.color = { 0.1f, 0.8f, 0.1f };
		triangle.transform2d.translation.x = 0.2f;
		triangle.transform2d.scale = { 2.0f, 0.5f };
		triangle.transform2d.rotation = 0.25f * glm::two_pi<float>();
		sceneObjects_.push_back(std::move(triangle));
	}

}
