#include "HelperClasses.hpp"

std::string path(const std::string & x)
{
	return "../../../../" + x;
}

std::string loadFile(const std::string & filename)
{
	std::ifstream file(filename);

	std::string source;

	std::getline(file, source, '\0');

	file.close();

	return source;
}

void Texture2D::init()
{
	glGenTextures(1, &mTexture); OpenGL::ErrorTest();
	glActiveTexture(GL_TEXTURE0 + (int)mSlot); OpenGL::ErrorTest();
	glBindTexture(GL_TEXTURE_2D, mTexture); OpenGL::ErrorTest();
}

void Texture2D::setParams(GLint param1, GLint param2, GLint param3)
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, param1); OpenGL::ErrorTest();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, param2); OpenGL::ErrorTest();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, param3); OpenGL::ErrorTest();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, param3); OpenGL::ErrorTest();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, param3); OpenGL::ErrorTest();
}

Texture2D::Texture2D(const std::string & fullpath) :
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

Texture2D::Texture2D(const Image * image) :
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

Texture2D::Texture2D(Texture2D && other)
{
	swap(mTexture, other.mTexture);
	swap(mSlot, other.mSlot);
}

Texture2D::~Texture2D()
{
	if (mSlot + 1 == smAvalaibleSlot)
	{
		--smAvalaibleSlot;
	}

	glDeleteTextures(1, &mTexture);
}

Texture2D& Texture2D::operator= (Texture2D&& other)
{
	if (this != &other)
	{
		swap(mTexture, other.mTexture);
		swap(mSlot, other.mSlot);

		other.mTexture = 0;
	}

	return *this;
}

GLuint Texture2D::getTexture() const
{
	return mTexture;
}

unsigned int Texture2D::getSlot() const
{
	return mSlot;
}

unsigned int Texture2D::smAvalaibleSlot = 0;

void Sprite::setWndSizes()
{
	RECT rect;
	if (GetClientRect(smHwnd, &rect))
	{
		smWndWidth = rect.right - rect.left;
		smWndHeight = rect.bottom - rect.top;
	}
	else
	{
		throw std::exception("fuck, window sizes\n");
	}
}

void Sprite::init()
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

Sprite::Sprite(float x, float y, float w, float h, float ang) :
	posX(x), posY(y),
	sizeX(w), sizeY(h),
	angle(ang)
{
	init();
}

Sprite::Sprite(float x, float y, float w, float h, float ang, unsigned int prior, Texture2D * textur) :
	Sprite(x, y, w, h, ang)
{
	setPriority(prior);

	texture = textur;
}

Sprite::Sprite(const Sprite & other) :
	Sprite(other.posX, other.posY, other.sizeX, other.sizeY, other.angle, other.mPriority, other.texture)
{
}

Sprite::Sprite(Sprite && other)
{
	*this = std::move(other);
}

Sprite::~Sprite()
{
	smObjects.erase(std::find(smObjects.begin(), smObjects.end(), this));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &mVertexBuffer);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &mIndexBuffer);

	glBindVertexArray(0);
	glDeleteVertexArrays(1, &mAttributeBuffer);
}

Sprite& Sprite::operator= (const Sprite& other)
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

Sprite& Sprite::operator= (Sprite&& other)
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

void Sprite::setPriority(unsigned int priority)
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

void Sprite::init(const HWND hwnd, GLuint program, GLint texColor, GLint transformMat)
{
	smHwnd = hwnd;

	mProgram = program;

	smUnifLocTexColor = texColor;
	smUnifLocTransfromMat = transformMat;
}

void Sprite::renderAll()
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
		float angleToRadians = M_PI / 180.0f * (360.0f - sprite->angle);

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

		std::vector<float> transformMatrix = { //Rotate, Scale and Translate => Tx(SxR)
			cos(angleToRadians)*kx, -sin(angleToRadians)*kx, 0.0f, dx,
			sin(angleToRadians)*ky, cos(angleToRadians)*ky, 0.0f, dy,
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

HWND Sprite::smHwnd = nullptr;
GLuint Sprite::mProgram = -1;
unsigned int Sprite::smWndHeight = 0;
unsigned int Sprite::smWndWidth = 0;
GLint Sprite::smUnifLocTexColor = -1;
GLint Sprite::smUnifLocTransfromMat = -1;
std::vector<const Sprite *> Sprite::smObjects = std::vector<const Sprite *>();