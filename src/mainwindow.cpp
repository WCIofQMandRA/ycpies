//mainwindow.cpp
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
#include "mainwindow.hpp"
#include "wxpatch.hpp"
#include <wx/menu.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include "shared_variable.hpp"
#include <fstream>
#include "instruction_set_t.hpp"
#include "raw_program.hpp"
#include "program_t.hpp"
#include "my_exceptions.hpp"
#include <boost/nowide/iostream.hpp>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/checklst.h>
#include "open_extra.hpp"
#include <algorithm>
#include "new_dialog.hpp"
#include "input_index_dialog.hpp"
#include "about_dialog.hpp"
#include "import_extra.hpp"
#include "override_program_dialog.hpp"
#include "export_extra.hpp"
#include <ctime>

namespace fs=std::filesystem;
namespace io=boost::nowide;
namespace po=boost::program_options;

static po::options_description config_options;
static bool has_added_config_options=false;

static void add_options()
{
	if(has_added_config_options)return;
	has_added_config_options=true;
	config_options.add_options()
		("dir.opensave",po::value<std::string>()->default_value(""))
		("dir.importexport",po::value<std::string>()->default_value(""))
		("position.x",po::value<int>()->default_value(-1))
		("position.y",po::value<int>()->default_value(-1))
		("size.w",po::value<int>()->default_value(-1))
		("size.h",po::value<int>()->default_value(-1))
		("maxwindow",po::value<bool>()->default_value(false))
		;
}

//IDs
const int ID_INSTRUCTION_SET=1;
const int ID_BUT_SELECT_ALL=2;
const int ID_BUT_CLEAR_ALL=3;
const int ID_BUT_IMPORT=4;
const int ID_BUT_EXPORT=5;
const int ID_BUT_DELET=6;
const int ID_CLEAR_ALL=7;
const int ID_IMPORT=8;
const int ID_EXPORT=9;

void MainWindow::load_instruction_set()
{
	using namespace shared_var;
	iset_path="";
	if(auto i=args.find("iset");i!=args.end())
		iset_path=fs::u8path(i->second.as<std::string>());
	else
	{
		if(!fs::is_regular_file(appdata/"iset.cfg"))
		{
			std::ofstream fout(appdata/"iset.cfg",std::ios::binary);
			fout<<"exedir\n"<<"data/instructions.csv\n";
			fout.close();
		}
		std::ifstream fin(appdata/"iset.cfg",std::ios::binary);
		if(fin)
		{
			std::string s1,s2;
			getline(fin,s1);getline(fin,s2);
			if(s1=="exedir")
				iset_path=exedir;
			else if(s1=="appdata")
				iset_path=appdata;
			iset_path/=fs::u8path(s2);
		}
		else io::cerr<<"无法确定指令集文件的位置."<<std::endl;
	}
	try
	{
		p_iset->load_csv(iset_path);
		program_t::set_instruction_set(*p_iset);
		raw_program_t::set_program_length(*p_iset);
	}
	catch(cannot_open_file_error &err)
	{
		io::cerr<<"无法打开指令集文件: \n"<<err.what()<<std::endl;
	}
	catch(file_format_error &err)
	{
		io::cerr<<"指令集文件格式错误: \n"<<err.what()<<std::endl;
	}
	catch(std::exception &err)
	{
		io::cerr<<"在读取指令集时遇到错误:\n"
			<<"type:\t"<<boost::core::demangle(typeid(err).name())<<"\n"
			<<"what():\t"<<err.what()<<std::endl;
	}
}

void MainWindow::load_appdata()
{
	using namespace shared_var;
	add_options();
	std::ifstream fin(appdata/"default.cfg",std::ios::binary);
	try
	{
		if(fin)
		{
			po::variables_map config;
			po::store(po::parse_config_file(fin,config_options,true),config);
			default_dir_opensave=fs::u8path(config["dir.opensave"].as<std::string>());
			default_dir_importexport=fs::u8path(config["dir.importexport"].as<std::string>());
			default_position=wxPoint(config["position.x"].as<int>(),config["position.y"].as<int>());
			default_size=wxSize(config["size.w"].as<int>(),config["size.h"].as<int>());
			default_maxwindow=config["maxwindow"].as<bool>();
		}
	}
	catch(...){}
}

