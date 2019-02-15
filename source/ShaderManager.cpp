#include "ShaderManager.h"
ShaderManager::ShaderManager(){}
void ShaderManager::InitializeShaders()
{
	m_pDefault = Shader("media/shaders/default/DefaultVertexShader.vs", "media/shaders/default/DefaultFragmentShader.frag");
	m_pBasic = Shader("media/shaders/basic/BasicVertexShader.vs", "media/shaders/basic/BasicFragmentShader.frag");
	m_pLighting = Shader("media/shaders/lighting/LightVertexShader.vs", "media/shaders/lighting/LightFragmentShader.frag");
	m_pLamp = Shader("media/shaders/lighting/LightVertexShader.vs", "media/shaders/lighting/LampFragmentShader.frag");
	m_pToon = Shader("media/shaders/toon/ToonVertexShader.vs", "media/shaders/toon/ToonFragmentShader.frag");
	m_pHatch = Shader("media/shaders/hatch/HatchVertexShader.vs", "media/shaders/hatch/HatchFragmentShader.frag");
	m_pOutline = Shader("media/shaders/outline/OutlineVertexShader.vs", "media/shaders/outline/OutlineFragmentShader.frag");
	m_pTransparent = Shader("media/shaders/transparent/TransparentVertexShader.vs", "media/shaders/transparent/TransparentFragmentShader.frag");
	m_pScreen = Shader("media/shaders/screen/ScreenVertexShader.vs", "media/shaders/screen/ScreenFragmentShader.frag");
	m_pSkybox = Shader("media/shaders/skybox/SkyboxVertexShader.vs", "media/shaders/skybox/SkyboxFragmentShader.frag");
	m_pReflection = Shader("media/shaders/reflection/ReflectionVertexShader.vs", "media/shaders/reflection/ReflectionFragmentShader.frag");
	m_pRefraction = Shader("media/shaders/refraction/RefractionVertexShader.vs", "media/shaders/refraction/RefractionFragmentShader.frag");
	m_pNormal = Shader("media/shaders/normal/NormalVertexShader.vs", "media/shaders/normal/NormalFragmentShader.frag", "media/shaders/normal/NormalGeometryShader.geo");
}

ShaderManager::~ShaderManager()
{
}
