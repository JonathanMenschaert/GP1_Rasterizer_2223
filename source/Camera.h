#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle):
			origin{_origin},
			fovAngle{_fovAngle}
		{
		}


		Vector3 origin{};
		float fovAngle{90.f};
		float fov{ tanf((fovAngle * TO_RADIANS) / 2.f) };

		Vector3 forward{Vector3::UnitZ};
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{};
		float totalYaw{};

		Matrix invViewMatrix{};
		Matrix viewMatrix{};

		float nearPlane{0.0001f};
		float farPlane{ 100.f };

		void Initialize(float _fovAngle = 90.f, Vector3 _origin = {0.f,0.f,0.f})
		{
			fovAngle = _fovAngle;
			fov = tanf((fovAngle * TO_RADIANS) / 2.f);

			origin = _origin;
		}

		void CalculateViewMatrix()
		{
			//TODO W1
			//ONB => invViewMatrix
			//Inverse(ONB) => ViewMatrix
			const Matrix finalRotation = Matrix::CreateRotation({ totalPitch, totalYaw, 0.f });
			forward = finalRotation.TransformVector(Vector3::UnitZ);
			right = Vector3::Cross(Vector3::UnitY, forward).Normalized();
			up = Vector3::Cross(forward, right).Normalized();
			invViewMatrix = { right, up, forward, origin };
			
			viewMatrix = invViewMatrix.Inverse();
			//ViewMatrix => Matrix::CreateLookAtLH(...) [not implemented yet]
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixlookatlh
		}

		void CalculateProjectionMatrix()
		{
			//TODO W2

			//ProjectionMatrix => Matrix::CreatePerspectiveFovLH(...) [not implemented yet]
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixperspectivefovlh
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Camera Update Logic
			const float linearSpeed{ 10.f };
			const float rotationSpeed{ 50.f };

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			const bool isShiftPressed{ pKeyboardState[SDL_SCANCODE_LSHIFT] || pKeyboardState[SDL_SCANCODE_RSHIFT] };
			const float shiftModifier{ 4.f * isShiftPressed + 1.f * !isShiftPressed };
			const float speedModifier{ deltaTime * linearSpeed * shiftModifier };

			const bool isForwardsPressed{ pKeyboardState[SDL_SCANCODE_W] || pKeyboardState[SDL_SCANCODE_UP] };
			const bool isBackwardsPressed{ pKeyboardState[SDL_SCANCODE_S] || pKeyboardState[SDL_SCANCODE_DOWN] };
			const bool isRightPressed{ pKeyboardState[SDL_SCANCODE_D] || pKeyboardState[SDL_SCANCODE_RIGHT] };
			const bool isLeftPressed{ pKeyboardState[SDL_SCANCODE_A] || pKeyboardState[SDL_SCANCODE_LEFT] };
			origin += forward * speedModifier * isForwardsPressed;
			origin += forward * -speedModifier * isBackwardsPressed;
			origin += right * speedModifier * isRightPressed;
			origin += right * -speedModifier * isLeftPressed;

			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);
			const float rotationModifier{ deltaTime * rotationSpeed * shiftModifier };

			//Calculate rotation & movement on mouse movement
			if (mouseY != 0.f || mouseX != 0.f)
			{
				//Invert mouse Y
				mouseY *= -1;

				origin += forward * speedModifier * (mouseState == SDL_BUTTON_LMASK) * static_cast<float>(mouseY);
				origin += Vector3::UnitY * speedModifier * (mouseState == (SDL_BUTTON_RMASK | SDL_BUTTON_LMASK)) * static_cast<float>(mouseY);
				totalPitch += static_cast<float>(mouseY) * TO_RADIANS * (mouseState == SDL_BUTTON_RMASK) * rotationModifier;
				totalYaw += static_cast<float>(mouseX) * TO_RADIANS *
					(mouseState & SDL_BUTTON_LMASK || mouseState & SDL_BUTTON_RMASK) * rotationModifier;
			}

			//Update Matrices
			CalculateViewMatrix();
			CalculateProjectionMatrix(); //Try to optimize this - should only be called once or when fov/aspectRatio changes
		}
	};
}
