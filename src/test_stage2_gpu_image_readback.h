#pragma once
#include "d3d11_renderer.h"
#include <stdexcept>
#include <vector>

namespace TestStage2GpuReadback {
struct Image {uint32_t width=0,height=0;std::vector<uint32_t> rgba;};
inline Image Read(ID3D11ShaderResourceView* view){
    if(!view)throw std::runtime_error("No successfully presented GPU image");
    ComPtr<ID3D11Resource> resource;view->GetResource(resource.GetAddressOf());
    ComPtr<ID3D11Texture2D> source;
    if(FAILED(resource.As(&source)))throw std::runtime_error("Presented image is not a texture");
    D3D11_TEXTURE2D_DESC desc{};source->GetDesc(&desc);
    if(desc.Format!=DXGI_FORMAT_R8G8B8A8_UNORM||desc.SampleDesc.Count!=1u)
        throw std::runtime_error("Unsupported presented readback format");
    Image result{desc.Width,desc.Height,std::vector<uint32_t>(size_t(desc.Width)*desc.Height)};
    desc.Usage=D3D11_USAGE_STAGING;desc.BindFlags=0;desc.CPUAccessFlags=D3D11_CPU_ACCESS_READ;desc.MiscFlags=0;
    ComPtr<ID3D11Device> device;source->GetDevice(device.GetAddressOf());
    ComPtr<ID3D11Texture2D> staging;
    if(FAILED(device->CreateTexture2D(&desc,nullptr,staging.GetAddressOf())))throw std::runtime_error("Presented staging allocation failed");
    ComPtr<ID3D11DeviceContext> context;device->GetImmediateContext(context.GetAddressOf());
    context->CopyResource(staging.Get(),source.Get());D3D11_MAPPED_SUBRESOURCE mapped{};
    if(FAILED(context->Map(staging.Get(),0,D3D11_MAP_READ,0,&mapped)))throw std::runtime_error("Presented image readback failed");
    for(uint32_t y=0;y<result.height;++y){
        const auto* row=reinterpret_cast<const uint32_t*>(static_cast<const uint8_t*>(mapped.pData)+size_t(y)*mapped.RowPitch);
        for(uint32_t x=0;x<result.width;++x)result.rgba[size_t(y)*result.width+x]=row[x];
    }
    context->Unmap(staging.Get(),0);return result;
}
}
