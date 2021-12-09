#include "Main.h"
#include "Scene.h"
#include "Shader.h"
#include "Input.h"
#include "UI.h"
#include "Debug.h"

Input* Input::sInstance = nullptr;
DebugOverlay* gDebugOverlay = nullptr;

Scene_ CreateScene(const int argc, const char** argv) {
	auto scene = std::make_shared<Scene>();
	scene->Load(argc == 1 ? "scene.json" : argv[1]);
	scene->Init();
	scene->SelectNext();
	return scene;
}

void RenderNode(GLint uniformModel, ModelNode_ node, const glm::mat4& parentTransform) {
	glm::mat4 transform = parentTransform * node->mTransform;
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, (GLfloat*)&transform[0]);
	for (auto& mesh : node->mMeshes) {
		if (mesh->mHidden) continue;
		mesh->Bind();
		glDrawElements(GL_TRIANGLES, mesh->mIndices.size(), GL_UNSIGNED_INT, 0);
	}
	for (auto& childNode : node->mChildren) {
		RenderNode(uniformModel, childNode, transform);
	}
};

void RenderSkeleton(Model_ model, AnimationController_ ac, float now, const glm::mat4& parentTransform, bool points, bool lines) {
	int counter = 0;
	ac->BlendNodeHierarchy([parentTransform, points, lines, &counter](auto index, const auto& t, const auto& pt, const auto& ot) {
		if(++counter < 2) return;
		auto p = parentTransform * t * glm::vec4(0, 0, 0, 1.0f);
		if(points) {
			gDebugOverlay->AddPoint(DebugPoint(p.x, p.y, p.z, 0.02f));
		}
		if(lines) {
			auto p2 = parentTransform * pt * glm::vec4(0, 0, 0, 1.0f);
			gDebugOverlay->AddLine({ {p.x, p.y, p.z}, { p2.x, p2.y, p2.z }, {1,1,0} });
		}
	}, now);
}