void MainWindow::save_appdata()
{
	using namespace shared_var;
	std::ofstream fout(appdata/"default.cfg",std::ios::binary);
	fout<<"maxwindow="<<default_maxwindow<<"\n";
	fout<<"\n[position]\nx="<<default_position.x<<"\ny="<<default_position.y<<"\n";
	fout<<"\n[size]\nw="<<default_size.GetWidth()<<"\nh="<<default_size.GetHeight()<<"\n";
	fout<<"\n[dir]\nopensave="<<default_dir_opensave.u8string()<<"\nimportexport="<<default_dir_importexport.u8string()<<"\n";
	fout.close();
}

const wxString program_title=U8S("安川控制器程序导入导出系统");

MainWindow::MainWindow():
	wxFrame(nullptr,wxID_ANY,program_title),
	p_iset(std::make_unique<instruction_set_t>()),
	raw_program(std::make_unique<raw_program_t>())
{
	//菜单
	auto *menuFile=new wxMenu;
	menuFile->Append(wxID_NEW,U8S("新建(&N)...\tCtrl+N"));
	menuFile->AppendSeparator();
	menuFile->Append(wxID_OPEN,U8S("打开(&O)...\tCtrl+O"));
	menuFile->Append(wxID_SAVE,U8S("保存(&S)...\tCtrl+S"));
	menuFile->Append(wxID_SAVEAS,U8S("另存为..."));
	menuFile->AppendSeparator();
	menuFile->Append(wxID_CLOSE,U8S("关闭(&W)...\tCtrl+W"));
	menuEdit=new wxMenu;
	menuEdit->Append(wxID_SELECTALL,U8S("全选(&A)\tCtrl+A"));
	menuEdit->Append(ID_CLEAR_ALL,U8S("清除选择\tCtrl+Shift+A"));
	menuEdit->AppendSeparator();
	menuEdit->Append(wxID_DELETE,U8S("删除(&D)\tDelete"));
	menuEdit->AppendSeparator();
	menuEdit->Append(ID_IMPORT,U8S("导入...\tCtrl+Alt+I"));
	menuEdit->Append(ID_EXPORT,U8S("导出...\tCtrl+Alt+E"));
	menuEdit->Enable(wxID_SELECTALL,false);
	menuEdit->Enable(ID_CLEAR_ALL,false);
	menuEdit->Enable(wxID_DELETE,false);
	menuEdit->Enable(ID_EXPORT,false);
	menuEdit->Enable(ID_IMPORT,false);
	auto *menuOption=new wxMenu;
	menuOption->Append(ID_INSTRUCTION_SET,U8S("选择指令集(&I)...\tCtrl+Shift+I"));
	auto *menuHelp=new wxMenu;
	menuHelp->Append(wxID_HELP,U8S("帮助(&H)...\tF1"));
	menuHelp->Append(wxID_ABOUT,U8S("关于(&A)..."));
	
	auto *menuBar=new wxMenuBar;
	menuBar->Append(menuFile,U8S("文件(&F)"));
	menuBar->Append(menuEdit,U8S("编辑(&E)"));
	menuBar->Append(menuOption,U8S("选项(&O)"));
	menuBar->Append(menuHelp,U8S("帮助(&H)"));
	
	SetMenuBar(menuBar);
	
	//编辑区
	editPlace=new wxBoxSizer(wxVERTICAL);
	programViewer=new wxCheckListBox(this,wxID_ANY);
	/*for(int i=0;i<50;++i)
		programViewer->Append(std::to_string(i));*/
	
	editPlace->Add(programViewer,1,wxEXPAND|wxRESERVE_SPACE_EVEN_IF_HIDDEN);
	auto buttonSizer1=new wxBoxSizer(wxHORIZONTAL);
	auto buttonSizer2=new wxBoxSizer(wxHORIZONTAL);
	buttonSizer1->Add(new wxButton(this,ID_BUT_SELECT_ALL,U8S("全选")),0,wxALL|wxRESERVE_SPACE_EVEN_IF_HIDDEN,3);
	buttonSizer1->Add(new wxButton(this,ID_BUT_CLEAR_ALL,U8S("清除选择")),0,wxALL|wxRESERVE_SPACE_EVEN_IF_HIDDEN,3);
	buttonSizer2->Add(new wxButton(this,ID_BUT_IMPORT,U8S("导入程序")),0,wxALL|wxRESERVE_SPACE_EVEN_IF_HIDDEN,3);
	buttonSizer2->Add(new wxButton(this,ID_BUT_EXPORT,U8S("导出选中程序")),0,wxALL|wxRESERVE_SPACE_EVEN_IF_HIDDEN,3);
	buttonSizer2->Add(new wxButton(this,ID_BUT_DELET,U8S("删除选中程序")),0,wxALL|wxRESERVE_SPACE_EVEN_IF_HIDDEN,3);
	
	editPlace->Add(buttonSizer1,0,wxALIGN_RIGHT|wxALL|wxRESERVE_SPACE_EVEN_IF_HIDDEN,3);
	editPlace->Add(buttonSizer2,0,wxALIGN_RIGHT|wxALL|wxRESERVE_SPACE_EVEN_IF_HIDDEN,3);
	editPlace->ShowItems(false);
	
	SetSizerAndFit(editPlace);
	
	//事件处理函数
	Bind(wxEVT_MENU,[this](wxCommandEvent&){OnNew();},wxID_NEW);
	Bind(wxEVT_MENU,[this](wxCommandEvent&){OnOpen();},wxID_OPEN);
	Bind(wxEVT_MENU,[this](wxCommandEvent&){OnSave();},wxID_SAVE);
	Bind(wxEVT_MENU,[this](wxCommandEvent&){OnSaveAs();},wxID_SAVEAS);
	Bind(wxEVT_MENU,[this](wxCommandEvent&){OnCloseFile();},wxID_CLOSE);
	Bind(wxEVT_MENU,[this](wxCommandEvent&){OnInstructionSet();},ID_INSTRUCTION_SET);
	Bind(wxEVT_MENU,[this](wxCommandEvent&){OnHelp();},wxID_HELP);
	Bind(wxEVT_MENU,[this](wxCommandEvent&){OnAbout();},wxID_ABOUT);
	Bind(wxEVT_MENU,[this](wxCommandEvent&){OnSelectAll(true);},wxID_SELECTALL);
	Bind(wxEVT_MENU,[this](wxCommandEvent&){OnSelectAll(false);},ID_CLEAR_ALL);
	Bind(wxEVT_MENU,[this](wxCommandEvent&){OnDelete();},wxID_DELETE);
	Bind(wxEVT_MENU,[this](wxCommandEvent&){OnExport();},ID_EXPORT);
	Bind(wxEVT_MENU,[this](wxCommandEvent&){OnImport();},ID_IMPORT);
	Bind(wxEVT_BUTTON,[this](wxCommandEvent&){OnSelectAll(true);},ID_BUT_SELECT_ALL);
	Bind(wxEVT_BUTTON,[this](wxCommandEvent&){OnSelectAll(false);},ID_BUT_CLEAR_ALL);
	Bind(wxEVT_CLOSE_WINDOW,&MainWindow::OnCloseWindow,this);
	Bind(wxEVT_BUTTON,[this](wxCommandEvent&){OnExport();},ID_BUT_EXPORT);
	Bind(wxEVT_BUTTON,[this](wxCommandEvent&){OnImport();},ID_BUT_IMPORT);
	Bind(wxEVT_BUTTON,[this](wxCommandEvent&){OnDelete();},ID_BUT_DELET);
	
	//加载指令集
	load_instruction_set();
	
	//默认设置
	load_appdata();
	SetSize(default_size);
	SetPosition(default_position);
	Maximize(default_maxwindow);
}

