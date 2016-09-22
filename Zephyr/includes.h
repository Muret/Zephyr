
#ifndef INCLUDES_H_
#define INCLUDES_H_


#include <windowsx.h>
#include <windows.h>

#include <d3dcommon.h>
#include <D3D11_1.h>
#include <d3dx10math.h>
#include <d3dx11async.h>

//#include "FW1FontWrapper.h"

#define PI 3.14

#include <fstream>

#include <math.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <assert.h>
#include <iostream>
#include <algorithm>
#include <functional>
#include <array>
#include <iostream>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")
#pragma comment(lib,"D3dcompiler.lib")
//#pragma comment(lib,"FW1FontWrapper.lib")
#pragma comment( lib, "dxguid.lib")

extern int g_screenWidth, g_screenHeight;

using namespace std;

D3DXVECTOR3 operator*(const D3DXVECTOR3 &lhs, const D3DXVECTOR3 &rhs);

#define SAFE_DELETE(px) if(px) { delete px; px = nullptr;}

#endif