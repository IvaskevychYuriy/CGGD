#pragma region Include

#define _USE_MATH_DEFINES
#include <cstdint>
#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>
#include <utility>
#include <algorithm>
#include <ctime>
using namespace std;

#include <WinAPI/Window.hpp>
using namespace CGGD;
using namespace CGGD::WinAPI;

#include <OpenGL/WinAPI.hpp>
#include <OpenGL/Functions.hpp>
using namespace CGGD::OpenGL;
using namespace CGGD::OpenGL::WinAPI;

#include <OpenIL/Functions.hpp>
using namespace CGGD::OpenIL;
#pragma endregion

#define PATH(x) "../../../../" + (std::string)x
std::string path(const std::string& x)
{
	return "../../../../" + x;
}
std::string loadFile(const std::string& filename)
{
	std::ifstream file(filename);

	std::string source;

	std::getline(file, source, '\0');

	file.close();

	return source;
}

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

	void init()
	{
		glGenTextures(1, &mTexture); OpenGL::ErrorTest();
		glActiveTexture(GL_TEXTURE0 + (int)mSlot); OpenGL::ErrorTest();
		glBindTexture(GL_TEXTURE_2D, mTexture); OpenGL::ErrorTest();
	}

	void setParams(GLint param1, GLint param2, GLint param3)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, param1); OpenGL::ErrorTest();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, param2); OpenGL::ErrorTest();

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, param3); OpenGL::ErrorTest();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, param3); OpenGL::ErrorTest();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, param3); OpenGL::ErrorTest();
	}

public:
	Texture2D(const std::string& fullpath) :
		mSlot(smAvalaibleSlot)
	{
		Image* pImage = Image::Load2D(fullpath);
		if (!pImage)
		{
			throw std::exception("fucking image\n");
		}
		
		init();
		setParams(GL_NEAREST, GL_NEAREST, GL_REPEAT);

		glTexImage2D(
			GL_TEXTURE_2D,
			0, // mip level = 0
			GL_RGBA8, // internal format
			pImage->GetWidth(), // width
			pImage->GetHeight(), // height
			0, // border = 0
			(GLenum)pImage->GetFormat(), // format
			(GLenum)pImage->GetComponentType(), //GL_UNSIGNED_BYTE,
			pImage->GetData()
		);
		OpenGL::ErrorTest();

		delete pImage;

		++smAvalaibleSlot;
	}

	Texture2D(const Image* image) :
		mSlot(smAvalaibleSlot)
	{
		if (!image)
		{
			throw std::exception("fucking image\n");
		}

		init();
		setParams(GL_NEAREST, GL_NEAREST, GL_REPEAT);

		++smAvalaibleSlot;
	}

	Texture2D(const Texture2D& other) = delete;

	Texture2D(Texture2D&& other)
	{
		swap(mTexture, other.mTexture);
		swap(mSlot, other.mSlot);
	}

	Texture2D& operator= (const Texture2D& other) = delete;

	Texture2D& operator= (Texture2D&& other)
	{
		if (this != &other)
		{
			swap(mTexture, other.mTexture);
			swap(mSlot, other.mSlot);

			other.mTexture = 0;
		}

		return *this;
	}

	~Texture2D()
	{
		if (mSlot + 1 == smAvalaibleSlot)
		{
			--smAvalaibleSlot;
		}

		glDeleteTextures(1, &mTexture);
	}


	GLuint getTexture() const
	{
		return mTexture;
	}

	unsigned int getSlot() const
	{
		return mSlot;
	}
};