void MainWindow::show_edit_place()
{
	editPlace->ShowItems(true);
	menuEdit->Enable(wxID_SELECTALL,true);
	menuEdit->Enable(ID_CLEAR_ALL,true);
	menuEdit->Enable(wxID_DELETE,true);
	menuEdit->Enable(ID_EXPORT,true);
	menuEdit->Enable(ID_IMPORT,true);
}
void MainWindow::hide_edit_place()
{
	editPlace->ShowItems(false);
	menuEdit->Enable(wxID_SELECTALL,false);
	menuEdit->Enable(ID_CLEAR_ALL,false);
	menuEdit->Enable(wxID_DELETE,false);
	menuEdit->Enable(ID_EXPORT,false);
	menuEdit->Enable(ID_IMPORT,false);
}

bool MainWindow::OnNew()
{
	if(raw_program_unsave)
	{
		auto choice=wxMessageBox(U8S("当前控制器程序集尚未保存, 是否保存?"),
		U8S("提示"),wxYES|wxNO|wxCANCEL,this);
		if(choice==wxCANCEL)return false;
		if(choice==wxYES)
		{
			if(!OnSave())return false;
		}
	}
	NewDialog ndia(this,U8S("新建"));
	//io::cout<<ndia.ShowModal()<<std::endl;
	if(ndia.ShowModal()!=wxID_OK)return false;
	if(is_opened)
	{
		if(!OnCloseFile(false))return false;
	}
	raw_program->load_null(ndia.get_first(),ndia.get_size());
	raw_program_unsave=true;
	is_opened=true;
	opened_file="";
	SetTitle(wxString::Format(U8S("%s-*"),program_title));
	programViewer->Clear();
	program_list.clear();
	show_edit_place();
	return true;
}

