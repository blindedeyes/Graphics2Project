#include "pch.h"
#include "RenderObject.h"

void RenderObject::LoadObjFile(char * path)
{
	
	FILE * file = fopen(path, "r");


	if (file != NULL) {
		while (true) {
			char header[128];
			int res = fscanf(file, "%s", header);

			//End of file check
			if (res = EOF) return;

			//read vertex
			if (strcmp(header, "v")==0)
			{
				
				DX11UWA::VertexPositionColor vert;
				std::fscanf(file, "%f %f %f\n", &vert.pos.x, &vert.pos.y, &vert.pos.z);
				verts.push_back(vert);
				
			}
			else if (strcmp(header, "vt") == 0)
			{
				DirectX::XMFLOAT2 _uv;
				fscanf(file, "%f %f\n", &_uv.x, &_uv.y);
				UV.push_back(_uv);
			}
			else if (strcmp(header, "vn") == 0) {
				//normals
				DirectX::XMFLOAT3 norm;
				fscanf(file, "%f %f %f\n", &norm.x, &norm.y, &norm.z);
				Normals.push_back(norm);
			}
			else if (strcmp(header, "f") == 0) {
				std::string vertex1, vertex2, vertex3;
				unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
				int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
				if (matches != 9) {
					printf("File can't be read by our simple parser : ( Try exporting with other options\n");
					return;
				}

				triangles.push_back(vertexIndex[0]);
				triangles.push_back(vertexIndex[1]);
				triangles.push_back(vertexIndex[2]);


				//FOR LATER. Not sure how to use this yet.
				/*
				vertexIndices.push_back(vertexIndex[0]);
				vertexIndices.push_back(vertexIndex[1]);
				vertexIndices.push_back(vertexIndex[2]);
				uvIndices.push_back(uvIndex[0]);
				uvIndices.push_back(uvIndex[1]);
				uvIndices.push_back(uvIndex[2]);
				normalIndices.push_back(normalIndex[0]);
				normalIndices.push_back(normalIndex[1]);
				normalIndices.push_back(normalIndex[2]);
				*/
			}
			
		}
		fclose(file);
		delete file;
	}
	else {
		std::cout << "Error loading: " << path;
	}

}


