#pragma once
namespace Lobelia::Graphics {
	class BufferCreator final {
	public:
		static void CreateBuffer(ID3D11Buffer** buffer, void* resource, UINT byte_width, D3D11_USAGE usage, UINT bind_flag, UINT cpu_flag, UINT struct_stride);
	};
}