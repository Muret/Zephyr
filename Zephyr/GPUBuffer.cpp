
#include "GPUBuffer.h"

GPUBuffer::GPUBuffer()
{
	buffer_ = nullptr;
	uav_ = nullptr;
}

GPUBuffer::GPUBuffer(int bytewidth, int number_of_elements, void * initial_data, UINT creation_flags)
{
	create_buffer(bytewidth, number_of_elements, initial_data, creation_flags);
}

void GPUBuffer::create_buffer(int bytewidth, int number_of_elements, void *initial_data, UINT creation_flags)
{
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = bytewidth * number_of_elements;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

	desc.MiscFlags = 0;

	if (creation_flags & (UINT)BufferCreationFlags::structured_buffer)
	{
		desc.MiscFlags |= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
	}

	desc.StructureByteStride = bytewidth;

	D3D11_SUBRESOURCE_DATA init_data;
	init_data.pSysMem = initial_data;
	init_data.SysMemPitch = 0;
	init_data.SysMemSlicePitch = 0;

	HRESULT res = g_device->CreateBuffer(&desc, &init_data, &buffer_);
	_ASSERT(SUCCEEDED(res));

	D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc;
	ZeroMemory(&uav_desc, sizeof(uav_desc));
	uav_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uav_desc.Buffer.FirstElement = 0;

	if (creation_flags & (UINT)BufferCreationFlags::structured_buffer)
	{
		// This is a Structured Buffer
		uav_desc.Format = DXGI_FORMAT_UNKNOWN;      // Format must be must be DXGI_FORMAT_UNKNOWN, when creating a View of a Structured Buffer
		uav_desc.Buffer.NumElements = number_of_elements;
	}
	
	if (creation_flags & (UINT)BufferCreationFlags::append_consume_buffer)
	{
		uav_desc.Buffer.Flags |= D3D11_BUFFER_UAV_FLAG_APPEND;
	}

	res = g_device->CreateUnorderedAccessView(buffer_, &uav_desc, &uav_);
	_ASSERT(SUCCEEDED(res));
}

void GPUBuffer::set_as_uav(int slot, shaderType type)
{
	if (type == shader_type_pixel)
	{
		SetUAVToPixelShader(uav_, slot);
	}
	else if (type == shader_type_compute)
	{
		g_deviceContext->CSSetUnorderedAccessViews(slot, 1, &uav_, NULL);
	}
	else
	{
		_ASSERT(false);
	}
}
