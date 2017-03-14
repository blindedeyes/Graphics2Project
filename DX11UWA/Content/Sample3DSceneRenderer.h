#pragma once

#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\Common\StepTimer.h"

//Render object
#include "RenderObject.h"
#include "Light.h"

namespace DX11UWA
{
	// This sample renderer instantiates a basic rendering pipeline.
	class Sample3DSceneRenderer
	{
	public:
		Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void CreateDeviceDependentResources(void);
		void CreateWindowSizeDependentResources(void);
		void ReleaseDeviceDependentResources(void);
		void Update(DX::StepTimer const& timer);
		void Render(void);
		void StartTracking(void);
		void TrackingUpdate(float positionX);
		void StopTracking(void);
		inline bool IsTracking(void) { return m_tracking; }

		// Helper functions for keyboard and mouse input
		void SetKeyboardButtons(const char* list);
		void SetMousePosition(const Windows::UI::Input::PointerPoint^ pos);
		void SetInputDeviceData(const char* kb, const Windows::UI::Input::PointerPoint^ pos);
		void RenderToShadow();

	private:
		void Rotate(float radians);
		void UpdateCamera(DX::StepTimer const& timer, float const moveSpd, float const rotSpd);
		void CreatePlane();
		void LoadOBJFiles();
		void CreateLights();
		void UpdateLights(const DX::StepTimer& time);
	private:
		DirectX::XMMATRIX camProjMat;
		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// Direct3D resources for cube geometry.
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_inputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBuffer;



		//Shaders for RenderObjects
		CComPtr<ID3D11InputLayout> objinputLayout;
		CComPtr<ID3D11VertexShader>	objvertexShader;
		CComPtr<ID3D11PixelShader>	objpixelShader;
		
		CComPtr<ID3D11PixelShader>	objBMPixelShader;
		CComPtr<ID3D11GeometryShader> m_GeoShader;
		CComPtr<ID3D11VertexShader>	m_GeoVertexShader;
		CComPtr<ID3D11VertexShader>	m_ShadowShader;
		CComPtr<ID3D11PixelShader>	m_ShadowPShader;
		

		RenderObject shadowMapObj;
		
		//Light data
		bool lightsOn[4];
		CComPtr<ID3D11Buffer>	m_lightBuffer;
		CComPtr<ID3D11Texture2D> lightShadowMap;
		
		CComPtr<ID3D11DepthStencilView> DSVShadowMap;
		CComPtr<ID3D11ShaderResourceView> SRVShadowMap;
		CComPtr<ID3D11PixelShader>	objTrollPS;

		std::vector<Light> lights;
		DirectX::XMMATRIX lightProj;


		CComPtr<ID3D11InputLayout>  instanceInputLayout;
		CComPtr<ID3D11VertexShader>	instanceVertexShader;
		CComPtr<ID3D11PixelShader>	instancePixelShader;
		CComPtr<ID3D11Buffer> m_InstanceConstBuffer;
		InstancedModelViewProjectionConstantBuffer m_InstanceBufferData;

		//Vector of objects
		std::vector<RenderObject> renderObjects;
		std::vector<RenderObject> lightModels;
		std::vector<RenderObject> InstanceObjects;

		//elps time for light movement
		float elpsTime=0;

		// System resources for cube geometry.
		ModelViewProjectionConstantBuffer	m_constantBufferData;
		uint32	m_indexCount;

		// Variables used with the rendering loop.
		bool	m_loadingComplete=false;
		bool	objloadingComplete=false;
		float	m_degreesPerSecond;
		bool	m_tracking;
		// Data members for keyboard and mouse input
		char	m_kbuttons[256];
		char	m_Prevbuttons[256];
		bool OnButtonPress(char c);
		Windows::UI::Input::PointerPoint^ m_currMousePos;
		Windows::UI::Input::PointerPoint^ m_prevMousePos;

		// Matrix data member for the camera
		DirectX::XMFLOAT4X4 m_camera;
	};
}

