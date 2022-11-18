//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Texture.h"
#include "Utils.h"

//Debug includes
#include <iostream>

using namespace dae;

Renderer::Renderer(SDL_Window* pWindow) :
	m_pWindow(pWindow)
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

	//Create Buffers
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

	m_AspectRatio = static_cast<float>(m_Width) / m_Height;
	//m_pDepthBufferPixels = new float[m_Width * m_Height];

	//Initialize Camera
	m_Camera.Initialize(60.f, { .0f,.0f,-10.f });
}

Renderer::~Renderer()
{
	//delete[] m_pDepthBufferPixels;
}

void Renderer::Update(Timer* pTimer)
{
	m_Camera.Update(pTimer);
}

void Renderer::Render()
{
	//@START
	//Lock BackBuffer
	SDL_LockSurface(m_pBackBuffer);

	//RENDER LOGIC
	//Render_W1_Part1();
	//Render_W1_Part2();
	//Render_W1_Part3();
	Render_W1_Part4();
	//Render_W1_Part5();

	//@END
	//Update SDL Surface
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}

void Renderer::VertexTransformationFunction(const std::vector<Vertex>& vertices_in, std::vector<Vertex>& vertices_out) const
{
	vertices_out.reserve(vertices_in.size());
	for (const auto& vertexIn : vertices_in)
	{
		Vertex vertexOut{};
		vertexOut.position = m_Camera.invViewMatrix.TransformPoint(vertexIn.position);

		vertexOut.position.x = vertexOut.position.x / vertexOut.position.z / (m_Camera.fov * m_AspectRatio);
		vertexOut.position.y = vertexOut.position.y / vertexOut.position.z / (m_Camera.fov);
		vertices_out.emplace_back(vertexOut);
	}
}

void dae::Renderer::Render_W1_Part1()
{
	//Define Triangle - Vertices in NDC space
	std::vector<Vector3> vertices_ndc
	{
		{0.f, 0.5f, 1.f},
		{0.5f, -0.5f, 1.f},
		{-0.5f, -0.5f, 1.f}
	};

	std::vector<Vector2> screenSpaceVertices;
	screenSpaceVertices.reserve(vertices_ndc.size());
	for (const auto& vertexNdc : vertices_ndc)
	{
		screenSpaceVertices.emplace_back(
			Vector2{
				(vertexNdc.x + 1) * 0.5f * m_Width,
				(1 - vertexNdc.y) * 0.5f * m_Height
			}
		);
	}

	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			
			const Vector2 pixelCoordinates{ static_cast<float>(px), static_cast<float>(py) };
			

			ColorRGB finalColor{
				GeometryUtils::IsPointInTriangle(screenSpaceVertices[0], screenSpaceVertices[1], screenSpaceVertices[2], pixelCoordinates) ?
				colors::White : colors::Black			
			};

			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}
}

void dae::Renderer::Render_W1_Part2()
{
	//Define Triangle - Vertices in NDC space
	std::vector<Vertex> vertices_world
	{
		Vertex{Vector3{0.f, 2.f, 0.f} },
		Vertex{Vector3{1.f, 0.f, 0.f}},
		Vertex{Vector3{-1.f, 0.f, 0.f}}
	};


	std::vector<Vertex> vertices_ndc;
	VertexTransformationFunction(vertices_world, vertices_ndc);

	std::vector<Vector2> screenSpaceVertices;
	screenSpaceVertices.reserve(vertices_ndc.size());
	for (const auto& vertexNdc : vertices_ndc)
	{
		screenSpaceVertices.emplace_back(
			Vector2{
				(vertexNdc.position.x + 1) * 0.5f * m_Width,
				(1 - vertexNdc.position.y) * 0.5f * m_Height
			}
		);
	}

	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{

			const Vector2 pixelCoordinates{ static_cast<float>(px), static_cast<float>(py) };


			ColorRGB finalColor{
				GeometryUtils::IsPointInTriangle(screenSpaceVertices[0], screenSpaceVertices[1], screenSpaceVertices[2], pixelCoordinates) ?
				colors::White : colors::Black
			};

			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}
}

