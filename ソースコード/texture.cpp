#include "main.h"
#include "manager.h"
#include "renderer.h"
#include "texture.h"

unordered_map < string, ID3D11ShaderResourceView*> Texture::m_TexturePool;

ID3D11ShaderResourceView* Texture::Load(const char* filename)
{

    if (m_TexturePool.count(filename) > 0)
    {
		return m_TexturePool[filename];
    }

	wchar_t wFileNmae[512];

	mbstowcs(wFileNmae, filename,strlen(filename)+1);

    TexMetadata metadata;
	ScratchImage image;
    ID3D11ShaderResourceView* texture;
	LoadFromWICFile(
		wFileNmae,
		WIC_FLAGS_NONE,
		&metadata,
		image
	);

	CreateShaderResourceView(Renderer::GetDevice(),image.GetImages(),
		image.GetImageCount(), metadata, &texture);

	assert(texture);

	m_TexturePool[filename] = texture;
	return texture;
}
