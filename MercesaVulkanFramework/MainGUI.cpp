#include "MainGUI.h"

#include "imgui.h"

#include <iostream>


MainGUI::MainGUI()
{
}


MainGUI::~MainGUI()
{
}


bool showProfiler = false;
static bool showLightMenu = false;

static void ShowExampleMenuFile()
{
	ImGui::MenuItem("(dummy menu)", NULL, false, false);
	if (ImGui::MenuItem("New")) {}
	if (ImGui::MenuItem("Open", "Ctrl+O")) {}
	if (ImGui::BeginMenu("Open Recent"))
	{
	
		ImGui::EndMenu();
	}
}

static void ShowUtilityMenuFile()
{
	if (ImGui::MenuItem("New"))
	{
		showLightMenu = !showLightMenu;
	}
}

void ShowLightMenu(std::vector<Light>& iLights)
{
	static float arr[3] = { 0.0f, 0.0f, 0.0f };

	if(ImGui::Begin("Light Menu", &showLightMenu))
	{

		for (auto& e : iLights)
		{
			if (ImGui::TreeNode("Lights"))
			{
			
					ImGui::ColorPicker3("##Picker", arr);
					e.diffuseColor.r = arr[0];
					e.diffuseColor.g = arr[1];
					e.diffuseColor.b = arr[2];
					
			}
			
			ImGui::TreePop();
		}
		ImGui::End();
	}
}

void MainGUI::Update(Game* const iGame)
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			ShowExampleMenuFile();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Utility"))
		{
			ShowUtilityMenuFile();
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}


	if (showLightMenu) {
		ShowLightMenu(iGame->lights);
	}
	
}