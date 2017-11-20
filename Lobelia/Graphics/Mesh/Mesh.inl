#pragma once
namespace Lobelia::Graphics {
	template<class T> inline Mesh<T>::Mesh(int buffer_count) :originBuffer(buffer_count), bufferCount(static_cast<size_t>(buffer_count)) {
		BufferCreator::CreateBuffer(vertexBuffer.GetAddressOf(), originBuffer.data(), static_cast<UINT>(sizeof(T)*bufferCount), D3D11_USAGE_DEFAULT, D3D11_BIND_VERTEX_BUFFER, 0, sizeof(float));
	}
	template<class T> inline Mesh<T>::~Mesh() = default;
	template<class T> inline void Mesh<T>::SetDelimiters(const std::vector<std::pair<std::string, int>>& delimiters) {
		this->delimiters = delimiters;
		delimiterCount = static_cast<int>(delimiters.size());
	}
	template<class T> inline const ComPtr<ID3D11Buffer>& Mesh<T>::GetVertexBuffer() { return vertexBuffer; }
	template<class T> inline T* Mesh<T>::GetBuffer() { return originBuffer.data(); }
	template<class T> inline size_t Mesh<T>::GetCount() { return bufferCount; }
	template<class T> inline const std::string& Mesh<T>::GetMaterialName(int delimiter_index) { return delimiters[delimiter_index].first; }
	template<class T> inline int Mesh<T>::GetDelimiter(int index) { return delimiters[index].second; }
	template<class T> inline size_t Mesh<T>::GetDelimiterCount() { return delimiterCount; }
	template<class T> void Mesh<T>::Update() { Device::GetContext()->UpdateSubresource(vertexBuffer.Get(), 0, nullptr, originBuffer.data(), 0, 0); }
	template<class T> inline void Mesh<T>::Set() {
		Update();
		UINT strides = sizeof(T);
		UINT offset = 0;
		Device::GetContext()->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &strides, &offset);
	}
}