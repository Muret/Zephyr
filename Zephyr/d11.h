
#ifndef _D11_H_
#define _D11_H_

#include "includes.h"
#include "pix3.h"

class Texture;

#define GPU_EVENT(A) PixEvent p_event(A)

struct PixEvent
{
	PixEvent(const char * format, ...);

	~PixEvent();
};




extern ID3D11DeviceContext *g_deviceContext;
extern ID3D11Device *g_device;
extern ID3DUserDefinedAnnotation *pPerf;

enum class CreationFlags : UINT
{
	structured_buffer		= 0x00000001,
	append_consume_buffer	= 0x00000002,
	cpu_write_acces			= 0x00000004,
	constant_buffer			= 0x00000008,
	has_atomic_counter		= 0x00000010,
	staging					= 0x00000020,
	vertex_buffer			= 0x00000040,
	index_buffer			= 0x00000080,
};


inline UINT operator | (CreationFlags lhs, CreationFlags rhs)
{
	return UINT(lhs) | UINT(rhs);
}

inline UINT operator & (UINT lhs, CreationFlags rhs)
{
	return UINT(lhs) & UINT(rhs);
}

struct DebugginEvent
{
	
	DebugginEvent(LPCWSTR name)
	{
		pPerf->BeginEvent( name );
	}
	~DebugginEvent()
	{
		pPerf->EndEvent( );

	}

};


enum class device_stencil_op
{
	keep = 0,
	zero,
	replace,
	increase_clamp,
	decrease_clamp,
	invert,
	increase,
	decrease,
	count,
};

D3D11_STENCIL_OP get_api_stencil_op(device_stencil_op op);

enum class device_comparison_func
{
	never = 0,
	less,
	equal,
	less_equal,
	greater,
	not_equal,
	greater_equal,
	always,
	count,
};

D3D11_COMPARISON_FUNC get_api_comparison_func(device_comparison_func op);

enum depthState
{
	depth_state_enable_test_enable_write,
	depth_state_enable_test_disable_write,
	depth_state_disable_test_enable_write,
	depth_state_disable_test_disable_write,
	number_of_depth_states
};

enum blendState
{
	blend_state_disable_color_write,
	blend_state_enable_color_write,
	blend_state_alpha_blend_add,
	number_of_blend_states
};

enum rasterState
{
	raster_state_fill_mode,
	raster_state_wireframe_mode,
	number_of_raster_states
};


enum class shaderType : unsigned long long
{
	vertex = 0,
	pixel,
	compute,
	count,
};


using namespace std;

//full screen texture output 

struct TextureOutputToScreenFunctionality
{
	ID3D11Buffer *fullScreenQuadVertexBuffer;
	ID3D11Buffer *fullScreenQuadIndexBuffer;

	ID3D11VertexShader *textureOutputVertexShader;
	ID3D11PixelShader *textureOutputPixelShader;

	TextureOutputToScreenFunctionality();

	void OutputTextureToScreen(ID3D11ShaderResourceView* texture, D3DXVECTOR4 pos, D3DXVECTOR4 scale, int forced_lod, ID3D11PixelShader *enforced_pixel_shader);
};


bool init_engine();

void closeEngine();

void CreateRasterStates();

void BeginScene();

void EndScene();

void ClearRenderView(D3DXVECTOR4 col, int slot);

void SetRasterState(rasterState state);

void clearScreen(D3DXVECTOR4 col =D3DXVECTOR4(0,0,0,0), float depth = 1);

void CreateDepthStencilStates();

ID3D11DepthStencilState* CreateDepthStencilState(bool depth_test_enable, bool depth_write_enable, device_comparison_func depth_func,
	bool stencil_test_enable, bool stencil_write_enable, device_comparison_func stencil_func,  device_stencil_op stencil_success_op);

void CreateBlendStates();

void CreateSamplerStates();

ID3D11Buffer* CreateFullScreenQuadVertexBuffer();
ID3D11Buffer* CreateFullScreenQuadIndexBuffer();

void RenderFullScreenQuad();

void SetBlendState(blendState state);

ID3D11Buffer* CreateConstantBuffer(int byteWidth);

ID3D11VertexShader* CreateVertexShader(std::string path);

ID3D11ComputeShader* CreateComputeShader(std::string path);

ID3D11PixelShader* CreatePixelShader(std::string shader_name);

ID3D11GeometryShader* CreateGeometryShader(std::string shader_name);

ID3D11ComputeShader* CreateComputeShader(LPCTSTR name);

ID3D11RenderTargetView* GetDefaultRenderTargetView();
ID3D11DepthStencilView* GetDefaultDepthStencilView();

ID3D11Buffer *CreateVertexBuffer(int vertex_count, void *data, int vertex_struct_size);

ID3D11Buffer *CreateIndexBuffer(int index_count, void *data, int index_struct_size);

ID3D11Buffer *CreateStructuredBuffer(int vertex_count, float* data, int byteWidth);

ID3D11UnorderedAccessView *CreateUnorderedAccessView( ID3D11Buffer* pBuffer);

ID3D11ShaderResourceView *CreateShaderResourceView(ID3D11Buffer* pBuffer , int vertex_count);	

ID3D11Buffer *CreateReadableBuffer(int buffer_size, float* data);	

