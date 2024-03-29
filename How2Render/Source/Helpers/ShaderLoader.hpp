#pragma once

#include <d3dcompiler.h>

namespace h2r
{

	inline HRESULT CompileShaderFromFile(
		const WCHAR *filename, D3D_SHADER_MACRO const* pDefines, LPCSTR entryPoint, LPCSTR shaderModel, ID3DBlob **ppBlobOut)
	{
		HRESULT hr = S_OK;

		DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
		// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
		// Setting this flag improves the shader debugging experience, but still allows
		// the shaders to be optimized and to run exactly the way they will run in
		// the release configuration of this program.
		dwShaderFlags |= D3DCOMPILE_DEBUG;

		// Disable optimizations to further improve shader debugging
		dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		ID3DBlob *pErrorBlob = nullptr;
		hr = D3DCompileFromFile(filename, pDefines, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, shaderModel,
								dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

		if (FAILED(hr))
		{
			if (pErrorBlob)
			{
				OutputDebugStringA(reinterpret_cast<const char *>(pErrorBlob->GetBufferPointer()));
				pErrorBlob->Release();
			}
			return hr;
		}

		if (pErrorBlob)
		{
			pErrorBlob->Release();
		}

		return S_OK;
	}

} // namespace h2r