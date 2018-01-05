#pragma once
#include <QtWidgets/QMainWindow>
#include "ui_Tool.h"
#include "Lobelia/Lobelia.hpp"
#include "MeshWindow.h"
#include "FbxLoader.h"
#include <DirectXMath.h>
#pragma comment(lib,"FL Library.lib")
#include "Header/ModelImporter.hpp"
#include "Header/Camera.hpp"
#include "MeshWindow.h"
#include <io.h>
#include "MaterialWindow.h"
#include "qfiledialog.h"
#include "qtextcodec.h"
#include "qmouseeventtransition.h"
#include <QMouseEvent>
#include "qdrag.h"
#include "qevent.h"
#include "qmimedata.h"

namespace Lobelia {
	namespace Future {
		class DxdExporter;
		class MaterialExporter;
	}
	inline namespace Ancient {
		class DxdExporter;
		class MaterialExporter;
		class AnimationExporter;
	};
};
class Tool : public QMainWindow {
	Q_OBJECT
private:
	Ui::ToolClass ui;
	std::unique_ptr<Lobelia::Graphics::SwapChain> swapChain;
	std::unique_ptr<Lobelia::Camera> camera;
	std::unique_ptr<Lobelia::FbxImporter> importer;
	std::unique_ptr<Lobelia::Future::DxdExporter> dxdExporter;
	//std::unique_ptr<Lobelia::Ancient::DxdExporter> dxdExporter;
	std::unique_ptr<Lobelia::Future::MaterialExporter> matExporter;
	//std::unique_ptr<Lobelia::Ancient::MaterialExporter> matExporter;
	std::unique_ptr<Lobelia::AnimationExporter> anmExporter;
	std::unique_ptr<Lobelia::ModelMolding> modelMolding;
	std::unique_ptr<MeshWindow> meshWindow;
	std::unique_ptr<MaterialWindow> materialWindow;
	QPointF oldPoint;
	Lobelia::Mouse mouse;
	Lobelia::Keyboard keyboard;
	std::string filePath;
	_finddata_t fileDate;
public:
	Tool(QWidget *parent = Q_NULLPTR);
	~Tool();
public:
	void SetTextToList(QStringList list);
private:
	void mousePressEvent(QMouseEvent* event)override;
	void mouseReleaseEvent(QMouseEvent* event)override;
	void wheelEvent(QWheelEvent* event)override;
	void mouseMoveEvent(QMouseEvent* event)override;
	void keyPressEvent(QKeyEvent *event)override;
	void keyReleaseEvent(QKeyEvent *event)override;
	void dragEnterEvent(QDragEnterEvent* event)override;
	void dropEvent(QDropEvent *event)override;
	bool eventFilter(QObject* obj, QEvent* event)override;
private:
	void LoadFbx(const char* file_path);
public:
	Ui::ToolClass& GetUI();
	void ShowMeshWindow(const MeshWindow::Parameter& parameter);
	void ShowMaterialWindow(const MaterialWindow::Parameter& parameter);
	void Update();
	void Render();
private:
	private slots : void FileOpen();
	private slots:	void FileSaveDxd();
	private slots:	void FileSaveMt();
	private slots:	void FileSaveAnimation();
	private slots:	void FileSaveAll();
	private slots:	void OpenMeshWindow();
	private slots:	void OpenMaterialWindow();
};
#include "Header/Widget.hpp"
#include "Header/ModelMolding.hpp"
#include "Header/ModelExporter.hpp"
