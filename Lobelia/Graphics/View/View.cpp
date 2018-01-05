#include "Common/Common.hpp"
#include "Math/Math.hpp"
#include "Exception/Exception.hpp"
#include "Graphics/GraphicDriverInfo/GraphicDriverInfo.hpp"
#include "Graphics/Device/Device.hpp"
#include "Graphics/BufferCreator/BufferCreator.h"
#include "Graphics/ConstantBuffer/ShaderStageList.hpp"
#include "Graphics/ConstantBuffer/ConstantBuffer.hpp"

#include "Graphics/View/View.hpp"

namespace Lobelia::Graphics {
	DirectX::XMMATRIX View::nowView;
	DirectX::XMMATRIX View::nowProjection;
	Math::Vector2 View::nowSize;

	View::View(const Math::Vector2& left_up, const Math::Vector2& size, float fov_rad, float near_z, float far_z) {
		this->size = size;
		constantBuffer = std::make_unique<ConstantBuffer<Constant>>(0, Graphics::ShaderStageList::VS | Graphics::ShaderStageList::PS);
		CreateViewport(left_up, size);
		CreateProjection(fov_rad, static_cast<float>(size.x) / static_cast<float>(size.y), near_z, far_z);
	}
	void View::SetEyePos(const Math::Vector3& pos) { data.eye = DirectX::XMVectorSet(pos.x, pos.y, pos.z, 1.0f); }
	void View::SetEyeTarget(const Math::Vector3& target) { data.at = DirectX::XMVectorSet(target.x, target.y, target.z, 1.0f); }
	void View::SetEyeUpDirection(const Math::Vector3& up_direction) { data.up = DirectX::XMVectorSet(up_direction.x, up_direction.y, up_direction.z, 1.0f); }
	View::~View() = default;
	void View::ChangeViewport(const Math::Vector2& pos, const Math::Vector2& size) { CreateViewport(pos, size); }
	void View::Activate() {
		CreateView(data);
		constantBuffer->Activate(buffer);
		Device::GetContext()->RSSetViewports(1, &viewport);
		nowSize = size;
		nowView = buffer.view; nowProjection = buffer.projection;
	}
	void View::CreateProjection(float fov_rad, float aspect, float near_z, float far_z) {
		buffer.projection = DirectX::XMMatrixPerspectiveFovLH(fov_rad, aspect, near_z, far_z);
		buffer.projection = DirectX::XMMatrixTranspose(buffer.projection);
	}
	void View::CreateView(const Data& data) {
		buffer.view = DirectX::XMMatrixLookAtLH(data.eye, data.at, data.up);
		buffer.view = DirectX::XMMatrixTranspose(buffer.view);
	}
	void View::CreateViewport(const Math::Vector2& pos, const Math::Vector2& size) {
		viewport.Width = static_cast<FLOAT>(size.x);
		viewport.Height = static_cast<FLOAT>(size.y);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.TopLeftX = static_cast<FLOAT>(pos.x);
		viewport.TopLeftY = static_cast<FLOAT>(pos.y);
	}
	DirectX::XMMATRIX View::GetColumnViewMatrix() { return buffer.view; }
	DirectX::XMMATRIX View::GetColumnProjectionMatrix() { return buffer.projection; }
	DirectX::XMMATRIX View::GetRawViewMatrix() { return DirectX::XMMatrixTranspose(buffer.view); }
	DirectX::XMMATRIX View::GetRawProjectionMatrix() { return DirectX::XMMatrixTranspose(buffer.projection); }

	DirectX::XMMATRIX View::GetNowColumnViewMatrix() { return nowView; }
	DirectX::XMMATRIX View::GetNowColumnProjectionMatrix() { return nowProjection; }
	DirectX::XMMATRIX View::GetNowRawViewMatrix() { return DirectX::XMMatrixTranspose(nowView); }
	DirectX::XMMATRIX View::GetNowRawProjectionMatrix() { return DirectX::XMMatrixTranspose(nowProjection); }

}