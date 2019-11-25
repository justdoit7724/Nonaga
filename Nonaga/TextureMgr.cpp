#include "TextureMgr.h"
#include <DirectXTK/inc/WICTextureLoader.h>

TextureMgr::~TextureMgr()
{
	for (auto e : SRVs) {

		e.second->Release();
	}
}

int CalculateMaxMiplevel(int width, int height)
{
	return fmaxf(log2(fmaxf(width, height)),1);
}
void TextureMgr::Load(std::string key, std::string fileName)
{
	ComPtr<ID3D11Resource> ori_resources;
	D3D11_TEXTURE2D_DESC ori_desc;
	r_assert(
		DirectX::CreateWICTextureFromFile(
			DX_Device,
			std::wstring(fileName.begin(), fileName.end()).c_str(),
			ori_resources.GetAddressOf(),
			nullptr)
	);

	ComPtr<ID3D11Texture2D> ori_tex = nullptr;
	r_assert(
		ori_resources.Get()->QueryInterface(__uuidof(ID3D11Texture2D), (void**)ori_tex.GetAddressOf())
	);
	ori_tex->GetDesc(&ori_desc);

	UINT miplevel = CalculateMaxMiplevel(ori_desc.Width, ori_desc.Height);

	D3D11_TEXTURE2D_DESC newTex_desc;
	newTex_desc.Format = ori_desc.Format;
	newTex_desc.ArraySize = 1;
	newTex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	newTex_desc.CPUAccessFlags = 0;
	newTex_desc.Width = ori_desc.Width;
	newTex_desc.Height = ori_desc.Height;
	newTex_desc.MipLevels = miplevel;
	newTex_desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	newTex_desc.SampleDesc.Count = 1;
	newTex_desc.SampleDesc.Quality = 0;
	newTex_desc.Usage = D3D11_USAGE_DEFAULT;

	ComPtr<ID3D11Texture2D> newTex;
	r_assert(
		DX_Device->CreateTexture2D(
			&newTex_desc, nullptr, newTex.GetAddressOf())
	);

	ComPtr<ID3D11Texture2D> stagTex;
	D3D11_TEXTURE2D_DESC stagDesc = ori_desc;
	stagDesc.BindFlags = 0;
	stagDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	stagDesc.Usage = D3D11_USAGE_STAGING;
	r_assert(DX_Device->CreateTexture2D(&stagDesc, nullptr, &stagTex));

	DX_DContext->CopyResource(stagTex.Get(), ori_tex.Get());

	D3D11_MAPPED_SUBRESOURCE mapped;
	r_assert(DX_DContext->Map(stagTex.Get(), 0, D3D11_MAP_READ, 0, &mapped));
	// use 'UINT' because format of images from file is one of 8888
	UINT* arr = new UINT[(mapped.RowPitch / sizeof(UINT)) * ori_desc.Height];
	ZeroMemory(arr, mapped.RowPitch * ori_desc.Height);
	CopyMemory(arr, mapped.pData, mapped.RowPitch * ori_desc.Height);
	DX_DContext->Unmap(stagTex.Get(), 0);

	DX_DContext->UpdateSubresource(
		newTex.Get(), 0,
		nullptr,
		arr,
		mapped.RowPitch, mapped.DepthPitch);
	delete[] arr;
	


	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = ori_desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = miplevel;
	srvDesc.Texture2D.MostDetailedMip = 0;
	ID3D11ShaderResourceView* newSRV = nullptr;
	r_assert(DX_Device->CreateShaderResourceView(newTex.Get(), &srvDesc, &newSRV));
	DX_DContext->GenerateMips(newSRV);

	SRVs.insert(std::pair<std::string, ID3D11ShaderResourceView*>(key, newSRV));
}


