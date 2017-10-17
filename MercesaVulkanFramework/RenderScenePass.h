#pragma once

#include "RenderingIncludes.h"

class RenderScenePass
{
public:
	RenderScenePass();
	~RenderScenePass();


	vk::RenderPass renderPassPostProc;
};

