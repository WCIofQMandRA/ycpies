//mainwindow.hpp: 图形界面下，启动程序时加载的窗口（主窗口）
//包含类MainWindow
//Copyright (C) 2021 张子辰 <zichen350@gmail.com>

// 本文件是软件安川控制器程序导入导出系统 (Yaskawa Controller Program Import
// and Export System, ycpies, 以下简称“本软件”)的一部分。
// 在遵守以下条款的前提下，允许以任何方式和任何目的，运行、复制、分发和/或修改本
// 软件或将本软件与其他软件组合使用：
// 1. 在发布修改后的本软件或本软件的衍生作品的源代码时，在源代码中保留版权声明、
// 使用条款和免责声明。
// 2. 在发布修改后的本软件或本软件的衍生作品的二进制可执行文件时，以人类可读的方
// 式提及本软件的原作者和原作者发布本软件时使用的软件名称。
// 3. 不得以原作者的名义发布修改后的本软件或本软件的衍生作品；在发布修改后的本软
// 件时，应避免修改后的本软件被误认为是未经修改的本软件。
// 4. 除非技术上不可行或法律上不允许，使用(不含BOM的)UTF-8编码保存修改后的本软件
// 或本软件的衍生作品的源代码。

// 【免责声明】本软件“照原样”提供，不提供任何担保，包括但不限于适销性、适合于特定的
// 目的和不侵权。对于使用本软件所引起的任何损失、纠纷或侵权，本软件的作者概不负责。
#pragma once
#include <wx/frame.h>
#include <filesystem>
#include <memory>
#include <vector>
#include <wx/sizer.h>
#include <wx/checklst.h>

class instruction_set_t;
class raw_program_t;

class MainWindow: public wxFrame
{
public:
	MainWindow();

protected:
	bool OnNew();
	bool OnOpen();
	bool OnSave();
	bool OnSaveAs();
	bool OnCloseFile(bool confirm=true);
	void OnInstructionSet();
	void OnHelp();
	void OnAbout();
	void OnCloseWindow(wxCloseEvent&);
	void OnExport();
	void OnImport();
	void OnDelete();
private:
	std::filesystem::path iset_path,opened_file;
	std::unique_ptr<instruction_set_t> p_iset;
	std::unique_ptr<raw_program_t> raw_program;
	//一个打开的程序集(csv)中的非空程序列表
	//升序
	std::vector<unsigned> program_list;
	wxBoxSizer *editPlace;
	wxCheckListBox *programViewer;
	bool raw_program_unsave=false,is_opened=false;
	void load_instruction_set();
	
//保存在appdata下的信息
private:
	std::filesystem::path default_dir_opensave;//打开/另存为 文件时，对话框的默认路径
	std::filesystem::path default_dir_importexport;//导入导出的默认路径
	wxPoint default_position;//默认的窗口位置
	wxSize default_size;//默认的窗口大小
	bool default_maxwindow;//启动程序时是否全屏
	
	void load_appdata();
	void save_appdata();
};
