#include "Tool.h"
#include "qfiledialog.h"
#include "qtextcodec.h"
//TODO : 後でタイマーイベントを送ってやる

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
	view = std::make_unique<Lobelia::Graphics::View>(Lobelia::Math::Vector2(0, 0), Lobelia::Math::Vector2(glWidget->size().width(), glWidget->size().height()));
	view->SetEyePos(Lobelia::Math::Vector3(0, 30, 0));
	view->SetEyeTarget(Lobelia::Math::Vector3(0, 0, 30));
	view->SetEyeUpDirection(Lobelia::Math::Vector3(0, 1, 0));
	installEventFilter(this);
	startTimer(1);
	connect(ui.actionOpen, SIGNAL(triggered()), this, SLOT(FileOpen()));
	importer = std::make_unique<Lobelia::FbxImporter>();
	dxdExporter = std::make_unique<Lobelia::DxdExporter>();
	matExporter = std::make_unique<Lobelia::MaterialExporter>();
	anmExporter = std::make_unique<Lobelia::AnimationExporter>();
	//QObject::connect(timer, SIGNAL(timeout()), myClass, SLOT(onTimer()));
}
Tool::~Tool() {
	Lobelia::Shutdown();
	importer.reset(nullptr);
	FL::System::GetInstance()->Finalize();
};
void Tool::FileOpen() {
	QString path = QFileDialog::getOpenFileName(this, "Open File");
	importer->Load(path.toLocal8Bit().data());
	dxdExporter->Save(importer.get(), "test.dxd");
	matExporter->Save(importer.get(), "test.mt");
	anmExporter->Save(importer.get(), "");
}
bool Tool::eventFilter(QObject* obj, QEvent* eve) {
	Render();
	return false;
}
void Tool::Render() {
	view->Activate();
	swapChain->Clear(0xFFFFFFFF);
	Lobelia::Graphics::Transform2D transform = {};
	auto& glWidget = ui.openGLWidget;


	swapChain->Present();
}