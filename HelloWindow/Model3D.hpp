#pragma once

#include <vector>
#include <string>

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>      

#include "Mesh.hpp"
#include "stb_image.h"

	class Model3D
	{
	public:
		Model3D();

		void LoadModel(const std::string& fileName, const std::string path, bool isTransparentModel);
		void RenderModel(gps::Shader shaderProgram);
		
		~Model3D();

	private:

		void LoadNode(aiNode* node, const aiScene* scene);
		void LoadMesh(aiMesh* mesh, const aiScene* scene);

		// Reads the pixel data from an image file and loads it into the video memory
		GLuint ReadTextureFromFile(const char* file_name);

		std::vector<Mesh*> meshList;
		std::vector<Texture> loadedTextures;

		std::vector<Texture> loadTextures(aiMaterial* mat, aiTextureType type, std::string textureName);

		std::string directory;
		bool isTransparentModel = false;
	};


