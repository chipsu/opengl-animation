#pragma once

#include "Main.h"
#include "Mesh.h"
#include "Animation.h"
#include "AABB.h"

struct ModelNode {
	typedef std::shared_ptr<ModelNode> ModelNode_;
	std::string mName;
	std::vector<ModelNode_> mChildren;
	ModelNode_ mParent;
	glm::mat4 mTransform;
	std::vector<Mesh_> mMeshes;

	ModelNode(const std::string& name, ModelNode_ parent, const glm::mat4& transform) : mName(name), mParent(parent), mTransform(transform) {
	}

	void Recurse(std::function<void(ModelNode& node)> callback) {
		callback(*this);
		for (auto& child : mChildren) {
			child->Recurse(callback);
		}
	}
};
typedef ModelNode::ModelNode_ ModelNode_;

struct ModelOptions {
	float mScale = 1.0f;
	bool mAnimations = true;
};

struct Model {
	std::string mName;
	ModelNode_ mRootNode; // FIXME
	AnimationSet_ mAnimationSet;
	glm::mat4 mGlobalInverseTransform;
	AABB mAABB;
	void Load(const std::string& fileName) { Load(fileName, {}); }
	void Load(const std::string& fileName, const ModelOptions& options);
	void LoadAnimation(const std::string& fileName, bool append = false) {
		LoadAnimation(fileName, {}, append);
	}
	void LoadAnimation(const std::string& fileName, const ModelOptions& options, bool append = false);
	void UpdateAABB() {
		AABB aabb;
		glm::mat4 transform = glm::identity<glm::mat4>();
		// FIXME idk about this
		mRootNode->Recurse([&aabb, &transform](ModelNode& node) {
			transform = transform * node.mTransform;
			glm::vec4 pos = transform * glm::vec4(0, 0, 0, 1);
			aabb = aabb.Extend(glm::vec3(pos.x, pos.y, pos.z));
		});
		mAABB = aabb;
	}
	bool HasAnimations() const {
		// FIXME
		return nullptr != mAnimationSet && mAnimationSet->mAnimations.size() > 0;
	}
};
typedef std::shared_ptr<Model> Model_;