unsigned int Texture2D::smAvalaibleSlot = 0;

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

	static void setWndSizes()
	{
		RECT rect;
		if (GetClientRect(smHwnd, &rect))
		{
			smWndWidth = rect.right - rect.left;
			smWndHeight = rect.bottom - rect.top;
		}
		else
		{
			std::cerr << "cannot get wnd sizes\n";
			throw std::exception("fuck, window sizes\n");
		}
	}

	void init()
	{
		//vbo
		{
			glGenBuffers(1, &mVertexBuffer);
			OpenGL::ErrorTest();

			glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
			OpenGL::ErrorTest();

			glBufferData(
				GL_ARRAY_BUFFER,
				sizeof(decltype(mVertexSource)::value_type) * mVertexSource.size(), // sizeof(float) * source.size()
				mVertexSource.data(), // float*
				GL_STATIC_DRAW // GL_STATIC_DRAW GL_DYNAMIC_DRAW GL_STREAM_DRAW
			);
			OpenGL::ErrorTest();
		}

		//ibo
		{
			glGenBuffers(1, &mIndexBuffer);
			OpenGL::ErrorTest();

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
			OpenGL::ErrorTest();

			glBufferData(
				GL_ELEMENT_ARRAY_BUFFER,
				sizeof(decltype(mIndexSource)::value_type) * mIndexSource.size(),
				mIndexSource.data(),
				GL_STATIC_DRAW
			);
			OpenGL::ErrorTest();
		}

		//vao
		{
			glGenVertexArrays(1, &mAttributeBuffer);
			OpenGL::ErrorTest();
			glBindVertexArray(mAttributeBuffer);
			OpenGL::ErrorTest();
		}

		//attribs
		auto attribute_vPos = glGetAttribLocation(mProgram, "vPos"); OpenGL::ErrorTest();
		if (attribute_vPos != -1)
		{
			glEnableVertexAttribArray(attribute_vPos);
			OpenGL::ErrorTest();
			glVertexAttribPointer(attribute_vPos, 2, GL_FLOAT, GL_FALSE, sizeof(decltype(mVertexSource)::value_type), 0);
			OpenGL::ErrorTest();
		}

		auto attribute_vColor = glGetAttribLocation(mProgram, "vColor"); OpenGL::ErrorTest();
		if (attribute_vColor != -1)
		{
			glVertexAttribPointer(attribute_vColor, 3, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(decltype(mVertexSource)::value_type), (void*)8);
			OpenGL::ErrorTest();
			glEnableVertexAttribArray(attribute_vColor);
			OpenGL::ErrorTest();
		}

		auto attribute_vTex = glGetAttribLocation(mProgram, "vTex"); OpenGL::ErrorTest();
		if (attribute_vTex != -1)
		{
			glVertexAttribPointer(attribute_vTex, 2, GL_FLOAT, GL_FALSE, sizeof(decltype(mVertexSource)::value_type), (void*)12);
			OpenGL::ErrorTest();
			glEnableVertexAttribArray(attribute_vTex);
			OpenGL::ErrorTest();
		}
	}

