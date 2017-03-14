#include "pch.h"
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
	/*
	// This is a simple example of change that can be made when the app is in
	// portrait or snapped view.
	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}
	*/
	// Note that the OrientationTransform3D matrix is post-multiplied here
	// in order to correctly orient the scene to match the display orientation.
	// This post-multiplication step is required for any draw calls that are
	// made to the swap chain render target. For draw calls to other targets,
	// this transform should not be applied.

	// This sample makes use of a right-handed coordinate system using row-major matrices.
	XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, 0.01f, 100.0f);

	XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

	XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);
	camProjMat = XMMatrixTranspose(perspectiveMatrix * orientationMatrix);
	XMStoreFloat4x4(&m_constantBufferData.projection, camProjMat);
	XMStoreFloat4x4(&m_InstanceBufferData.projection, camProjMat);


	XMMATRIX orthProMat = XMMatrixOrthographicLH(20, 20, -1.0f, 100.0f);//XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, 0.01f, 100.0f);

	//XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

	//XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);
	if (camSetUp == false) {
		lightProj = XMMatrixTranspose(orthProMat * orientationMatrix);

		// Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.
		static const XMVECTORF32 eye = { 0.0f, 0.7f, -1.5f, 0.0f };
		//static const XMVECTORF32 eye = { 0.0f, 1.0f, 0.0f, 0.0f };
		static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
		static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

		XMStoreFloat4x4(&m_camera, XMMatrixInverse(nullptr, XMMatrixLookAtLH(eye, at, up)));
		XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtLH(eye, at, up)));
		camSetUp = true;
	}
	else {
		//do nothin
	}

	//Directional light shadow
	D3D11_TEXTURE2D_DESC textureDesc;
	memset(&textureDesc, 0, sizeof(textureDesc));

	textureDesc.Width = outputSize.Width;
	textureDesc.Height = outputSize.Height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	D3D11_DEPTH_STENCIL_VIEW_DESC desc;
	memset(&desc, 0, sizeof(desc));
	desc.Format = DXGI_FORMAT_D32_FLOAT;// textureDesc.Format;
	desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	desc.Texture2D.MipSlice = 0;

	lightShadowMap.Release();
	DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateTexture2D(&textureDesc, NULL, &lightShadowMap.p));
	DSVShadowMap.Release();
	DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateDepthStencilView(lightShadowMap.p, &desc, &DSVShadowMap.p));
}

// Called once per frame, rotates the cube and calculates the model and view matrices.
void Sample3DSceneRenderer::Update(DX::StepTimer const& timer)
{
	if (!m_loadingComplete || !objloadingComplete)
	{
		return;
	}

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
	//XMStoreFloat4x4(&InstanceObjects[0].transform[0].Position, XMMatrixTranslation(1, 0, 0));
	//XMStoreFloat4x4(&InstanceObjects[0].transform[1].Position, XMMatrixTranslation(2, 0, 0));

	//renderObjects[1].transform->Position._42 = .5f;
}

