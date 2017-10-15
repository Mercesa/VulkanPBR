#pragma once

#include <string>

class iTexture
{
public:
	iTexture(std::string iFilePath) : filePath(iFilePath) {}
	virtual ~iTexture() {}

	std::string GetFilepath() { return filePath; }

protected:
	std::string filePath;
};