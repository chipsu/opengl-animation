#pragma once

#include "Main.h"
#include "Scene.h"
#include "UI.h"

struct Input {
	static Input* sInstance;
	GLFWwindow* mWindow;
	Scene_ mScene;
	double mMouseX = 0;
	double mMouseY = 0;
	float mLimitY = glm::half_pi<float>() * 0.9f;

	Input(GLFWwindow* window, Scene_ scene) : mWindow(window), mScene(scene) {
		sInstance = this;
		glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
			Input::sInstance->OnKey(window, key, scancode, action, mods);
		});
		glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
			Input::sInstance->OnMousePos(window, xpos, ypos);
		});
		glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
			Input::sInstance->OnMouseButton(window, button, action, mods);
		});
		glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {
			Input::sInstance->OnMouseScroll(window, xoffset, yoffset);
		});
	}

	~Input() {
		glfwSetKeyCallback(mWindow, nullptr);
		glfwSetCursorPosCallback(mWindow, nullptr);
		glfwSetMouseButtonCallback(mWindow, nullptr);
		glfwSetScrollCallback(mWindow, nullptr);
		sInstance = nullptr;
	}

	void OnKey(GLFWwindow* window, int key, int scancode, int action, int mods) {
		if (false) printf("key_callback: %d %d %d\n", key, action, mods);
		if (action == GLFW_RELEASE) {
			if (key == GLFW_KEY_SPACE) {
				mScene->SelectNext();
			}
		}
	}

	void OnMousePos(GLFWwindow* window, double xpos, double ypos) {
		if(ImGui::GetIO().WantCaptureMouse) return;
		if (false) printf("cursor_position_callback: %f %f\n", xpos, ypos);
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS) {
			mScene->mCameraRotationX += (mMouseX - xpos) * 0.025f;
			mScene->mCameraRotationY += (mMouseY - ypos) * 0.015f;
			mScene->mCameraRotationY = glm::clamp(mScene->mCameraRotationY, -mLimitY, mLimitY);
			//std::cout << mScene->mCameraRotationX << ", " << mScene->mCameraRotationY << std::endl;
		}
		mMouseX = xpos;
		mMouseY = ypos;
	}

	void OnMouseButton(GLFWwindow* window, int button, int action, int mods) {
		if(ImGui::GetIO().WantCaptureMouse) return;
		if (false) printf("mouse_button_callback: %d %d %d\n", button, action, mods);
	}

	void OnMouseScroll(GLFWwindow* window, double xoffset, double yoffset) {
		if(ImGui::GetIO().WantCaptureMouse) return;
		if (false) printf("scroll_callback: %f %f\n", xoffset, yoffset);
		const float scale = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ? 5.0f : 0.25f;
		mScene->mCameraDistance -= (float)yoffset * scale;
		mScene->mCameraDistance = glm::clamp(mScene->mCameraDistance, mScene->mSelected ? mScene->mSelected->GetMinDistance() : 0.5f, 1000.0f);
		//printf("SCROLL: %f\n", mScene->mCameraDistance);
	}
};
