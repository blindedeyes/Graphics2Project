#pragma once
#include "pch.h"
#include <atlcomcli.h>
#include <vector>
#include <fstream>
#include <string>
#include "./Common/DeviceResources.h"
#include "./Content/ShaderStructures.h"



class RenderObject
{
public:
	
	std::vector<DX11UWA::VertexPositionUVNormal> vertexs;
	std::vector<unsigned short> indexes;


	D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
	D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
	CD3D11_BUFFER_DESC indexBufferDesc;

	DirectX::XMFLOAT4X4  Position;
	DirectX::XMFLOAT4X4 Rotation;// = DirectX::XMMatrixIdentity();
	DirectX::XMFLOAT4X4 Scale;// = DirectX::XMMatrixIdentity();

	CComPtr<ID3D11Buffer> indexBuffer;
	CComPtr<ID3D11Buffer> constBuffer;
	CComPtr<ID3D11Buffer> vertexBuffer;


	void (*UpdateObject)(void);

	//std::vector<CComPtr<Reource
	//Need to add shaders here

	RenderObject() {
		DirectX::XMStoreFloat4x4(&Position, DirectX::XMMatrixIdentity());
		DirectX::XMStoreFloat4x4(&Rotation, DirectX::XMMatrixIdentity());
		DirectX::XMStoreFloat4x4(&Scale, DirectX::XMMatrixIdentity());

	}

	//RenderObject(char * path) {
	//	//Load model here.
	//	LoadObjFile(path);
	//	Position = DirectX::XMFLOAT3(0, 0, 0);
	//	Rotation = DirectX::XMFLOAT3(0, 0, 0);
	//	Scale = DirectX::XMFLOAT3(1, 1, 1);
	//}

	//void LoadOBJ(char * path) {
	//	LoadObjFile(path);
	//	Position = DirectX::XMFLOAT3(0, 0, 0);
	//	Rotation = DirectX::XMFLOAT3(0, 0, 0);
	//	Scale = DirectX::XMFLOAT3(1, 1, 1);
	//}


	void LoadObjFile(char* path);

	HRESULT SetupVertexBuffers(DX::DeviceResources* dresources);
	HRESULT SetupIndexBuffer(DX::DeviceResources* dresources);


};

