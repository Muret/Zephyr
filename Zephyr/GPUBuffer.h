
#ifndef __INCLUDE_GPU_BUFFER_H
#define __INCLUDE_GPU_BUFFER_H

#include "d11.h"

class GPUBuffer
{
public:
	enum class CreationFlags : UINT
	{
		structured_buffer = 0x00000001,
		append_consume_buffer = 0x00000002,
		cpu_write_acces = 0x00000004,
		constant_buffer = 0x00000008,
		has_atomic_counter = 0x0000010,
		staging = 0x0000020,
	};



	GPUBuffer();
	GPUBuffer(int bytewidth, int number_of_elements, void *initial_data, UINT creation_flags);

	void create_buffer(int bytewidth, int number_of_elements, void *initial_data, UINT creation_flags);

	void update_data(void *data, int size);

	void set_as_uav(int slot, shaderType type, unsigned int initial_count = -1);
	
	int get_current_element_count() const;

	void set_as_constant_buffer(int slot);

	void validate_element_count_fetch_staging_buffer() const;

	ID3D11Buffer *get_buffer() const;

	void get_data(void *data, unsigned int length);
private:

	ID3D11Buffer *buffer_;
	ID3D11UnorderedAccessView *uav_;

	UINT creation_flags_;

	mutable GPUBuffer *element_count_fetch_staging_buffer_;

};

inline UINT operator | (GPUBuffer::CreationFlags lhs, GPUBuffer::CreationFlags rhs)
{
	return UINT(lhs) | UINT(rhs);
}

inline UINT operator & (UINT lhs, GPUBuffer::CreationFlags rhs)
{
	return UINT(lhs) & UINT(rhs);
}


#endif