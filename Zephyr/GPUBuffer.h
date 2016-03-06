
#ifndef __INCLUDE_GPU_BUFFER_H
#define __INCLUDE_GPU_BUFFER_H

#include "d11.h"

class GPUBuffer
{
public:




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



#endif