#include <string>
#include <memory>

#include "glm/glm.hpp"

struct GLFWwindow;
class Shader;
class FontAtlas;
class Renderer
{
	//Projects the ingame units to normalized opengl coordinates ([-1,1])
	glm::mat4 projection_;

	// position of the camera in engine units (EU)
	glm::vec2 cameraPosition_;

	float zoom_;

	std::shared_ptr<Shader> shader_;

	glm::vec2 screenSize_;
	glm::vec2 worldSize_;
	GLFWwindow* window_;

public:
	Renderer();
	GLFWwindow* CreateWindow(std::string name, glm::vec2 resolution, glm::vec2 worldUnits);
	void BeginFrame();
	void EndFrame(FontAtlas& atlas);

	glm::vec2 GetCameraPosition();
	void SetCameraPosition(glm::vec2 position);

	float GetZoom();
	void SetZoom(float zoom);

	glm::vec2 GetResolution();

	glm::vec2 EuToPixel(glm::vec2 size);

	std::shared_ptr<Shader> GetShader();

	void DrawText(FontAtlas& atlas, std::string text, glm::vec3 position, float size, glm::vec4 color, bool center = true);
};