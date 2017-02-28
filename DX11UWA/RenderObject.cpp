#include "pch.h"
#include "RenderObject.h"

void RenderObject::LoadObjFile(char * path)
{
	std::vector<DX11UWA::VertexPositionColor> tempverts;

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
				return;

			//read vertex
			if (strcmp(header, "v") == 0)
			{

				DX11UWA::VertexPositionColor vert;
				fscanf_s(file, "%f %f %f\n", &vert.pos.x, &vert.pos.y, &vert.pos.z);
				tempverts.push_back(vert);

			}
			else if (strcmp(header, "vt") == 0)
			{
				DirectX::XMFLOAT2 _uv;
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
				std::string vertex1, vertex2, vertex3;
				unsigned int vertexIndex1, vertexIndex2, vertexIndex3;
				unsigned int uvIndex1, uvIndex2, uvIndex3;
				unsigned int normalIndex1, normalIndex2, normalIndex3;
				int matches = fscanf_s(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n",
					&vertexIndex1, &uvIndex1, &normalIndex1,
					&vertexIndex2, &uvIndex2, &normalIndex2,
					&vertexIndex3, &uvIndex3, &normalIndex3);
				if (matches != 9) {
					printf("File can't be read by our simple parser : ( Try exporting with other options\n");
					return;
				}

				triangles.push_back(verts.size());
				verts.push_back(tempverts[vertexIndex1 - 1]);
				triangles.push_back(verts.size());
				verts.push_back(tempverts[vertexIndex2 - 1]);
				triangles.push_back(verts.size());
				verts.push_back(tempverts[vertexIndex3 - 1]);


				//FOR LATER. Not sure how to use this yet.
				/*
				uvIndices.push_back(uvIndex1);
				uvIndices.push_back(uvIndex2);
				uvIndices.push_back(uvIndex3);
				normalIndices.push_back(normalIndex1);
				normalIndices.push_back(normalIndex2);
				normalIndices.push_back(normalIndex3);
				*/
			}

		}
		fclose(file);
		delete file;
	}
	else {
		//ERROR

		std::cout << "wat";
		return;

	}

}

HRESULT RenderObject::SetupVertexBuffers(DX::DeviceResources* dresources)
{

	D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
	vertexBufferData.pSysMem = &verts[0];
	vertexBufferData.SysMemPitch = 0;
	vertexBufferData.SysMemSlicePitch = 0;
	CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(DX11UWA::VertexPositionColor) * verts.size(), D3D11_BIND_VERTEX_BUFFER);
	return dresources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &vertexBuffer.p);

}

HRESULT RenderObject::SetupIndexBuffer(DX::DeviceResources * dresources)
{

	D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
	indexBufferData.pSysMem = &triangles[0];
	indexBufferData.SysMemPitch = 0;
	indexBufferData.SysMemSlicePitch = 0;
	CD3D11_BUFFER_DESC indexBufferDesc(sizeof(unsigned short) * triangles.size(), D3D11_BIND_INDEX_BUFFER);
	return dresources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &indexBuffer.p);

}


