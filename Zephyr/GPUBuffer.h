
#ifndef __INCLUDE_GPU_BUFFER_H
#define __INCLUDE_GPU_BUFFER_H

#include "d11.h"

class GPUBuffer
{
public:
	enum class BufferCreationFlags : UINT
	{
		structured_buffer = 0x00000001,
		append_consume_buffer = 0x00000002,

	};



	GPUBuffer();
	GPUBuffer(int bytewidth, int number_of_elements, void *initial_data, UINT creation_flags);

	void create_buffer(int bytewidth, int number_of_elements, void *initial_data, UINT creation_flags);


	void set_as_uav(int slot, shaderType type);
private:

	ID3D11Buffer *buffer_;
	ID3D11UnorderedAccessView *uav_;


};

inline UINT operator | (GPUBuffer::BufferCreationFlags lhs, GPUBuffer::BufferCreationFlags rhs)
{
	return UINT(lhs) | UINT(rhs);
}

inline UINT operator & (UINT lhs, GPUBuffer::BufferCreationFlags rhs)
{
	return UINT(lhs) & UINT(rhs);
}


#endif