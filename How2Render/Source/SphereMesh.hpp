#pragma once
#include "Wrapper/Context.hpp"
#include "Math.hpp"
#include "SinCosTable.hpp"

struct Vertex
{
	XMFLOAT3 pos;
	XMFLOAT3 normal;
	XMFLOAT2 texCoord;
};

struct Face
{
	uint16_t v[3];

	Face() = default;
	Face(uint16_t _0, uint16_t _1, uint16_t _2)
	{
		v[0] = _0;
		v[1] = _1;
		v[2] = _2;
	}
};

struct Mesh
{
	ID3D11Buffer *vertexBuffer;
	ID3D11Buffer *indexBuffer;
	uint32_t numIndices = 0;
	DXGI_FORMAT indexFormat = DXGI_FORMAT_R16_UINT;
	D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
};

inline uint16_t CalcVertexIndex(uint32_t slices, int slice, int stack)
{
	const uint32_t index = stack * slices + slice + 1;
	assert(index < USHRT_MAX);
	return (uint16_t)index;
}

void SpherePole(float radius, Vertex *vert)
{
	vert->pos.x = 0.f;
	vert->pos.y = radius;
	vert->pos.z = 0.f;
	vert->normal.x = 0.f;
	vert->normal.y = (radius > 0.f) ? 1.f : -1.f;
	vert->normal.z = 0.f;
	const float theta = atan2f(vert->normal.z, vert->normal.x);
	const float phi = acosf(-vert->normal.y);
	vert->texCoord.x = (theta + XM_PI) * XM_1DIV2PI;
	vert->texCoord.y = phi * XM_1DIVPI;
}

std::tuple<bool, Mesh> GenerateSphere(Context const& context, float radius, uint32_t slices, uint32_t stacks)
{
	Mesh sphere;

	if (radius <= 0.f || (slices < 3) || (stacks < 3))
		return {false, sphere};

	const uint32_t numVertices = 2 + slices * (stacks - 1);
	const uint32_t numFaces = 2 * slices + (stacks - 2) * (2 * slices);

	std::vector<Vertex> vertices(numVertices);
	std::vector<Face> faces(numFaces);

	// phi = angle on xz plane wrt z axis
	const float phiStep = -XM_2PI/slices;
	const float phiStart = XM_PIDIV2;

	auto [result, phi] = CreateSinCosTable(phiStart, phiStep, slices);
	if (!result)
		return {false, sphere};

	// theta = angle on xy plane wrt x axis
	const float thetaStep = XM_PI / stacks;
	float theta = thetaStep;

	Vertex *currVert = vertices.data();
	Face *currFace = faces.data();

	SpherePole(radius, currVert);
	++currVert;

	uint32_t stack, slice;
	for (stack = 0; stack < stacks - 1; ++stack)
	{
		float sinTheta;
		float cosTheta;
		XMScalarSinCosEst(&sinTheta, &cosTheta, theta);

		for (slice = 0; slice < slices; ++slice)
		{
			currVert->pos.x = radius * sinTheta * phi.pCos[slice];
			currVert->pos.z = radius * sinTheta * phi.pSin[slice];
			currVert->pos.y = radius * cosTheta;
			currVert->normal.x = sinTheta * phi.pCos[slice];
			currVert->normal.z = sinTheta * phi.pSin[slice];
			currVert->normal.y = cosTheta;
			const float arcTan = atan2f(currVert->normal.z, currVert->normal.x);
			const float arcCos = acosf(-currVert->normal.y);
			currVert->texCoord.x = (arcTan + XM_PI) * XM_1DIV2PI;
			currVert->texCoord.y = arcCos * XM_1DIVPI;
			++currVert;

			if (slice > 0)
			{
				if (0 == stack)
				{
					// Top stack is triangle fan
					currFace->v[0] = 0;
					currFace->v[2] = slice + 1;
					currFace->v[1] = slice;
					++currFace;
				}
				else
				{
					// Stacks in between top and bottom are quad strips
					currFace->v[0] = CalcVertexIndex(slices, slice - 1, stack - 1);
					currFace->v[2] = CalcVertexIndex(slices, slice, stack - 1);
					currFace->v[1] = CalcVertexIndex(slices, slice - 1, stack);
					++currFace;
					currFace->v[0] = CalcVertexIndex(slices, slice, stack - 1);
					currFace->v[2] = CalcVertexIndex(slices, slice, stack);
					currFace->v[1] = CalcVertexIndex(slices, slice - 1, stack);
					++currFace;
				}
			}
		}

		theta += thetaStep;

		if (0 == stack)
		{
			currFace->v[0] = 0;
			currFace->v[2] = 1;
			currFace->v[1] = slice;
			++currFace;
		}
		else
		{
			currFace->v[0] = CalcVertexIndex(slices, slice - 1, stack - 1);
			currFace->v[2] = CalcVertexIndex(slices, 0, stack - 1);
			currFace->v[1] = CalcVertexIndex(slices, slice - 1, stack);
			++currFace;
			currFace->v[0] = CalcVertexIndex(slices, 0, stack - 1);
			currFace->v[2] = CalcVertexIndex(slices, 0, stack);
			currFace->v[1] = CalcVertexIndex(slices, slice - 1, stack);
			++currFace;
		}
	}

	SpherePole(-radius, currVert);

	// Bottom stack is triangle fan
	for (slice = 1; slice < slices; ++slice)
	{
		currFace->v[0] = CalcVertexIndex(slices, slice - 1, stack - 1);
		currFace->v[2] = CalcVertexIndex(slices, slice, stack - 1);
		currFace->v[1] = numVertices - 1;
		++currFace;
	}

	currFace->v[0] = CalcVertexIndex(slices, slice - 1, stack - 1);
	currFace->v[2] = CalcVertexIndex(slices, 0, stack - 1);
	currFace->v[1] = numVertices - 1;

	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.ByteWidth = sizeof(Vertex) * numVertices;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initialData;
	initialData.pSysMem = vertices.data();
	initialData.SysMemPitch = 0;
	initialData.SysMemSlicePitch = 0;

	auto hr = context.pd3dDevice->CreateBuffer(&bufferDesc, &initialData, &sphere.vertexBuffer);
	assert(SUCCEEDED(hr));

	bufferDesc.ByteWidth = sizeof(Face) * numFaces;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	initialData.pSysMem = faces.data();

	hr = context.pd3dDevice->CreateBuffer(&bufferDesc, &initialData, &sphere.indexBuffer);
	assert(SUCCEEDED(hr));

	sphere.numIndices = numFaces * 3;

	CleanupSinCosTable(phi);
	return {true, sphere};
}

void CleanupMesh(Mesh& mesh)
{
	if (mesh.vertexBuffer != nullptr)
	{
		mesh.vertexBuffer->Release();
		mesh.vertexBuffer = nullptr;
	}
	if (mesh.indexBuffer != nullptr)
	{
		mesh.indexBuffer->Release();
		mesh.indexBuffer = nullptr;
	}
	mesh.numIndices = 0;
}
