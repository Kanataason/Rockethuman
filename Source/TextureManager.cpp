#include "Texture.h"
#include "TextureManager.h"

std::shared_ptr<Texture> TextureManager::CheckTexture(const std::wstring& path)
{
    //テクスチャがすでにロードされているか確認
	auto tex = _textures.find(path);

	if (tex != _textures.end())
	{
		return tex->second;
	}

	//テクスチャをロード
	auto texture = std::make_shared<Texture>();
	texture->Load(_device,_context,path);

	_textures[path] = texture;

	return texture;
}