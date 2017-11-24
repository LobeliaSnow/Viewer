#include "Tool.h"
#include "qfiledialog.h"
#include "qtextcodec.h"
#include "qmouseeventtransition.h"
#include <QMouseEvent>

//TODO : まともに使えるようにする。
//TODO : 描画順を入れ替えれるようにしたり、同じマテリアルなのに複数のサブセットに分かれている場合は結合できるように組み替える。
//TODO : ↑構想としてはimporterをいじる。それに伴い、Exporterの各種モデルクラスから呼び出している関数を、importerで並び替えられた関数に差し替え。
//TODO : ↑並び替えはコンバータークラスを作るかインポーターで直接するか
//TODO : マルチスレッド & (プログレスバー || ログ)
//TODO : ボーン表示(急ぎ)
//TODO : 描画順変更機能
//TODO : テクスチャを見れる機能とかも欲しい
//TODO : 今適当だから例外吐きます
//TODO : dxdも読めるように

Tool::Tool(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	// バッファリングをしないようにする
	setAttribute(Qt::WA_PaintOnScreen, true);
	// ハンドラの再利用を禁止する
	setAttribute(Qt::WA_NativeWindow, true);
	Lobelia::Bootup();
	FL::System::GetInstance()->Initialize();
	auto& glWidget = ui.openGLWidget;
	swapChain = std::make_unique<Lobelia::Graphics::SwapChain>(reinterpret_cast<HWND>(glWidget->winId()), Lobelia::Math::Vector2(glWidget->size().width(), glWidget->size().height()), DXGI_SAMPLE_DESC{ 1,0 });
	//View(const Math::Vector2& left_up, const Math::Vector2& size, float fov_rad = PI / 4.0f, float near_z = 1.0f, float far_z = 1000.0f);
	camera = std::make_unique<Lobelia::Camera>(Lobelia::Math::Vector2(glWidget->size().width(), glWidget->size().height()));
	Lobelia::Graphics::Environment::GetInstance()->SetAmbientColor(0xFFFFFFFF);
	Lobelia::Graphics::Environment::GetInstance()->SetLightDirection(Lobelia::Math::Vector3(0.0f, 1.0f, 1.0f));
	Lobelia::Graphics::Environment::GetInstance()->Activate();
	mouse = {};
	installEventFilter(this);
	setMouseTracking(true);
	startTimer(16);
	connect(ui.actionOpen, SIGNAL(triggered()), this, SLOT(FileOpen()));
	connect(ui.action_model_dxd, SIGNAL(triggered()), this, SLOT(FileSaveDxd()));
	connect(ui.actionmaterial_mt, SIGNAL(triggered()), this, SLOT(FileSaveMt()));
	connect(ui.action_Animation_anm, SIGNAL(triggered()), this, SLOT(FileSaveAnimation()));
	importer = std::make_unique<Lobelia::FbxImporter>();
	dxdExporter = std::make_unique<Lobelia::DxdExporter>();
	matExporter = std::make_unique<Lobelia::MaterialExporter>();
	anmExporter = std::make_unique<Lobelia::AnimationExporter>();
	//QObject::connect(timer, SIGNAL(timeout()), myClass, SLOT(onTimer()));
	oldPoint = {};
}
Tool::~Tool() {
	Lobelia::Shutdown();
	importer.reset(nullptr);
	FL::System::GetInstance()->Finalize();
};
void Tool::FileOpen() {
	QString path = QFileDialog::getOpenFileName(this, "Import File");
	ui.listWidget->clear();
	importer->Load(path.toLocal8Bit().data());
	//ui.textBrowser->setText(QString::fromLocal8Bit("インポート開始"));
	modelMolding = std::make_unique<Lobelia::ModelMolding>(importer.get(), this);
	//ui.textBrowser->setText(QString::fromLocal8Bit("インポート終了"));
}
void Tool::FileSaveDxd() {
	if (importer->IsEmpty())return;
	QString path = QFileDialog::getSaveFileName(this, "Export File");
	dxdExporter->Save(importer.get(), path.toLocal8Bit().data());
}
void Tool::FileSaveMt() {
	if (importer->IsEmpty())return;
	QString path = QFileDialog::getSaveFileName(this, "Export File");
	matExporter->Save(importer.get(), path.toLocal8Bit().data());
}
void Tool::FileSaveAnimation() {
	if (importer->IsEmpty())return;
	QString path = QFileDialog::getExistingDirectory(this, "Export File");
	anmExporter->Save(importer.get(), path.toLocal8Bit().data());
}
void Tool::FileSaveAll() {
	if (importer->IsEmpty())return;
	QString path = QFileDialog::getExistingDirectory(this, "Export File");
	dxdExporter->Save(importer.get(), path.toLocal8Bit().data());
	matExporter->Save(importer.get(), path.toLocal8Bit().data());
	anmExporter->Save(importer.get(), path.toLocal8Bit().data());
}
Ui::ToolClass& Tool::GetUI() { return ui; }
void Tool::mousePressEvent(QMouseEvent* event) {
	switch (event->button()) {
	case Qt::LeftButton:			mouse.left = true;		break;
	case Qt::RightButton:		mouse.right = true;		break;
	case Qt::MiddleButton:		mouse.center = true;	break;
	}
}
void Tool::mouseReleaseEvent(QMouseEvent* event) {
	switch (event->button()) {
	case Qt::LeftButton:			mouse.left = false;		break;
	case Qt::RightButton:		mouse.right = false;		break;
	case Qt::MiddleButton:		mouse.center = false;	break;
	}
}
void Tool::wheelEvent(QWheelEvent* event) {
	QPoint wheel = event->angleDelta();
	mouse.xWheel = wheel.x();
	mouse.yWheel = wheel.y();
}
void Tool::mouseMoveEvent(QMouseEvent* event) {
	QPointF point = event->screenPos();
	if (!(oldPoint.x() == 0.0f && oldPoint.y() == 0.0f)) {
		QPointF move = point - oldPoint;
		mouse.move.x = move.x();
		mouse.move.y = move.y();
	}
	oldPoint = point;
}
void Tool::keyPressEvent(QKeyEvent *event) {
	switch (event->key()) {
	case Qt::Key::Key_Escape:		qApp->quit();				break;
	case Qt::Key::Key_Control:	keyboard.ctrl = true;		break;
	case Qt::Key::Key_Alt:			keyboard.alt = true;		break;
	}
}
void Tool::keyReleaseEvent(QKeyEvent *event) {
	switch (event->key()) {
	case Qt::Key::Key_Control:	keyboard.ctrl = false;	break;
	case Qt::Key::Key_Alt:			keyboard.alt = false;		break;
	}
}

bool Tool::eventFilter(QObject* obj, QEvent* event) {
	if (camera)camera->Update(mouse, keyboard);
	keyboard = {}; mouse.xWheel = 0.0f; mouse.yWheel = 0.0f;
	Render();
	return false;
}
void Tool::Render() {
	swapChain->Clear(0xFF000000);
	Lobelia::Graphics::Transform2D transform = {};
	auto& glWidget = ui.openGLWidget;
	if (modelMolding)modelMolding->Render(this);
	swapChain->Present();
}