#include "Renderer.hpp"
#include "Window.hpp"

int main(int argc, char *args[])
{
	Window window = CreateNewWindow(640, 640);
	RenderLoop(window);
	DestroyWindow(window);

	return 0;
}