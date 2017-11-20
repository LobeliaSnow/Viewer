#include <Windows.h>
#include <d3d11.h>
#include <wrl.h>
#include <memory>
#include <map>
#include <unordered_map>
#include <vector>
#include <string>
#include <array>
#include <future>
#include <queue>
#include <list>
#include <shlwapi.h>
#include <sstream>
#include <iostream>
#include <filesystem>
//
//#include <mfapi.h>
//#include <mfidl.h>
//#include <mfreadwrite.h>
//#include <mferror.h>
//#include <evr.h>

//#include <mfapi.h>
//#include <mfidl.h>
//#include <mfreadwrite.h>
//#include <mferror.h>
//#include <evr.h>

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"shlwapi.lib")

//#pragma comment(lib, "mf.lib")
//#pragma comment(lib, "mfplat.lib")
//#pragma comment(lib, "mfuuid.lib")
//#pragma comment(lib, "mfreadwrite.lib")
//#pragma comment(lib,"strmiids.lib")

#include "Define/Define.hpp"
#include "Math/Math.hpp"
#include "Timer/Timer.h"
#include "Window/Window.h"
#include "Utility/Utility.hpp"

