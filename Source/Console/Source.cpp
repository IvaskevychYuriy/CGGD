#pragma region Include

#include "HelperClasses.hpp"

#include <iostream>
#include <ctime>
#include <chrono>
using namespace std;

#include <glm/vec2.hpp>


#pragma endregion


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

	//std::chrono::steady_clock clock;
	auto start = std::chrono::steady_clock::now();
	auto end = start;

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

	Sprite sprite1(500, 400, 200, 200, 0, 1000000, &texture1);

	Texture2D texture2(path("Media/Images/image3.png"));

	Sprite sprite2(150, 200, 200, 200, 0, 1000001, &texture2);

	Sprite sprite3(400, 300, 200, 200, 20, 1000002, &texture2);

	std::vector<Sprite> sprites(20000, { 10, 10, 100, 100, 0, 0, &texture1 });
	for (auto& sprite : sprites)
	{
		sprite.posX = rand() % 800;
		sprite.posY = rand() % 800;
		sprite.sizeX = rand() % 10 + 5;
		sprite.sizeY = rand() % 10 + 5;
		sprite.angle = rand() % 360;
		sprite.setPriority(rand() % 15000);
		sprite.texture = &(rand() % 2 ? texture1 : texture2);
	}

	float t = 0.0f;

	Beep(200, 100);

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

		start = std::chrono::steady_clock::now();
		Sprite::renderAll();
		end = std::chrono::steady_clock::now();

		auto time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
		std::cout << "time per frame: " << time << " mks  ~" << 1000000.0f/time << " FPS\n";

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