#pragma once
class iObjectRenderingData
{
public:
	iObjectRenderingData() = default;
	virtual ~iObjectRenderingData() = default;


	bool shouldCastShadow = true;
	bool shouldRender = true;
};

