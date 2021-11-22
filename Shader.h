#pragma once

#include "Main.h"

struct Shader {
	GLuint mID = 0;
	GLenum mType;

	Shader(const std::string& path, GLenum type) : mType(type) {
		mID = glCreateShader(type);
		Load(path);
	}

	~Shader() {
		glDeleteShader(mID);
	}

	void Load(const std::string& path) {
		std::cout << "Loading shader: " << path << "..." << std::endl;

		const auto source = ReadFile(path);

		const char* sourcePtr = source.c_str();
		glShaderSource(mID, 1, &sourcePtr, NULL);
		glCompileShader(mID);

		GLint status = 0;
		GLint infoLogLength = 0;
		glGetShaderiv(mID, GL_COMPILE_STATUS, &status);
		glGetShaderiv(mID, GL_INFO_LOG_LENGTH, &infoLogLength);

		if (infoLogLength > 0) {
			std::string message;
			message.resize(infoLogLength + 1);
			glGetShaderInfoLog(mID, infoLogLength, NULL, &message[0]);
			throw new std::runtime_error(message);
		}

		std::cout << "Shader loaded: " << path << std::endl;
	}
};
typedef std::shared_ptr<Shader> Shader_;

struct ShaderProgram {
	GLuint mID = 0;

	ShaderProgram(const std::vector<Shader_>& shaders) {
		mID = glCreateProgram();
		for (auto& shader : shaders) {
			glAttachShader(mID, shader->mID);
		}
		glLinkProgram(mID);

		GLint status = 0;
		GLint infoLogLength = 0;
		glGetProgramiv(mID, GL_LINK_STATUS, &status);
		glGetProgramiv(mID, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength > 0) {
			std::string message;
			message.resize(infoLogLength + 1);
			glGetProgramInfoLog(mID, infoLogLength, NULL, &message[0]);
			throw new std::runtime_error(message);
		}

		for (auto& shader : shaders) {
			glDetachShader(mID, shader->mID);
		}
	}
	~ShaderProgram() {
		glDeleteProgram(mID);
	}

	static std::shared_ptr<ShaderProgram> Load(const std::string& name) {
		std::vector<Shader_> shaders;
		shaders.push_back(std::make_shared<Shader>(name + ".vert.glsl", GL_VERTEX_SHADER));
		shaders.push_back(std::make_shared<Shader>(name + ".frag.glsl", GL_FRAGMENT_SHADER));
		return std::make_shared<ShaderProgram>(shaders);
	}
};
typedef std::shared_ptr<ShaderProgram> ShaderProgram_;
