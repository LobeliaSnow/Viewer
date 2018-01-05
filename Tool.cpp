#include "Tool.h"
#include "qmessagebox.h"
#include <future>
#include <qthread.h>
#include <QtConcurrent/qtconcurrentrun.h>
//TODO : 複数ウインドウの使い方を覚えたのでそれを使い、メッシュウインドウ作成
//TODO : ↑テクスチャビュワー等の作成
//TODO : まともに使えるようにする。
//TODO : 同じマテリアルなのに複数のサブセットに分かれている場合は結合できるように組み替える。
//TODO : エクスポートをMoldingクラスからするように改良。それにより描画順などその他情報を編集することが可能となる。
//TODO : マルチスレッド & (プログレスバー || ログ)
//TODO : ボーン表示(急ぎ)
//TODO : テクスチャを見れる機能とかも欲しい
//TODO : 今適当だから例外吐きます
//TODO : dxdも読めるように
//TODO : このツール用にライブラリ改造しています(テクスチャ読み込みやリソースバンク等)
//面白そう std::initializer_list<>

Tool::Tool(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	// バッファリングをしないようにする
	setAttribute(Qt::WA_PaintOnScreen, true);
	// ハンドラの再利用を禁止する
	setAttribute(Qt::WA_NativeWindow, true);
	//ドラッグアンドドロップを受け付ける 
	setAcceptDrops(true);
	Lobelia::Bootup();
	FL::System::GetInstance()->Initialize();
	auto& glWidget = ui.openGLWidget;
	swapChain = std::make_unique<Lobelia::Graphics::SwapChain>(reinterpret_cast<HWND>(glWidget->winId()), Lobelia::Math::Vector2(glWidget->size().width(), glWidget->size().height()), DXGI_SAMPLE_DESC{ 1,0 });
	//View(const Math::Vector2& left_up, const Math::Vector2& size, float fov_rad = PI / 4.0f, float near_z = 1.0f, float far_z = 1000.0f);
	camera = std::make_unique<Lobelia::Camera>(Lobelia::Math::Vector2(glWidget->size().width(), glWidget->size().height()));
	Lobelia::Graphics::Environment::GetInstance()->SetAmbientColor(0xFFFFFFFF);
	Lobelia::Graphics::Environment::GetInstance()->SetLightDirection(Lobelia::Math::Vector3(0.0f, 1.0f, 1.0f));
	Lobelia::Graphics::Environment::GetInstance()->Activate();
	mouse = {}; keyboard = {};
	installEventFilter(this);
	setMouseTracking(true);
	startTimer(16);
	connect(ui.actionOpen, SIGNAL(triggered()), this, SLOT(FileOpen()));
	connect(ui.action_model_dxd, SIGNAL(triggered()), this, SLOT(FileSaveDxd()));
	connect(ui.actionmaterial_mt, SIGNAL(triggered()), this, SLOT(FileSaveMt()));
	connect(ui.action_Animation_anm, SIGNAL(triggered()), this, SLOT(FileSaveAnimation()));
	connect(ui.actionA_ll, SIGNAL(triggered()), this, SLOT(FileSaveAll()));
	connect(ui.listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(OpenMeshWindow()));
	connect(ui.listMaterial, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(OpenMaterialWindow()));
	importer = std::make_unique<Lobelia::FbxImporter>();
	dxdExporter = std::make_unique<Lobelia::Future::DxdExporter>();
	//dxdExporter = std::make_unique<Lobelia::Ancient::DxdExporter>();
	matExporter = std::make_unique<Lobelia::Future::MaterialExporter>();
	//matExporter = std::make_unique<Lobelia::Ancient::MaterialExporter>();
	anmExporter = std::make_unique<Lobelia::Ancient::AnimationExporter>();
	//QObject::connect(timer, SIGNAL(timeout()), myClass, SLOT(onTimer()));
	oldPoint = {};
	meshWindow = std::make_unique<MeshWindow>(this);
	materialWindow = std::make_unique<MaterialWindow>(this);
}
Tool::~Tool() {
	meshWindow.reset();
	materialWindow.reset();
	Lobelia::Shutdown();
	importer.reset(nullptr);
	FL::System::GetInstance()->Finalize();

};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  アイコンへのドラッグイベント
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Tool::SetTextToList(QStringList list) {
	foreach(QString str, list) {
		std::string path = str.toStdString();
		std::string extension = Lobelia::Utility::FilePathControl::GetExtension(path);
		transform(extension.begin(), extension.end(), extension.begin(), tolower);
		if (extension == ".fbx")	LoadFbx(path.c_str());
		break;
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  ファイル読み込み関連
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Tool::LoadFbx(const char* file_path) {
	filePath = file_path;
	meshWindow->Hide();
	materialWindow->Hide();
	ui.listWidget->setCurrentRow(-1);
	importer->Load(filePath.c_str());
	modelMolding = std::make_unique<Lobelia::ModelMolding>(importer.get(), this);
	_findfirst(filePath.c_str(), &fileDate);
	//CoUninitialize();
}
void Tool::FileOpen() {
	QString path = QFileDialog::getOpenFileName(this, "Import File");
	if (path.isEmpty())return;
	LoadFbx(path.toLocal8Bit().data());
}
void Tool::FileSaveDxd() {
	if (importer->IsEmpty())return;
	QString path = QFileDialog::getSaveFileName(this, "Export File");
	if (path.isEmpty())return;
	dxdExporter->Save(modelMolding.get(), path.toLocal8Bit().data());
	//dxdExporter->Save(importer.get(), path.toLocal8Bit().data());
}
void Tool::FileSaveMt() {
	if (importer->IsEmpty())return;
	QString path = QFileDialog::getSaveFileName(this, "Export File");
	if (path.isEmpty())return;
	matExporter->Save(modelMolding.get(), path.toLocal8Bit().data());
	//matExporter->Save(importer.get(), path.toLocal8Bit().data());
}
void Tool::FileSaveAnimation() {
	if (importer->IsEmpty())return;
	QString path = QFileDialog::getExistingDirectory(this, "Export File");
	if (path.isEmpty())return;
	anmExporter->Save(importer.get(), path.toLocal8Bit().data());
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//  文字列を置換する
void StringReplace(std::string& source, const std::string& old_str, const std::string& new_str) {
	std::string::size_type  pos(source.find(old_str));
	while (pos != std::string::npos) {
		source.replace(pos, old_str.length(), new_str);
		pos = source.find(old_str, pos + new_str.length());
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void Tool::FileSaveAll() {
	if (importer->IsEmpty())return;
	QString path = QFileDialog::getSaveFileName(this, "Export File");
	if (path.isEmpty())return;
	std::string pathStr = path.toLocal8Bit();
	std::string directory = Lobelia::Utility::FilePathControl::GetParentDirectory(pathStr);
	std::string matName = directory + "/" + Lobelia::Utility::FilePathControl::GetFilename(pathStr);
	StringReplace(matName, "dxd", "mt");
	dxdExporter->Save(modelMolding.get(), pathStr.c_str());
	matExporter->Save(modelMolding.get(), matName.c_str());
	//dxdExporter->Save(importer.get(), pathStr.c_str());
	//matExporter->Save(importer.get(), matName.c_str());
	anmExporter->Save(importer.get(), (directory + "/").c_str());
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Tool::OpenMeshWindow() {
	Lobelia::MeshWidget* item = dynamic_cast<Lobelia::MeshWidget*>(ui.listWidget->currentItem());
	modelMolding->GetMeshWidget(item->GetIndex())->OpenMeshWindow();
}
void Tool::OpenMaterialWindow() {
	Lobelia::MaterialWidget* item = dynamic_cast<Lobelia::MaterialWidget*>(ui.listMaterial->currentItem());
	modelMolding->GetMaterialWidget(item->GetIndex())->OpenMaterialWindow();
}
Ui::ToolClass& Tool::GetUI() { return ui; }
void Tool::ShowMeshWindow(const MeshWindow::Parameter& parameter) {
	meshWindow->Show(parameter);
}
void Tool::ShowMaterialWindow(const MaterialWindow::Parameter& parameter) {
	materialWindow->Show(parameter);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	イベント関数
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
	auto QuitConfirmation = [this]() {
		QMessageBox msgBox(this);
		msgBox.setText(QString::fromLocal8Bit("終了しますか？"));
		msgBox.setWindowTitle(QString::fromLocal8Bit("Quit?"));
		msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
		msgBox.setDefaultButton(QMessageBox::No);
		msgBox.setButtonText(QMessageBox::Yes, QString::fromLocal8Bit("はい"));
		msgBox.setButtonText(QMessageBox::Cancel, QString::fromLocal8Bit("キャンセル"));
		msgBox.setIcon(QMessageBox::Icon::Question);
		int res = msgBox.exec();
		if (res == QMessageBox::Yes)return true;
		else if (res == QMessageBox::Cancel)return false;
	};
	switch (event->key()) {
	case Qt::Key::Key_Escape:		if (QuitConfirmation())qApp->quit();	break;
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
void Tool::dragEnterEvent(QDragEnterEvent* event) {
	QString path = event->mimeData()->text();
	if (path.isEmpty())return;
	std::string extension = Lobelia::Utility::FilePathControl::GetExtension(path.toStdString());
	transform(extension.begin(), extension.end(), extension.begin(), tolower);
	if (extension == ".fbx")	event->acceptProposedAction();
}
void Tool::dropEvent(QDropEvent *event) {
	QString path = event->mimeData()->urls().first().toLocalFile();
	if (path.isEmpty())return;
	std::string extension = Lobelia::Utility::FilePathControl::GetExtension(path.toStdString());
	transform(extension.begin(), extension.end(), extension.begin(), tolower);
	if (extension == ".fbx")
		LoadFbx(path.toLocal8Bit().data());
}
bool Tool::eventFilter(QObject* obj, QEvent* event) {
	if (camera)camera->Update(mouse, keyboard);
	if (keyboard.ctrl)ui.listWidget->setCurrentRow(-1);
	mouse.xWheel = 0.0f; mouse.yWheel = 0.0f;
	Update();
	Render();
	return false;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Tool::Update() {
	if (filePath.empty())return;
	_finddata_t fileDataNow;
	_findfirst(filePath.c_str(), &fileDataNow);
	if (fileDataNow.time_access > fileDate.time_access)	LoadFbx(filePath.c_str());
	if (modelMolding)modelMolding->Update(this);
}
void Tool::Render() {
	swapChain->Clear(0xFF000000);
	swapChain->GetRenderTarget()->Activate();
	//Lobelia::Graphics::Transform2D transform = {};
	//auto& glWidget = ui.openGLWidget;
	if (modelMolding)modelMolding->Render(this);
	swapChain->Present();
}
