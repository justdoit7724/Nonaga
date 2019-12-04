#include "Cylinder.h"
#include "ShaderFormat.h"

Cylinder::Cylinder(const int sliceCount)
	:Shape()
{
	assert(sliceCount >= 3);

	const float smoothOffsetSide = 0.8f;
	const float smoothOffsetCap = 0.95f;
	float dTheta = XM_2PI / sliceCount;
	float hHeight = 1.0f;

#pragma region cap
	// top
	std::vector<Vertex> capTopVertice;
	std::vector<UINT> capTopIndice;
	for (int i = 0; i < sliceCount + 1; ++i)
	{
		float x = 0.5f * cosf(i*dTheta)*smoothOffsetCap;
		float z = 0.5f * sinf(i*dTheta)*smoothOffsetCap;
		float u = x / 2.0f + 0.5f;
		float v = z / 2.0f + 0.5f;

		Vertex vertex;
		XMFLOAT3 pos = XMFLOAT3(x, hHeight, z);
		vertex.pos = pos;
		pos.y = 0;
		vertex.n = Normalize(UP+pos*3.5f);
		vertex.tex = XMFLOAT2(u, v);
		capTopVertice.push_back(vertex);
	}

	Vertex vertex;
	vertex.pos = XMFLOAT3(0, hHeight, 0);
	vertex.n = UP;
	vertex.tex = XMFLOAT2(0.5f, 0.5f);
	capTopVertice.push_back(vertex);

	int centerIdx = capTopVertice.size()-1;
	for (int i = 0; i < sliceCount; ++i)
	{
		capTopIndice.push_back(centerIdx);
		capTopIndice.push_back(i + 1);
		capTopIndice.push_back(i);
	}

	//bottom
	std::vector<Vertex> capBottomVertice;
	std::vector<UINT> capBottomIndice;
	for (int i = 0; i < sliceCount + 1; ++i)
	{
		float x = 0.5f * cosf(i*dTheta);
		float z = 0.5f * sinf(i*dTheta);
		float u = x / 2.0f + 0.5f;
		float v = z / 2.0f + 0.5f;

		Vertex vertex;
		vertex.pos = XMFLOAT3(x, -hHeight, z);
		vertex.n = -UP;
		vertex.tex = XMFLOAT2(u, v);
		capBottomVertice.push_back(vertex);
	}

	vertex.pos = XMFLOAT3(0, -hHeight, 0);
	vertex.n = -UP;
	vertex.tex = XMFLOAT2(0.5f, 0.5f);
	capBottomVertice.push_back(vertex);

	centerIdx = capBottomVertice.size() - 1;
	for (int i = 0; i < sliceCount; ++i)
	{
		capBottomIndice.push_back(centerIdx);
		capBottomIndice.push_back( i);
		capBottomIndice.push_back( i + 1);
	}
#pragma endregion

#pragma region smooth

	std::vector<Vertex> smoothTopVertice;
	std::vector<UINT> smoothTopIndice;
	for (int i = 0; i < sliceCount + 1; ++i)
	{
		float x = 0.5f * cosf(i * dTheta) * smoothOffsetCap;
		float z = 0.5f * sinf(i * dTheta) * smoothOffsetCap;
		float u = x / 2.0f + 0.5f;
		float v = z / 2.0f + 0.5f;

		Vertex vertex;
		XMFLOAT3 pos = XMFLOAT3(x, hHeight, z);
		vertex.pos = pos;
		pos.y = 0;
		vertex.n = Normalize(UP + pos * 3.5f);
		vertex.tex = XMFLOAT2(u, v);
		smoothTopVertice.push_back(vertex);
	}

	float y = smoothOffsetSide;
	for (int j = 0; j <= sliceCount; ++j) {
		Vertex vertex;
		float c = cosf(j * dTheta);
		float s = sinf(j * dTheta);
		XMFLOAT3 pos = XMFLOAT3(0.5f * c, y, 0.5f * s);
		vertex.pos = pos;
		vertex.tex.x = (0.5f * c) / 2.0f + 0.5f;
		vertex.tex.y = (0.5f * s) / 2.0f + 0.5f;

		pos.y = 0;
		vertex.n = Normalize(pos * 40 + UP);

		smoothTopVertice.push_back(vertex);
	}

	for (int i = 0; i < sliceCount; ++i)
	{
		smoothTopIndice.push_back(i);
		smoothTopIndice.push_back(i+1);
		smoothTopIndice.push_back(i+ sliceCount+1);
		smoothTopIndice.push_back(i+1);
		smoothTopIndice.push_back(i + sliceCount + 2);
		smoothTopIndice.push_back(i + sliceCount + 1);
	}
#pragma endregion


#pragma region side

	std::vector<Vertex> sideVertice;
	std::vector<UINT> sideIndice;
	y = hHeight * smoothOffsetSide;

	for (int j = 0; j <= sliceCount; ++j) {
		Vertex vertex;
		float c = cosf(j*dTheta);
		float s = sinf(j*dTheta);
		XMFLOAT3 pos = XMFLOAT3(0.5f * c, y, 0.5f * s);
		vertex.pos = pos;
		vertex.tex.x = float(j) / sliceCount;
		vertex.tex.y = 1.0f;

		pos.y = 0;
		vertex.n = Normalize(pos * 40 + UP);

		sideVertice.push_back(vertex);
	}
	y = -hHeight;
	for (int j = 0; j <= sliceCount; ++j) {
		Vertex vertex;
		float c = cosf(j * dTheta);
		float s = sinf(j * dTheta);
		vertex.pos = XMFLOAT3(0.5f * c, y, 0.5f * s);
		vertex.tex.x = float(j) / sliceCount;
		vertex.tex.y = 0;

		XMFLOAT3 tangent = XMFLOAT3(-s, 0, c);
		XMFLOAT3 bitangent = -UP;
		vertex.n = Cross(tangent, bitangent);

		sideVertice.push_back(vertex);
	}

	int ringVertexCount = sliceCount + 1;
	for (int i = 0; i < sliceCount; ++i)
	{
		sideIndice.push_back(i);
		sideIndice.push_back(i+1);
		sideIndice.push_back(i+ringVertexCount);
		sideIndice.push_back(i+1);
		sideIndice.push_back(i+ringVertexCount+1);
		sideIndice.push_back(i+ringVertexCount);
	}
#pragma endregion


	std::vector<Vertex> vertice;
	std::vector<UINT> indice;
	for (Vertex& cap : capTopVertice)
		vertice.push_back(cap);
	for (UINT cap : capTopIndice)
		indice.push_back(cap);
	int startIdx = vertice.size();
	for (Vertex& smooth : smoothTopVertice)
		vertice.push_back(smooth);
	for (UINT smooth : smoothTopIndice)
		indice.push_back(smooth + startIdx);
	startIdx = vertice.size();
	for (Vertex& side : sideVertice)
		vertice.push_back(side);
	for (UINT side : sideIndice)
		indice.push_back(side + startIdx);
	startIdx = vertice.size();
	for (Vertex& cap : capBottomVertice)
		vertice.push_back(cap);
	for (UINT cap : capBottomIndice)
		indice.push_back(cap + startIdx);

	int polySize = indice.size()/3;
	for (UINT64 i=0; i< polySize; ++i)
	{
		XMFLOAT3 tangent;
		Vertex v0 = vertice[indice[i * 3]];
		Vertex v1 = vertice[indice[i * 3 + 1]];
		Vertex v2 = vertice[indice[i * 3 + 2]];

		CalculateTangent(
			v0.pos,
			v1.pos,
			v2.pos,
			v0.tex,
			v1.tex,
			v2.tex, &tangent);
		tangent = Normalize(tangent);

		vertice[indice[i * 3]].tangent = tangent;
		vertice[indice[i * 3+1]].tangent = tangent;
		vertice[indice[i * 3+2]].tangent = tangent;
	}

	Init(vertice.data(), sizeof(Vertex), vertice.size(), indice.data(), indice.size(), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}



