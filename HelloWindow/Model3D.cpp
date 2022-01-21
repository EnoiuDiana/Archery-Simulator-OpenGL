#include "Model3D.hpp"


	Model3D::Model3D()
	{
	}

	void Model3D::RenderModel(gps::Shader shaderProgram)
	{
		for (size_t i = 0; i < meshList.size(); i++)
		{
			meshList[i]->Draw(shaderProgram);
		}
	}

	void Model3D::LoadModel(const std::string& fileName, const std::string path, bool transparentModel)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(fileName, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices);

		if (!scene)
		{
			printf("Model (%s) failed to load: %s", fileName, importer.GetErrorString());
			return;
		}

		directory = path.substr(0, path.find_last_of("/"));

		isTransparentModel = transparentModel;
		
		LoadNode(scene->mRootNode, scene);
		
		
		//LoadMaterials(scene);
	}

	void Model3D::LoadNode(aiNode* node, const aiScene* scene)
	{
		for (size_t i = 0; i < node->mNumMeshes; i++)
		{
			LoadMesh(scene->mMeshes[node->mMeshes[i]], scene);
		}
		for (size_t i = 0; i < node->mNumChildren; i++)
		{
			LoadNode(node->mChildren[i], scene);
		}

	}

	void Model3D::LoadMesh(aiMesh* mesh, const aiScene* scene)
	{
		std::vector<Vertex> vertices;
		std::vector<GLuint> indices;

		for (size_t i = 0; i < mesh->mNumVertices; i++)
		{

			Vertex new_vertex;
			new_vertex.Position = glm::vec3( mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z );
			
			if (mesh->mTextureCoords[0])
			{
				new_vertex.TexCoords = glm::vec2( mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y );
			}
			else {
				new_vertex.TexCoords = glm::vec2(0.0f, 0.0f);
			}
			new_vertex.Normal = glm::vec3(-mesh->mNormals[i].x, -mesh->mNormals[i].y, -mesh->mNormals[i].z );

			vertices.push_back(new_vertex);
		}

		for (size_t i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (size_t j = 0; j < face.mNumIndices; j++)
			{
				indices.push_back(face.mIndices[j]);
			}
		}

		//process material
		if (mesh->mMaterialIndex >= 0) {
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

			//ambient maps
			std::vector<Texture> ambientMaps = loadTextures(material, aiTextureType_AMBIENT, "ambientTexture");
			loadedTextures.insert(loadedTextures.end(), ambientMaps.begin(), ambientMaps.end());

			//diffuse maps
			std::vector<Texture> diffuseMaps = loadTextures(material, aiTextureType_DIFFUSE, "diffuseTexture");
			loadedTextures.insert(loadedTextures.end(), diffuseMaps.begin(), diffuseMaps.end());

			//specular maps
			std::vector<Texture> specularMaps = loadTextures(material, aiTextureType_SPECULAR, "specularTexture");
			loadedTextures.insert(loadedTextures.end(), specularMaps.begin(), specularMaps.end());

		}

		Mesh* newMesh = new Mesh(vertices, indices, loadedTextures);
		meshList.push_back(newMesh);
		
	}


	// load list of textures
	std::vector<Texture> Model3D::loadTextures(aiMaterial* mat, aiTextureType type, std::string textureName) {
		std::vector<Texture> textures;

		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
			aiString str;
			mat->GetTexture(type, i, &str);

			std::string resultedStr = str.C_Str();

			std::size_t found = resultedStr.find_last_of("/\\");
			std::string filename = resultedStr.substr(found + 1);

			// prevent duplicate loading
			bool skip = false;
			
			for (unsigned int j = 0; j < loadedTextures.size(); j++) {
				if (std::strcmp(loadedTextures[j].path.data(), (directory + "/" + filename).c_str()) == 0) {
					//textures.push_back(loadedTextures[j]);
					skip = true;
					break;
				}
			}

			if (!skip) {
				// not loaded yet
				Texture newTexture;
				newTexture.id = ReadTextureFromFile((directory + "/" + filename).c_str());
				newTexture.type = textureName;
				newTexture.path = directory + "/" + filename;
				textures.push_back(newTexture);
			}
		}

		return textures;
	}


	// Reads the pixel data from an image file and loads it into the video memory
	GLuint Model3D::ReadTextureFromFile(const char* file_name) {

		int width, height, nChannels;

		unsigned char* data = stbi_load(file_name, &width, &height, &nChannels, 0);

		GLenum colorMode = GL_RGB;
		switch (nChannels) {
		case 1:
			colorMode = GL_RED;
			break;
		case 3:
			colorMode = GL_RGB;
			break;
		case 4:
			colorMode = GL_RGBA;
			break;
		};

		if (data) {
			GLuint textureID;
			glGenTextures(1, &textureID);
			glBindTexture(GL_TEXTURE_2D, textureID);
			

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


			if (isTransparentModel) {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, colorMode, GL_UNSIGNED_BYTE, data);
			}
			else {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, width, height, 0, colorMode, GL_UNSIGNED_BYTE, data);
			}
			glGenerateMipmap(GL_TEXTURE_2D);

			glBindTexture(GL_TEXTURE_2D, 0);

			stbi_image_free(data);
			return textureID;
		}
		else {
			std::cout << "Image not loaded at " << file_name << std::endl;
			return false;
		}
	}

	Model3D::~Model3D()
	{
	}


