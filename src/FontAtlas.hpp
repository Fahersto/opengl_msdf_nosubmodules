#include <string>
#include <memory>
#include <map>
#include <vector>

#include "glm/glm.hpp"

class Texture;
struct VertexData
{
	glm::vec3 ep_position;
	glm::vec2 atlasUV;
	glm::vec4 color;
};


class FontAtlas
{
	unsigned int quadVAO_;
	unsigned int vbo_;
	unsigned int fontTexture_;

	void Initialize(std::string fontFile);


	
public:
	FontAtlas(std::string fontFile);

	void GetFontCharUVBounds(unsigned int atlas, uint32_t unicodeChar, float& out_l, float& out_r, float& out_b, float& out_t);
	void GetFontCharQuadBounds(unsigned int atlas, uint32_t unicodeChar, float& out_l, float& out_r, float& out_b, float& out_t, uint32_t prevChar);
	double GetFontCharAdvance(unsigned int atlas, uint32_t unicodeChar);
	void GetFontVerticalMetrics(unsigned int atlas, double& out_lineHeight, double& out_ascenderHeight, double& out_descenderHeight);

	unsigned int GetTexture();
	unsigned int GetVBO();
	unsigned int GetQuadVAO();

};