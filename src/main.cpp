//main.cpp: 主函数、命令行参数处理
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
#include <wx/app.h>
#include <memory>
#include <filesystem>
#include "fix-program_options-bug.hpp"
#include <boost/nowide/iostream.hpp>
#include "mainwindow.hpp"
#include "shared_variable.hpp"
#include <string>
#include <boost/nowide/args.hpp>
#include <wx/intl.h>
namespace po=boost::program_options;
namespace io=boost::nowide;

static po::options_description shown_options(65536);
static po::options_description all_options;

static void add_options()
{
	po::options_description op_help("帮助选项",65536);
	op_help.add_options()
		("help,h","输出本帮助信息, 然后退出.")
		("version,v","输出版本信息，然后退出.")
		;
	po::options_description op_config("配置信息选项",65536);
	op_config.add_options()
		("config-dir,C",po::value<std::string>()->value_name("<路径>")->default_value(shared_var::appdata.generic_u8string()),"指定配置目录的位置.")
		("iset,I",po::value<std::string>()->value_name("<路径>"),"指定控制器程序使用的指令集 (默认从配置目录读取).")
		;
	
	shown_options.add(op_help).add(op_config);
	
	po::options_description hidden_options;
	hidden_options.add_options()
		("license",po::value<std::string>()->implicit_value("ycpies"),"")
		;

	all_options.add(shown_options).add(hidden_options);
}

static void version_info()
{
	io::cout<<"安川控制器程序导入导出系统 Version " YCPIES_VERSION<<"\n"
		"Copyright (C) 2021 张子辰\n"
		"本软件是自由软件，在遵守本软件的使用条款的前提下，允许以任何方式和任何\n"
		"目的，运行、复制、分发和/或修改本软件或将本软件与其他软件组合使用.\n"
		"【免责声明】本软件“照原样”提供，不提供任何担保.\n\n"
		"详细信息请使用"<<shared_var::argv0<<" --license查看."<<std::endl;
}

static void license_info(const std::string &component)
{
	try
	{
		auto s=shared_var::license.at(component);
		{
			size_t i,j;
			for(i=0,j=0;i<s.size();++i)
			{
				if(s[i]!='%'||s[i+1]!='\n')s[j++]=s[i];
			}
			s.resize(j);
		}
		io::cout<<s<<std::endl;
		if(component=="ycpies")
		{
			io::cout<<"本软件使用的外部依赖有:\n1. libcsv-3.0.3, 许可证可使用 "<<shared_var::argv0<<" --license libcsv 查看\n"
			<<"2. xlnt-1.5.0, 许可证可使用 "<<shared_var::argv0<<" --license xlnt 查看"<<std::endl;
		}
	}
	catch(...)
	{
		io::cout<<"本软件不包含组件 `"<<component<<"`"<<std::endl;
	}
}

static void help_info()
{
	io::cout<<"用法: "<<shared_var::argv0<<" [选项]"<<std::endl;
	print_options_decription(io::cout,shown_options);
	io::cout<<std::endl;
}

class YcpiesApp: public wxApp
{
public:
	bool OnInit()override
	{
		the_window=new MainWindow;
		the_window->Show(true);
		return true;
	}
private:
	MainWindow *the_window;
	wxLocale m_local{wxLANGUAGE_DEFAULT};
};

#ifdef _WIN32
extern "C"
{
#ifdef _MSC_VER
	int __declspec(dllimport) FreeConsole(void);
	int __declspec(dllimport) SetProcessDPIAware(void);
#else
	int __attribute__((dllimport)) FreeConsole(void);
	int __attribute__((dllimport)) SetProcessDPIAware(void);
#endif
}
#endif

int main(int argc,char **argv)
{
	wxDISABLE_DEBUG_SUPPORT();
	io::args ____(argc,argv);
	shared_var::argv0=argv[0];
	add_options();
	try
	{	
		po::store(po::parse_command_line(argc,argv,all_options),shared_var::args);
	}
	catch(std::exception &x)
	{
		io::cerr<<"在分析命令行参数时遇到错误:\n"
			<<"type:\t"<<boost::core::demangle(typeid(x).name())<<"\n"
			<<"what():\t"<<x.what()<<std::endl;
		io::cerr<<"使用--help查看帮助."<<std::endl;
		return false;
	}
	if(shared_var::args.count("help"))
	{
		help_info();
		return 0;
	}
	if(shared_var::args.count("version"))
	{
		version_info();
		return 0;
	}
	if(shared_var::args.count("license"))
	{
		license_info(shared_var::args["license"].as<std::string>());
		return 0;
	}
	if(auto i=shared_var::args.find("config-dir");i!=shared_var::args.end())
	{
		shared_var::appdata=std::filesystem::u8path(i->second.as<std::string>());
	}
#ifdef _WIN32
	SetProcessDPIAware();
	FreeConsole();
#endif
	return wxEntry(argc,argv);
}
wxIMPLEMENT_APP_NO_MAIN(YcpiesApp);