public:
	Sprite(float x, float y, float w, float h, float ang) :
		posX(x), posY(y),
		sizeX(w), sizeY(h),
		angle(ang)
	{
		init();
	}

	Sprite(float x, float y, float w, float h, float ang, unsigned int prior, Texture2D* textur) :
		Sprite(x,y,w,h,ang)
	{
		setPriority(prior);

		texture = textur;
	}

	Sprite(const Sprite& other) :
		Sprite(other.posX, other.posY, other.sizeX, other.sizeY, other.angle, other.mPriority, other.texture)
	{
	}

	Sprite(Sprite&& other)
	{
		*this = std::move(other);
	}

	~Sprite()
	{
		smObjects.erase(std::find(smObjects.begin(), smObjects.end(), this));

		glBindBuffer(GL_ARRAY_BUFFER, 0); 
		glDeleteBuffers(1, &mVertexBuffer);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glDeleteBuffers(1, &mIndexBuffer);

		glBindVertexArray(0); 
		glDeleteVertexArrays(1, &mAttributeBuffer);
	}

	Sprite& operator= (const Sprite& other)
	{
		if (this != &other)
		{
			this->~Sprite();

			posX = other.posX;
			posY = other.posY;
			sizeX = other.sizeX;
			sizeY = other.sizeY;
			texture = other.texture;

			init();
			setPriority(other.mPriority);
		}

		return *this;
	}

	Sprite& operator= (Sprite&& other)
	{
		if (this == &other)
		{
			return *this;
		}

		using std::swap;

		swap(posX, other.posX);
		swap(posY, other.posY);
		swap(sizeX, other.sizeX);
		swap(sizeY, other.sizeY);
		swap(texture, other.texture);
		swap(angle, other.angle);
		swap(mVertexBuffer, other.mVertexBuffer);
		swap(mIndexBuffer, other.mIndexBuffer);
		swap(mAttributeBuffer, other.mAttributeBuffer);

		other.mVertexBuffer = -1;
		other.mIndexBuffer = -1;
		other.mAttributeBuffer = -1;

		setPriority(other.mPriority);

		return *this;
	}

	void setPriority(unsigned int priority)
	{
		mPriority = priority;

		auto me = std::find(smObjects.begin(), smObjects.end(), this);
		if (me != smObjects.end())
		{
			smObjects.erase(me);
		}

		smObjects.insert(std::find_if(smObjects.begin(), smObjects.end(), [this](const Sprite* sprite)
		{
			return sprite->mPriority > mPriority;
		}), this);
	}

	static void init(const HWND hwnd, GLuint program, GLint texColor, GLint transformMat)
	{
		smHwnd = hwnd;

		mProgram = program;

		smUnifLocTexColor = texColor;
		smUnifLocTransfromMat = transformMat;
	}

	static void renderAll()
	{
		setWndSizes();

		for (const auto sprite : smObjects)
		{
			//use current texture
			glUniform1i(smUnifLocTexColor, sprite->texture->getSlot());
			OpenGL::ErrorTest();

			//bind current buffers
			glBindBuffer(GL_ARRAY_BUFFER, sprite->mVertexBuffer);
			OpenGL::ErrorTest();
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sprite->mIndexBuffer);
			OpenGL::ErrorTest();
			glBindVertexArray(sprite->mAttributeBuffer);
			OpenGL::ErrorTest();

			//rotate
			float angleToRadians = M_PI / 180.0f * sprite->angle;
			
			//distances from centre
			float dpx = sprite->posX - (smWndWidth / 2.0f); // DeltaX /from center/ in pixels
			float dpy = sprite->posY - (smWndHeight / 2.0f);
			float dpz = 0.0f;

			float dx = 2.0f * dpx / smWndWidth; // DeltaX /from center/ in OpenGL coords;
			float dy = 2.0f * dpy / smWndHeight;
			float dz = 0.0f;

			//scale coefs
			float kx = sprite->sizeX / smWndWidth;
			float ky = sprite->sizeY / smWndHeight;
			float kz = 1;

			std::vector<float> transformMatrix = { //Scale, Rotate and Translate => (TxR)xS
				cos(angleToRadians)*kx, -sin(angleToRadians)*ky, 0.0f, dx,
				sin(angleToRadians)*kx, cos(angleToRadians)*ky, 0.0f, dy,
				0.0f, 0.0f, 1.0f*kz, dz,
				0.0f, 0.0f, 0.0f, 1.0f
			};

			glUniformMatrix4fv(sprite->smUnifLocTransfromMat, 1, GL_FALSE, transformMatrix.data());
			OpenGL::ErrorTest();

			//call to draw
			glDrawElements(GL_TRIANGLES, sprite->mVertexDraw, GL_UNSIGNED_BYTE, nullptr);
			OpenGL::ErrorTest();
		}
	}
};

HWND Sprite::smHwnd = nullptr;
GLuint Sprite::mProgram = -1;
unsigned int Sprite::smWndHeight = 0;
unsigned int Sprite::smWndWidth = 0;
GLint Sprite::smUnifLocTexColor = -1;
GLint Sprite::smUnifLocTransfromMat = -1;
std::vector<const Sprite *> Sprite::smObjects = std::vector<const Sprite *>();

