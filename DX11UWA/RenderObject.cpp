#include "pch.h"
#include "RenderObject.h"
#include "Common\DDSTextureLoader.h"
void RenderObject::CalcTangents()
{
	//for each tri
	for (int i = 0; i < indexes.size(); i += 3) {
		//Directions from vert i, to other two verts of tri.
		DirectX::XMFLOAT3 edge1, edge2, uvEdge1, uvEdge2;

		edge1.x = vertexs[indexes[i + 1]].pos.x - vertexs[indexes[i]].pos.x;
		edge1.y = vertexs[indexes[i + 1]].pos.y - vertexs[indexes[i]].pos.y;
		edge1.z = vertexs[indexes[i + 1]].pos.z - vertexs[indexes[i]].pos.z;

		edge2.x = vertexs[indexes[i + 2]].pos.x - vertexs[indexes[i]].pos.x;
		edge2.y = vertexs[indexes[i + 2]].pos.y - vertexs[indexes[i]].pos.y;
		edge2.z = vertexs[indexes[i + 2]].pos.z - vertexs[indexes[i]].pos.z;


		uvEdge1.x = vertexs[indexes[i + 1]].uv.x - vertexs[indexes[i]].uv.x;
		uvEdge1.y = vertexs[indexes[i + 1]].uv.y - vertexs[indexes[i]].uv.y;
		uvEdge1.z = vertexs[indexes[i + 1]].uv.z - vertexs[indexes[i]].uv.z;

		uvEdge2.x = vertexs[indexes[i + 2]].uv.x - vertexs[indexes[i]].uv.x;
		uvEdge2.y = vertexs[indexes[i + 2]].uv.y - vertexs[indexes[i]].uv.y;
		uvEdge2.z = vertexs[indexes[i + 2]].uv.z - vertexs[indexes[i]].uv.z;

		float ratio = 1.0f / (uvEdge1.x * uvEdge2.y - uvEdge2.x * uvEdge1.y);
		
		DirectX::XMFLOAT3 directionU(
			(uvEdge2.y*uvEdge1.x - uvEdge1.y*uvEdge2.x) * ratio,
			(uvEdge2.y*uvEdge1.y - uvEdge1.y*uvEdge2.y) * ratio,
			(uvEdge2.y*uvEdge1.z - uvEdge1.y*uvEdge2.z) * ratio
		);
		DirectX::XMFLOAT3 directionV(
			(uvEdge1.x*uvEdge2.x - uvEdge2.x*uvEdge1.x) * ratio,
			(uvEdge1.x*uvEdge2.y - uvEdge2.x*uvEdge1.y) * ratio,
			(uvEdge1.x*uvEdge2.z - uvEdge2.x*uvEdge1.z) * ratio
		);
		DirectX::XMVECTOR vecDirU = DirectX::XMLoadFloat3(&directionU);
		DirectX::XMVECTOR vecDirV = DirectX::XMLoadFloat3(&directionV);
		vecDirU = DirectX::XMVector3Normalize(vecDirU);
		vecDirV = DirectX::XMVector3Normalize(vecDirV);

		for (int j = 0; j < 3; j++) {
			//part1
			DirectX::XMVECTOR normal = DirectX::XMLoadFloat3(&vertexs[indexes[i+j]].normal);
			DirectX::XMVECTOR dotRes = DirectX::XMVector3Dot(normal, vecDirU);

			DirectX::XMVECTOR tang = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(vecDirU, DirectX::XMVectorMultiply( normal,dotRes)));
			DirectX::XMFLOAT3 temp;

			DirectX::XMStoreFloat3(&temp, tang);

			vertexs[indexes[i + j]].tangent.x = temp.x;
			vertexs[indexes[i + j]].tangent.y = temp.y;
			vertexs[indexes[i + j]].tangent.z = temp.z;
			//part2
			DirectX::XMVECTOR cross = DirectX::XMVector3Cross(normal, vecDirU);
			DirectX::XMVECTOR dotRes2 = DirectX::XMVector3Dot(cross, vecDirV);
			DirectX::XMFLOAT3 Res2;
			DirectX::XMStoreFloat3(&Res2, dotRes2);

			vertexs[indexes[i + j]].tangent.w = (Res2.x < 0.0f) ? -1.0f : 1.0f;


		}
		

	}
}
void RenderObject::LoadObjFile(const char * path)
{
	std::vector<DirectX::XMFLOAT3> verts;
	std::vector<DirectX::XMFLOAT3> Normals;
	std::vector<DirectX::XMFLOAT3> UV;


	std::vector<unsigned int> trianglesVert;
	std::vector<unsigned int> trianglesUV;
	std::vector<unsigned int> trianglesNormal;


	//FILE * file = 
	FILE * file;
	fopen_s(&file, path, "r");
	TCHAR cwd[128];

	GetCurrentDirectory(128, cwd);
	OutputDebugString(cwd);

	if (file != NULL) {
		while (true) {
			char header[128];
			int res = fscanf_s(file, "%s", header, 128);

			//End of file check
			if (res == EOF)
				break;

			//read vertex
			if (strcmp(header, "v") == 0)
			{

				DirectX::XMFLOAT3 vert;
				fscanf_s(file, "%f %f %f\n", &vert.x, &vert.y, &vert.z);
				verts.push_back(vert);

			}
			else if (strcmp(header, "vt") == 0)
			{
				DirectX::XMFLOAT3 _uv;
				fscanf_s(file, "%f %f\n", &_uv.x, &_uv.y);
				UV.push_back(_uv);
			}
			else if (strcmp(header, "vn") == 0) {
				//normals
				DirectX::XMFLOAT3 norm;
				fscanf_s(file, "%f %f %f\n", &norm.x, &norm.y, &norm.z);
				Normals.push_back(norm);
			}
			else if (strcmp(header, "f") == 0) {
				//std::string vertex1, vertex2, vertex3;
				unsigned int vertexIndex1, vertexIndex2, vertexIndex3;
				unsigned int uvIndex1, uvIndex2, uvIndex3;
				unsigned int normalIndex1, normalIndex2, normalIndex3;

				int matches = fscanf_s(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n",
					&vertexIndex1, &uvIndex1, &normalIndex1,
					&vertexIndex2, &uvIndex2, &normalIndex2,
					&vertexIndex3, &uvIndex3, &normalIndex3);

				if (matches != 9) {
					printf("File can't be read");
					break;
				}

				trianglesVert.push_back(vertexIndex1 - 1);
				trianglesVert.push_back(vertexIndex2 - 1);
				trianglesVert.push_back(vertexIndex3 - 1);

				trianglesUV.push_back(uvIndex1 - 1);
				trianglesUV.push_back(uvIndex2 - 1);
				trianglesUV.push_back(uvIndex3 - 1);

				trianglesNormal.push_back(normalIndex1 - 1);
				trianglesNormal.push_back(normalIndex2 - 1);
				trianglesNormal.push_back(normalIndex3 - 1);

			}

		}
		fclose(file);
		//delete file;

		for (int i = 0; i < trianglesNormal.size(); ++i) {
			DX11UWA::VertexPositionUVNormal temp;
			temp.pos = verts[trianglesVert[i]];
			temp.uv = UV[trianglesUV[i]];
			temp.normal = Normals[trianglesNormal[i]];
			temp.uv.y = 1 - temp.uv.y;
			vertexs.push_back(temp);
			indexes.push_back(i);
		}

		CalcTangents();

	}
	else {
		//ERROR

		std::cout << "wat";
		return;

	}

}


