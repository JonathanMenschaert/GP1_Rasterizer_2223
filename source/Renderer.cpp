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

Renderer::Renderer(SDL_Window* pWindow) 
	: m_pWindow(pWindow)
	, m_pTexture{Texture::LoadFromFile("Resources/tuktuk.png") }
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

	//Create Buffers
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

	m_AspectRatio = static_cast<float>(m_Width) / m_Height;

	const int nrPixels{ m_Width * m_Height };
	m_pDepthBufferPixels = new float[nrPixels];
	std::fill_n(m_pDepthBufferPixels, nrPixels, FLT_MAX);

	//Initialize Camera
	m_Camera.Initialize(60.f, { .0f,5.f,-30.f }, m_AspectRatio);
	Mesh mesh{ {},{}, PrimitiveTopology::TriangleList};
	Utils::ParseOBJ("Resources/tuktuk.obj", mesh.vertices, mesh.indices);
	m_Meshes.push_back(mesh);
}

Renderer::~Renderer()
{
	delete[] m_pDepthBufferPixels;
	m_pDepthBufferPixels = nullptr;
	delete m_pTexture;
	m_pTexture = nullptr;
}

void Renderer::Update(Timer* pTimer)
{
	m_Camera.Update(pTimer);
	for (auto& mesh : m_Meshes)
	{
		mesh.worldMatrix *= Matrix::CreateRotationY(m_RotationSpeed * pTimer->GetElapsed());
	}
}

void Renderer::Render()
{
	//@START
	//Lock BackBuffer
	SDL_FillRect(m_pBackBuffer, NULL, SDL_MapRGB(m_pBackBuffer->format, 100, 100, 100));
	const int nrPixels{ m_Width * m_Height };
	std::fill_n(m_pDepthBufferPixels, nrPixels, 1.f);
	SDL_LockSurface(m_pBackBuffer);

	//RENDER LOGIC
	//Render_W1();
	//Render_W2();
	Render_W3();
	//@END
	//Update SDL Surface
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}

