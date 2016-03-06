
#include "GPUBuffer.h"

GPUBuffer::GPUBuffer()
{
	buffer_ = nullptr;
	uav_ = nullptr;
	element_count_fetch_staging_buffer_ = nullptr;

	creation_flags_ = 0;
}

GPUBuffer::GPUBuffer(int bytewidth, int number_of_elements, void * initial_data, UINT creation_flags)
{
	create_buffer(bytewidth, number_of_elements, initial_data, creation_flags);
}

void GPUBuffer::create_buffer(int bytewidth, int number_of_elements, void *initial_data, UINT creation_flags)
{
	creation_flags_ = creation_flags;

	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = bytewidth * number_of_elements;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

	if (creation_flags & (UINT)CreationFlags::cpu_write_acces)
	{
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}

	if (creation_flags & (UINT)CreationFlags::staging)
	{
		desc.Usage = D3D11_USAGE_STAGING;
		desc.BindFlags = 0;
	}

	desc.MiscFlags = 0;

	if (creation_flags & (UINT)CreationFlags::structured_buffer)
	{
		desc.MiscFlags |= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
	}
	
	if (creation_flags & (UINT)CreationFlags::constant_buffer)
	{
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.Usage = D3D11_USAGE_DYNAMIC;
	}

	desc.StructureByteStride = bytewidth;

	D3D11_SUBRESOURCE_DATA init_data;
	init_data.pSysMem = initial_data;
	init_data.SysMemPitch = 0;
	init_data.SysMemSlicePitch = 0;

	HRESULT res = g_device->CreateBuffer(&desc, initial_data ? &init_data : NULL, &buffer_);
	_ASSERT(SUCCEEDED(res));

	uav_ = nullptr;

	if (creation_flags & (UINT)CreationFlags::structured_buffer)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc;
		ZeroMemory(&uav_desc, sizeof(uav_desc));
		uav_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uav_desc.Buffer.FirstElement = 0;

		// This is a Structured Buffer
		uav_desc.Format = DXGI_FORMAT_UNKNOWN;      // Format must be must be DXGI_FORMAT_UNKNOWN, when creating a View of a Structured Buffer
		uav_desc.Buffer.NumElements = number_of_elements;

		if (creation_flags & (UINT)CreationFlags::append_consume_buffer)
		{
			uav_desc.Buffer.Flags |= D3D11_BUFFER_UAV_FLAG_APPEND;
		}

		if (creation_flags & (UINT)CreationFlags::has_atomic_counter)
		{
			uav_desc.Buffer.Flags |= D3D11_BUFFER_UAV_FLAG_COUNTER;
		}

		res = g_device->CreateUnorderedAccessView(buffer_, &uav_desc, &uav_);
		_ASSERT(SUCCEEDED(res));
	}
}

void GPUBuffer::update_data(void * data, int size)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	float* dataPtr;

	// Lock the constant buffer so it can be written to.
	result = g_deviceContext->Map(buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		_ASSERT(false);
		return;
	}

	// Get a pointer to the data in the constant buffer.
	dataPtr = (float*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	memcpy(dataPtr, data, size);

	// Unlock the constant buffer.
	g_deviceContext->Unmap(buffer_, 0);

}

void GPUBuffer::set_as_uav(int slot, shaderType type, unsigned int initial_count /*= -1*/)
{
	_ASSERT(uav_ != nullptr);

	if (type == shader_type_pixel)
	{
		SetUAVToPixelShader(uav_, slot, initial_count);
	}
	else if (type == shader_type_compute)
	{
		g_deviceContext->CSSetUnorderedAccessViews(slot, 1, &uav_, &initial_count);
	}
	else
	{
		_ASSERT(false);
	}
}

int GPUBuffer::get_current_element_count() const
{
	if (creation_flags_ & (CreationFlags::append_consume_buffer | CreationFlags::has_atomic_counter))
	{
		validate_element_count_fetch_staging_buffer();

		CopyStructureCount(element_count_fetch_staging_buffer_->get_buffer(), 0, uav_);

		int count = 0;
		element_count_fetch_staging_buffer_->get_data(&count, 4);
		return count;
	}

	return 0;
}

void GPUBuffer::set_as_constant_buffer(int slot)
{
	_ASSERT(creation_flags_ & CreationFlags::constant_buffer);

	SetConstantBufferToSlot(slot, buffer_);
}

void GPUBuffer::validate_element_count_fetch_staging_buffer() const 
{
	if (element_count_fetch_staging_buffer_ == nullptr)
	{
		element_count_fetch_staging_buffer_ = new GPUBuffer(4, 1, nullptr, (UINT)CreationFlags::staging);
	}
}

ID3D11Buffer* GPUBuffer::get_buffer() const
{
	return buffer_;
}

void GPUBuffer::get_data(void * data, unsigned int length)
{
	void *mapped_m = MapBuffer(buffer_);
	memcpy(data, mapped_m, length);
	UnMapBuffer(buffer_);
}