// Rotate the 3D cube model a set amount of radians.
void Sample3DSceneRenderer::Rotate(float radians)
{
	// Prepare to pass the updated model matrix to the shader
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(radians)));

	//XMStoreFloat4x4(&InstanceObjects[0].transform[0].Rotation, XMMatrixRotationY(radians));
	//XMStoreFloat4x4(&InstanceObjects[0].transform[0].Rotation, XMMatrixRotationY(radians));
	//XMStoreFloat4x4(&InstanceObjects[0].transform[1].Rotation, XMMatrixRotationY(radians * 2));

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

	//ORDER MATTERS.
	skybox.vertexs.push_back({ XMFLOAT3(-1.0f,-1.0f,1.0f), XMFLOAT3(-1.0f, -1.0f, 1.0f) , XMFLOAT3(0.0f, 1.0f, 0.0f) });
	skybox.vertexs.push_back({ XMFLOAT3(1.0f, -1.0f,1.0f), XMFLOAT3(1.0f, -1.0f, 1.0f) , XMFLOAT3(0.0f, 1.0f, 0.0f) });
	skybox.vertexs.push_back({ XMFLOAT3(1.0f, -1.0f,-1.0f), XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) });
	skybox.vertexs.push_back({ XMFLOAT3(-1.0f,-1.0f,-1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) });

	skybox.vertexs.push_back({ XMFLOAT3(-1.0f,1.0f,1.0f), XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) });
	skybox.vertexs.push_back({ XMFLOAT3(1.0f, 1.0f,1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) });
	skybox.vertexs.push_back({ XMFLOAT3(1.0f, 1.0f,-1.0f), XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) });
	skybox.vertexs.push_back({ XMFLOAT3(-1.0f,1.0f,-1.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) });


	//IF ABOVE ORDER IS CORRECT, THIS IS CLOCKWISE PLANE.

	//Bottom
	skybox.indexes.push_back(0);
	skybox.indexes.push_back(1);
	skybox.indexes.push_back(2);

	skybox.indexes.push_back(3);
	skybox.indexes.push_back(0);
	skybox.indexes.push_back(2);

	//top
	skybox.indexes.push_back(4);
	skybox.indexes.push_back(6);
	skybox.indexes.push_back(5);

	skybox.indexes.push_back(6);
	skybox.indexes.push_back(4);
	skybox.indexes.push_back(7);

	//front
	skybox.indexes.push_back(2);
	skybox.indexes.push_back(6);
	skybox.indexes.push_back(7);


	skybox.indexes.push_back(2);
	skybox.indexes.push_back(7);
	skybox.indexes.push_back(3);

	//back

	skybox.indexes.push_back(4);
	skybox.indexes.push_back(5);
	skybox.indexes.push_back(0);

	skybox.indexes.push_back(0);
	skybox.indexes.push_back(5);
	skybox.indexes.push_back(1);

	//left

	skybox.indexes.push_back(3);
	skybox.indexes.push_back(7);
	skybox.indexes.push_back(4);

	skybox.indexes.push_back(0);
	skybox.indexes.push_back(3);
	skybox.indexes.push_back(4);

	//right

	skybox.indexes.push_back(1);
	skybox.indexes.push_back(5);
	skybox.indexes.push_back(6);

	skybox.indexes.push_back(1);
	skybox.indexes.push_back(6);
	skybox.indexes.push_back(2);
	
	skybox.CalcTangents();
	skybox.SetupVertexBuffers(m_deviceResources.get());
	skybox.SetupIndexBuffer(m_deviceResources.get());

	skybox.LoadTexture(m_deviceResources.get(), "assets/OutputCube.dds");
	//skybox.LoadNormalMap(m_deviceResources.get(), "assets/172_norm.dds");
	skybox.InstanceCnt = 1;
	//skybox.Position._42 = -.5f;
	//	InstanceObjects.push_back(obj);

	//RenderObject obj;

	//ORDER MATTERS.
	shadowMapObj.vertexs.push_back({ XMFLOAT3(-1.0f,1.0f ,1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) , XMFLOAT3(0.0f, 0.0f, 1.0f) });
	shadowMapObj.vertexs.push_back({ XMFLOAT3(1.0f, 1.0f ,1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f) , XMFLOAT3(0.0f, 0.0f, 1.0f) });
	shadowMapObj.vertexs.push_back({ XMFLOAT3(1.0f, -1.0f,1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(0.0f , 0.0f,  1.0f) });
	shadowMapObj.vertexs.push_back({ XMFLOAT3(-1.0f,-1.0f,1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f , 0.0f,  1.0f) });


	//IF ABOVE ORDER IS CORRECT, THIS IS CLOCKWISE PLANE.
	shadowMapObj.indexes.push_back(0);
	shadowMapObj.indexes.push_back(1);
	shadowMapObj.indexes.push_back(2);

	shadowMapObj.indexes.push_back(3);
	shadowMapObj.indexes.push_back(0);
	shadowMapObj.indexes.push_back(2);


	shadowMapObj.CalcTangents();
	shadowMapObj.SetupVertexBuffers(m_deviceResources.get());

	shadowMapObj.SetupIndexBuffer(m_deviceResources.get());

	XMStoreFloat4x4( &shadowMapObj.transform->Position, XMMatrixTranslation(0, 4, 0));

	//obj.Position._42 = -.5f;
	//renderObjects.push_back(obj);

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
	/*
	auto LoadObj = createPSTask.then([this](void)
	{
		//retired.
		//obj|diffuse
		std::ifstream strm;


		strm.open("assets/ModelList.txt");
		char base[] = "./assets/";

		if (strm.is_open()) {
			std::string str;
			//read
			int prevInstance = 0;
			while (std::getline(strm, str)) {

				//handle comments
				if (str.length() == 0 || str[0] == '#')
					continue;

				int substrPos = (int)str.find('|', prevInstance);

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
	*/

	auto LoadObj = createPSTask.then([this](void)
	{
		//obj | diffuse | normal | posX | posY | posZ | rotX | rotY | rotZ | scaleX | scaleY | scaleZ
		std::ifstream strm;


		strm.open("assets/ModelList.txt");
		char base[] = "./assets/";

		if (strm.is_open()) {
			std::string str;
			std::vector<std::string> allreadyLoaded;
			//read
			int prevInstance = 0;
			while (std::getline(strm, str)) {

				//handle comments
				if (str.length() == 0 || str[0] == '#')
					continue;

				std::vector<std::string> lineData;

				//Split the string into the vector
				//Used to denote the start of the current string
				int StartOfWord = 0;
				//Length of the string when used in substring, sstarts at the position of first delim
				int EndOfWord = str.find('|');
				while (str.length() >= EndOfWord) {

					lineData.push_back(str.substr(StartOfWord, EndOfWord - StartOfWord));
					StartOfWord = EndOfWord + 1;
					EndOfWord = str.find('|', StartOfWord);
					if (EndOfWord == -1 && str.length() > StartOfWord) {
						lineData.push_back(str.substr(StartOfWord, str.length() - StartOfWord));
					}
				}
				if (lineData.size() < 13) {
					lineData.push_back("");
				}
				//If already loaded this model/texture.
				bool Exists = false;
				//Check if already loaded.
				int i = 0;
				for (; i < allreadyLoaded.size(); ++i) {
					if (allreadyLoaded[i] == (lineData[0] + '|' + lineData[1] + '|' + lineData[2] + '|' + lineData[12]))
					{
						Exists = true;
						break;
					}
				}
				if (!Exists) {
					RenderObject obj;

					obj.LoadObjFile((base + lineData[0]).c_str());

					//read the obj name
					//strm.getline(line, 256);
					if (lineData[1].length() > 0)
						obj.LoadTexture(m_deviceResources.get(), (base + lineData[1]).c_str());

					if (lineData[2].length() > 0)
						obj.LoadNormalMap(m_deviceResources.get(), (base + lineData[1]).c_str());

					if (lineData.size() > 12 && lineData[12].length() > 0) {
						obj.LoadTroll(m_deviceResources.get(), (base + lineData[12]).c_str());
					}


					//Load position 
					float degtorad = XM_PI / 180.0f;
					XMStoreFloat4x4(&obj.transform[0].Position, XMMatrixTranslation(std::stof(lineData[3]), std::stof(lineData[4]), std::stof(lineData[5])));
					XMStoreFloat4x4(&obj.transform[0].Rotation, XMMatrixMultiply(XMMatrixRotationX(std::stof(lineData[6])*degtorad), XMMatrixMultiply(XMMatrixRotationY(std::stof(lineData[7])*degtorad), XMMatrixRotationZ(std::stof(lineData[8])*degtorad))));
					XMStoreFloat4x4(&obj.transform[0].Scale, XMMatrixScaling(std::stof(lineData[9]), std::stof(lineData[10]), std::stof(lineData[11])));

					//setup the buffers
					DX::ThrowIfFailed(obj.SetupIndexBuffer(m_deviceResources.get()));
					DX::ThrowIfFailed(obj.SetupVertexBuffers(m_deviceResources.get()));

					obj.InstanceCnt = 1;

					allreadyLoaded.push_back((lineData[0] + '|' + lineData[1] + '|' + lineData[2] + '|' + lineData[12]));

					InstanceObjects.push_back(obj);
				}
				else {

					XMStoreFloat4x4(&InstanceObjects[i].transform[InstanceObjects[i].InstanceCnt].Position, XMMatrixTranslation(std::stof(lineData[3]), std::stof(lineData[4]), std::stof(lineData[5])));
					XMStoreFloat4x4(&InstanceObjects[i].transform[InstanceObjects[i].InstanceCnt].Rotation, XMMatrixRotationX(std::stof(lineData[6])) * XMMatrixRotationY(std::stof(lineData[7])) * XMMatrixRotationZ(std::stof(lineData[8])));
					XMStoreFloat4x4(&InstanceObjects[i].transform[InstanceObjects[i].InstanceCnt].Scale, XMMatrixScaling(std::stof(lineData[9]), std::stof(lineData[10]), std::stof(lineData[11])));
					InstanceObjects[i].InstanceCnt += 1;
				}
			}
			strm.close();
		}
	});

	//retired
	/*
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

	*/

	//auto finish = 
	(createVSTask && LoadObj && createInstancedVSTask && createBumpMapTask).then([this]()
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
	lights[0].color = DirectX::XMFLOAT4(1, 1, 1, 0);
	lights[0].radius = DirectX::XMFLOAT4(0, 0, 0, 0);


	//point light
	//World pos, 2 is point light, on w, doesn't use world pos
	lights[1].dir = DirectX::XMFLOAT4(0, 0, 0, 0);
	lights[1].pos = DirectX::XMFLOAT4(0, -1, 0, 2);
	lights[1].color = DirectX::XMFLOAT4(0, 1, 0, 0);
	lights[1].radius = DirectX::XMFLOAT4(2.5f, 0, 0, 0);


	//spot light
	//World pos, 3 is spot light, on w, doesn't use world pos
	lights[2].dir = DirectX::XMFLOAT4(0, -1, 0, 0);
	lights[2].pos = DirectX::XMFLOAT4(1, 0, 0, 3);
	lights[2].color = DirectX::XMFLOAT4(1, 0, 0, 0);
	lights[2].radius = DirectX::XMFLOAT4(5, .9f, .85f, 0);

	//ambient light, 4, ratio is held in radius x
	lights[3].dir = DirectX::XMFLOAT4(0, 0, 0, 0);
	lights[3].pos = DirectX::XMFLOAT4(0, 0, 0, 4);
	lights[3].color = DirectX::XMFLOAT4(1, 1, 1, 0);
	lights[3].radius = DirectX::XMFLOAT4(.25f, 0, 0, 0);

	CD3D11_BUFFER_DESC constantBufferDesc(sizeof(Light) * lights.size(), D3D11_BIND_CONSTANT_BUFFER);
	DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, &m_lightBuffer.p));

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
	for (int i = 0; i < 4; i++)
		lightsOn[i] = true;
}

