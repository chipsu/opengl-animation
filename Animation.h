#pragma once

#include "Main.h"

template<typename T>
struct KeyFrame {
	float mTime;
	T mValue;
	KeyFrame(float time, T value) : mTime(time), mValue(value) {}
};

typedef KeyFrame<glm::vec3> VectorKey;
typedef KeyFrame<glm::quat> QuatKey;

template<typename T>
inline size_t GetKeyFrameIndex(const float time, const std::vector<T>& keys) {
	for (size_t i = 0; i < keys.size() - 1; i++) {
		if (time < (float)keys[i + 1].mTime) {
			return i;
		}
	}
	return 0;
}

template<typename TValue, typename TMixer>
inline TValue InterpolateKeyFrames(const float time, const std::vector<KeyFrame<TValue>>& keys, TMixer mix) {
	if (keys.size() == 1) {
		return keys[0].mValue;
	}
	const size_t frameIndex = GetKeyFrameIndex(time, keys);
	const auto& currentFrame = keys[frameIndex];
	const auto& nextFrame = keys[(frameIndex + 1) % keys.size()];
	const float delta = (time - (float)currentFrame.mTime) / (float)(nextFrame.mTime - currentFrame.mTime);
	const auto start = currentFrame.mValue;
	const auto end = nextFrame.mValue;
	return mix(start, end, delta);
}

inline glm::vec3 InterpolateKeyFrames(const float time, const std::vector<VectorKey>& keys) {
	return InterpolateKeyFrames(time, keys, [](const glm::vec3& a, const glm::vec3& b, const float t) -> glm::vec3 { return glm::mix(a, b, t); });
}

inline glm::quat InterpolateKeyFrames(const float time, const std::vector<QuatKey>& keys) {
	return InterpolateKeyFrames(time, keys, [](const glm::quat& a, const glm::quat& b, const float t) -> glm::quat { return glm::slerp(a, b, t); });
}

struct AnimationTrack {
	std::string mName;
	std::vector<VectorKey> mPositionKeys;
	std::vector<QuatKey> mRotationKeys;
	std::vector<VectorKey> mScalingKeys;
	// TODO: aiAnimNode aiAnimBehaviour mPreState;
	// TODO: aiAnimNode aiAnimBehaviour mPostState;

	glm::vec3 InterpolateTranslation(const float time) const {
		return InterpolateKeyFrames(time, mPositionKeys);
	}
	glm::quat InterpolateRotation(const float time) const {
		return InterpolateKeyFrames(time, mRotationKeys);
	}
	glm::vec3 InterpolateScale(const float time) const {
		return InterpolateKeyFrames(time, mScalingKeys);
	}
	glm::mat4 InterpolateTransform(const float time) const {
		const auto translation = InterpolateTranslation(time);
		const auto rotation = InterpolateRotation(time);
		const auto scale = InterpolateScale(time);

		glm::mat4 transform = glm::translate(glm::identity<glm::mat4>(), translation);
		transform *= glm::mat4_cast(rotation);
		transform = glm::scale(transform, scale);

		return transform;
	}
};
typedef std::shared_ptr<AnimationTrack> AnimationTrack_;

struct AnimationNode {
	typedef std::shared_ptr<AnimationNode> AnimationNode_;
	std::string mName;
	std::vector<AnimationNode_> mChildren;
	AnimationNode_ mParent;
	glm::mat4 mTransform;
	uint32_t mCachedBoneIndex = -2; // FIXME!

	AnimationNode(const std::string& name, AnimationNode_ parent, const glm::mat4& transform) : mName(name), mParent(parent), mTransform(transform) {
	}

	void Recurse(std::function<void(AnimationNode& node)> callback) {
		callback(*this);
		for (auto& child : mChildren) {
			child->Recurse(callback);
		}
	}
};
typedef AnimationNode::AnimationNode_ AnimationNode_;

struct Animation {
	std::string mName;
	std::vector<AnimationTrack_> mAnimationTracks;
	float mTicksPerSecond = 0;
	float mDuration = 0;

	// TODO: Map by Name
	AnimationTrack_ GetAnimationTrack(const std::string& name) const {
		for (const auto& track : mAnimationTracks) {
			if (track->mName == name) {
				return track;
			}
		}
		return nullptr;
	}

	glm::mat4 GetNodeTransform(const float time, const AnimationNode_ node) const {
		const auto& track = GetAnimationTrack(node->mName);
		if (nullptr == track) {
			return node->mTransform;
		}
		return track->InterpolateTransform(time);
	}

	float GetAnimationTime(const float time) {
		const float tps = mTicksPerSecond ? mTicksPerSecond : 25.0f;
		const float ticks = time * tps;
		return fmod(ticks, mDuration);
	}
};
typedef std::shared_ptr<Animation> Animation_;

struct AnimationSet {
	std::vector<Animation_> mAnimations;
	std::unordered_map<std::string, uint32_t> mBoneMappings;
	std::vector<glm::mat4> mBoneOffsets;
	AnimationNode_ mRootNode;

	size_t GetAnimationIndex(const std::string& name) const {
		for (size_t i = 0; i < mAnimations.size(); ++i) {
			if (mAnimations[i]->mName == name) {
				return i;
			}
		}
		return -1;
	}

