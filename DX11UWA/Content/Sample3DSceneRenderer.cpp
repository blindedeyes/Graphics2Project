﻿#include "pch.h"
#include "Sample3DSceneRenderer.h"

#include "..\Common\DirectXHelper.h"

using namespace DX11UWA;

using namespace DirectX;
using namespace Windows::Foundation;

// Loads vertex and pixel shaders from files and instantiates the cube geometry.
Sample3DSceneRenderer::Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_loadingComplete(false),
	m_degreesPerSecond(45),
	m_indexCount(0),
	m_tracking(false),
	m_deviceResources(deviceResources)
{
	memset(m_kbuttons, 0, sizeof(m_kbuttons));
	m_currMousePos = nullptr;
	m_prevMousePos = nullptr;
	memset(&m_camera, 0, sizeof(XMFLOAT4X4));

	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

// Initializes view parameters when the window size changes.
void Sample3DSceneRenderer::CreateWindowSizeDependentResources(void)
{
	Size outputSize = m_deviceResources->GetOutputSize();
	float aspectRatio = outputSize.Width / outputSize.Height;
	float fovAngleY = 70.0f * XM_PI / 180.0f;

	// This is a simple example of change that can be made when the app is in
	// portrait or snapped view.
	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}

	// Note that the OrientationTransform3D matrix is post-multiplied here
	// in order to correctly orient the scene to match the display orientation.
	// This post-multiplication step is required for any draw calls that are
	// made to the swap chain render target. For draw calls to other targets,
	// this transform should not be applied.

	// This sample makes use of a right-handed coordinate system using row-major matrices.
	XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, 0.01f, 100.0f);

	XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

	XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

	XMStoreFloat4x4(&m_constantBufferData.projection, XMMatrixTranspose(perspectiveMatrix * orientationMatrix));
	XMStoreFloat4x4(&m_InstanceBufferData.projection, XMMatrixTranspose(perspectiveMatrix * orientationMatrix));

	// Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.
	static const XMVECTORF32 eye = { 0.0f, 0.7f, -1.5f, 0.0f };
	static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMStoreFloat4x4(&m_camera, XMMatrixInverse(nullptr, XMMatrixLookAtLH(eye, at, up)));
	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtLH(eye, at, up)));
}

// Called once per frame, rotates the cube and calculates the model and view matrices.
void Sample3DSceneRenderer::Update(DX::StepTimer const& timer)
{
	if (!m_tracking)
	{
		// Convert degrees to radians, then convert seconds to rotation angle
		float radiansPerSecond = XMConvertToRadians(m_degreesPerSecond);
		double totalRotation = timer.GetTotalSeconds() * radiansPerSecond;
		float radians = static_cast<float>(fmod(totalRotation, XM_2PI));

		Rotate(radians);
	}


	// Update or move camera here
	UpdateCamera(timer, 1.0f, 0.75f);
	UpdateLights(timer);

}

// Rotate the 3D cube model a set amount of radians.
void Sample3DSceneRenderer::Rotate(float radians)
{
	// Prepare to pass the updated model matrix to the shader
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(radians)));
}

void Sample3DSceneRenderer::UpdateCamera(DX::StepTimer const& timer, float const moveSpd, float const rotSpd)
{
	const float delta_time = (float)timer.GetElapsedSeconds();

	if (m_kbuttons['W'])
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, 0.0f, moveSpd * delta_time);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons['S'])
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, 0.0f, -moveSpd * delta_time);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons['A'])
	{
		XMMATRIX translation = XMMatrixTranslation(-moveSpd * delta_time, 0.0f, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons['D'])
	{
		XMMATRIX translation = XMMatrixTranslation(moveSpd * delta_time, 0.0f, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons['X'])
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, -moveSpd * delta_time, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons[VK_SPACE])
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, moveSpd * delta_time, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}

	if (m_currMousePos)
	{
		if (m_currMousePos->Properties->IsRightButtonPressed && m_prevMousePos)
		{
			float dx = m_currMousePos->Position.X - m_prevMousePos->Position.X;
			float dy = m_currMousePos->Position.Y - m_prevMousePos->Position.Y;

			XMFLOAT4 pos = XMFLOAT4(m_camera._41, m_camera._42, m_camera._43, m_camera._44);

			m_camera._41 = 0;
			m_camera._42 = 0;
			m_camera._43 = 0;

			XMMATRIX rotX = XMMatrixRotationX(dy * rotSpd * delta_time);
			XMMATRIX rotY = XMMatrixRotationY(dx * rotSpd * delta_time);

			XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
			temp_camera = XMMatrixMultiply(rotX, temp_camera);
			temp_camera = XMMatrixMultiply(temp_camera, rotY);

			XMStoreFloat4x4(&m_camera, temp_camera);

			m_camera._41 = pos.x;
			m_camera._42 = pos.y;
			m_camera._43 = pos.z;
		}
		m_prevMousePos = m_currMousePos;
	}


}

