
#ifndef _D11_H_
#define _D11_H_

#include "includes.h"

class Texture;

//constant variables
struct MatrixBuffer
{
	float g_GridDimensionCube;
	float g_GridDimensionSquare;
	float g_GridDimension;
	float g_InverseGridDimension[3];
	float g_MiddleGridOffset[3];
};

struct GridCostantBuffer
{
	float g_GridDimensionCube;
	float g_GridDimensionSquare;
	float g_GridDimension;
	float g_InverseGridDimension[3];
	float g_MiddleGridOffset[3];
	float padding[3];
};

__declspec(align(16)) struct SortConstantBuffer
{
	UINT g_sortLevel;
	UINT g_sortAlternateMask;
	UINT g_iWidth;
	UINT g_iHeight;
	D3DXVECTOR4 padding;
};

struct SimulationConstantBuffer
{
	float g_fTimeStep;
	float g_fDensityCoef;
	float g_fGradPressureCoef;
	float g_fLapViscosityCoef;
	float g_vGravity[4];
	D3DXVECTOR4 g_vPlanes[6];
};

//declarations
struct RenderConstantsBuffer
{
	D3DXMATRIX WorldViewProjectionMatrix;
	D3DXMATRIX WorldMatrix;
	D3DXMATRIX inverseWorldViewProjectionMatrix;
	D3DXMATRIX inverseProjectionMatrix;
	D3DXMATRIX projectionMatrix;
	D3DXMATRIX viewMatrix;
	D3DXMATRIX inverseView;
	D3DXMATRIX viewProjection;

	D3DXVECTOR4 right_direction;
	D3DXVECTOR4 up_direction;
	D3DXVECTOR4 view_direction;
	D3DXVECTOR4 camera_position;
	D3DXVECTOR4 screen_texture_half_pixel_forced_mipmap;
	D3DXVECTOR4 near_far_padding2;

	D3DXVECTOR4 diffuse_color;
};

struct LightingInfoBuffer
{
	D3DXVECTOR4 ws_light_position;
	D3DXVECTOR4 ss_light_position;
	D3DXVECTOR4 light_color;
};

struct ComputeBuffer
{
	float number_of_particles;
	float time;
	float padding1;
	float padding2;
};

//grid key structure
struct GridKeyStructure
{
	UINT gridKey;
	UINT particleIndex;
};

struct ForceStructure
{
	D3DXVECTOR4 acceleration;
};

//grid border structure
struct GridBorderStructure
{
	UINT gridStart;
	UINT gridEnd;
};


struct DensityStructure
{
	float density;
};

extern ID3D11DeviceContext *g_deviceContext;
extern ID3DUserDefinedAnnotation *pPerf;

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

enum shaderType
{
	shader_type_vertex,
	shader_type_pixel,
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


void UpdateGlobalBuffers();

bool init_engine();

void closeEngine();

void BeginScene();

void EndScene();

void ClearRenderView(D3DXVECTOR4 col, int slot);
void clearScreen(D3DXVECTOR4 col =D3DXVECTOR4(0,0,0,0), float depth = 1);

void CreateDepthStencilStates();

void CreateBlendStates();

void CreateSamplerStates();

ID3D11Buffer* CreateFullScreenQuadVertexBuffer();
ID3D11Buffer* CreateFullScreenQuadIndexBuffer();

void RenderFullScreenQuad();

void SetBlendState(blendState state);

ID3D11Query* CreateQuery(D3D11_QUERY type);

ID3D11Buffer* CreateConstantBuffer(int byteWidth);

ID3D11VertexShader* CreateVertexShader(std::string path);

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

ID3D11ShaderResourceView *CreateTextureResourceView(ID3D11Texture2D *text, DXGI_FORMAT format, int mip_map_start, int mip_map_count);

void CopySubResource(ID3D11Texture2D* source_texture, ID3D11Texture2D* destination_texture, int width, int height, int destination_subresource, int source_subresource);
void CopySubResource(Texture* source_texture, Texture* destination_texture, int width, int height, int destination_subresource, int source_subresource);

ID3D11Texture2D *CreateTexture(int width, int height, void *data, DXGI_FORMAT format, int mipmap_count = 1);

ID3D11RenderTargetView* CreateRenderTargetView(ID3D11Texture2D* texture, int mip_map = 0, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN);

void CreateComputeResourceBuffer(ID3D11Buffer** pBuffer, ID3D11UnorderedAccessView** uav, ID3D11ShaderResourceView **srv, int vertex_count, float* data, int bytewidth);

void CopyResourceContents(ID3D11Resource *to, ID3D11Resource *from);

void BeginQuery(ID3D11Query *query_object);

void EndQuery(ID3D11Query *query_object);

void* MapBuffer(ID3D11Buffer *buffer);

void UnMapBuffer(ID3D11Buffer *buffer);

UINT64 GetQueryData(ID3D11Query *query_object);

void SetViewPort(int w_start, int h_start, int width, int height);
void SetViewPortToDefault();

void SetVertexBuffer(ID3D11Buffer *  vertex_buffer, int vertex_size);

void SetIndexBuffer(ID3D11Buffer * index_buffer);

void SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY state);

void SetCameraBuffer(D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix);

void SetShaders(ID3D11VertexShader *vertex_shader, ID3D11GeometryShader *geometry_shader, ID3D11PixelShader *pixel_shader);

void SetComputeShader(ID3D11ComputeShader *compute_shader);

void SetConstantBufferForRendering(int start_slot, ID3D11Buffer *buffer_to_set);

void UpdateBuffer(float *data , int byteWidth, ID3D11Buffer* buffer);

void setComputeConstantBuffer(ID3D11Buffer* buffer, int index);

void SetDepthState(int depth_state);

void SetBlendState(int blend_state);

void SetSamplerState();

ID3D11ShaderResourceView *GetDepthTextureSRV();

void SetRenderViews(ID3D11RenderTargetView *view, ID3D11DepthStencilView *depth_view, int rt_slot);

void SetRenderTargetView(ID3D11RenderTargetView *view, int slot = 0);

void SetDepthStencilView(ID3D11DepthStencilView *view);

void SetViewTargetsAux();

ID3D11DepthStencilView* GetDefaultDepthView();

void setCShaderUAVResources(ID3D11UnorderedAccessView **uav_list, int number_of_resources);

void SetCShaderRV(ID3D11ShaderResourceView **uav_list, int number_of_resources);

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

extern ID3D11Buffer* render_constantsBuffer;
extern ID3D11Buffer* lighting_InfoBuffer;

extern RenderConstantsBuffer render_constantsBuffer_cpu;
extern LightingInfoBuffer lighting_InfoBuffer_cpu;


#endif