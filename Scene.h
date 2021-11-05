#pragma once

#include "Main.h"
#include "Model.h"

struct Entity {
	Model_ mModel = nullptr;
	AnimationController_ mAnimationController;
	glm::vec3 mPos = { 0,0,0 };
	glm::vec3 mFront = { 0,0,1 };
	glm::vec3 mUp = { 0,1,0 };
	glm::quat mRot = { 1,0,0,0 };
	glm::vec3 mScale = { 1,1,1 };

	Entity() {}
	Entity(Model_ model) : mModel(model) {}
	Entity(Model_ model, const glm::vec3& pos) : mModel(model), mPos(pos) {}

	void Init() {
		if (mModel && mModel->mAnimationSet) {
			mAnimationController = std::make_shared<AnimationController>(mModel->mAnimationSet);
			mAnimationController->SetAnimationIndex(0);
		}
	}

	void Update(float absoluteTime, float deltaTime) {
		if (mAnimationController) {
			mAnimationController->Update(absoluteTime);
		}
	}

	void Walk(float f) {
		mPos += mFront * f;
	}

	void Strafe(float f) {
		mPos += glm::normalize(glm::cross(mFront, mUp)) * f;
	}
};
typedef std::shared_ptr<Entity> Entity_;

struct Scene {
	std::vector<Entity_> mEntities;

	float mCameraDistance = 10.0f;
	float mCameraRotationX = 0.0f;
	float mCameraRotationY = 0.0f;

	Entity_ mSelected;
	size_t mSelectedIndex = -1;

	void Load(const std::string& fileName);

	void Init() {
		for (auto& entity : mEntities) {
			entity->Init();
		}
	}

	void Update(float absoluteTime, float deltaTime) {
		for (auto& entity : mEntities) {
			entity->Update(absoluteTime, deltaTime);
		}
	}

	void SelectNext() {
		mSelectedIndex++;
		if (mSelectedIndex > mEntities.size()) {
			mSelectedIndex = 0;
		}
		mSelected = mSelectedIndex < mEntities.size() ? mEntities[mSelectedIndex] : nullptr;
		if (nullptr != mSelected) {
			mCameraDistance = glm::length(mSelected->mModel->mAABB.mHalfSize) * 2.0f; // FIXME
		}
	}
};
typedef std::shared_ptr<Scene> Scene_;