void TextureMgr::LoadArray(std::string key,std::wstring folderName, std::vector<std::string> fileNames)
{
	const UINT spriteCount = fileNames.size();

	std::vector<ComPtr<ID3D11Resource>> ori_resources(spriteCount);
	D3D11_TEXTURE2D_DESC ori_desc;
	D3D11_TEXTURE2D_DESC prev_desc;
	for (int i = 0; i < spriteCount; ++i)
	{
		ID3D11Resource* newResource = nullptr;
		r_assert(
			DirectX::CreateWICTextureFromFile(
				DX_Device,
				(folderName+std::wstring(fileNames[i].begin(), fileNames[i].end())).c_str(),
				&newResource,
				nullptr)
		);

		ori_resources[i] = newResource;
		ID3D11Texture2D* newTex = nullptr; 
		r_assert(
			newResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&newTex)
		);
		newTex->GetDesc(&ori_desc);
		if (i!=0)
		{
			assert(ori_desc.Width == prev_desc.Width && ori_desc.Height == prev_desc.Height);
		}
		prev_desc = ori_desc;
	}
	
	UINT miplevel = CalculateMaxMiplevel(ori_desc.Width, ori_desc.Height);

	D3D11_TEXTURE2D_DESC arr_desc;
	arr_desc.Format = ori_desc.Format;
	arr_desc.ArraySize = spriteCount;
	arr_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	arr_desc.CPUAccessFlags = 0;
	arr_desc.Width = ori_desc.Width;
	arr_desc.Height = ori_desc.Height;
	arr_desc.MipLevels = miplevel;
	arr_desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	arr_desc.SampleDesc.Count = 1;
	arr_desc.SampleDesc.Quality = 0;
	arr_desc.Usage = D3D11_USAGE_DEFAULT;

	ComPtr<ID3D11Texture2D> arrTex;
	r_assert(
		DX_Device->CreateTexture2D(
			&arr_desc, nullptr, arrTex.GetAddressOf())
	);

	for (int i = 0; i < spriteCount; ++i)
	{
		ComPtr<ID3D11Texture2D> stagTex;
		D3D11_TEXTURE2D_DESC stagDesc = ori_desc;
		stagDesc.BindFlags = 0;
		stagDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		stagDesc.Usage = D3D11_USAGE_STAGING;
		r_assert(DX_Device->CreateTexture2D(&stagDesc, nullptr, &stagTex));

		// use staging texture to read and paste image
		// because for copying, we should match miplevels between dest and src
		DX_DContext->CopyResource(stagTex.Get(), ori_resources[i].Get());

		D3D11_MAPPED_SUBRESOURCE mapped;
		r_assert(DX_DContext->Map(stagTex.Get(), 0, D3D11_MAP_READ, 0, &mapped));
		// use 'UINT' because format of images from file is one of 8888
		UINT* arr = new UINT[(mapped.RowPitch/sizeof(UINT)) * ori_desc.Height];
		ZeroMemory(arr, mapped.RowPitch * ori_desc.Height);
		CopyMemory(arr, mapped.pData, mapped.RowPitch * ori_desc.Height);
		DX_DContext->Unmap(stagTex.Get(), 0);
		
		DX_DContext->UpdateSubresource(
			arrTex.Get(), D3D11CalcSubresource(0, i, miplevel), 
			nullptr, 
			arr, 
			mapped.RowPitch, mapped.DepthPitch);
		delete[] arr;
	}

	
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = ori_desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Texture2DArray.ArraySize = spriteCount;
	srvDesc.Texture2DArray.FirstArraySlice = 0;
	srvDesc.Texture2DArray.MipLevels = miplevel;
	srvDesc.Texture2DArray.MostDetailedMip = 0;
	ID3D11ShaderResourceView* integratedSRV = nullptr;
	r_assert(DX_Device->CreateShaderResourceView(arrTex.Get(), &srvDesc, &integratedSRV));
	DX_DContext->GenerateMips(integratedSRV);
	

	SRVs.insert(std::pair<std::string, ID3D11ShaderResourceView*>(key, integratedSRV));

}

void TextureMgr::LoadCM(std::string key, const std::vector<std::string>& fileNames)
{
	ComPtr<ID3D11Resource> ori_resources[6];
	D3D11_TEXTURE2D_DESC ori_desc;
	D3D11_TEXTURE2D_DESC prev_desc;
	for (int i = 0; i < 6; ++i)
	{
		r_assert(
			DirectX::CreateWICTextureFromFile(
				DX_Device,
				std::wstring(fileNames[i].begin(), fileNames[i].end()).c_str(),
				ori_resources[i].GetAddressOf(),
				nullptr)
		);

		ID3D11Texture2D* newTex = nullptr;
		r_assert(
			ori_resources[i].Get()->QueryInterface(__uuidof(ID3D11Texture2D), (void**)& newTex)
		);
		newTex->GetDesc(&ori_desc);
		if (i != 0)
		{
			assert(ori_desc.Width == prev_desc.Width && ori_desc.Height == prev_desc.Height);
		}
		prev_desc = ori_desc;
	}

	D3D11_TEXTURE2D_DESC cm_desc;
	cm_desc.Format = ori_desc.Format;
	cm_desc.ArraySize = 6;
	cm_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	cm_desc.CPUAccessFlags = 0;
	cm_desc.Width = ori_desc.Width;
	cm_desc.Height = ori_desc.Height;
	cm_desc.MipLevels = 1;
	cm_desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	cm_desc.SampleDesc.Count = 1;
	cm_desc.SampleDesc.Quality = 0;
	cm_desc.Usage = D3D11_USAGE_DEFAULT;

	ComPtr<ID3D11Texture2D> cmTex;
	r_assert(
		DX_Device->CreateTexture2D(
			&cm_desc, nullptr, cmTex.GetAddressOf())
	);

	for (int i = 0; i < 6; ++i)
	{
		DX_DContext->CopySubresourceRegion(
			cmTex.Get(), i,
			0,0,0,
			ori_resources[i].Get(), 0,
			nullptr);
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = ori_desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = 1;
	srvDesc.TextureCube.MostDetailedMip = 0;
	ID3D11ShaderResourceView* integratedSRV = nullptr;
	r_assert(DX_Device->CreateShaderResourceView(cmTex.Get(), &srvDesc, &integratedSRV));

	SRVs.insert(std::pair<std::string, ID3D11ShaderResourceView*>(key, integratedSRV));
}

ID3D11ShaderResourceView* TextureMgr::Get(std::string key)
{
	assert(SRVs.find(key) != SRVs.end());

	return SRVs[key];
}
//
//ID3D11Texture2D* TextureMgr::GetTexture(std::string fileName)
//{
//	assert(SRVs.find(fileName) != SRVs.end());
//
//	ID3D11Resource* resource=nullptr;
//	SRVs[fileName].srv->GetResource(&resource);
//	ID3D11Texture2D* tex=nullptr;
//	r_assert( resource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&tex) );
//	return tex;
//}


