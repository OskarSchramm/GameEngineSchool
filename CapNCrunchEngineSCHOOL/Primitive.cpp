#include "Primitive.h"
#include "Vertex.h"
#include <d3d11.h>

#pragma comment(lib, "D3DCompiler.lib")
#include <d3dcompiler.h>

#include <string>

Primitive::Primitive(ID3D11Device* aDevice, ID3D11DeviceContext* aDeviceContext)
	: myDevicePtr(aDevice), myDeviceContextPtr(aDeviceContext)
{}

Primitive::~Primitive()
{
	Shutdown();
}

bool Primitive::Init(Vertex* vertices, UINT aVertexCount, UINT* indices, UINT aIndexCount)
{
	myIndexCount = aIndexCount;

	HRESULT res;
	D3D11_SUBRESOURCE_DATA myVertexData;
	D3D11_SUBRESOURCE_DATA myIndexData;

	myVertexData.pSysMem = vertices;
	myIndexData.pSysMem = indices;

	D3D11_BUFFER_DESC vertexDesc = {};
	vertexDesc.ByteWidth = aVertexCount * sizeof(Vertex);
	vertexDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	res = myDevicePtr->CreateBuffer(&vertexDesc, &myVertexData, &myVertexBuffer);
	if (FAILED(res))
		return false;

	D3D11_BUFFER_DESC indexDesc = {};
	indexDesc.ByteWidth = aIndexCount * sizeof(UINT);
	indexDesc.Usage = D3D11_USAGE_DEFAULT;
	indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;;
	res = myDevicePtr->CreateBuffer(&indexDesc, &myIndexData, &myIndexBuffer);
	if (FAILED(res))
		return false;

	return true;
}

bool Primitive::SetVertexShader(const LPCWSTR aShaderFilename)
{
	HRESULT res;

	ID3DBlob* myVertexBlob;

	std::wstring shaderFilenameCso(aShaderFilename);
	std::wstring shaderFilenameHlsl(aShaderFilename);
	shaderFilenameCso = shaderFilenameCso + L".cso";
	shaderFilenameHlsl = shaderFilenameHlsl + L".hlsl";
	res = D3DReadFileToBlob(shaderFilenameCso.c_str(), &myVertexBlob);
	if (FAILED(res))
	{
		res = D3DCompileFromFile(shaderFilenameHlsl.c_str(), NULL, NULL, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &myVertexBlob, nullptr);
		if (FAILED(res))
			return false;
	}

	res = myDevicePtr->CreateVertexShader(myVertexBlob->GetBufferPointer(), myVertexBlob->GetBufferSize(), nullptr, &myVertexShader);
	if (FAILED(res))
		return false;

	D3D11_INPUT_ELEMENT_DESC polygonLayout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	UINT numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	res = myDevicePtr->CreateInputLayout(polygonLayout, numElements, myVertexBlob->GetBufferPointer(), myVertexBlob->GetBufferSize(), &myInputLayout);
	if (FAILED(res))
		return false;

	myVertexBlob->Release();

	SetIAStuff();

	return true;
}

bool Primitive::SetPixelShader(const LPCWSTR aShaderFilename)
{
	HRESULT res;
	ID3DBlob* myPixelBlob;

	std::wstring shaderFilenameCso(aShaderFilename);
	std::wstring shaderFilenameHlsl(aShaderFilename);
	shaderFilenameCso = shaderFilenameCso + L".cso";
	shaderFilenameHlsl = shaderFilenameHlsl + L".hlsl";
	res = D3DReadFileToBlob(shaderFilenameCso.c_str(), &myPixelBlob);
	if (FAILED(res))
	{
		res = D3DCompileFromFile(shaderFilenameHlsl.c_str(), NULL, NULL, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &myPixelBlob, nullptr);
		if (FAILED(res))
			return false;
	}

	res = myDevicePtr->CreatePixelShader(myPixelBlob->GetBufferPointer(), myPixelBlob->GetBufferSize(), nullptr, &myPixelShader);
	if (FAILED(res))
		return false;

	myPixelBlob->Release();

	return true;
}

void Primitive::SetIAStuff()
{
	myDeviceContextPtr->IASetInputLayout(myInputLayout);
	myDeviceContextPtr->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Primitive::Render()
{
	UINT strides = sizeof(Vertex);
	UINT offset = 0;

	myDeviceContextPtr->IASetVertexBuffers(0, 1, &myVertexBuffer, &strides, &offset);
	myDeviceContextPtr->IASetIndexBuffer(myIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	myDeviceContextPtr->VSSetShader(myVertexShader, NULL, 0);
	myDeviceContextPtr->PSSetShader(myPixelShader, NULL, 0);

	myDeviceContextPtr->DrawIndexed(myIndexCount, 0, 0);
}

void Primitive::Shutdown()
{
	if (myDevicePtr)
	{
		myDevicePtr->Release();
		myDevicePtr = nullptr;
	}

	if (myDeviceContextPtr)
	{
		myDeviceContextPtr->Release();
		myDeviceContextPtr = nullptr;
	}

	if (myVertexBuffer)
	{
		myVertexBuffer->Release();
		myVertexBuffer = nullptr;
	}

	if (myIndexBuffer)
	{
		myIndexBuffer->Release();
		myIndexBuffer = nullptr;
	}

	if (myVertexShader)
	{
		myVertexShader->Release();
		myVertexShader = nullptr;
	}

	if (myPixelShader)
	{
		myPixelShader->Release();
		myPixelShader = nullptr;
	}

	if (myInputLayout)
	{
		myInputLayout->Release();
		myInputLayout = nullptr;
	}
}