void func()
{
	srand(time(0));
	auto instance = Instance::Get();
	auto windowClass = new WindowClass(instance, "class");
	auto window = new Window(windowClass, "window");
	auto deviceContext = new DeviceContext(window);
	deviceContext->SetPixelFormat();
	auto renderContext = new RenderContextExtended(deviceContext);
	renderContext->Set();

	GLuint program = glCreateProgram();
	{
		auto CompileShader = [](GLuint shader)
		{
			glCompileShader(shader);

			GLint compileResult;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);

			if (compileResult != GL_TRUE)
			{
				GLint errorCodeLength;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &errorCodeLength);

				char *buffer = (char*)malloc(errorCodeLength + 1);
				int length;
				glGetShaderInfoLog(shader, errorCodeLength, &length, buffer);

				buffer[errorCodeLength] = '\0';

				auto errorText = "[glCompileShader] Error:\n" + string(buffer);

				free(buffer);

				throw OpenGL::Exception(errorText);
			}
		};
		auto LinkProgram = [](GLuint program)
		{
			glLinkProgram(program);

			GLint linkResult;
			glGetProgramiv(program, GL_LINK_STATUS, &linkResult);

			if (linkResult != GL_TRUE)
			{
				GLint errorCodeLength;

				glGetProgramiv(program, GL_INFO_LOG_LENGTH, &errorCodeLength);

				char *buffer = (char*)malloc(errorCodeLength + 1);
				int length = errorCodeLength;

				glGetProgramInfoLog(program, errorCodeLength, &length, buffer);

				buffer[errorCodeLength] = '\0';
				string code = string(buffer);

				free(buffer);

				throw OpenGL::Exception(code);
			}
		};

		OpenGL::ErrorTest();

		GLuint shaderVertex = glCreateShader(GL_VERTEX_SHADER);
		{
			OpenGL::ErrorTest();

			std::string source = loadFile(path("Media/Shaders/1.vs")); // "#version 330 core\nin vec2 vPos; void main(){gl_Position = vec4(vPos,0,1);}";
			auto data = source.data();
			GLint length = source.size();

			glShaderSource(shaderVertex, 1, &data, &length);
			OpenGL::ErrorTest();

			CompileShader(shaderVertex);
			OpenGL::ErrorTest();
		}
		GLuint shaderFragment = glCreateShader(GL_FRAGMENT_SHADER);
		{
			OpenGL::ErrorTest();

			std::string source = loadFile(path("Media/Shaders/1.fs")); // "#version 330 core\nout vec4 outColor; void main(){outColor = vec4(0,1,0,1);}";
			auto data = source.data();
			GLint length = source.size();

			glShaderSource(shaderFragment, 1, &data, &length);
			OpenGL::ErrorTest();

			CompileShader(shaderFragment);
			OpenGL::ErrorTest();
		}

		glAttachShader(program, shaderVertex);
		OpenGL::ErrorTest();

		glAttachShader(program, shaderFragment);
		OpenGL::ErrorTest();

		LinkProgram(program);
		OpenGL::ErrorTest();

		glDeleteShader(shaderVertex);
		glDeleteShader(shaderFragment);

		glUseProgram(program);
		OpenGL::ErrorTest();
	}

	auto transformLoc = glGetUniformLocation(program, "transformMatrix");
	OpenGL::ErrorTest();

	auto uniformTextureColor = glGetUniformLocation(program, "textureColor");
	OpenGL::ErrorTest();
	if (uniformTextureColor == -1)
	{
		std::cout << "fuck\n";
	}

	Sprite::init(window->GetHandle(), program, uniformTextureColor, transformLoc);

	Texture2D texture1(path("Media/Images/image.png"));

	Sprite sprite1(500, 400, 200, 200, 0, 0, &texture1);

	Texture2D texture2(path("Media/Images/image3.png"));

	Sprite sprite2(150, 200, 200, 200, 0, 1, &texture2);

	std::vector<Sprite> sprites(10, { 100, 100, 100, 100, 0, 0, &texture1 });
	for (auto& sprite : sprites)
	{
		sprite.posX = rand() % 700 + 50;
		sprite.posY = rand() % 500 + 50;
		sprite.sizeX = rand() % 100 + 100;
		sprite.sizeY = rand() % 100 + 100;
		sprite.angle = rand() % 360;
		sprite.setPriority(rand() % 15);
		sprite.texture = &(rand() % 2 ? texture1 : texture2);
	}

	float t = 0.0f;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	while(!GetAsyncKeyState(VK_ESCAPE))
	{
		glClearColor(0.16f, 0.16f, 0.16f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		t += 0.005f;
		
		sprite1.posX = rand() % 800;
		sprite1.posY = rand() % 600;

		sprite2.texture = &(rand() % 2 ? texture1 : texture2);
		sprite2.angle = t*100;
		sprite2.sizeX = fabs(sin(t) * 800);

		Sprite::renderAll();

		window->Loop();
		deviceContext->SwapBuffers();
	
		Sleep(10);
	}

	glUseProgram(0); glDeleteProgram(program);

	delete renderContext;
	delete deviceContext;
	delete window;
	delete windowClass;
	delete instance;
}

void main()
{
	try
	{
		func();
	}
	catch(CGGD::WinAPI::Exception exception)
	{
		cout << "WinAPI exception:\n" << exception.GetText() << endl;
	}
	catch(CGGD::OpenGL::Exception exception)
	{
		cout << "OpenGL exception:\n" << exception.GetText() << endl;
	}
	catch(CGGD::OpenIL::Exception exception)
	{
		cout << "OpenIL exception:\n" << exception.GetText() << endl;
	}
	catch (const std::exception& e)
	{
		cout << "Exception:\n" << e.what() << endl;
	}

	system("pause");
}