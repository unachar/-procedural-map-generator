#pragma once

#include <unordered_map>
#include <string>

class Texture
{
private:
	static unordered_map<string, ID3D11ShaderResourceView*> m_TexturePool;
public:
	static ID3D11ShaderResourceView* Load(const char* filename);
};