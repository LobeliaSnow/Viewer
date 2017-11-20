#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_Tool.h"
#include "Lobelia/Lobelia.hpp"
#include "FbxLoader.h"
#include <DirectXMath.h>
#pragma comment(lib,"FL Library.lib")

#include "Header/ModelImporter.hpp"
#include "Header/ModelExporter.hpp"

class Tool : public QMainWindow
{
	Q_OBJECT
private:
	Ui::ToolClass ui;
	std::unique_ptr<Lobelia::Graphics::SwapChain> swapChain;
	std::unique_ptr<Lobelia::Graphics::View> view;
	std::unique_ptr<Lobelia::FbxImporter> importer;
	std::unique_ptr<Lobelia::DxdExporter> dxdExporter;
	std::unique_ptr<Lobelia::MaterialExporter> matExporter;
	std::unique_ptr<Lobelia::AnimationExporter> anmExporter;

public:
	Tool(QWidget *parent = Q_NULLPTR);
	~Tool();
private:
public:
	bool eventFilter(QObject* obj, QEvent* eve)override;
	void Render();
	public slots:	void FileOpen();
};