void dae::Renderer::Render_W1_Part3()
{
	//Define Triangle - Vertices in NDC space
	std::vector<Vertex> vertices_world
	{
		//Triangle 0
		Vertex{Vector3{0.f, 2.f, 0.f}, ColorRGB{1.f, 0.f, 0.f} },
		Vertex{Vector3{1.5f, -1.f, 0.f}, ColorRGB{0.f, 1.f, 0.f}},
		Vertex{Vector3{-1.5f, -1.f, 0.f}, ColorRGB{0.f, 0.f, 1.f}}
	};


	std::vector<Vertex> vertices_ndc;
	VertexTransformationFunction(vertices_world, vertices_ndc);

	std::vector<Vector2> screenSpaceVertices;
	screenSpaceVertices.reserve(vertices_ndc.size());
	for (const auto& vertexNdc : vertices_ndc)
	{
		screenSpaceVertices.emplace_back(
			Vector2{
				(vertexNdc.position.x + 1) * 0.5f * m_Width,
				(1 - vertexNdc.position.y) * 0.5f * m_Height
			}
		);
	}

	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{

			const Vector2 pixelCoordinates{ static_cast<float>(px), static_cast<float>(py) };
			float signedAreaV0V1, signedAreaV1V2, signedAreaV2V0;

			ColorRGB finalColor{ colors::Black };


			if (GeometryUtils::IsPointInTriangle(screenSpaceVertices[0], screenSpaceVertices[1], screenSpaceVertices[2],
				pixelCoordinates, signedAreaV0V1, signedAreaV1V2, signedAreaV2V0))
			{
				const float triangleArea{ 1.f / (Vector2::Cross(screenSpaceVertices[1] - screenSpaceVertices[0],
					screenSpaceVertices[2] - screenSpaceVertices[0])) };

				const float weightV0{ signedAreaV1V2 * triangleArea };
				const float weightV1{ signedAreaV2V0 * triangleArea };
				const float weightV2{ signedAreaV0V1 * triangleArea };

				finalColor =
				{
					vertices_world[0].color * weightV0 +
					vertices_world[1].color * weightV1 +
					vertices_world[2].color * weightV2
				};
			}


			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}
}

void dae::Renderer::Render_W1_Part4()
{
	SDL_FillRect(m_pBackBuffer, NULL, SDL_MapRGB(m_pBackBuffer->format, 100, 100, 100));
	//Define Triangle - Vertices in NDC space
	std::vector<Vertex> vertices_world
	{
		//Triangle 0
		Vertex{Vector3{0.f, 4.f, 2.f}, ColorRGB{1.f, 0.f, 0.f} },
		Vertex{Vector3{3.f, -2.f, 2.f}, ColorRGB{0.f, 1.f, 0.f}},
		Vertex{Vector3{-3.f, -2.f, 2.f}, ColorRGB{0.f, 0.f, 1.f}},

		//Triangle 1
		Vertex{Vector3{0.f, 4.f, 2.f}, ColorRGB{1.f, 0.f, 0.f} },
		Vertex{Vector3{3.f, -2.f, 2.f}, ColorRGB{0.f, 1.f, 0.f}},
		Vertex{Vector3{-3.f, -2.f, 2.f}, ColorRGB{0.f, 0.f, 1.f}}
	};


	std::vector<Vertex> vertices_ndc;
	VertexTransformationFunction(vertices_world, vertices_ndc);

	std::vector<Vector2> screenSpaceVertices;
	screenSpaceVertices.reserve(vertices_ndc.size());
	for (const auto& vertexNdc : vertices_ndc)
	{
		screenSpaceVertices.emplace_back(
			Vector2{
				(vertexNdc.position.x + 1) * 0.5f * m_Width,
				(1 - vertexNdc.position.y) * 0.5f * m_Height
			}
		);
	}

	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			const Vector2 pixelCoordinates{ static_cast<float>(px), static_cast<float>(py) };
			float signedAreaV0V1, signedAreaV1V2, signedAreaV2V0;
			ColorRGB finalColor{ colors::Black };

			for (size_t currentTriIdx{}; currentTriIdx < screenSpaceVertices.size(); currentTriIdx += 3)
			{
				if (GeometryUtils::IsPointInTriangle(screenSpaceVertices[currentTriIdx], screenSpaceVertices[currentTriIdx + 1], 
					screenSpaceVertices[currentTriIdx + 2],	pixelCoordinates, signedAreaV0V1, signedAreaV1V2, signedAreaV2V0))
				{
					const float triangleArea{ 1.f / (Vector2::Cross(screenSpaceVertices[currentTriIdx + 1] - screenSpaceVertices[currentTriIdx],
						screenSpaceVertices[currentTriIdx + 2] - screenSpaceVertices[currentTriIdx])) };

					const float weightV0{ signedAreaV1V2 * triangleArea };
					const float weightV1{ signedAreaV2V0 * triangleArea };
					const float weightV2{ signedAreaV0V1 * triangleArea };

					finalColor =
					{
						vertices_world[currentTriIdx].color * weightV0 +
						vertices_world[currentTriIdx + 1].color * weightV1 +
						vertices_world[currentTriIdx + 2].color * weightV2
					};
				}

			}
			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}
}

void dae::Renderer::Render_W1_Part5()
{
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}
