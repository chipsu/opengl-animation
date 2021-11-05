#include "Scene.h"

void Scene::Load(const std::string& fileName) {
	rapidjson::Document config;
	std::ifstream ifs(fileName);
	assert(ifs.is_open());
	rapidjson::IStreamWrapper isw(ifs);
	config.ParseStream(isw);

	for (const auto& cfg : config["entities"].GetArray()) {
		if (cfg.HasMember("disabled") && cfg["disabled"].GetBool()) continue;
		auto model = std::make_shared<Model>();
		auto entity = std::make_shared<Entity>(model);
		ModelOptions modelOptions;
		if (cfg.HasMember("modelOptions")) {
			const auto& opts = cfg["modelOptions"].GetObject();
			if (opts.HasMember("scale")) {
				modelOptions.mScale = opts["scale"].GetFloat();
			}
		}
		model->Load(cfg["model"].GetString(), modelOptions);
		if (cfg.HasMember("animations")) {
			for (const auto& anim : cfg["animations"].GetArray()) {
				model->LoadAnimation(anim.GetString(), modelOptions, true);
			}
		}
		if (cfg.HasMember("position")) {
			const auto& pos = cfg["position"].GetArray();
			entity->mPos = { pos[0].GetFloat(), pos[1].GetFloat(), pos[2].GetFloat() };
		}
		if (cfg.HasMember("rotation")) {
			const auto& pos = cfg["rotation"].GetArray();
			entity->mRot = glm::quat(glm::vec3(glm::radians(pos[0].GetFloat()), glm::radians(pos[1].GetFloat()), glm::radians(pos[2].GetFloat())));
		}
		if (cfg.HasMember("scale")) {
			const auto& pos = cfg["scale"].GetArray();
			entity->mScale = { pos[0].GetFloat(), pos[1].GetFloat(), pos[2].GetFloat() };
		}
		mEntities.push_back(entity);
	}
}