#pragma once

#include "Main.h"
#include "Shader.h"

struct DebugLine {
	glm::vec3 mStart;
	glm::vec3 mEnd;
	glm::vec3 mColor;
};

struct DebugPoint {
	glm::vec3 mPos;
	glm::vec3 mColor = { 1,1,1 };
	float mScale = 1.0f;
	DebugPoint(float x, float y, float z, float s = 1.0f) : DebugPoint(glm::vec3(x, y, z), s) {}
	DebugPoint(const glm::vec3& p, float s = 1.0f) {
		mPos = p;
		mScale = s;
	}
};

struct DebugOverlay {
	std::vector<DebugLine> mLines;
	std::vector<DebugPoint> mPoints;
	ShaderProgram_ mLineProgram;
	ShaderProgram_ mPointProgram;
	bool mDepthTest = true;
	bool mEnabled = true;

	DebugOverlay() {
		mLineProgram = ShaderProgram::Load("debug");
		mPointProgram = ShaderProgram::Load("particles");
	}

	void Clear() {
		mLines.clear();
		mPoints.clear();
	}

	void AddLine(const DebugLine& line) { mLines.push_back(line); }

	void AddGrid(float scale, float halfSize, const glm::vec3& color) {
		for(float dx = -halfSize; dx <= halfSize; ++dx) {
			AddLine({ {dx * scale, 0, -halfSize * scale}, {dx * scale, 0, halfSize * scale}, color });
			AddLine({ {-halfSize * scale, 0, dx * scale}, {halfSize * scale, 0, dx * scale}, color });
		}
	}

	void AddPoint(const DebugPoint& point) {
		mPoints.push_back(point);
	}

	void AddPoint(const glm::vec3& point) {
		mPoints.push_back({ point });
	}

	void Render(const Camera& cam) {
		if(mDepthTest) glEnable(GL_DEPTH_TEST);
		else glDisable(GL_DEPTH_TEST);

		auto drawDebugLines = [](auto& debugLines) {
			size_t count = 0;
			for(auto& debugLine : debugLines) {
				if(count++ > 10000) break;
				// FIXME!
				auto debugMesh = std::make_shared<Mesh>();
				debugMesh->mIndices.push_back(0);
				debugMesh->mIndices.push_back(1);
				debugMesh->mVertices.push_back({
					debugLine.mStart,
					{},
					debugLine.mColor
				});
				debugMesh->mVertices.push_back({
					debugLine.mEnd,
					{},
					debugLine.mColor
				});
				debugMesh->Bind();
				glDrawElements(GL_LINES, debugMesh->mIndices.size(), GL_UNSIGNED_INT, 0);
			}
		};

		auto drawDebugPoints = [](auto& debugPoints) {
			auto debugMesh = std::make_shared<Mesh>();
			for(auto& debugPoint : debugPoints) {
				float ps = 0.25f;
				float quad[] = {
					0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
					0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
				};
				for(int v = 0; v < 12; v += 2) {
					Vertex vertex;
					vertex.mPos = debugPoint.mPos;
					vertex.mNormal.x = (quad[v] - 0.5f) * ps;
					vertex.mNormal.y = (quad[v + 1] - 0.5f) * ps;
					vertex.mNormal.z = debugPoint.mScale;
					debugMesh->mIndices.push_back(debugMesh->mIndices.size());
					debugMesh->mVertices.push_back(vertex);
				}
			}
			debugMesh->Bind();
			glDrawElements(GL_TRIANGLES, debugMesh->mIndices.size(), GL_UNSIGNED_INT, 0);
		};

		auto debugTransform = glm::identity<glm::mat4>();

		if(mLines.size()) {
			glUseProgram(mLineProgram->mID);
			glUniformMatrix4fv(glGetUniformLocation(mLineProgram->mID, "uProj"), 1, GL_FALSE, (GLfloat*)&cam.mProjection[0]);
			glUniformMatrix4fv(glGetUniformLocation(mLineProgram->mID, "uView"), 1, GL_FALSE, (GLfloat*)&cam.mView[0]);
			glUniformMatrix4fv(glGetUniformLocation(mLineProgram->mID, "uModel"), 1, GL_FALSE, (GLfloat*)&debugTransform[0]);

			drawDebugLines(mLines);
		}

		if(mPoints.size()) {
			glUseProgram(mPointProgram->mID);
			glUniformMatrix4fv(glGetUniformLocation(mPointProgram->mID, "uProj"), 1, GL_FALSE, (GLfloat*)&cam.mProjection[0]);
			glUniformMatrix4fv(glGetUniformLocation(mPointProgram->mID, "uView"), 1, GL_FALSE, (GLfloat*)&cam.mView[0]);
			glUniformMatrix4fv(glGetUniformLocation(mPointProgram->mID, "uModel"), 1, GL_FALSE, (GLfloat*)&debugTransform[0]);

			drawDebugPoints(mPoints);
		}
	}
};
