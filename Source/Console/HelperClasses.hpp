#include <OpenGL/WinAPI.hpp>
#include <OpenGL/Functions.hpp>
using namespace CGGD::OpenGL;
using namespace CGGD::OpenGL::WinAPI;

#include <OpenIL/Functions.hpp>
using namespace CGGD::OpenIL;

#include <WinAPI/Window.hpp>
using namespace CGGD;
using namespace CGGD::WinAPI;

#define _USE_MATH_DEFINES
#include <cstdint>
#include <vector>
#include <fstream>
#include <utility>
#include <math.h>
#include <algorithm>

using namespace std;

#define PATH(x) "../../../../" + (std::string)x

std::string path(const std::string& x);
std::string loadFile(const std::string& filename);

struct Vertex
{
	float x;
	float y;
	unsigned char r;
	unsigned char g;
	unsigned char b;
	float u;
	float v;
};

class Texture2D
{
private:
	GLuint mTexture;
	unsigned int mSlot = -1;

	static unsigned int smAvalaibleSlot;

	void init();
	void setParams(GLint param1, GLint param2, GLint param3);

public:
	Texture2D(const std::string& fullpath);
	Texture2D(const Image* image);
	Texture2D(const Texture2D& other) = delete;
	Texture2D(Texture2D&& other);
	~Texture2D();

	Texture2D& operator= (const Texture2D& other) = delete;
	Texture2D& operator= (Texture2D&& other);


	GLuint getTexture() const;
	unsigned int getSlot() const;
};

class Sprite  // init -> create texture obj -> create sprite obj -> loop [renderAll]
{
public:
	float posX;
	float posY;
	float sizeX;
	float sizeY;
	float angle;
	Texture2D* texture = nullptr;

private:
	unsigned int mPriority = -1;

	std::vector<Vertex> mVertexSource = {
		Vertex{ -1.0f, -1.0f, 0xFF, 0xFF, 0xFF, 0.0f, 0.0f },
		Vertex{ +1.0f, -1.0f, 0xFF, 0xFF, 0xFF, 1.0f, 0.0f },
		Vertex{ -1.0f, +1.0f, 0xFF, 0xFF, 0xFF, 0.0f, 1.0f },
		Vertex{ +1.0f, +1.0f, 0xFF, 0xFF, 0xFF, 1.0f, 1.0f },
	};
	std::vector<std::uint8_t> mIndexSource = {
		0, 1, 2,
		1, 2, 3
	};

	GLuint mVertexBuffer;
	GLuint mIndexBuffer;
	GLuint mAttributeBuffer;
	static GLuint mProgram;

	static GLint smUnifLocTexColor;
	static GLint smUnifLocTransfromMat;

	const unsigned int mVertexDraw = 6;
	static std::vector<const Sprite *> smObjects;

	static HWND smHwnd;
	static unsigned int smWndHeight;
	static unsigned int smWndWidth;

	static void setWndSizes();
	void init();

public:
	Sprite(float x, float y, float w, float h, float ang);
	Sprite(float x, float y, float w, float h, float ang, unsigned int prior, Texture2D* textur);
	Sprite(const Sprite& other);
	Sprite(Sprite&& other);
	~Sprite();

	Sprite& operator= (const Sprite& other);
	Sprite& operator= (Sprite&& other);

	void setPriority(unsigned int priority);
	static void init(const HWND hwnd, GLuint program, GLint texColor, GLint transformMat);
	static void renderAll();
};