bool MainWindow::OnOpen()
{
	if(raw_program_unsave)
	{
		auto choice=wxMessageBox(U8S("当前控制器程序集尚未保存, 是否保存?"),
		U8S("提示"),wxYES|wxNO|wxCANCEL,this);
		if(choice==wxCANCEL)return false;
		if(choice==wxYES)
		{
			if(!OnSave())return false;
		}
	}
	wxFileDialog ofdia(this,U8S("打开控制器程序集"),U8S(default_dir_opensave.u8string().c_str()),"",
		U8S("逗号分割文件 (*.csv)|*.csv|所有文件|*"),wxFD_OPEN|wxFD_FILE_MUST_EXIST);
	ofdia.SetExtraControlCreator(OpenExtraCreator);
	if(ofdia.ShowModal()==wxID_CANCEL)return false;
	if(is_opened)
	{
		if(!OnCloseFile(false))return false;
	}
	try
	{
		auto path=fs::u8path(ofdia.GetPath().utf8_str().data());
		raw_program->load_csv(path,OpenExtra_first_index(ofdia.GetExtraControl()));
		
		default_dir_opensave=path.parent_path();
//		save_default_dir();
		
		raw_program_unsave=false;
		is_opened=true;
		opened_file=path;
		SetTitle(wxString::Format(U8S("%s-%s"),program_title,ofdia.GetPath()));
		programViewer->Clear();
		program_list.clear();
		for(auto it=raw_program->cbegin();it!=raw_program->cend();++it)
		{
			programViewer->Append(wxString::Format(U8S("程序 #%03u"),(unsigned)it->first));
			program_list.push_back(it->first);
		}
		show_edit_place();
			//io::cout<<it->first<<"\n";
		return true;
		
	}
	catch(file_format_error &err)
	{
		io::cerr<<"文件格式错误: \n"<<err.what()<<std::endl;
	}
	catch(std::exception &err)
	{
		io::cerr<<"错误:\n"
			<<"type:\t"<<boost::core::demangle(typeid(err).name())<<"\n"
			<<"what():\t"<<err.what()<<std::endl;
	}
	return false;
}

bool MainWindow::OnSave()
{
	if(!is_opened)return false;
	if(!raw_program_unsave)return true;
	if(opened_file=="")return OnSaveAs();
	try
	{
		raw_program->save_csv_origin(opened_file);
		SetTitle(wxString::Format(U8S("%s-%s"),program_title,U8S(opened_file.u8string().c_str())));
		raw_program_unsave=false;
		return true;
	}
	catch(std::exception &err)
	{
		io::cerr<<"错误:\n"
			<<"type:\t"<<boost::core::demangle(typeid(err).name())<<"\n"
			<<"what():\t"<<err.what()<<std::endl;
	}
	return false;
}

