#include "Renderer.hpp"
#include "Window.hpp"

void MainLoop(Window const &window)
{
	//Scene const scene = CreateSimpleScene();
	Scene const scene = CreateDefaultScene();
	//Scene const scene = CreateCornellBox();

	Camera camera = CreateDefaultCamera();
	InputEvents events = CreateDefaultInputEvents();
	uint32_t frameCount = 0;

	while (!events.quit)
	{
		UpdateInput(events);

		if (UpdateCamera(events, camera, window))
		{
			frameCount = 0;
		}

		Scene const sceneView = TransformScene2ViewSpace(scene, camera);

		RenderFrame(window, sceneView, frameCount++);
	}
}

int main(int argc, char *args[])
{
	Window const window = CreateWindow(320, 320);

	MainLoop(window);

	return 0;
}