#include<wx/wx.h>
#include <wx/toolbar.h>
#include <wx/filedlg.h>
#include <wx/image.h>
#include <wx/artprov.h>
#include <vector>
#include <wx/frame.h>       // 包含框架窗口相关功能
#include <wx/treectrl.h>    // 包含树控件的相关功能
#include <fstream>
#include <nlohmann/json.hpp>
#include "MyFrame.cpp"
#include "Common.cpp"

using namespace nlohmann; // 使用 nlohmann 命名空间

class MyApp : public wxApp {
public:
    virtual bool OnInit() {
        MyFrame* frame = new MyFrame();
        return true;
    }
};

// 实现应用程序入口点
wxIMPLEMENT_APP(MyApp);