void DX11UWA::Sample3DSceneRenderer::CreatePlane()
{
	using namespace DirectX;
	RenderObject obj;

	//ORDER MATTERS.
	obj.vertexs.push_back({ XMFLOAT3(-1.0f,0,1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) , XMFLOAT3(0.0f, 1.0f, 0.0f) });
	obj.vertexs.push_back({ XMFLOAT3(1.0f, 0,1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f) , XMFLOAT3(0.0f, 1.0f, 0.0f) });
	obj.vertexs.push_back({ XMFLOAT3(1.0f, 0,-1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) });
	obj.vertexs.push_back({ XMFLOAT3(-1.0f,0,-1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) });
	

	//IF ABOVE ORDER IS CORRECT, THIS IS CLOCKWISE PLANE.
	obj.indexes.push_back(0);
	obj.indexes.push_back(1);
	obj.indexes.push_back(2);

	obj.indexes.push_back(3);
	obj.indexes.push_back(0);
	obj.indexes.push_back(2);


	obj.CalcTangents();
	obj.SetupVertexBuffers(m_deviceResources.get());
	
	obj.SetupIndexBuffer(m_deviceResources.get());
	obj.LoadTexture(m_deviceResources.get(), "assets/172.dds");
	obj.LoadNormalMap(m_deviceResources.get(), "assets/172_norm.dds");

	//obj.Position._42 = -.5f;
	renderObjects.push_back(obj);

}

