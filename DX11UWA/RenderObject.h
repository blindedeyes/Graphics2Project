#pragma once
#include "pch.h"
#include <atlcomcli.h>
#include <vector>
#include <fstream>
#include <string>
#include "./Common/DeviceResources.h"
#include "./Content/ShaderStructures.h"


struct Transform {

	DirectX::XMFLOAT4X4 Position;
	DirectX::XMFLOAT4X4 Rotation;// = DirectX::XMMatrixIdentity();
	DirectX::XMFLOAT4X4 Scale;// = DirectX::XMMatrixIdentity();

	DirectX::XMMATRIX MultTransform() {
		DirectX::XMMATRIX temp1, temp2;
		temp1 = XMLoadFloat4x4(&this->Scale);
		temp2 = XMLoadFloat4x4(&this->Position);
		temp1 = XMMatrixMultiply(temp1, temp2);
		temp2 = XMLoadFloat4x4(&this->Rotation);
		temp1 = XMMatrixMultiply(temp1, temp2);
		return ( temp1);
	}
};
class RenderObject
{
	
public:
	
	void CalcTangents();
	std::vector<DX11UWA::VertexPositionUVNormal> vertexs;
	std::vector<unsigned int> indexes;


	D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
	D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
	CD3D11_BUFFER_DESC indexBufferDesc;

	CComPtr<ID3D11Buffer> indexBuffer;
	CComPtr<ID3D11Buffer> vertexBuffer;
	
	Transform transform[16];
	
	//Used for the texture, pass into DDS file converter, it puts it into this for us.
	CComPtr<ID3D11ShaderResourceView> constTextureBuffer;
	CComPtr<ID3D11ShaderResourceView> constTrollBuffer;
	CComPtr<ID3D11ShaderResourceView> constBumpMapBuffer;

	
	CComPtr<ID3D11SamplerState> sampState;

	//bool instance=false;
	unsigned int InstanceCnt = 0;
	void (*UpdateObject)(void);

	//std::vector<CComPtr<Reource
	//Need to add shaders here

	RenderObject() {
		//transform = new Transform[1];
		/*DirectX::XMStoreFloat4x4(&transform->Position, DirectX::XMMatrixIdentity());
		DirectX::XMStoreFloat4x4(&transform->Rotation, DirectX::XMMatrixIdentity());
		DirectX::XMStoreFloat4x4(&transform->Scale, DirectX::XMMatrixIdentity());*/
		for (int i = 0; i < 5; ++i) {

			DirectX::XMStoreFloat4x4(&transform[i].Position, DirectX::XMMatrixIdentity());
			DirectX::XMStoreFloat4x4(&transform[i].Rotation, DirectX::XMMatrixIdentity());
			DirectX::XMStoreFloat4x4(&transform[i].Scale, DirectX::XMMatrixIdentity());
		}
	}

	
	//RenderObject(int instanceCount) {
	//	//transform = new Transform[instanceCount];
	//	for (int i = 0; i < instanceCount; ++i) {

	//		DirectX::XMStoreFloat4x4(&transform[i].Position, DirectX::XMMatrixIdentity());
	//		DirectX::XMStoreFloat4x4(&transform[i].Rotation, DirectX::XMMatrixIdentity());
	//		DirectX::XMStoreFloat4x4(&transform[i].Scale, DirectX::XMMatrixIdentity());
	//	}
	//	InstanceCnt = instanceCount;
	//	
	//}
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
	DirectX::XMFLOAT4X4& Scale() {
		return transform->Scale;
	}
	DirectX::XMFLOAT4X4& Rotation() {
		return transform->Scale;
	}
	DirectX::XMFLOAT4X4& Position() {
		return transform->Position;
	}

	

	void LoadObjFile(const char* path);
	void LoadTexture(DX::DeviceResources* dresources, const char * path);
	void LoadNormalMap(DX::DeviceResources* dresources, const char * path);
	HRESULT SetupVertexBuffers(DX::DeviceResources* dresources);
	HRESULT SetupIndexBuffer(DX::DeviceResources* dresources);
	void LoadTroll(DX::DeviceResources* dresources, const char * path);


};

