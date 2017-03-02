#pragma once
#include <d3d11.h>
struct Light {
	DirectX::XMFLOAT4 pos;
	DirectX::XMFLOAT4 dir;
	DirectX::XMFLOAT4 color;
	DirectX::XMFLOAT4 radius;
};