void DX11UWA::Sample3DSceneRenderer::LoadOBJFiles() {

	//objLayout
	// Load shaders asynchronously.
	auto loadVSTask = DX::ReadDataAsync(L"TempVertexShader.cso");
	auto loadInstancedVSTask = DX::ReadDataAsync(L"InstancedVertexShader.cso");
	auto loadPSTask = DX::ReadDataAsync(L"TempPixelShader.cso");
	auto loadBumpMap = DX::ReadDataAsync(L"BumpMapPixelShader.cso");

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createVSTask = loadVSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateVertexShader(&fileData[0], fileData.size(), nullptr, &objvertexShader.p));

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UV", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },

		};

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc), &fileData[0], fileData.size(), &objinputLayout));
	});

	auto createInstancedVSTask = loadInstancedVSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateVertexShader(&fileData[0], fileData.size(), nullptr, &instanceVertexShader.p));
		/*
				static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
				{
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
					{ "UV", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
					{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },

				};

				DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc), &fileData[0], fileData.size(), &objinputLayout));*/
	});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createPSTask = loadPSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&fileData[0], fileData.size(), nullptr, &objpixelShader.p));


		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(InstancedModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, &m_InstanceConstBuffer.p));
	});

	auto createBumpMapTask = loadBumpMap.then([this](const std::vector<byte>& filedata) {
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&filedata[0], filedata.size(), nullptr, &objBMPixelShader.p));
	});
	/*
		RenderObject obj;
		obj.LoadObjFile("assets/test pyramid.obj");
		DX::ThrowIfFailed(obj.SetupIndexBuffer(m_deviceResources.get()));
		DX::ThrowIfFailed(obj.SetupVertexBuffers(m_deviceResources.get()));
		renderObjects.push_back(obj);
		RenderObject obj2;

		obj2.LoadObjFile("assets/banana.obj");
		DX::ThrowIfFailed(obj2.SetupIndexBuffer(m_deviceResources.get()));
		DX::ThrowIfFailed(obj2.SetupVertexBuffers(m_deviceResources.get()));
		renderObjects.push_back(obj2);
	*/
	auto LoadObj = createPSTask.then([this](void)
	{
		std::ifstream strm;
		strm.open("assets/ModelList.txt");
		char base[] = "./assets/";

		if (strm.is_open()) {
			std::string str;
			//read
			while (std::getline(strm, str)) {

				//handle comments
				if (str.length() == 0 || str[0] == '#')
					continue;

				int substrPos = (int)str.find_last_of('|');

				RenderObject obj;
				std::string path = base;
				path += str.substr(0, substrPos).c_str();
				obj.LoadObjFile(path.c_str());

				//read the obj name
				//strm.getline(line, 256);
				path = base;
				path += str.substr(substrPos + 1).c_str();
				obj.LoadTexture(m_deviceResources.get(), path.c_str());

				//setup the buffers
				DX::ThrowIfFailed(obj.SetupIndexBuffer(m_deviceResources.get()));
				DX::ThrowIfFailed(obj.SetupVertexBuffers(m_deviceResources.get()));
				renderObjects.push_back(obj);
			}
			strm.close();
		}
	});

	auto CreateInstancedBanana = LoadObj.then([this](void) {
		RenderObject obj;
		obj.InstanceCnt = 5;
		obj.LoadObjFile("assets/banana.obj");
		obj.LoadTexture(m_deviceResources.get(), "assets/banana_128.dds");
		DX::ThrowIfFailed(obj.SetupIndexBuffer(m_deviceResources.get()));
		DX::ThrowIfFailed(obj.SetupVertexBuffers(m_deviceResources.get()));
		for (int i = 0; i < 5; i++) {

			XMStoreFloat4x4(&obj.transform[i].Rotation, XMMatrixRotationX(.5f*i));
			XMStoreFloat4x4(&obj.transform[i].Position, XMMatrixTranslation(1.5*i + 1, i * 1, 0));
			XMStoreFloat4x4(&obj.transform[i].Scale, XMMatrixScaling(1 + (i*.5f), 1 + (i*.5f), 1 + (i*.5f)));

		}
		InstanceObjects.push_back(obj);
	});


	//auto finish = 
	(createVSTask && createPSTask && LoadObj && createInstancedVSTask && createBumpMapTask).then([this]()
	{
		objloadingComplete = true;
	});
}