void RenderObject::LoadTroll(DX::DeviceResources* dresources, const char * path)
{
	size_t size = strlen(path) + 1;
	wchar_t * wpath = new wchar_t[size];
	size_t out;
	//number of char converted, destination, size of destination, path, size of path
	mbstowcs_s(&out, wpath, size, path, size - 1);

	//setup the resources
	HRESULT RES = CreateDDSTextureFromFile(dresources->GetD3DDevice(), wpath, NULL, &(this->constTrollBuffer.p));
	if (RES != S_OK)
		OutputDebugString(L"Fuck");
	//sampler

	delete[] wpath;
}
void RenderObject::LoadTexture(DX::DeviceResources* dresources, const char * path)
{
	size_t size = strlen(path) + 1;
	wchar_t * wpath = new wchar_t[size];
	size_t out;
	//number of char converted, destination, size of destination, path, size of path
	mbstowcs_s(&out, wpath, size, path, size - 1);

	//setup the resources
	HRESULT RES = CreateDDSTextureFromFile(dresources->GetD3DDevice(), wpath, NULL, &(this->constTextureBuffer.p));
	if (RES != S_OK)
		OutputDebugString(L"Fuck");
	//sampler
	D3D11_SAMPLER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Filter = D3D11_FILTER_ANISOTROPIC;
	//CLAMP EDGES, NOT WRAP
	desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	desc.MaxLOD = D3D11_FLOAT32_MAX;

	dresources->GetD3DDevice()->CreateSamplerState(&desc, &(this->sampState));

	delete[] wpath;
}
void RenderObject::LoadNormalMap(DX::DeviceResources* dresources, const char * path)
{
	size_t size = strlen(path) + 1;
	wchar_t * wpath = new wchar_t[size];
	size_t out;
	//number of char converted, destination, size of destination, path, size of path
	mbstowcs_s(&out, wpath, size, path, size - 1);

	//setup the resources
	HRESULT RES = CreateDDSTextureFromFile(dresources->GetD3DDevice(), wpath, NULL, &(this->constBumpMapBuffer.p));
	//if (RES != S_OK)
		//OutputDebugString(L"Fuck");


	delete[] wpath;
}
HRESULT RenderObject::SetupVertexBuffers(DX::DeviceResources* dresources)
{

	D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
	vertexBufferData.pSysMem = vertexs.data();
	vertexBufferData.SysMemPitch = 0;
	vertexBufferData.SysMemSlicePitch = 0;
	CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(DX11UWA::VertexPositionUVNormal) * vertexs.size(), D3D11_BIND_VERTEX_BUFFER);
	return dresources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &vertexBuffer.p);


}

HRESULT RenderObject::SetupIndexBuffer(DX::DeviceResources * dresources)
{

	D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
	indexBufferData.pSysMem = indexes.data();
	indexBufferData.SysMemPitch = 0;
	indexBufferData.SysMemSlicePitch = 0;
	CD3D11_BUFFER_DESC indexBufferDesc(sizeof(unsigned int) * indexes.size(), D3D11_BIND_INDEX_BUFFER);
	return dresources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &indexBuffer.p);

}


