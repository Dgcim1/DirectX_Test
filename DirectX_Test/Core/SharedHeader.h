#pragma once

#include <string>
#include <vector>
#include <cassert>
#include <memory>
#include <d3d11.h>
#include <wrl.h>
#include <algorithm>
#include <unordered_map>
#include <ppltasks.h>
#include "../DirectXTK/DirectXTK.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "DirectXTK.lib")

#define EFLAG_HAS(Object, eFlag) (Object & eFlag) == eFlag

using namespace Microsoft::WRL;
using namespace DirectX;

using std::string;
using std::wstring;
using std::to_string;
using std::vector;
using std::unique_ptr;
using std::make_unique;
using std::min;
using std::max;
using std::unordered_map;

/// <summary>
/// Тип шейдера
/// </summary>
enum class EShaderType
{
	/// <summary>
	/// Вертексный шейдер
	/// </summary>
	VertexShader,
	/// <summary>
	/// Пиксельный шейдер
	/// </summary>
	PixelShader,
	/// <summary>
	/// Геометрическийи шейдер
	/// </summary>
	GeometryShader,
};

/// <summary>
/// Полигон, состоящий из трех точек вектора SVertex3D (порядковый номер)
/// </summary>
struct STriangle
{
	STriangle() {}
	STriangle(uint32_t _0, uint32_t _1, uint32_t _2) : I0{ _0 }, I1{ _1 }, I2{ _2 } {}

	uint32_t I0{};
	uint32_t I1{};
	uint32_t I2{};
};

#define ENUM_CLASS_FLAG(enum_type)\
static enum_type operator|(enum_type a, enum_type b)\
{\
	return static_cast<enum_type>(static_cast<int>(a) | static_cast<int>(b));\
}\
static enum_type& operator|=(enum_type& a, enum_type b)\
{\
	a = static_cast<enum_type>(static_cast<int>(a) | static_cast<int>(b));\
	return a;\
}\
static enum_type operator&(enum_type a, enum_type b)\
{\
	return static_cast<enum_type>(static_cast<int>(a) & static_cast<int>(b));\
}\
static enum_type& operator&=(enum_type& a, enum_type b)\
{\
	a = static_cast<enum_type>(static_cast<int>(a) & static_cast<int>(b));\
	return a;\
}\
static enum_type operator^(enum_type a, enum_type b)\
{\
	return static_cast<enum_type>(static_cast<int>(a) ^ static_cast<int>(b)); \
}\
static enum_type& operator^=(enum_type& a, enum_type b)\
{\
	a = static_cast<enum_type>(static_cast<int>(a) ^ static_cast<int>(b)); \
	return a; \
}\
static enum_type operator~(enum_type a)\
{\
	return static_cast<enum_type>(~static_cast<int>(a)); \
}