void DX11UWA::Sample3DSceneRenderer::CreateLights()
{
	//Directional lights
	lights.resize(16);
	ZeroMemory(lights.data(), sizeof(Light) * 16);
	//Light lght;
	lights[0].dir = DirectX::XMFLOAT4(1, -1, 0, 0);
	//World pos, 1 is directional light, on w, doesn't use world pos
	lights[0].pos = DirectX::XMFLOAT4(0, 0, 0, 1);
	lights[0].color = DirectX::XMFLOAT4(1, .5, 0, 0);
	lights[0].radius = DirectX::XMFLOAT4(0, 0, 0, 0);


	//point light
	//World pos, 2 is point light, on w, doesn't use world pos
	lights[1].dir = DirectX::XMFLOAT4(0, 0, 0, 0);
	lights[1].pos = DirectX::XMFLOAT4(0, -1, 0, 2);
	lights[1].color = DirectX::XMFLOAT4(0, .75, 0, 0);
	lights[1].radius = DirectX::XMFLOAT4(3, 0, 0, 0);


	//spot light
	//World pos, 3 is spot light, on w, doesn't use world pos
	lights[2].dir = DirectX::XMFLOAT4(0, -1, 0, 0);
	lights[2].pos = DirectX::XMFLOAT4(1, 0, 0, 3);
	lights[2].color = DirectX::XMFLOAT4(1, 1, 1, 0);
	lights[2].radius = DirectX::XMFLOAT4(3, .99f, .8f, 0);

	//ambient light, 4, ratio is held in radius x
	lights[3].dir = DirectX::XMFLOAT4(0, 0, 0, 0);
	lights[3].pos = DirectX::XMFLOAT4(0, 10, 0, 4);
	lights[3].color = DirectX::XMFLOAT4(1, 1, 1, 0);
	lights[3].radius = DirectX::XMFLOAT4(.30, 0, 0, 0);

	CD3D11_BUFFER_DESC constantBufferDesc(sizeof(Light) * lights.size(), D3D11_BIND_CONSTANT_BUFFER);
	DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, &m_lightBuffer));

	RenderObject obj;
	obj.LoadObjFile("./assets/Sphere.obj");
	obj.InstanceCnt = 2;
	//obj.transform[0].Position  lights[0].pos;
	//XMStoreFloat4x4(&obj.transform[0].Position, XMMatrixTranslation(lights[0].pos.x, lights[0].pos.y, lights[0].pos.z));
	//XMStoreFloat4x4(&obj.transform[0].Scale, XMMatrixScaling(.2f, .2f, .2f));
	XMStoreFloat4x4(&obj.transform[0].Position, XMMatrixTranslation(lights[1].pos.x, lights[1].pos.y, lights[1].pos.z));
	XMStoreFloat4x4(&obj.transform[0].Scale, XMMatrixScaling(.2f, .2f, .2f));

	XMStoreFloat4x4(&obj.transform[1].Position, XMMatrixTranslation(lights[2].pos.x, lights[2].pos.y, lights[2].pos.z));
	XMStoreFloat4x4(&obj.transform[1].Scale, XMMatrixScaling(.2f, .2f, .2f));
	//XMStoreFloat4x4(&obj.transform[0].Position, XMMatrixTranslation(lights[0].pos.x, lights[0].pos.y, lights[0].pos.z));
	obj.SetupIndexBuffer(m_deviceResources.get());
	obj.SetupVertexBuffers(m_deviceResources.get());
	lightModels.push_back(obj);
}

void DX11UWA::Sample3DSceneRenderer::UpdateLights(const DX::StepTimer &time)
{
	//Move lights
	//XMMATRIX pos = XMMATRIX(lights[1].pos,);
	elpsTime += time.GetElapsedSeconds();
	float angle = (((elpsTime*100)));
	//1 rotation per 2 seconds.
	angle = angle * XM_PI / 180.0f;
	lights[0].dir.x = cosf(angle);
	lights[0].dir.y = sinf(angle);

	lights[1].pos.x = cosf(angle);
	lights[1].pos.y = sinf(angle);

	angle = ((elpsTime));
	lights[2].pos.x = cosf(angle)+1;
	lights[2].pos.y = sinf(angle)+1;
	//angle = (elpsTime * 100);
	//lights[2].dir.z = sinf(angle);
	//lights[2].dir.y = cosf(angle);

	//hard codedededededededede Weee.
	XMStoreFloat4x4(&lightModels[0].transform[0].Position, XMMatrixTranslation(lights[1].pos.x, lights[1].pos.y, lights[1].pos.z));
	XMStoreFloat4x4(&lightModels[0].transform[0].Scale, XMMatrixScaling(.2f, .2f, .2f));

	XMStoreFloat4x4(&lightModels[0].transform[1].Position, XMMatrixTranslation(lights[2].pos.x, lights[2].pos.y, lights[2].pos.z));
	XMStoreFloat4x4(&lightModels[0].transform[1].Scale, XMMatrixScaling(.2f, .2f, .2f));
}

void Sample3DSceneRenderer::SetKeyboardButtons(const char* list)
{
	memcpy_s(m_kbuttons, sizeof(m_kbuttons), list, sizeof(m_kbuttons));
}

void Sample3DSceneRenderer::SetMousePosition(const Windows::UI::Input::PointerPoint^ pos)
{
	m_currMousePos = const_cast<Windows::UI::Input::PointerPoint^>(pos);
}

void Sample3DSceneRenderer::SetInputDeviceData(const char* kb, const Windows::UI::Input::PointerPoint^ pos)
{
	SetKeyboardButtons(kb);
	SetMousePosition(pos);
}

