#include "Texture.h"
#include "Vector2.h"
#include <SDL_image.h>

namespace dae
{
	Texture::Texture(SDL_Surface* pSurface) :
		m_pSurface{ pSurface },
		m_pSurfacePixels{ (uint32_t*)pSurface->pixels }
	{
	}

	Texture::~Texture()
	{
		if (m_pSurface)
		{
			SDL_FreeSurface(m_pSurface);
			m_pSurface = nullptr;
		}
	}

	Texture* Texture::LoadFromFile(const std::string& path)
	{
		//TODO
		//Load SDL_Surface using IMG_LOAD
		//Create & Return a new Texture Object (using SDL_Surface)
		SDL_Surface* pSurface = IMG_Load(path.c_str());
		Texture* loadedTexture{ new Texture(pSurface) };

		return loadedTexture;
	}

	ColorRGB Texture::DoSomthing(const Vector2& uv) const
	{
		Vector3 test{ 0.f, 0.f, 0.f };
		return ColorRGB{};
	}

	ColorRGB Texture::Sample(const Vector2& uv) const
	{
		uint32_t u{ static_cast<uint32_t>(uv.x * m_pSurface->w) };
		uint32_t v{ static_cast<uint32_t>(uv.y * m_pSurface->h) };
		//TODO
		//Sample the correct texel for the given uv

		uint8_t r{}, g{}, b{};
		uint32_t pixel{ m_pSurfacePixels[u + v * m_pSurface->w] };
		SDL_GetRGB(pixel, m_pSurface->format, &r, &g, &b);
		//SDL_GetRGB()
		
		return ColorRGB{
			static_cast<float>(r) * m_ColorModifier, 
			static_cast<float>(g)* m_ColorModifier,
			static_cast<float>(b) * m_ColorModifier
		};
	}
	

	Vector3 Texture::SampleNormal(const Vector2& uv) const
	{
		uint32_t u{ static_cast<uint32_t>(uv.x * m_pSurface->w) };
		uint32_t v{ static_cast<uint32_t>(uv.y * m_pSurface->h) };
		/*TODO
		Sample the correct texel for the given uv*/

		uint8_t r{}, g{}, b{};
		uint32_t pixel{ m_pSurfacePixels[u + v * m_pSurface->w] };
		SDL_GetRGB(pixel, m_pSurface->format, &r, &g, &b);


		return Vector3{
			static_cast<float>(r) * m_ColorModifier,
			static_cast<float>(g) * m_ColorModifier,
			static_cast<float>(b) * m_ColorModifier
		};
	}

	//Vector3 Texture::SampleNormal(const Vector2& uv) const
	//{
	//	ColorRGB sampledPixel{ Sample(uv) };

	//	Vector3 sampledNormal{ sampledPixel.r, sampledPixel.g, sampledPixel.b }; //This always initializes to the wrong value
	//	sampledNormal = 2.f * sampledNormal - Vector3{ 1.f, 1.f, 1.f };
	//	return sampledNormal;
	//}
}