void dae::Renderer::SwitchRenderMode()
{
	int count{ static_cast<int>(RenderMode::COUNT) };
	int currentMode{ static_cast<int>(m_RenderMode) };
	m_RenderMode = static_cast<RenderMode>((currentMode + 1) % count);
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

void Renderer::VertexTransformationFunction(Mesh& mesh) const
{
	mesh.vertices_out.clear();
	mesh.vertices_out.reserve(mesh.vertices.size());
	const Matrix worldViewProjectionMatrix{ mesh.worldMatrix * m_Camera.viewMatrix * m_Camera.projectionMatrix };
	for (const auto& vertexIn : mesh.vertices)
	{
		Vertex_Out vertexOut{ Vector4{ vertexIn.position, 1.f}, vertexIn.color, vertexIn.uv, vertexIn.normal, vertexIn.tangent };
		vertexOut.position = worldViewProjectionMatrix.TransformPoint(vertexOut.position);
		const float perspectiveDiv{ 1.f / vertexOut.position.w };
		vertexOut.position.x *= perspectiveDiv;
		vertexOut.position.y *= perspectiveDiv;
		vertexOut.position.z *= perspectiveDiv;
		/*vertexOut.position.x = vertexOut.position.x / vertexOut.position.z / (m_Camera.fov * m_AspectRatio);
		vertexOut.position.y = vertexOut.position.y / vertexOut.position.z / (m_Camera.fov);*/
		mesh.vertices_out.emplace_back(vertexOut);
	}
}

void dae::Renderer::Render_W1()
{
	//Define Triangle - Vertices in NDC space
	std::vector<Vertex> vertices_world
	{
		//Triangle 0
		Vertex{Vector3{0.f, 2.f, 0.f}, ColorRGB{1.f, 0.f, 0.f} },
		Vertex{Vector3{1.5f, -1.f, 0.f}, ColorRGB{1.f, 0.f, 0.f}},
		Vertex{Vector3{-1.5f, -1.f, 0.f}, ColorRGB{1.f, 0.f, 0.f}},

		//Triangle 1
		Vertex{Vector3{0.f, 4.f, 2.f}, ColorRGB{1.f, 0.f, 0.f} },
		Vertex{Vector3{3.f, -2.f, 2.f}, ColorRGB{0.f, 1.f, 0.f}},
		Vertex{Vector3{-3.f, -2.f, 2.f}, ColorRGB{0.f, 0.f, 1.f}}

	};


	std::vector<Vertex> vertices_ndc;
	VertexTransformationFunction(vertices_world, vertices_ndc);

	std::vector<Vector2> screenVertices;
	screenVertices.reserve(vertices_ndc.size());
	for (const auto& vertexNdc : vertices_ndc)
	{
		screenVertices.emplace_back(
			Vector2{
				(vertexNdc.position.x + 1) * 0.5f * m_Width,
				(1 - vertexNdc.position.y) * 0.5f * m_Height
			}
		);
	}
	const Vector2 screenVector{ static_cast<float>(m_Width), static_cast<float>(m_Height) };
	for (size_t triIdx{}; triIdx < screenVertices.size(); triIdx += 3)
	{
		//Add in the width and height in the min max too and set those as index boundaries
		Vector2 boundingBoxMin{ Vector2::Min(screenVertices[triIdx], Vector2::Min(screenVertices[triIdx + 1], screenVertices[triIdx + 2])) };
		Vector2 boundingBoxMax{ Vector2::Max(screenVertices[triIdx], Vector2::Max(screenVertices[triIdx + 1], screenVertices[triIdx + 2])) };

		boundingBoxMin = Vector2::Max(Vector2::Zero, Vector2::Min(boundingBoxMin, screenVector));
		boundingBoxMax = Vector2::Max(Vector2::Zero, Vector2::Min(boundingBoxMax, screenVector));
		for (int px{ static_cast<int>(boundingBoxMin.x) }; px < boundingBoxMax.x; ++px)
		{
			for (int py{ static_cast<int>(boundingBoxMin.y) }; py < boundingBoxMax.y; ++py)
			{
				const int pixelIdx{ px + py * m_Width };
				const Vector2 pixelCoordinates{ static_cast<float>(px), static_cast<float>(py) };
				float signedAreaV0V1, signedAreaV1V2, signedAreaV2V0;

				if (GeometryUtils::IsPointInTriangle(screenVertices[triIdx], screenVertices[triIdx + 1],
					screenVertices[triIdx + 2], pixelCoordinates, signedAreaV0V1, signedAreaV1V2, signedAreaV2V0))
				{
					const float triangleArea{ 1.f / (Vector2::Cross(screenVertices[triIdx + 1] - screenVertices[triIdx],
						screenVertices[triIdx + 2] - screenVertices[triIdx])) };

					const float weightV0{ signedAreaV1V2 * triangleArea };
					const float weightV1{ signedAreaV2V0 * triangleArea };
					const float weightV2{ signedAreaV0V1 * triangleArea };

					const float depthWeight
					{
						(vertices_world[triIdx].position.z - m_Camera.origin.z) * weightV0 +
						(vertices_world[triIdx + 1].position.z - m_Camera.origin.z) * weightV1 +
						(vertices_world[triIdx + 2].position.z - m_Camera.origin.z) * weightV2
					};

					if (m_pDepthBufferPixels[pixelIdx] < depthWeight) continue;
					m_pDepthBufferPixels[pixelIdx] = depthWeight;
					ColorRGB finalColor =
					{
						vertices_world[triIdx].color * weightV0 +
						vertices_world[triIdx + 1].color * weightV1 +
						vertices_world[triIdx + 2].color * weightV2
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
	}
}

//void dae::Renderer::Render_W2()
//{
//	std::vector<Mesh> meshesWorld
//	{
//		Mesh
//		{
//			{
//				Vertex{Vector3{-3.f, 3.f, -2.f}, colors::White, Vector2{0.f, 0.f}},
//				Vertex{Vector3{0.f, 3.f, -2.f}, colors::White, Vector2{0.5f, 0.f}},
//				Vertex{Vector3{3.f, 3.f, -2.f}, colors::White, Vector2{1.f, 0.f}},
//				Vertex{Vector3{-3.f, 0.f, -2.f}, colors::White, Vector2{0.f, 0.5f}},
//				Vertex{Vector3{0.f, 0.f, -2.f}, colors::White, Vector2{0.5f, 0.5f}},
//				Vertex{Vector3{3.f, 0.f, -2.f}, colors::White, Vector2{1.f, 0.5f}},
//				Vertex{Vector3{-3.f, -3.f, -2.f}, colors::White, Vector2{0.f, 1.f}},
//				Vertex{Vector3{0.f, -3.f, -2.f}, colors::White, Vector2{0.5f, 1.f}},
//				Vertex{Vector3{3.f, -3.f, -2.f}, colors::White, Vector2{1.f, 1.f}}
//			},
//			{
//				3, 0, 4, 1, 5, 2,
//				2, 6,
//				6, 3, 7, 4, 8, 5
//
//				//TriangleList Indices
//				/*3, 0, 1,
//				1, 4, 3,
//				4, 1, 2, 
//				2, 5, 4,
//				6, 3, 4, 
//				4, 7, 6,
//				7, 4, 5, 
//				5, 8, 7*/
//
//			},
//			PrimitiveTopology::TriangleStrip
//		}	
//
//	};
//	
//	for (const auto& mesh : meshesWorld)
//	{
//		//Check this later
//		std::vector<Vertex> vertices_ndc;
//		VertexTransformationFunction(mesh.vertices, vertices_ndc);
//		std::vector<Vector2> screenVertices;
//		screenVertices.reserve(vertices_ndc.size());
//		for (const auto& vertexNdc : vertices_ndc)
//		{
//			screenVertices.emplace_back(
//				Vector2{
//					(vertexNdc.position.x + 1) * 0.5f * m_Width,
//					(1 - vertexNdc.position.y) * 0.5f * m_Height
//				}
//			);
//		}
//		
//		switch (mesh.primitiveTopology)
//		{
//		case PrimitiveTopology::TriangleStrip:
//			for (size_t vertIdx{}; vertIdx < mesh.indices.size() - 2; ++vertIdx)
//			{
//				RenderMeshTriangle(mesh, screenVertices, vertices_ndc, vertIdx, vertIdx % 2);
//			}
//			break;
//		case PrimitiveTopology::TriangleList:
//			for (size_t vertIdx{}; vertIdx < mesh.indices.size() - 2; vertIdx += 3)
//			{
//				RenderMeshTriangle(mesh, screenVertices, vertices_ndc, vertIdx);
//			}
//			break;
//		}		
//	}
//}

void dae::Renderer::Render_W3()
{
	std::vector<Mesh> meshesWorld
			{
				Mesh
				{
					{
						Vertex{Vector3{-3.f, 3.f, -2.f}, colors::White, Vector2{0.f, 0.f}},
						Vertex{Vector3{0.f, 3.f, -2.f}, colors::White, Vector2{0.5f, 0.f}},
						Vertex{Vector3{3.f, 3.f, -2.f}, colors::White, Vector2{1.f, 0.f}},
						Vertex{Vector3{-3.f, 0.f, -2.f}, colors::White, Vector2{0.f, 0.5f}},
						Vertex{Vector3{0.f, 0.f, -2.f}, colors::White, Vector2{0.5f, 0.5f}},
						Vertex{Vector3{3.f, 0.f, -2.f}, colors::White, Vector2{1.f, 0.5f}},
						Vertex{Vector3{-3.f, -3.f, -2.f}, colors::White, Vector2{0.f, 1.f}},
						Vertex{Vector3{0.f, -3.f, -2.f}, colors::White, Vector2{0.5f, 1.f}},
						Vertex{Vector3{3.f, -3.f, -2.f}, colors::White, Vector2{1.f, 1.f}}
					},
					{
						3, 0, 4, 1, 5, 2,
						2, 6,
						6, 3, 7, 4, 8, 5
		
						//TriangleList Indices
						/*3, 0, 1,
						1, 4, 3,
						4, 1, 2, 
						2, 5, 4,
						6, 3, 4, 
						4, 7, 6,
						7, 4, 5, 
						5, 8, 7*/
		
					},
					PrimitiveTopology::TriangleStrip
				}	
		
			};
	for (auto& mesh : m_Meshes)
	{
		//Check this later
		VertexTransformationFunction(mesh);
		std::vector<Vector2> screenVertices;
		screenVertices.reserve(mesh.vertices_out.size());
		for (const auto& vertexNdc : mesh.vertices_out)
		{
			screenVertices.emplace_back(
				Vector2{
					(vertexNdc.position.x + 1) * 0.5f * m_Width,
					(1 - vertexNdc.position.y) * 0.5f * m_Height
				}
			);
		}

		switch (mesh.primitiveTopology)
		{
		case PrimitiveTopology::TriangleStrip:
			for (size_t vertIdx{}; vertIdx < mesh.indices.size() - 2; ++vertIdx)
			{
				RenderMeshTriangle(mesh, screenVertices, vertIdx, vertIdx % 2);
			}
			break;
		case PrimitiveTopology::TriangleList:
			for (size_t vertIdx{}; vertIdx < mesh.indices.size() - 2; vertIdx += 3)
			{
				RenderMeshTriangle(mesh, screenVertices, vertIdx);
			}
			break;
		}
	}
}

void dae::Renderer::RenderMeshTriangle(const Mesh& mesh, const std::vector<Vector2>& screenVertices, size_t vertIdx, bool swapVertices)
{
	const uint32_t vertIdx0{ mesh.indices[vertIdx + swapVertices * 2] };
	const uint32_t vertIdx1{ mesh.indices[vertIdx + 1] };
	const uint32_t vertIdx2{ mesh.indices[vertIdx + !swapVertices * 2] };

	if (vertIdx0 == vertIdx1 || vertIdx1 == vertIdx2 || vertIdx2 == vertIdx0
		|| !GeometryUtils::IsVertexInFrustrum(mesh.vertices_out[vertIdx0].position)
		|| !GeometryUtils::IsVertexInFrustrum(mesh.vertices_out[vertIdx1].position)
		|| !GeometryUtils::IsVertexInFrustrum(mesh.vertices_out[vertIdx2].position)) return;

	Vector2 boundingBoxMin{ Vector2::Min(screenVertices[vertIdx0], Vector2::Min(screenVertices[vertIdx1], screenVertices[vertIdx2])) };
	Vector2 boundingBoxMax{ Vector2::Max(screenVertices[vertIdx0], Vector2::Max(screenVertices[vertIdx1], screenVertices[vertIdx2])) };

	const Vector2 screenVector{ static_cast<float>(m_Width), static_cast<float>(m_Height) };
	boundingBoxMin = Vector2::Max(Vector2::Zero, Vector2::Min(boundingBoxMin, screenVector));
	boundingBoxMax = Vector2::Max(Vector2::Zero, Vector2::Min(boundingBoxMax, screenVector));
	for (int px{ static_cast<int>(boundingBoxMin.x) }; px < boundingBoxMax.x; ++px)
	{
		for (int py{ static_cast<int>(boundingBoxMin.y) }; py < boundingBoxMax.y; ++py)
		{
			const int pixelIdx{ px + py * m_Width };
			const Vector2 pixelCoordinates{ static_cast<float>(px), static_cast<float>(py) };
			float signedAreaV0V1, signedAreaV1V2, signedAreaV2V0;

			if (GeometryUtils::IsPointInTriangle(screenVertices[vertIdx0], screenVertices[vertIdx1],
				screenVertices[vertIdx2], pixelCoordinates, signedAreaV0V1, signedAreaV1V2, signedAreaV2V0))
			{
				const float triangleArea{ 1.f / (Vector2::Cross(screenVertices[vertIdx1] - screenVertices[vertIdx0],
					screenVertices[vertIdx2] - screenVertices[vertIdx0])) };

				const float weightV0{ signedAreaV1V2 * triangleArea };
				const float weightV1{ signedAreaV2V0 * triangleArea };
				const float weightV2{ signedAreaV0V1 * triangleArea };

				const float depthInterpolated
				{
					1.f / (1.f / mesh.vertices_out[vertIdx0].position.z * weightV0 +
					1.f / mesh.vertices_out[vertIdx1].position.z * weightV1 +
					1.f / mesh.vertices_out[vertIdx2].position.z * weightV2)
				};

				if (m_pDepthBufferPixels[pixelIdx] <= depthInterpolated || depthInterpolated < 0.f || depthInterpolated > 1.f) continue;
				m_pDepthBufferPixels[pixelIdx] = depthInterpolated;

				ColorRGB finalColor{};
				switch (m_RenderMode)
				{
				default:
				case RenderMode::FinalColor:
				{
					const float viewDepthInterpolated
					{
						1.f / (1.f / mesh.vertices_out[vertIdx0].position.w * weightV0 +
						1.f / mesh.vertices_out[vertIdx1].position.w * weightV1 +
						1.f / mesh.vertices_out[vertIdx2].position.w * weightV2)
					};

					Vector2 pixelUV
					{
						(mesh.vertices_out[vertIdx0].uv / mesh.vertices_out[vertIdx0].position.w * weightV0 +
						mesh.vertices_out[vertIdx1].uv / mesh.vertices_out[vertIdx1].position.w * weightV1 +
						mesh.vertices_out[vertIdx2].uv / mesh.vertices_out[vertIdx2].position.w * weightV2) * viewDepthInterpolated
					};
					
					finalColor = m_pTexture->Sample(pixelUV);
				}
				break;
				case RenderMode::DepthBuffer:
				{
					const float depthRemapped{ DepthRemap(depthInterpolated, 0.995f, 1.f) };
					finalColor = ColorRGB{ depthRemapped, depthRemapped, depthRemapped };
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
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}
