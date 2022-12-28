#pragma once
#include "Vector3.h"
#include <SDL_surface.h>
#include <string>
#include "ColorRGB.h"

namespace dae
{
	struct Vector2;
	
	class Texture final
	{
	public:
		~Texture();

		static Texture* LoadFromFile(const std::string& path);
		ColorRGB Sample(const Vector2& uv) const;
		ColorRGB DoSomthing(const Vector2& uv) const;
		Vector3 SampleNormal(const Vector2& uv) const;

	private:
		Texture(SDL_Surface* pSurface);

		SDL_Surface* m_pSurface{ nullptr };
		uint32_t* m_pSurfacePixels{ nullptr };
		const float m_ColorModifier{ 1.f / 255.f };
	};
}