void DX11UWA::Sample3DSceneRenderer::UpdateLights(const DX::StepTimer &time)
{
	//Move lights
	//XMMATRIX pos = XMMATRIX(lights[1].pos,);
	elpsTime += time.GetElapsedSeconds();
	float angle = (((elpsTime * 100)));
	//1 rotation per 2 seconds.

	angle = angle * XM_PI / 540.0f;

	lights[0].dir.x = cosf(angle);
	lights[0].dir.y = sinf(angle);
	lights[0].pos.x = -5 * cosf(angle);
	lights[0].pos.y = -5 * sinf(angle);
	angle = (((elpsTime * 100)));
	angle = angle * XM_PI / 180.0f;
	lights[1].pos.x = cosf(angle);
	lights[1].pos.y = sinf(angle);

	angle = ((elpsTime));
	lights[2].pos.x = cosf(angle) * 2;
	lights[2].pos.y = .75f;


	lights[2].pos.z = sinf(angle) * 2;

	//angle = (elpsTime * 100);
	lights[2].dir.x = -1 * cosf(angle);
	lights[2].dir.y = 0;// -1 * cosf(angle);
	lights[2].dir.z = -1 * sinf(angle);

	if (OnButtonPress('0')) {
		lightsOn[0] = true;
		lightsOn[1] = true;
		lightsOn[2] = true;
		lightsOn[3] = true;
	}

	if (OnButtonPress('1')) {
		lightsOn[0] = !lightsOn[0];
	}
	if (OnButtonPress('2')) {
		lightsOn[1] = !lightsOn[1];

	}
	if (OnButtonPress('3')) {
		lightsOn[2] = !lightsOn[2];

	}
	if (OnButtonPress('4')) {
		lightsOn[3] = !lightsOn[3];
	}

	if (lightsOn[0])
		lights[0].color = DirectX::XMFLOAT4(1, 1, 1, 0);
	else
		lights[0].color = DirectX::XMFLOAT4(0, 0, 0, 0);

	if (lightsOn[1])
		lights[1].color = DirectX::XMFLOAT4(0, 1, 0, 0);
	else
		lights[1].color = DirectX::XMFLOAT4(0, 0, 0, 0);


	if (lightsOn[2])
		lights[2].color = DirectX::XMFLOAT4(1, 0, 0, 0);
	else
		lights[2].color = DirectX::XMFLOAT4(0, 0, 0, 0);


	if (lightsOn[3])
		lights[3].color = DirectX::XMFLOAT4(1, 1, 1, 0);
	else
		lights[3].color = DirectX::XMFLOAT4(0, 0, 0, 0);



	//hard codedededededededede Weee.
	XMStoreFloat4x4(&lightModels[0].transform[0].Position, XMMatrixTranslation(lights[1].pos.x, lights[1].pos.y, lights[1].pos.z));
	XMStoreFloat4x4(&lightModels[0].transform[0].Scale, XMMatrixScaling(.2f, .2f, .2f));

	XMStoreFloat4x4(&lightModels[0].transform[1].Position, XMMatrixTranslation(lights[2].pos.x, lights[2].pos.y, lights[2].pos.z));
	XMStoreFloat4x4(&lightModels[0].transform[1].Scale, XMMatrixScaling(.2f, .2f, .2f));
}

