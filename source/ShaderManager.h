#pragma once
#include "shader\Shader.h"
#include <vector>

class ShaderManager {
public:
	ShaderManager();
	~ShaderManager();

	void InitializeShaders();
public:
	Shader m_pDefault;
	Shader m_pBasic;
	Shader m_pLighting;
	Shader m_pLamp;
	Shader m_pToon;
	Shader m_pHatch;
	Shader m_pOutline;
	Shader m_pTransparent;
	Shader m_pScreen;
	Shader m_pSkybox;
	Shader m_pReflection;
	Shader m_pRefraction;
	Shader m_pNormal;
};