ID3D11ShaderResourceView *CreateTextureResourceView(ID3D11Resource *text, DXGI_FORMAT format, int mip_map_start, int mip_map_count, D3D11_SRV_DIMENSION dimension);

void CopySubResource(ID3D11Resource* source_texture, ID3D11Resource* destination_texture, const D3DXVECTOR3 &dim, int destination_subresource, int source_subresource);

void CopySubResource(Texture* source_texture, Texture* destination_texture, const D3DXVECTOR3 &dim, int destination_subresource, int source_subresource);

ID3D11Texture2D *CreateTexture2D(int width, int height, void *data, DXGI_FORMAT format, UINT creation_flags, int msaa_count, int mipmap_count = 1);

ID3D11Texture3D *CreateTexture3D(int width, int height, int depth, void *data, DXGI_FORMAT format, UINT creation_flags, int mipmap_count = 1);

ID3D11RenderTargetView* CreateRenderTargetView(ID3D11Resource* texture, D3D11_RTV_DIMENSION dimension, int mip_map = 0, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN);

void CreateComputeResourceBuffer(ID3D11Buffer** pBuffer, ID3D11UnorderedAccessView** uav, ID3D11ShaderResourceView **srv, int vertex_count, float* data, int bytewidth);

void CopyResourceContents(ID3D11Resource *to, ID3D11Resource *from);

void BeginQuery(ID3D11Query *query_object);

void EndQuery(ID3D11Query *query_object);

void* MapBuffer(ID3D11Buffer *buffer);

void UnMapBuffer(ID3D11Buffer *buffer);

UINT64 GetQueryData(ID3D11Query *query_object);

void SetViewPort(int w_start, int h_start, int width, int height);

void SetViewPortToDefault();

void SetScissorTest(int w_start, int h_start, int width, int height);

void SetVertexBuffer(ID3D11Buffer *  vertex_buffer, int vertex_size);

void SetIndexBuffer(ID3D11Buffer * index_buffer);

void SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY state);

void SetShaders(ID3D11VertexShader *vertex_shader, ID3D11GeometryShader *geometry_shader, ID3D11PixelShader *pixel_shader);

void SetComputeShader(ID3D11ComputeShader *compute_shader);

void SetConstantBufferToSlot(int start_slot, ID3D11Buffer *buffer_to_set);

void UpdateBuffer(void *data , int byteWidth, ID3D11Buffer* buffer);

void setComputeConstantBuffer(ID3D11Buffer* buffer, int index);

void SetDepthState(bool depth_test_enable, bool depth_write_enable, device_comparison_func depth_func, bool stencil_test_enable,
	bool stencil_write_enable, device_comparison_func stencil_func, device_stencil_op stencil_success_op, float stencil_value);

void SetBlendState(int blend_state);

void SetSamplerState();

ID3D11ShaderResourceView *GetDepthTextureSRV();

void SetRenderViews(ID3D11RenderTargetView *view, ID3D11DepthStencilView *depth_view, int rt_slot);

void SetUAVToPixelShader(ID3D11UnorderedAccessView *uav, int uav_slot, int index = -1);

void SetPixelShaderOutputMergerStates();

void SetRenderTargetView(ID3D11RenderTargetView *view, int slot = 0);

void SetDepthStencilView(ID3D11DepthStencilView *view);

void ResetUAVToPixelShader();

ID3D11DepthStencilView* GetDefaultDepthView();

void setCShaderUAVResources(ID3D11UnorderedAccessView **uav_list, int number_of_resources, int start_index);

void SetCShaderRV(ID3D11ShaderResourceView **uav_list, int number_of_resources);

bool check_srv_cache(ID3D11ShaderResourceView **uav_list, int number_of_resources, shaderType shader_type, int slot);
void set_srv_cache(ID3D11ShaderResourceView **uav_list, int number_of_resources, shaderType shader_type, int slot);

void SetSRV(ID3D11ShaderResourceView **uav_list, int number_of_resources, shaderType shader_type, int slot);
void SetSRV(Texture *texture, shaderType shader_type, int slot);

void DispatchComputeShader(int thread_count_x, int thread_count_y, int thread_count_z);

void Render(int vertext_count);
void RenderIndexed(int index_count);

void drawText(int fps, int y_pos);

void Flush();

void OutputTextureToScreen(Texture* texture, D3DXVECTOR4 pos, D3DXVECTOR4 scale, int forced_lod = -1 , ID3D11PixelShader *enforced_pixel_shader = nullptr);
void OutputTextureToScreen(ID3D11ShaderResourceView* texture, D3DXVECTOR4 pos, D3DXVECTOR4 scale, int forced_lod = -1, ID3D11PixelShader *enforced_pixel_shader = nullptr);

void invalidate_srv(shaderType shader_type);

void CopyStructureCount(ID3D11Buffer *dest_buffer, int offset, ID3D11UnorderedAccessView *uav);

extern ID3D11Buffer* render_constantsBuffer;
extern ID3D11Buffer* lighting_InfoBuffer;

#define PIX_EVENT(a) ScopeProfiler scope(a, __LINE__);

class ScopeProfiler
{
public:
	ScopeProfiler(const char *Name, int Line);
	~ScopeProfiler();

private:
	ScopeProfiler();
};


#endif