bool DX11UWA::Sample3DSceneRenderer::OnButtonPress(char c)
{
	if (m_kbuttons[c] & !m_Prevbuttons[c]) {
		return true;
	}
	return false;
}

void Sample3DSceneRenderer::SetKeyboardButtons(const char* list)
{
	memcpy_s(m_Prevbuttons, sizeof(m_Prevbuttons), m_kbuttons, sizeof(m_kbuttons));
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

void DX11UWA::Sample3DSceneRenderer::RenderToShadow()
{

	if (!m_loadingComplete || !objloadingComplete)
	{
		return;
	}
	auto context = m_deviceResources->GetD3DDeviceContext();

	ID3D11ShaderResourceView * nul = nullptr;
	context->PSSetShaderResources(0, 1, &nul);
	context->PSSetShaderResources(1, 1, &nul);
	context->PSSetShaderResources(2, 1, &nul);
	context->PSSetShader(nullptr, nullptr, 0);

	context->OMSetRenderTargets(0, 0, DSVShadowMap.p);
	context->ClearDepthStencilView(DSVShadowMap.p, D3D11_CLEAR_DEPTH, 1, 0);

	XMVECTORF32 pos = { lights[0].pos.x, lights[0].pos.y, lights[0].pos.z,0.0f };
	XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
	XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMMATRIX camMat = XMMatrixLookAtLH(pos, at, up);
	//render the whole scene using the light view/proj


	DirectX::XMStoreFloat4x4(&m_constantBufferData.projection, camProjMat);
	DirectX::XMStoreFloat4x4(&m_InstanceBufferData.projection, camProjMat);

	DirectX::XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera))));
	DirectX::XMStoreFloat4x4(&m_InstanceBufferData.view, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera))));

	DirectX::XMStoreFloat4x4(&m_constantBufferData.lightProj, lightProj);
	DirectX::XMStoreFloat4x4(&m_InstanceBufferData.lightProj, lightProj);
	DirectX::XMStoreFloat4x4(&m_constantBufferData.lightView, XMMatrixTranspose(camMat));
	DirectX::XMStoreFloat4x4(&m_InstanceBufferData.lightView, XMMatrixTranspose(camMat));

	//XMMatrixLookAtLH(XMLoadFloat4(&lights[0].dir)*-1000, (XMVectorAdd(XMLoadFloat4(&lights[0].dir), XMLoadFloat4(&lights[0].dir) * -1000)), XMLoadFloat4(&XMFLOAT4(0, 1, 0, 0)));

	//context->UpdateSubresource1(m_lightBuffer.p, 0, NULL, lights.data(), 0, 0, 0);

	UINT stride = sizeof(VertexPositionUVNormal);
	UINT offset = 0;

	//Retired
	/*
	//Draw my custom indexed objects
	XMMATRIX temp1, temp2;
	for (int i = 0; i < renderObjects.size(); i++)
	{
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
		context->VSSetShader(m_ShadowShader.p, nullptr, 0);

		// Send the constant buffer to the graphics device.
		context->VSSetConstantBuffers1(0, 1, m_constantBuffer.GetAddressOf(), nullptr, nullptr);

		context->GSSetShader(NULL, nullptr, 0);

		// Draw the objects. Number of Tri's
		context->DrawIndexed(renderObjects[i].indexes.size(), 0, 0);
	}
	*/

	////Draw my Instanced Indexed Objects
	for (int i = 0; i < InstanceObjects.size(); ++i) {
		//update the constant buffer with specific objects rotation, and orientation
		for (int j = 0; j < InstanceObjects[i].InstanceCnt; ++j)
			XMStoreFloat4x4(&m_InstanceBufferData.model[j], XMMatrixTranspose(InstanceObjects[i].transform[j].MultTransform()));// *renderObjects[i].Position);

		context->UpdateSubresource1(m_InstanceConstBuffer.p, 0, NULL, &m_InstanceBufferData, 0, 0, 0);
		// Each vertex is one instance of the VertexPositionColor struct.

		context->IASetVertexBuffers(0, 1, &(InstanceObjects[i].vertexBuffer.p), &stride, &offset);

		// Each index is one 16-bit unsigned integer (short).
		context->IASetIndexBuffer((InstanceObjects[i].indexBuffer.p), DXGI_FORMAT_R32_UINT, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->IASetInputLayout(objinputLayout.p);

		// Attach our vertex shader.
		context->VSSetShader(m_ShadowShader.p, nullptr, 0);

		// Send the constant buffer to the graphics device.
		context->VSSetConstantBuffers1(0, 1, &m_InstanceConstBuffer.p, nullptr, nullptr);
		context->GSSetShader(NULL, nullptr, 0);

		// Draw the objects. Number of Tri's
		context->DrawIndexedInstanced(InstanceObjects[i].indexes.size(), InstanceObjects[i].InstanceCnt, 0, 0, 0);
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	memset(&srvDesc, 0, sizeof(srvDesc));

	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;

	SRVShadowMap.Release();
	DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateShaderResourceView(lightShadowMap.p, &srvDesc, &SRVShadowMap.p));


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

	if (m_kbuttons['L'] == true) {
		XMVECTORF32 pos = { lights[0].pos.x, lights[0].pos.y, lights[0].pos.z,0.0f };
		XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
		XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

		XMMATRIX camMat = XMMatrixLookAtLH(pos, at, up);

		DirectX::XMStoreFloat4x4(&m_constantBufferData.projection, lightProj);
		DirectX::XMStoreFloat4x4(&m_InstanceBufferData.projection, lightProj);
		DirectX::XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(camMat));
		DirectX::XMStoreFloat4x4(&m_InstanceBufferData.view, XMMatrixTranspose(camMat));
	}
	else {
		XMStoreFloat4x4(&m_constantBufferData.projection, camProjMat);
		XMStoreFloat4x4(&m_InstanceBufferData.projection, camProjMat);

		XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera))));
		XMStoreFloat4x4(&m_InstanceBufferData.view, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera))));

		//XMMatrixLookAtLH(XMLoadFloat4(&lights[0].dir)*-1000, (XMVectorAdd(XMLoadFloat4(&lights[0].dir), XMLoadFloat4(&lights[0].dir) * -1000)), XMLoadFloat4(&XMFLOAT4(0, 1, 0, 0)));
	}





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





	//update the constant buffer with specific objects rotation, and orientation
	XMStoreFloat4x4(&m_constantBufferData.model, 	XMMatrixTranspose(XMMatrixTranslation(m_camera._41, m_camera._42, m_camera._43)));// *renderObjects[i].Position);
																		   // Prepare the constant buffer to send it to the graphics device.
	context->UpdateSubresource1(m_constantBuffer.Get(), 0, NULL, &m_constantBufferData, 0, 0, 0);
	// Each vertex is one instance of the VertexPositionColor struct.

	context->IASetVertexBuffers(0, 1, &(skybox.vertexBuffer.p), &stride, &offset);

	// Each index is one 16-bit unsigned integer (short).
	context->IASetIndexBuffer((skybox.indexBuffer.p), DXGI_FORMAT_R32_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(objinputLayout.p);

	// Attach our vertex shader.
	context->VSSetShader(objvertexShader.p, nullptr, 0);

	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(0, 1, m_constantBuffer.GetAddressOf(), nullptr, nullptr);

	context->PSSetShader(skyboxPShader.p, nullptr, 0);
	context->PSSetShaderResources(0,1,&skybox.constTextureBuffer.p);
	context->PSSetSamplers(0, 1, &skybox.sampState.p);
	context->GSSetShader(NULL, nullptr, 0);

	// Draw the objects. Number of Tri's
	context->DrawIndexed(skybox.indexes.size(), 0, 0);

	context->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1, 0);

	//draw shadow depth buffer
	
	//render shadow plane
	temp1 = XMLoadFloat4x4(&shadowMapObj.transform->Scale);
	temp2 = XMLoadFloat4x4(&shadowMapObj.transform->Rotation);
	temp1 = XMMatrixMultiply(temp1, temp2);
	temp2 = XMLoadFloat4x4(&shadowMapObj.transform->Position);
	temp1 = XMMatrixMultiply(temp1, temp2);

	//update the constant buffer with specific objects rotation, and orientation
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(temp1));// *renderObjects[i].Position);

																		   // Prepare the constant buffer to send it to the graphics device.
	context->UpdateSubresource1(m_constantBuffer.Get(), 0, NULL, &m_constantBufferData, 0, 0, 0);
	// Each vertex is one instance of the VertexPositionColor struct.

	context->IASetVertexBuffers(0, 1, &(shadowMapObj.vertexBuffer.p), &stride, &offset);

	// Each index is one 16-bit unsigned integer (short).
	context->IASetIndexBuffer((shadowMapObj.indexBuffer.p), DXGI_FORMAT_R32_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(objinputLayout.p);

	// Attach our vertex shader.
	context->VSSetShader(objvertexShader.p, nullptr, 0);

	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(0, 1, m_constantBuffer.GetAddressOf(), nullptr, nullptr);

	context->PSSetShader(m_ShadowPShader.p, nullptr, 0);
	context->PSSetSamplers(0, 1, &InstanceObjects[0].sampState.p);
	context->PSSetShaderResources(0, 1, &SRVShadowMap.p);
	context->PSSetConstantBuffers(0, 1, &m_lightBuffer.p);

	context->GSSetShader(NULL, nullptr, 0);

	// Draw the objects. Number of Tri's
	context->DrawIndexed(shadowMapObj.indexes.size(), 0, 0);
	

	//Retired render loop
	/*
	for (int i = 0; i < renderObjects.size(); i++)
	{
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
			context->PSSetShaderResources(1, 1, &SRVShadowMap.p);

			context->PSSetConstantBuffers(0, 1, &m_lightBuffer.p);
		}
		else {
			// Attach our pixel shader.
			context->PSSetShader(objBMPixelShader.p, nullptr, 0);
			context->PSSetSamplers(0, 1, &renderObjects[i].sampState.p);
			context->PSSetShaderResources(0, 1, &renderObjects[i].constTextureBuffer.p);
			context->PSSetShaderResources(1, 1, &renderObjects[i].constBumpMapBuffer.p);
			context->PSSetShaderResources(2, 1, &SRVShadowMap.p);

			context->PSSetConstantBuffers(0, 1, &m_lightBuffer.p);
			//context->psset
		}
		context->GSSetShader(NULL, nullptr, 0);

		// Draw the objects. Number of Tri's
		context->DrawIndexed(renderObjects[i].indexes.size(), 0, 0);
	}
	*/


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
		context->IASetIndexBuffer((InstanceObjects[i].indexBuffer.p), DXGI_FORMAT_R32_UINT, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->IASetInputLayout(objinputLayout.p);

		// Attach our vertex shader.
		context->VSSetShader(instanceVertexShader.p, nullptr, 0);

		// Send the constant buffer to the graphics device.
		context->VSSetConstantBuffers1(0, 1, &m_InstanceConstBuffer.p, nullptr, nullptr);
		context->GSSetShader(NULL, nullptr, 0);
		if (InstanceObjects[i].constTextureBuffer == NULL)
		{
			context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

		}
		else if (InstanceObjects[i].constBumpMapBuffer == NULL) {
			// Attach our pixel shader.
			if (InstanceObjects[i].constTrollBuffer != NULL) {

				context->PSSetShader(objTrollPS.p, nullptr, 0);
				context->PSSetSamplers(0, 1, &InstanceObjects[i].sampState.p);
				context->PSSetShaderResources(0, 1, &InstanceObjects[i].constTextureBuffer.p);
				context->PSSetShaderResources(1, 1, &SRVShadowMap.p);
				context->PSSetShaderResources(2, 1, &InstanceObjects[i].constTrollBuffer.p);

				context->PSSetConstantBuffers(0, 1, &m_lightBuffer.p);
			}
			else {
				context->PSSetShader(objpixelShader.p, nullptr, 0);
				context->PSSetSamplers(0, 1, &InstanceObjects[i].sampState.p);
				context->PSSetShaderResources(0, 1, &InstanceObjects[i].constTextureBuffer.p);
				context->PSSetShaderResources(1, 1, &SRVShadowMap.p);

				context->PSSetConstantBuffers(0, 1, &m_lightBuffer.p);
			}
		}
		else {
			// Attach our pixel shader.
			context->PSSetShader(objBMPixelShader.p, nullptr, 0);
			context->PSSetSamplers(0, 1, &InstanceObjects[i].sampState.p);
			context->PSSetShaderResources(0, 1, &InstanceObjects[i].constTextureBuffer.p);
			context->PSSetShaderResources(1, 1, &InstanceObjects[i].constBumpMapBuffer.p);
			context->PSSetShaderResources(2, 1, &SRVShadowMap.p);

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
		context->IASetIndexBuffer((lightModels[i].indexBuffer.p), DXGI_FORMAT_R32_UINT, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);//D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->IASetInputLayout(objinputLayout.p);

		// Attach our vertex shader.
		//context->VSSetShader(instanceVertexShader.p, nullptr, 0);
		context->VSSetShader(m_GeoVertexShader.p, nullptr, 0);
		// Send the constant buffer to the graphics device.
		context->GSSetShader(m_GeoShader.p, nullptr, 0);
		context->GSSetConstantBuffers1(0, 1, &m_InstanceConstBuffer.p, nullptr, nullptr);

		//context->PSSetShader(NULL, nullptr, 0);
		context->PSSetShader(m_pixelShader.Get(), nullptr, 0);


		// Draw the objects. Number of Tri's
		context->DrawIndexedInstanced(lightModels[i].indexes.size(), lightModels[i].InstanceCnt, 0, 0, 0);
	}

}

void Sample3DSceneRenderer::CreateDeviceDependentResources(void)
{
	//lightProj = XMMatrixOrthographicLH(1028, 1028, .1, 100);


	// Load shaders asynchronously.
	auto loadVSTask = DX::ReadDataAsync(L"SampleVertexShader.cso");
	auto loadPSTask = DX::ReadDataAsync(L"SamplePixelShader.cso");
	auto loadGSTask = DX::ReadDataAsync(L"TriangleGeoShader.cso");
	auto loadVSGeoTask = DX::ReadDataAsync(L"ForwardToGeoVertexShader.cso");
	auto loadVSShadowTask = DX::ReadDataAsync(L"ShadowVSShader.cso");
	auto loadPSShadowTask = DX::ReadDataAsync(L"DrawShadowMapPixelShader.cso");
	auto loadTrollShader = DX::ReadDataAsync(L"TrollPixelShader.cso");
	auto loadSkyboxShader = DX::ReadDataAsync(L"SkyboxPixelShader.cso");
	// After the vertex shader file is loaded, create the shader and input layout.
	auto createVSShadowTask = loadVSShadowTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateVertexShader(&fileData[0], fileData.size(), nullptr, &m_ShadowShader.p));
	});

	auto createtrollTask = loadTrollShader.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&fileData[0], fileData.size(), nullptr, &objTrollPS.p));
	});
	auto createPSShadowTask = loadPSShadowTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&fileData[0], fileData.size(), nullptr, &m_ShadowPShader.p));
	});
	auto createSkyboxShader = loadSkyboxShader.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&fileData[0], fileData.size(), nullptr, &skyboxPShader.p));
	});

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

	auto createGSTase = loadGSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateGeometryShader(&fileData[0], fileData.size(), nullptr, &m_GeoShader.p));
	});
	auto createGeoVSTask = loadVSGeoTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateVertexShader(&fileData[0], fileData.size(), nullptr, &m_GeoVertexShader.p));

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
	(createMyStuff && createGSTase && createVSShadowTask && createtrollTask && createPSShadowTask && createGeoVSTask && createSkyboxShader).then([this]()
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