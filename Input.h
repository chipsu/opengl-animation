#pragma once

#include "Main.h"
#include "Scene.h"

struct Input {
	static Input* sInstance;
	GLFWwindow* mWindow;
	Scene_ mScene;
	size_t mDebugMesh = 0;

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

	void SetDebugMesh(Entity_ entity, size_t index) {
		auto& meshes = entity->mModel->mMeshes;
		mDebugMesh = index;

		if (mDebugMesh >= meshes.size()) {
			mDebugMesh = -1;
		}

		if (mDebugMesh == -1) {
			std::cout << "DEBUG: Show all meshes" << std::endl;
			for (auto& mesh : meshes) {
				mesh->mHidden = false;
			}
			//mScene->mCameraCenter = entity->mModel->mAABB.mCenter; // TODO OFFSET
		}
		else {
			std::cout << "DEBUG: Show mesh " << mDebugMesh << std::endl;
			for (auto& mesh : meshes) {
				mesh->mHidden = true;
			}
			meshes[mDebugMesh]->mHidden = false;
			//mScene->mCameraCenter = meshes[mDebugMesh]->mAABB.mCenter; // TODO OFFSET
		}
	}

	void OnKey(GLFWwindow* window, int key, int scancode, int action, int mods) {
		if (false) printf("key_callback: %d %d %d\n", key, action, mods);
		if (action == GLFW_RELEASE) {
			auto& entity = mScene->mEntities[0];
			auto& animationController = entity->mModel->mAnimationController;
			if (key == GLFW_KEY_LEFT) {
				SetDebugMesh(entity, mDebugMesh - 1);
			}
			if (key == GLFW_KEY_RIGHT) {
				SetDebugMesh(entity, mDebugMesh + 1);
			}
			if (key == GLFW_KEY_UP) {
				SetDebugMesh(entity, -1);
			}
			if (key == GLFW_KEY_SPACE) {
				size_t animationIndex = animationController->GetAnimationIndex() + 1;
				if (animationIndex >= animationController->GetAnimationCount()) animationIndex = -1;
				animationController->SetAnimationIndex(animationIndex);
				const auto animation = animationController->GetAnimation();
				std::cout << "Animation: " << animationIndex << ", " << (animation ? animation->mName : "DISABLED") << std::endl;
			}
			if (key == GLFW_KEY_TAB) {
				mScene->SelectNext();
			}
		}
	}

	double mMouseX = 0;
	double mMouseY = 0;

	void OnMousePos(GLFWwindow* window, double xpos, double ypos) {
		if (false) printf("cursor_position_callback: %f %f\n", xpos, ypos);
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS) {
			mScene->mCameraRotationX = (mMouseX - xpos) * 0.025f;
			mScene->mCameraRotationY = (mMouseY - xpos) * 0.025f;
			/*float newY = mScene->mCameraRotationY + (mMouseY - ypos) * 0.025f;
			const auto limitY = glm::half_pi<float>() * 0.9f;
			newY = std::max(-limitY * 0.5f, std::min(newY, limitY));
			mScene->mCameraRotationY = newY;
			mMouseY = ypos;*/
			//std::cout << mScene->mCameraRotationX << ", " << mScene->mCameraRotationY << std::endl;
		} else {
			mMouseX = xpos;
			mMouseY = ypos;
		}
	}

	void OnMouseButton(GLFWwindow* window, int button, int action, int mods) {
		if (false) printf("mouse_button_callback: %d %d %d\n", button, action, mods);
	}

	void OnMouseScroll(GLFWwindow* window, double xoffset, double yoffset) {
		if (false) printf("scroll_callback: %f %f\n", xoffset, yoffset);
		const float scale = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ? 10.0f : 1.0f;
		mScene->mCameraDistance += (float)yoffset * scale;
		printf("SCROLL: %f\n", mScene->mCameraDistance);
	}
};