bool MainWindow::OnSaveAs()
{
	if(!is_opened)return false;
	wxFileDialog sfdia(this,U8S("另存为"),U8S(default_dir_opensave.u8string().c_str()),U8S(opened_file.u8string().c_str()),
		U8S("逗号分割文件 (*.csv)|*.csv|所有文件|*"),wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
	//sfdia.SetExtraControlCreator(OpenExtraCreator);//TODO:额外的保存选项
	if(sfdia.ShowModal()==wxID_CANCEL)return false;
	try
	{
		auto path=fs::u8path(sfdia.GetPath().utf8_str().data());
		if(path.extension()!=".csv")path+=".csv";
		raw_program->save_csv_origin(path);
		
		default_dir_opensave=path.parent_path();
		opened_file=path;
		
		SetTitle(wxString::Format(U8S("%s-%s"),program_title,U8S(path.u8string().c_str())));
		raw_program_unsave=false;
		return true;
	}
	catch(std::exception &err)
	{
		io::cerr<<"错误:\n"
			<<"type:\t"<<boost::core::demangle(typeid(err).name())<<"\n"
			<<"what():\t"<<err.what()<<std::endl;
	}
	return false;
}

bool MainWindow::OnCloseFile(bool confirm)
{
	if(!is_opened)return true;
	if(confirm&&raw_program_unsave)
	{
		auto choice=wxMessageBox(U8S("当前控制器程序集尚未保存, 是否保存?"),
		U8S("提示"),wxYES|wxNO|wxCANCEL,this);
		if(choice==wxCANCEL)return false;
		if(choice==wxYES)
		{
			if(!OnSave())return false;
		}
	}
	raw_program_unsave=false;
	is_opened=false;
	opened_file="";
	SetTitle(program_title);
	programViewer->Clear();
	program_list.clear();
	hide_edit_place();
	return true;
}

void MainWindow::OnInstructionSet()
{
	wxMessageBox(U8S("暂不支持切换指令集，默认的指令集目录是<可执行文件目录>/data/instructions.csv ，"
	"您可以通过更改该文件修改指令集。或者，您也可以使用命令行参数--iset切换指令集。"),U8S("抱歉"),wxOK);
}

void MainWindow::OnHelp()
{
	wxMessageBox(wxString::Format(U8S("暂无帮助信息。命令行参数的帮助信息可以使用%s --help查看。"),U8S(shared_var::argv0.c_str())),
	U8S("抱歉"),wxOK);
}

void MainWindow::OnAbout()
{
	AboutDialog dialog(this,U8S("关于"));
	dialog.ShowModal();
}

void MainWindow::OnCloseWindow(wxCloseEvent &event)
{
	if(event.CanVeto()&&raw_program_unsave&&
		wxMessageBox(U8S("当前控制器程序集尚未保存, 是否仍然退出?"),
		U8S("提示"),wxYES_NO,this)==wxNO)
	{
		event.Veto();
		return;
	}
	default_position=GetPosition();
	default_size=GetSize();
	default_maxwindow=IsMaximized(); 
	save_appdata();
//	Destroy();
	event.Skip();
}

void MainWindow::OnExport()
{
	auto n=program_list.size();
	bool has_checked=false;
	for(unsigned i=0;i<n;++i)
	{
		if(programViewer->IsChecked(i))
		{
			has_checked=true;
			break;
		}
	}
	if(!has_checked)return;
	wxFileDialog sfdia(this,U8S("导出"),U8S(default_dir_importexport.u8string().c_str()),"",
		U8S("Mircrosoft Excel 表格 (*.xlsx)|*.xlsx|所有文件|*"),wxFD_SAVE);
	sfdia.SetExtraControlCreator(ExportExtraCreator);
	if(sfdia.ShowModal()==wxID_CANCEL)return;
	
	std::ofstream log("time_cost.log",std::ios::app);
	clock_t clock1=clock();
	
	program_xlsx_style export_style=ExportExtra_style(sfdia.GetExtraControl());
	auto path=fs::u8path(sfdia.GetPath().utf8_str().data());
	auto filename=path.filename();
	if(filename.extension()==".xlsx")filename=filename.stem();
	{
		auto s=filename.extension().u8string();
		auto x=[](char ch){return ch>='0'&&ch<='9';};
		if(s.size()==4&&x(s[1])&&x(s[2])&&x(s[3]))filename=filename.stem();
	}
	default_dir_importexport=path.parent_path();
	int override_when_exist=0;//1：覆盖, -1:不覆盖, 0:询问
	for(unsigned i=0;i<n;++i)
	{
		if(programViewer->IsChecked(i))
		{
			auto tostr=[](unsigned x)->fs::path
			{
				char c[4];
				c[2]=x%10+48;x/=10;
				c[1]=x%10+48;x/=10;
				c[0]=x%10+48;
				c[3]=0;
				return fs::u8path(c);
			};
			auto fn=filename;
			((fn+='.')+=tostr(program_list[i]))+=".xlsx";
			auto fp=default_dir_importexport/fn;
			if(fs::is_regular_file(fp))
			{
				if(override_when_exist==-1)continue;
				if(override_when_exist==0)
				{
					OverrideProgramDialog mdialog(this,U8S(fn.u8string().c_str()));
					int rv=mdialog.ShowModal();
					if(rv==wxID_CANCEL)
					{
						if(mdialog.remember())override_when_exist=-1;
						continue;
					}
					else if(mdialog.remember())
						override_when_exist=1;
				}
			}
			try
			{
				program_t pro;
				pro.load_raw_program(raw_program->get_raw_program(program_list[i]));
				pro.save_xlsx(fp,export_style,&io::cerr);
			}
			catch(file_format_error &err)
			{
				io::cerr<<"文件格式错误: \n"<<err.what()<<std::endl;
			}
			catch(std::exception &err)
			{
				io::cerr<<"错误:\n"
					<<"type:\t"<<boost::core::demangle(typeid(err).name())<<"\n"
					<<"what():\t"<<err.what()<<std::endl;
			}
		}
	}
	
	clock_t clock2=clock();
	log<<"Export "<<n<<" "<<double(clock2-clock1)/CLOCKS_PER_SEC<<std::endl;
	
}

void MainWindow::OnImport()
{
	wxFileDialog ofdia(this,U8S("导入"),U8S(default_dir_importexport.u8string().c_str()),"",
		U8S("Mircrosoft Excel 表格 (*.xlsx)|*.xlsx|所有文件|*"),wxFD_OPEN|wxFD_FILE_MUST_EXIST|wxFD_MULTIPLE);
	ofdia.SetExtraControlCreator(ImportExtraCreator);
	if(ofdia.ShowModal()==wxID_CANCEL)return;
	bool have_import=false;
	std::vector<fs::path> paths;
	{
		wxArrayString pas;
		ofdia.GetPaths(pas);
		auto n=pas.GetCount();
		for(unsigned i=0;i<n;++i)
			paths.emplace_back(fs::u8path(pas[i].utf8_str().data()));
	}
	default_dir_importexport=paths[0].parent_path();
	const bool force_reset_index=ImportExtra_force_reset_index(ofdia.GetExtraControl());
	for(auto &path:paths)
	{
		auto filename=path.filename();
		if(filename.extension()==".xlsx")filename=filename.stem();
		unsigned index=10000;
		if(!force_reset_index)
		{
			auto s=filename.extension().u8string();
			if(s.size()==4)
				index=unsigned(s[1]-'0')*100+unsigned(s[2]-'0')*10+unsigned(s[3]-'0');
		}
		auto [begin_index,end_index]=raw_program->index_range();
		auto contains=[this](unsigned index)->bool
		{
			auto p=std::lower_bound(program_list.begin(),program_list.end(),index);
			return p!=program_list.end()&&*p==index;
		};
		if(force_reset_index)
		{
			InputIndexDialog dialog(this,U8S("输入索引"),
				wxString::Format(U8S("请输入文件 `%s` 的程序索引："),
				U8S(filename.u8string().c_str())),begin_index,end_index,program_list);
			if(dialog.ShowModal()!=wxID_OK)continue;
			index=dialog.index();
		}
		else if(index==10000)
		{
			InputIndexDialog dialog(this,U8S("输入索引"),
				wxString::Format(U8S("无法从文件 `%s` 的文件名中获得程序索引，请输入程序索引："),
				U8S(filename.u8string().c_str())),begin_index,end_index,program_list);
			if(dialog.ShowModal()!=wxID_OK)continue;
			index=dialog.index();
		}
		else if(!(begin_index<=index&&index<end_index))
		{
			InputIndexDialog dialog(this,U8S("输入索引"),
				wxString::Format(U8S("索引%03u超出了当前打开的程序集的索引范围[%03u,%03u)，请重新输入："),
				index,begin_index,end_index),begin_index,end_index,program_list);
			if(dialog.ShowModal()!=wxID_OK)continue;
			index=dialog.index();
		}
		else if(contains(index))
		{
			InputIndexDialog dialog(this,U8S("警告"),
				wxString::Format(U8S("程序 #%03u 已存在，您可以重新输入索引或覆盖原有程序。\n注意：目前覆盖操作不可逆！"),index,
				index,begin_index,end_index),begin_index,end_index,program_list,index);
			if(dialog.ShowModal()!=wxID_OK)continue;
			index=dialog.index();
		}
		try
		{
			program_t pro;
			pro.load_xlsx(path,&io::cerr);
			std::vector<int16_t> raw;
			pro.save_raw_program(raw);
			raw_program->set_raw_program(index,raw);
			if(!contains(index))
			{
				auto p=std::lower_bound(program_list.begin(),program_list.end(),index);
				programViewer->Insert(wxString::Format(U8S("程序 #%03u"),index),p-program_list.begin());
				program_list.insert(p,index);
			}
			
			have_import=true;
		}
		catch(file_format_error &err)
		{
			io::cerr<<"文件格式错误: \n"<<err.what()<<std::endl;
		}
		catch(std::exception &err)
		{
			io::cerr<<"错误:\n"
				<<"type:\t"<<boost::core::demangle(typeid(err).name())<<"\n"
				<<"what():\t"<<err.what()<<std::endl;
		}
	}
	if(have_import)
	{
		raw_program_unsave=true;
		SetTitle(wxString::Format(U8S("%s-*%s"),program_title,U8S(opened_file.u8string().c_str())));
	}
}

void MainWindow::OnDelete()
{
	auto n=program_list.size();
	bool have_deleted=false;
	for(unsigned i=0;i<n;)
	{
		if(programViewer->IsChecked(i))
		{
			if(wxMessageBox(wxString::Format(U8S("删除程序 #%u?\n注意：目前该操作不可逆！"),program_list[i]),
				U8S("警告"),wxYES|wxNO)==wxNO)//TODO: 之后全选是 和 之后全选否 https://docs.wxwidgets.org/3.0.5/classwx_file_dialog.html
			{
				++i;continue;
			}
			raw_program->remove(program_list[i]);
			have_deleted=true;
			programViewer->Delete(i);
			program_list.erase(program_list.begin()+i);
			--n;
		}
		else ++i;
	}
	if(have_deleted)
	{
		raw_program_unsave=true;
		SetTitle(wxString::Format(U8S("%s-*%s"),program_title,U8S(opened_file.u8string().c_str())));
	}
}

// void MainWindow::OnKeyPress(wxKeyEvent &event)
// {
// 	std::cout<<"111"<<std::endl;
// 	switch(event.GetKeyCode())
// 	{
// 	case WXK_CONTROL|'A':OnSelectAll();break;
// 	default:
// 		std::cout<<event.GetKeyCode()<<std::endl;
// 	}
// }

void MainWindow::OnSelectAll(bool check)
{
	auto n=program_list.size();
	for(unsigned i=0;i<n;++i)
		programViewer->Check(i,check);
}