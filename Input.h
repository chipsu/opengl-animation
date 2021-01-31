#pragma once

#include "Main.h"
#include "Scene.h"

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
			auto& entity = mScene->mSelected;
			if (nullptr != entity) {
				auto& animationController = entity->mAnimationController;
				if (key == GLFW_KEY_SPACE) {
					size_t animationIndex = animationController->GetAnimationIndex() + 1;
					if (animationIndex >= animationController->GetAnimationCount()) animationIndex = -1;
					animationController->SetAnimationIndex(animationIndex);
					const auto animation = animationController->GetAnimation();
					std::cout << "Animation: " << animationIndex << ", " << (animation ? animation->mName : "DISABLED") << std::endl;
				}
			}
			if (key == GLFW_KEY_TAB) {
				mScene->SelectNext();
			}
		}
	}

	void OnMousePos(GLFWwindow* window, double xpos, double ypos) {
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
		if (false) printf("mouse_button_callback: %d %d %d\n", button, action, mods);
	}

	void OnMouseScroll(GLFWwindow* window, double xoffset, double yoffset) {
		if (false) printf("scroll_callback: %f %f\n", xoffset, yoffset);
		const float scale = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ? 10.0f : 1.0f;
		mScene->mCameraDistance -= (float)yoffset * scale;
		mScene->mCameraDistance = glm::clamp(mScene->mCameraDistance, 1.0f, 1000.0f);
		//printf("SCROLL: %f\n", mScene->mCameraDistance);
	}
};
