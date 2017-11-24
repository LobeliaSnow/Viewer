#pragma once
#include <QtWidgets/QMainWindow>
#include "ui_Tool.h"
#include "Lobelia/Lobelia.hpp"
#include "FbxLoader.h"
#include <DirectXMath.h>
#pragma comment(lib,"FL Library.lib")
#include "Header/ModelImporter.hpp"
#include "Header/ModelExporter.hpp"
#include "Header/Camera.hpp"

class Tool : public QMainWindow
{
	Q_OBJECT
private:
	Ui::ToolClass ui;
	std::unique_ptr<Lobelia::Graphics::SwapChain> swapChain;
	std::unique_ptr<Lobelia::Camera> camera;
	std::unique_ptr<Lobelia::FbxImporter> importer;
	std::unique_ptr<Lobelia::DxdExporter> dxdExporter;
	std::unique_ptr<Lobelia::MaterialExporter> matExporter;
	std::unique_ptr<Lobelia::AnimationExporter> anmExporter;
	std::unique_ptr<Lobelia::ModelMolding> modelMolding;
	std::thread thread;
	QPointF oldPoint;
	Lobelia::Mouse mouse;
	Lobelia::Keyboard keyboard;
public:
	Tool(QWidget *parent = Q_NULLPTR);
	~Tool();
private:
public:
	Ui::ToolClass& GetUI();
	void mousePressEvent(QMouseEvent* event)override;
	void mouseReleaseEvent(QMouseEvent* event)override;
	void wheelEvent(QWheelEvent* event)override;
	void mouseMoveEvent(QMouseEvent* event)override;
	void keyPressEvent(QKeyEvent *event)override;
	void keyReleaseEvent(QKeyEvent *event)override;
	bool eventFilter(QObject* obj, QEvent* event)override;
	void Render();
	public slots:	void FileOpen();
	public slots:	void FileSaveDxd();
	public slots:	void FileSaveMt();
	public slots:	void FileSaveAnimation();
	public slots:	void FileSaveAll();
};
#include "Header/ModelMolding.hpp"
