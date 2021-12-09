#pragma once

#define MAX_VERTEX_WEIGHTS 4
#define AI_LMW_MAX_WEIGHTS MAX_VERTEX_WEIGHTS

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include "glm/ext.hpp"

#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <vector>
#include <map>
#include <sstream>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <initializer_list>
#include <list>
#include <deque>
#include <unordered_map>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

inline float GetTime() {
	return (float)glfwGetTime();
}

inline glm::vec3 RandomColor() {
	return {
		(float)(rand()) / (float)(RAND_MAX),
		(float)(rand()) / (float)(RAND_MAX),
		(float)(rand()) / (float)(RAND_MAX)
	};
}

inline std::string ReadFile(const std::string& path) {
	std::ifstream stream(path, std::ios::in);
	if (!stream.is_open()) {
		std::string message = "Could not open file: " + path;
		throw new std::runtime_error(message);
	}
	std::stringstream buffer;
	buffer << stream.rdbuf();
	const auto source = buffer.str();
	stream.close();
	return source;
}

template<typename T>
struct Timer {
	T mTime = 0;
	T mLastUpdate = 0;
	T mScale = 1;

	Timer() {
		Reset();
	}

	void Set(T time) {
		mTime = time;
		mLastUpdate = (T)glfwGetTime();
	}

	void Reset() {
		Set(0);
		mScale = 1;
	}

	T Update() {
		auto now = (T)glfwGetTime();
		if(mScale == 0) {
			mLastUpdate = now;
			return 0;
		}
		auto delta = now - mLastUpdate;
		mTime += delta * mScale;
		mLastUpdate = now;
		return delta;
	}
};

template<typename T>
struct FrameCounter {
	std::deque<T> mHistory;
	size_t mHistoryLimit = 0;
	size_t mCounter = 0;
	T mInterval = 1.0;
	T mNextUpdate = 0;
	size_t mValue = 0;

	bool Tick(const T now) {
		mCounter++;
		if (mNextUpdate > now) return false;
		mValue = mCounter;
		if (mHistoryLimit > 0) {
			mHistory.push_back(mValue);
			if (mHistory.size() > mHistoryLimit) {
				mHistory.pop_front();
			}
		}
		mNextUpdate = now + mInterval;
		mCounter = 0;
		return true;
	}

	size_t fps = 0;
	float fps_timer = 0;
	size_t fps_counter = 0;
	std::deque<float> fps_history;
	float timer = GetTime();
};

struct Camera {
	glm::vec3 mPos = { 0,0,0 };
	glm::vec3 mFront = { 0,0,1 };
	glm::vec3 mUp = { 0,1,0 };
	glm::vec3 mRight = { 1,0,0 };

	float mFov = glm::radians(45.0f);
	float mAspect = 1.0f;
	float mNear = 0.1f;
	float mFar = 1000.0f;

	glm::mat4 mView;
	glm::mat4 mProjection;

	void Look(float yaw, float pitch) {
		glm::vec3 dir = {
			cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
			sin(glm::radians(pitch)),
			sin(glm::radians(yaw)) * cos(glm::radians(pitch)),
		};
		mFront = glm::normalize(dir);
	}

	void SetAspect(int width, int height) {
		mAspect = width / (float)height;
	}

	void UpdateView() {
		mView = glm::lookAt(mPos, mPos + mFront, mUp);
	}

	void UpdateProjection() {
		mProjection = glm::perspective(mFov, mAspect, mNear, mFar);
	}
};
