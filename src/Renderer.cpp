#include "Renderer.hpp"

#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "glm/ext.hpp"

#include "Shader.hpp"
#include "FontAtlas.hpp"


const char* vertexShaderSource = "#version 330 core\n"
"layout(location = 0) in vec3 vertex; \n"
"layout(location = 1) in vec2 uv;\n"
"layout(location = 2) in vec4 col;\n"
"\n"
"uniform mat4 model;\n"
"uniform mat4 projection;\n"
"uniform mat4 camera;\n"
"\n"
"out vec2 TexCoords;\n"
"out vec4 color;\n"
"\n"
"void main()\n"
"{\n"
"	gl_Position = projection * camera * model * vec4(vertex, 1.0);\n"
"	TexCoords = uv;\n"
"	color = col;\n"
"}\n";



const char* fragmentShaderSource = "#version 330 core\n"
"in vec2 TexCoords;\n"
"in vec4 color;\n"
"\n"
"uniform sampler2D image;\n"
"uniform float screenPxRange;\n"
"\n"
"float median(float r, float g, float b)\n"
"{\n"
"	return max(min(r, g), min(max(r, g), b));\n"
"}\n"
"\n"
"void main()\n"
"{\n"
"	vec3 msd = texture(image, TexCoords).rgb;\n"
"	float sd = median(msd.r, msd.g, msd.b);\n"
"\n"
"	vec4 bgColor = vec4(.0, .0, .0, .0);\n"
"\n"
"	float screenPxDistance = screenPxRange * (sd - 0.5);\n"
"	float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);\n"
"	gl_FragColor = mix(bgColor, color, opacity);\n"
"\n"
"	if (opacity < 0.5f)	\n"
"	{\n"
"		discard;\n"
"	}\n"
"}\n";

struct BatchData
{
	std::vector<VertexData> textureToQuadVertices = std::vector<VertexData>(256);
	int quadCount;
};

// initially space for 240 / 6 = 40 letters
std::map<unsigned int, BatchData> textureToBatch;


Renderer::Renderer()
	: cameraPosition_(glm::vec2(0,0)), zoom_(1.0f)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

GLFWwindow* Renderer::CreateWindow(std::string name, glm::vec2 resolution, glm::vec2 worldUnits)
{
	screenSize_ = resolution;
	worldSize_ = worldUnits;

	window_ = glfwCreateWindow(resolution.x, resolution.y, name.c_str(), nullptr, nullptr);

	glfwMakeContextCurrent(window_);

	if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress))
	{
		printf("[Error] - Renderer - Failed to initialize GLAD\n");
		return nullptr;
	}

	glViewport(0, 0, resolution.x, resolution.y);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glfwSwapInterval(0); //disable vsync



	projection_ = glm::ortho(0.0f, worldUnits.x, 0.0f, worldUnits.y, -1.f, 1.f);


	// setup the shader
	shader_ = std::make_shared<Shader>();
	shader_->Compile(vertexShaderSource, fragmentShaderSource);
	shader_->Use();


	shader_->SetMatrix4("projection", projection_, true);

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(80, 40, 0.0f));
	shader_->SetMatrix4("model", model);

	return window_;
}

void Renderer::BeginFrame()
{
	// wee don't have to clear quadVertices because we will just render the nearly entered totalQuads anyway
	// clearing and reallocating the memory would only slow things down

	for (auto& batchEntry: textureToBatch)
	{
		batchEntry.second.quadCount = 0;
	}

	glm::mat4 camera(1.0f);
	camera = glm::scale(camera, glm::vec3(zoom_, zoom_, 1.0f));
	camera = glm::translate(camera, glm::vec3(cameraPosition_, 0.f));
	shader_->SetMatrix4("camera", camera);

	// for 2d rendering https://github.com/Chlumsky/msdfgen states that screenPxRange can be a precomputed value even.. 
	// according to be docs sizeInPixels should be the quadsize (so a single letter)
	// we currently don't have a method to get this sice for each letter since we are batch rendering
	int pixelRange = 2;
	glm::vec2  distanceField = glm::vec2(256, 256);
	glm::vec2 sizeInPixels = this->EuToPixel(glm::vec2(20, 20)) * this->GetZoom();
	float screenPxRange = (sizeInPixels.x / distanceField.x) * pixelRange;
	shader_->SetFloat("screenPxRange", screenPxRange);
}

void Renderer::EndFrame(FontAtlas& atlas)
{
	glBindBuffer(GL_ARRAY_BUFFER, atlas.GetVBO());

	auto byteSize = sizeof(VertexData) * 6 * textureToBatch[atlas.GetTexture()].quadCount;
	glBufferData(GL_ARRAY_BUFFER, byteSize, textureToBatch[atlas.GetTexture()].textureToQuadVertices.data(), GL_DYNAMIC_DRAW);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, atlas.GetTexture());
	glBindVertexArray(atlas.GetQuadVAO());
	glDrawArrays(GL_TRIANGLES, 0, 6 * textureToBatch[atlas.GetTexture()].quadCount);
}

glm::vec2 Renderer::GetCameraPosition()
{
	return cameraPosition_;
}

void Renderer::SetCameraPosition(glm::vec2 position)
{
	this->cameraPosition_ = position;
}

float Renderer::GetZoom()
{
	return zoom_;
}

void Renderer::SetZoom(float zoom)
{
	this->zoom_ = zoom;
}

glm::vec2 Renderer::GetResolution()
{
	return screenSize_;
}

glm::vec2 Renderer::EuToPixel(glm::vec2 size)
{
	return glm::vec2(
		(screenSize_.x / worldSize_.x) * size.x,
		(screenSize_.y / worldSize_.y) * size.y
	);
}