void DX11UWA::Sample3DSceneRenderer::StartTracking(void)
{
	m_tracking = true;
}

// When tracking, the 3D cube can be rotated around its Y axis by tracking pointer position relative to the output screen width.
void Sample3DSceneRenderer::TrackingUpdate(float positionX)
{
	if (m_tracking)
	{
		float radians = XM_2PI * 2.0f * positionX / m_deviceResources->GetOutputSize().Width;
		Rotate(radians);
	}

	for (int i = 0; i < renderObjects.size(); i++)
		if (renderObjects[i].UpdateObject)
			renderObjects[i].UpdateObject();
}

void Sample3DSceneRenderer::StopTracking(void)
{
	m_tracking = false;
}

// Renders one frame using the vertex and pixel shaders.
void Sample3DSceneRenderer::Render(void)
{
	// Loading is asynchronous. Only draw geometry after it's loaded.
	if (!m_loadingComplete || !objloadingComplete)
	{
		return;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera))));
	XMStoreFloat4x4(&m_InstanceBufferData.view, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera))));

	// Prepare the constant buffer to send it to the graphics device.
	//context->UpdateSubresource1(m_constantBuffer.Get(), 0, NULL, &m_constantBufferData, 0, 0, 0);
	//// Each vertex is one instance of the VertexPositionColor struct.
	//UINT stride = sizeof(VertexPositionColor);
	//UINT offset = 0;
	//context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	//// Each index is one 16-bit unsigned integer (short).
	//context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	//context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//context->IASetInputLayout(m_inputLayout.Get());
	//// Attach our vertex shader.
	//context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	//// Send the constant buffer to the graphics device.
	//context->VSSetConstantBuffers1(0, 1, m_constantBuffer.GetAddressOf(), nullptr, nullptr);
	//// Attach our pixel shader.
	//context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
	//context->PSSetShaderResources(0,1,)
	// Draw the objects.
	//context->DrawIndexed(m_indexCount, 0, 0);


	/*
	//Plane
	// Prepare the constant buffer to send it to the graphics device.
	context->UpdateSubresource1(m_constantBuffer.Get(), 0, NULL, &m_constantBufferData, 0, 0, 0);
	// Each vertex is one instance of the VertexPositionColor struct.
	stride = sizeof(VertexPositionColor);
	offset = 0;
	context->IASetVertexBuffers(0, 1, &renderObjects[0].vertexBuffer.p, &stride, &offset);
	// Each index is one 16-bit unsigned integer (short).
	context->IASetIndexBuffer(renderObjects[0].indexBuffer, DXGI_FORMAT_R16_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(m_inputLayout.Get());
	// Attach our vertex shader.
	context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(0, 1, m_constantBuffer.GetAddressOf(), nullptr, nullptr);
	// Attach our pixel shader.
	context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
	// Draw the objects.
	context->DrawIndexed(6, 0, 0);
	*/

	context->UpdateSubresource1(m_lightBuffer.p, 0, NULL, lights.data(), 0, 0, 0);

	UINT stride = sizeof(VertexPositionUVNormal);
	UINT offset = 0;

	//Draw my custom indexed objects
	XMMATRIX temp1, temp2;
	for (int i = 0; i < renderObjects.size(); i++) {
		temp1 = XMLoadFloat4x4(&renderObjects[i].transform->Scale);
		temp2 = XMLoadFloat4x4(&renderObjects[i].transform->Rotation);
		temp1 = XMMatrixMultiply(temp1, temp2);
		temp2 = XMLoadFloat4x4(&renderObjects[i].transform->Position);
		temp1 = XMMatrixMultiply(temp1, temp2);

		//update the constant buffer with specific objects rotation, and orientation
		XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(temp1));// *renderObjects[i].Position);

		// Prepare the constant buffer to send it to the graphics device.
		context->UpdateSubresource1(m_constantBuffer.Get(), 0, NULL, &m_constantBufferData, 0, 0, 0);
		// Each vertex is one instance of the VertexPositionColor struct.

		context->IASetVertexBuffers(0, 1, &(renderObjects[i].vertexBuffer.p), &stride, &offset);

		// Each index is one 16-bit unsigned integer (short).
		context->IASetIndexBuffer((renderObjects[i].indexBuffer.p), DXGI_FORMAT_R16_UINT, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->IASetInputLayout(objinputLayout.p);

		// Attach our vertex shader.
		context->VSSetShader(objvertexShader.p, nullptr, 0);

		// Send the constant buffer to the graphics device.
		context->VSSetConstantBuffers1(0, 1, m_constantBuffer.GetAddressOf(), nullptr, nullptr);

		if (renderObjects[i].constTextureBuffer == NULL)
		{
			context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

		}
		else if (renderObjects[i].constBumpMapBuffer == NULL) {
			// Attach our pixel shader.
			context->PSSetShader(objpixelShader.p, nullptr, 0);
			context->PSSetSamplers(0, 1, &renderObjects[i].sampState.p);
			context->PSSetShaderResources(0, 1, &renderObjects[i].constTextureBuffer.p);
			context->PSSetConstantBuffers(0, 1, &m_lightBuffer.p);
		}
		else {
			// Attach our pixel shader.
			context->PSSetShader(objBMPixelShader.p, nullptr, 0);
			context->PSSetSamplers(0, 1, &renderObjects[i].sampState.p);
			context->PSSetShaderResources(0, 1, &renderObjects[i].constTextureBuffer.p);
			context->PSSetShaderResources(1, 1, &renderObjects[i].constBumpMapBuffer.p);
			context->PSSetConstantBuffers(0, 1, &m_lightBuffer.p);
			//context->psset
		}

		// Draw the objects. Number of Tri's
		context->DrawIndexed(renderObjects[i].indexes.size(), 0, 0);
	}


	//Draw my Instanced Indexed Objects
	for (int i = 0; i < InstanceObjects.size(); ++i) {
		//update the constant buffer with specific objects rotation, and orientation
		for (int j = 0; j < InstanceObjects[i].InstanceCnt; ++j)
			XMStoreFloat4x4(&m_InstanceBufferData.model[j], XMMatrixTranspose(InstanceObjects[i].transform[j].MultTransform()));// *renderObjects[i].Position);


																			   // Prepare the constant buffer to send it to the graphics device.
		context->UpdateSubresource1(m_InstanceConstBuffer.p, 0, NULL, &m_InstanceBufferData, 0, 0, 0);
		// Each vertex is one instance of the VertexPositionColor struct.

		context->IASetVertexBuffers(0, 1, &(InstanceObjects[i].vertexBuffer.p), &stride, &offset);

		// Each index is one 16-bit unsigned integer (short).
		context->IASetIndexBuffer((InstanceObjects[i].indexBuffer.p), DXGI_FORMAT_R16_UINT, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->IASetInputLayout(objinputLayout.p);

		// Attach our vertex shader.
		context->VSSetShader(instanceVertexShader.p, nullptr, 0);

		// Send the constant buffer to the graphics device.
		context->VSSetConstantBuffers1(0, 1, &m_InstanceConstBuffer.p, nullptr, nullptr);

		if (InstanceObjects[i].constTextureBuffer == NULL)
		{
			context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

		}
		else {
			// Attach our pixel shader.
			context->PSSetShader(objpixelShader.p, nullptr, 0);
			context->PSSetSamplers(0, 1, &InstanceObjects[i].sampState.p);
			context->PSSetShaderResources(0, 1, &InstanceObjects[i].constTextureBuffer.p);
			context->PSSetConstantBuffers(0, 1, &m_lightBuffer.p);
			//context->psset
		}

		// Draw the objects. Number of Tri's
		context->DrawIndexedInstanced(InstanceObjects[i].indexes.size(), InstanceObjects[i].InstanceCnt, 0, 0, 0);
	}

	for (int i = 0; i < lightModels.size(); ++i) {
		//update the constant buffer with specific objects rotation, and orientation
		for (int j = 0; j < lightModels[i].InstanceCnt; ++j)
			XMStoreFloat4x4(&m_InstanceBufferData.model[j], XMMatrixTranspose(lightModels[i].transform[j].MultTransform()));// *renderObjects[i].Position);

		context->UpdateSubresource1(m_InstanceConstBuffer.p, 0, NULL, &m_InstanceBufferData, 0, 0, 0);
		// Each vertex is one instance of the VertexPositionColor struct.

		context->IASetVertexBuffers(0, 1, &(lightModels[i].vertexBuffer.p), &stride, &offset);

		// Each index is one 16-bit unsigned integer (short).
		context->IASetIndexBuffer((lightModels[i].indexBuffer.p), DXGI_FORMAT_R16_UINT, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->IASetInputLayout(objinputLayout.p);

		// Attach our vertex shader.
		context->VSSetShader(instanceVertexShader.p, nullptr, 0);

		// Send the constant buffer to the graphics device.
		context->VSSetConstantBuffers1(0, 1, &m_InstanceConstBuffer.p, nullptr, nullptr);

		//context->PSSetShader(NULL, nullptr, 0);
		context->PSSetShader(m_pixelShader.Get(), nullptr, 0);



		// Draw the objects. Number of Tri's
		context->DrawIndexedInstanced(lightModels[i].indexes.size(), lightModels[i].InstanceCnt, 0, 0, 0);
	}

}

void Sample3DSceneRenderer::CreateDeviceDependentResources(void)
{
	// Load shaders asynchronously.
	auto loadVSTask = DX::ReadDataAsync(L"SampleVertexShader.cso");
	auto loadPSTask = DX::ReadDataAsync(L"SamplePixelShader.cso");

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createVSTask = loadVSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateVertexShader(&fileData[0], fileData.size(), nullptr, &m_vertexShader));

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UV", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc), &fileData[0], fileData.size(), &m_inputLayout));
	});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createPSTask = loadPSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&fileData[0], fileData.size(), nullptr, &m_pixelShader));

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, &m_constantBuffer));
	});

	// Once both shaders are loaded, create the mesh.
	auto createCubeTask = (createPSTask && createVSTask).then([this]()
	{
		// Load mesh vertices. Each vertex has a position and a color.
		static const VertexPositionColor cubeVertices[] =
		{
			{XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f)},
			{XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f)},
			{XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f)},
			{XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(0.0f, 1.0f, 1.0f)},
			{XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f)},
			{XMFLOAT3(0.5f, -0.5f,  0.5f), XMFLOAT3(1.0f, 0.0f, 1.0f)},
			{XMFLOAT3(0.5f,  0.5f, -0.5f), XMFLOAT3(1.0f, 1.0f, 0.0f)},
			{XMFLOAT3(0.5f,  0.5f,  0.5f), XMFLOAT3(1.0f, 1.0f, 1.0f)},
		};

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = cubeVertices;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(cubeVertices), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_vertexBuffer));

		// Load mesh indices. Each trio of indices represents
		// a triangle to be rendered on the screen.
		// For example: 0,2,1 means that the vertices with indexes
		// 0, 2 and 1 from the vertex buffer compose the 
		// first triangle of this mesh.
		static const unsigned short cubeIndices[] =
		{
			0,1,2, // -x
			1,3,2,

			4,6,5, // +x
			5,6,7,

			0,5,1, // -y
			0,4,5,

			2,7,6, // +y
			2,3,7,

			0,6,4, // -z
			0,2,6,

			1,7,3, // +z
			1,5,7,
		};

		m_indexCount = ARRAYSIZE(cubeIndices);

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = cubeIndices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(cubeIndices), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_indexBuffer));

	});

	auto createMyStuff = createCubeTask.then([this](void) {
		CreatePlane();
		LoadOBJFiles();
		CreateLights();

	});

	// Once the cube is loaded, the object is ready to be rendered.
	createMyStuff.then([this]()
	{
		m_loadingComplete = true;
	});
}

void Sample3DSceneRenderer::ReleaseDeviceDependentResources(void)
{
	m_loadingComplete = false;
	m_vertexShader.Reset();
	m_inputLayout.Reset();
	m_pixelShader.Reset();
	m_constantBuffer.Reset();
	m_vertexBuffer.Reset();
	m_indexBuffer.Reset();
}