	uint32_t GetBoneIndex(AnimationNode_ node) const {
		if(node->mCachedBoneIndex != -2) return node->mCachedBoneIndex;
		node->mCachedBoneIndex = GetBoneIndex(node->mName);
		return node->mCachedBoneIndex;
	}

	uint32_t GetBoneIndex(const std::string& name) const {
		const auto it = mBoneMappings.find(name);
		if(it == mBoneMappings.end()) return -1;
		return it->second;
	}

	uint32_t MapBone(const std::string& name, const glm::mat4& boneOffset) {
		const auto it = mBoneMappings.find(name);
		if (it != mBoneMappings.end()) {
			return it->second;
		}
		const uint32_t id = mBoneMappings.size();
		mBoneMappings[name] = id;
		mBoneOffsets.resize(mBoneMappings.size());
		mBoneOffsets[id] = boneOffset;
		//std::cout << "Bone " << name << " mapped to " << id << std::endl;
		return id;
	}
};
typedef std::shared_ptr<AnimationSet> AnimationSet_;

struct AnimationController {
	AnimationSet_ mAnimationSet;
	std::unordered_map<size_t, float> mAnimationWeights;
	std::vector<glm::mat4> mFinalTransforms;
	glm::mat4 mGlobalInverseTransform;

	AnimationController(AnimationSet_ animationSet, const glm::mat4& globalInverseTransform) {
		mAnimationSet = animationSet;
		mGlobalInverseTransform = globalInverseTransform;
	}

	void SetAnimationIndex(size_t animationIndex) {
		mAnimationWeights.clear();
		mAnimationWeights[animationIndex] = 1.0f;
	}

	void SetAnimationWeight(size_t animationIndex, float weight) {
		mAnimationWeights[animationIndex] = weight;
	}

	size_t GetAnimationCount() const {
		return mAnimationSet->mAnimations.size();
	}

	void Update(float absoluteTime) {
		mFinalTransforms.resize(mAnimationSet->mBoneMappings.size()); // FIXME
		BlendNodeHierarchy(mFinalTransforms, absoluteTime);
	}

	void BlendNodeHierarchy(std::vector<glm::mat4>& outputTransforms, float absoluteTime) {
		BlendNodeHierarchy(outputTransforms, absoluteTime, mAnimationSet->mRootNode, glm::identity<glm::mat4>());
	}

	void BlendNodeHierarchy(std::vector<glm::mat4>& outputTransforms, const float absoluteTime, const AnimationNode_ node, const glm::mat4 parentTransform) {
		BlendNodeHierarchy([&](auto boneIndex, const auto& combinedTransform, const auto& parentTransform, const auto& outputTransform) {
			outputTransforms[boneIndex] = mGlobalInverseTransform * outputTransform;
			}, absoluteTime, node, parentTransform);
	}

	void BlendNodeHierarchy(std::function<void(uint32_t, const glm::mat4&, const glm::mat4&, const glm::mat4&)> callback, const float absoluteTime) {
		BlendNodeHierarchy(callback, absoluteTime, mAnimationSet->mRootNode, glm::identity<glm::mat4>());
	}

	void BlendNodeHierarchy(std::function<void(uint32_t, const glm::mat4&, const glm::mat4&, const glm::mat4&)> callback, const float absoluteTime, const AnimationNode_ node, const glm::mat4 parentTransform) {
		const auto boneIndex = mAnimationSet->GetBoneIndex(node);
		auto combinedTransform = parentTransform;

		if(boneIndex != -1) {
			const auto nodeTransform = BlendNode(boneIndex, absoluteTime, node);
			combinedTransform *= nodeTransform;
			callback(boneIndex, combinedTransform, parentTransform, combinedTransform * mAnimationSet->mBoneOffsets[boneIndex]);
		}

		for(auto& childNode : node->mChildren) {
			BlendNodeHierarchy(callback, absoluteTime, childNode, combinedTransform);
		}
	}

	glm::mat4 BlendNode(const size_t boneIndex, const float absoluteTime, const AnimationNode_ node) {
		const auto minWeight = 0.005f;
		float totalWeight = 0.0f;
		for(auto& [k, w] : mAnimationWeights) {
			if(w < minWeight) continue;
			totalWeight += w;
		}

		glm::vec3 translation = { 0, 0, 0 };
		glm::quat rotation = glm::identity<glm::quat>();
		glm::vec3 scale = { 0, 0, 0 };

		for(auto& [k, w] : mAnimationWeights) {
			if(w < minWeight) continue;
			const auto animationWeight = w / totalWeight;
			auto& animation = mAnimationSet->mAnimations[k];
			const auto animationTime = animation->GetAnimationTime(absoluteTime);
			auto track = animation->GetAnimationTrack(node->mName);
			if(track == nullptr) continue; // node->mTransform?????? FIXME!
			translation += track->InterpolateTranslation(animationTime) * animationWeight;
			rotation *= glm::slerp(glm::identity<glm::quat>(), track->InterpolateRotation(animationTime), animationWeight);
			scale += track->InterpolateScale(animationTime) * animationWeight;
		}

		auto nodeTransform = glm::translate(glm::identity<glm::mat4>(), translation);
		nodeTransform *= glm::mat4_cast(rotation);
		nodeTransform = glm::scale(nodeTransform, scale);

		return nodeTransform;
	}
};
typedef std::shared_ptr<AnimationController> AnimationController_;