std::shared_ptr<Shader> Renderer::GetShader()
{
	return shader_;
}

void Renderer::DrawText(FontAtlas& atlas, std::string text, glm::vec3 position, float size, glm::vec4 color, bool center)
{
	BatchData& batchData = textureToBatch[atlas.GetTexture()];

	std::vector<VertexData>& fontVertexData = batchData.textureToQuadVertices;

	// we render each letter as two triangles with 3 verts each
	const int vertsPerCharacter = 6;

	constexpr double tabWidthInEms = 2.0;

	// check if our batch rendering has anough space for all vertices
	while (fontVertexData.size() <= (batchData.quadCount + text.length()) * vertsPerCharacter)
	{
		fontVertexData.resize(fontVertexData.size() * 2);
		printf("Resized capacity of batch renderer to: %lu\n", fontVertexData.size());
	}

	unsigned int fontTexture = atlas.GetTexture();

	double fontLineHeight = 0.0, fontAscenderHeight = 0.0, fontDescenderHeight = 0.0;
	atlas.GetFontVerticalMetrics(fontTexture, fontLineHeight, fontAscenderHeight, fontDescenderHeight);

	double xoffset = 0;
	double yoffset = fontDescenderHeight - 1.0;

	size_t currentQuadIndex = 0;
	unsigned int currentLine = 0;
	char prevChar = 0;
	double cursorPos = 0.0;

	// calculate line widths required for text alignment
	std::vector<double> lineWidths;
	lineWidths.emplace_back();
	size_t lineCount = 1;
	for (const char& c : text)
	{
		switch (c)
		{
		case '\r':
			lineWidths.back() = 0.0;
			break;
		case '\n': case '\f':
			lineWidths.emplace_back();
			lineCount++;
			break;
		case '\t':
		{
			unsigned int cursorPosRoundedDown = (unsigned int)lineWidths.back();
			lineWidths.back() = double(cursorPosRoundedDown) + tabWidthInEms - (fmod(cursorPosRoundedDown, tabWidthInEms));
			break;
		}
		default:
			lineWidths.back() += atlas.GetFontCharAdvance(fontTexture, c);
			break;
		}
	}


	// perform font calulation and add geometry to batch renderer   
	for (const char& c : text)
	{
		if (c == '\n')
		{
			currentLine++;
			cursorPos = 0.0;
			continue;
		}
		if (c == '\r')
		{
			cursorPos = 0.0;
			continue;
		}
		if (c == '\t')
		{
			unsigned int cursorPosRoundedDown = (unsigned int)cursorPos;
			cursorPos = double(cursorPosRoundedDown) + tabWidthInEms - fmod(cursorPosRoundedDown, tabWidthInEms);
			continue;
		}

		if (center)
		{
			xoffset = -lineWidths[currentLine] / 2.0;
		}

		float l, r, b, t;
		atlas.GetFontCharUVBounds(fontTexture, c, l, r, b, t);
		fontVertexData[batchData.quadCount * vertsPerCharacter].atlasUV	 = { l, t }; //lt
		fontVertexData[batchData.quadCount * vertsPerCharacter + 1].atlasUV = { r, b }; //rb
		fontVertexData[batchData.quadCount * vertsPerCharacter + 2].atlasUV = { l, b }; //lb
		fontVertexData[batchData.quadCount * vertsPerCharacter + 3].atlasUV = { l, t }; //lt
		fontVertexData[batchData.quadCount * vertsPerCharacter + 4].atlasUV = { r, t }; //rt
		fontVertexData[batchData.quadCount * vertsPerCharacter + 5].atlasUV = { r, b }; //rb

		atlas.GetFontCharQuadBounds(fontTexture, c, l, r, b, t, prevChar);
		fontVertexData[batchData.quadCount * vertsPerCharacter].ep_position	 = position + glm::vec3(size * (l + cursorPos + xoffset), size * (t - currentLine * fontLineHeight + yoffset), 0); // lt
		fontVertexData[batchData.quadCount * vertsPerCharacter + 1].ep_position = position + glm::vec3(size * (r + cursorPos + xoffset), size * (b - currentLine * fontLineHeight + yoffset), 0); // rb
		fontVertexData[batchData.quadCount * vertsPerCharacter + 2].ep_position = position + glm::vec3(size * (l + cursorPos + xoffset), size * (b - currentLine * fontLineHeight + yoffset), 0); // lb
		fontVertexData[batchData.quadCount * vertsPerCharacter + 3].ep_position = position + glm::vec3(size * (l + cursorPos + xoffset), size * (t - currentLine * fontLineHeight + yoffset), 0); // lt
		fontVertexData[batchData.quadCount * vertsPerCharacter + 4].ep_position = position + glm::vec3(size * (r + cursorPos + xoffset), size * (t - currentLine * fontLineHeight + yoffset), 0); // rt
		fontVertexData[batchData.quadCount * vertsPerCharacter + 5].ep_position = position + glm::vec3(size * (r + cursorPos + xoffset), size * (b - currentLine * fontLineHeight + yoffset), 0); // rb

		fontVertexData[batchData.quadCount * vertsPerCharacter].color = color;
		fontVertexData[batchData.quadCount * vertsPerCharacter + 1].color = color;
		fontVertexData[batchData.quadCount * vertsPerCharacter + 2].color = color;
		fontVertexData[batchData.quadCount * vertsPerCharacter + 3].color = color;
		fontVertexData[batchData.quadCount * vertsPerCharacter + 4].color = color;
		fontVertexData[batchData.quadCount * vertsPerCharacter + 5].color = color;

		batchData.quadCount++;
		prevChar = c;
		cursorPos += atlas.GetFontCharAdvance(fontTexture, c);
	}
}