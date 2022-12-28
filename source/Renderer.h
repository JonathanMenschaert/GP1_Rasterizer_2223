#pragma once

#include <cstdint>
#include <vector>

#include "Camera.h"
#include "DataTypes.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Texture;
	struct Mesh;
	struct Vertex;
	class Timer;
	class Scene;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(Timer* pTimer);
		void Render();
		void CycleRenderMode();
		void ToggleRotation();
		void ToggleNormalMap();
		void CycleShadingMode();

		bool SaveBufferToImage() const;

	private:
		SDL_Window* m_pWindow{};

		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};

		Camera m_Camera{};
		int m_Width{};
		int m_Height{};
		float m_AspectRatio{};
		float* m_pDepthBufferPixels{};
		Vector3 m_LightDirection{ 0.577f, -0.577f, 0.577f };
		Texture* m_pDiffuseTexture{ nullptr };
		Texture* m_pNormalTexture{ nullptr };
		Texture* m_pSpecularTexture{ nullptr };
		Texture* m_pGlossinessTexture{ nullptr };
		std::vector<Mesh> m_Meshes{};

		const float m_RotationSpeed{ 1.f };
		bool m_ShouldRotate{ true };

		bool m_ShouldRenderNormals{ true };

		enum class RenderMode
		{
			FinalColor,
			DepthBuffer,

			//Declare modes above
			COUNT
		};

		enum class ShadingMode
		{
			Combined,
			ObservedArea,
			Diffuse,
			Specular,
			//Declare modes above
			COUNT
		};


		RenderMode m_RenderMode{ RenderMode::FinalColor };
		ShadingMode m_ShadingMode{ ShadingMode::Combined };

		//Function that transforms the vertices from the mesh from World space to Screen space
		void VertexTransformationFunction(const std::vector<Vertex>& vertices_in, std::vector<Vertex>& vertices_out) const; //W1 Version
		void VertexTransformationFunction(Mesh& mesh) const;
		void RenderMeshTriangle(const Mesh& mesh, const std::vector<Vector2>& screenVertices, uint32_t vertIdxbool, bool swapVertices = false);
		void Render_W1();
		//void Render_W2();
		void Render_W3();

		ColorRGB PixelShading(const Vertex_Out& v);
		//std::vector<Vector2> ClipPolygonToFrustrum()
	};
}