int main(const int argc, const char **argv) {
	if (!glfwInit()) {
		std::cerr << "glfwInit failed" << std::endl;
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	const std::string windowTitle = "OpenGL Animation Demo";

	auto window = glfwCreateWindow(1280, 720, windowTitle.c_str(), NULL, NULL);
	if (!window) {
		std::cerr << "glfwCreateWindow failed" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "Failed to initialize OpenGL context" << std::endl;
		return -1;
	}

	gDebugOverlay = new DebugOverlay;
	gDebugOverlay->mDepthTest = false;

	auto scene = CreateScene(argc, argv);
	auto input = std::make_shared<Input>(window, scene);

	int windowWidth, windowHeight;
	glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
	glViewport(0, 0, windowWidth, windowHeight);

	auto ui = std::make_shared<UI>(window);

	auto program = ShaderProgram::Load("default");

	const GLuint uniformProj = glGetUniformLocation(program->mID, "uProj");
	const GLuint uniformView = glGetUniformLocation(program->mID, "uView");
	const GLuint uniformModel = glGetUniformLocation(program->mID, "uModel");
	const GLuint uniformBones = glGetUniformLocation(program->mID, "uBones");
	const GLuint uLightPos = glGetUniformLocation(program->mID, "uLightPos");
	const GLuint uViewPos = glGetUniformLocation(program->mID, "uViewPos");
	const GLuint uLightColor = glGetUniformLocation(program->mID, "uLightColor");

	glm::vec3 lightPos = { 100.0f, 100.0f, 100.0f };
	glm::vec3 lightColor = { 1.0f, 1.0f, 1.0f };
	glUseProgram(program->mID);
	glUniform3fv(uLightPos, 1, (GLfloat*)&lightPos[0]);
	glUniform3fv(uLightColor, 1, (GLfloat*)&lightColor[0]);

	Camera cam;
	cam.SetAspect(windowWidth, windowHeight);
	float camSpeed = 10.0f;

	FrameCounter<float> fps;
	Timer<float> timer;
	Timer<float> inputTimer;
	bool debugSkeleton = true;
	bool debugNodes = true;

	std::unordered_map<size_t, bool> animWeightBonesTest;
	std::unordered_map<size_t, bool> animTracksBonesTest;

	while (!glfwWindowShouldClose(window)) {
		const auto deltaTime = timer.Update();
		const auto inputDelta = inputTimer.Update();

		if (fps.Tick(glfwGetTime())) {
			glfwSetWindowTitle(window, (windowTitle + " - FPS: " + std::to_string(fps.mValue)).c_str());
		}

		glfwSwapBuffers(window);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_DEPTH_TEST);
		glUseProgram(program->mID);

		glfwPollEvents();

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, 1);
		}

		if (nullptr != scene->mSelected) {
			auto selectedCenter = scene->mSelected->mPos + scene->mSelected->mUp * scene->mSelected->mModel->mAABB.mHalfSize.y;
			const auto camOffset = selectedCenter + scene->mSelected->mFront * -scene->mCameraDistance;
			const auto camCenter = selectedCenter;
			const auto rotX = glm::rotate(glm::identity<glm::mat4>(), scene->mCameraRotationX, cam.mUp);
			const auto rotY = glm::rotate(glm::identity<glm::mat4>(), scene->mCameraRotationY, cam.mRight);
			const auto posRot = rotX * rotY * glm::vec4(camOffset - camCenter, 1.0f);
			const auto targetPos = glm::vec3(posRot) + camCenter;

			if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS) {
				cam.mPos = targetPos;
				cam.mFront = glm::normalize(selectedCenter - cam.mPos);
			} else {
				cam.mPos = glm::lerp(cam.mPos, targetPos, inputDelta * camSpeed);
				cam.mFront = glm::lerp(cam.mFront, glm::normalize(selectedCenter - cam.mPos), inputDelta * camSpeed);
			}
		}

		cam.UpdateView();
		cam.UpdateProjection();

		scene->Update(timer.mTime);

		ui->NewFrame();

		auto selectedModel = scene->mSelected ? scene->mSelected->mModel : nullptr;
		if (selectedModel) {
			ImGui::Checkbox("Debug Skeleton", &debugSkeleton);
			ImGui::Checkbox("Debug Nodes", &debugNodes);
			ImGui::Text("Name: %s", selectedModel->mName.c_str());
			ImGui::Text("Model: c=%s, s=%s | length=%f",
				glm::to_string(selectedModel->mAABB.mCenter).c_str(),
				glm::to_string(selectedModel->mAABB.mHalfSize).c_str(),
				glm::length(selectedModel->mAABB.mHalfSize) * 2.0f
			);

			if(ImGui::SliderFloat3("Light Pos", &lightPos[0], -100, 100)) {
				glUniform3fv(uLightPos, 1, (GLfloat*)&lightPos[0]);
			}

			if(ImGui::ColorPicker3("Light Color", &lightColor[0])) {
				glUniform3fv(uLightColor, 1, (GLfloat*)&lightColor[0]);
			}
		}

		if (selectedModel && scene->mSelected->mAnimationController) {
			const auto& ac = scene->mSelected->mAnimationController;
			const auto& as = ac->mAnimationSet;
			ImGui::Begin("Animations");

			auto maxDuration = std::max_element(as->mAnimations.begin(), as->mAnimations.end(), [](auto& a, auto& b) {
				return a->mDuration > b->mDuration;
			});
			auto animTime = fmod(timer.mTime, (*maxDuration)->mDuration);
			if(ImGui::SliderFloat("Time", &animTime, 0.0f, (*maxDuration)->mDuration)) {
				timer.Set(animTime);
			}
			ImGui::SliderFloat("Speed", &timer.mScale, 0.0f, 2.0f);

			for(size_t animIndex = 0; animIndex < as->mAnimations.size(); ++animIndex) {
				auto& anim = as->mAnimations[animIndex];
				ImGui::PushID(animIndex);
				std::string name = std::to_string(animIndex) + ": " + anim->mName + " - " + std::to_string(anim->mDuration) + "/" + std::to_string(anim->mTicksPerSecond);
				if(ImGui::Button("Bones")) {
					animWeightBonesTest[animIndex] = !animWeightBonesTest[animIndex];
				}
				ImGui::SameLine();
				if(ImGui::Button("Tracks")) {
					animTracksBonesTest[animIndex] = !animTracksBonesTest[animIndex];
				}
				ImGui::SameLine();
				ImGui::SliderFloat(name.c_str(), &ac->mAnimationWeights[animIndex], 0.0f, 1.0f);
				animTime = fmod(timer.mTime, anim->mDuration); // TOOD: Move animation details to new window?
				ImGui::SliderFloat("Time", &animTime, 0.0f, anim->mDuration);
				if(animWeightBonesTest[animIndex]) {
					auto& disabledBones = ac->mDisabledBones[animIndex];
					auto disableNode = [&disabledBones](auto& node, auto& self) -> void {
						if(node->mCachedBoneIndex >= 0) disabledBones[node->mCachedBoneIndex] = !disabledBones[node->mCachedBoneIndex];
						for(auto& child : node->mChildren) self(child, self);
					};
					auto nodes = [&disableNode](auto& node, auto& self) -> void {
						if(ImGui::TreeNode(node.get(), node->mName.c_str())) {
							if(ImGui::Button("Toggle")) {
								disableNode(node, disableNode);
							}
							for(auto& child : node->mChildren) self(child, self);
							ImGui::TreePop();
						}
					};
					nodes(as->mRootNode, nodes);
					for(const auto& [boneName, boneIndex] : as->mBoneMappings) {
						ImGui::Checkbox(boneName.c_str(), &disabledBones[boneIndex]);
					}
				}
				if(animTracksBonesTest[animIndex]) {
					for(auto& track : anim->mAnimationTracks) {
						ImGui::LabelText(track->mName.c_str(), "P:%d R:%d S:%d", track->mPositionKeys.size(), track->mRotationKeys.size(), track->mScalingKeys.size());
					}
				}
				ImGui::PopID();
			}

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

		glUniformMatrix4fv(uniformProj, 1, GL_FALSE, (GLfloat*)&cam.mProjection[0]);
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, (GLfloat*)&cam.mView[0]);
		glUniform3fv(uViewPos, 1, (GLfloat*)&cam.mPos[0]);

		for (auto& entity : scene->mEntities) {
			const auto& model = entity->mModel;
			if (!model) continue;
			if (entity->mAnimationController) {
				const auto& bones = entity->mAnimationController->mFinalTransforms;
				glUniformMatrix4fv(uniformBones, bones.size(), GL_FALSE, (GLfloat*)&bones[0]);
			}
			glm::mat4 transform = glm::translate(glm::identity<glm::mat4>(), entity->mPos);
			transform *= glm::mat4_cast(entity->mRot);
			transform = glm::scale(transform, entity->mScale);
			RenderNode(uniformModel, model->mRootNode, transform);
			if((debugSkeleton || debugNodes) && entity->mAnimationController) {
				RenderSkeleton(entity->mModel, entity->mAnimationController, timer.mTime, transform, debugNodes, debugSkeleton);
			}
		}

		gDebugOverlay->Render(cam);
		gDebugOverlay->Clear();

		ui->Render();
	}

	input.reset();
	scene.reset();
	program.reset();

	glfwTerminate();
	return 0;
}
