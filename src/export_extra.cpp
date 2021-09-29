//export_extra.cpp
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
#include "export_extra.hpp"
#include "wxpatch.hpp"
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/sizer.h>
#include <wx/valnum.h>

namespace
{
//在导入对话框上显示的额外信息，要求用户确定是否强制重新指定程序索引
class ExportExtra: public wxWindow
{
public:
	ExportExtra(wxWindow *parent);
	wxStaticText *label_comment,*label_width,*label_multi;
	wxChoice *style_comment,*style_width;
	wxTextCtrl *style_multi;
	bool width_auto=true;
	wxString style_multi_old_text=wxString::Format("%.1f",PXLSX_WIDTH_V0_1);
	program_xlsx_style style=PXLSX_DEFAULT_STYLE;
};

ExportExtra::ExportExtra(wxWindow *parent):
	wxWindow(parent,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxBORDER_NONE),
	label_comment(new wxStaticText(this,wxID_ANY,U8S("注释位置"))),label_width(new wxStaticText(this,wxID_ANY,U8S("列宽度"))),
	label_multi(new wxStaticText(this,wxID_ANY,U8S("修正倍率     ")))//翻译建议：在“修正倍率”后加空白字符，使其宽度大于“输入列宽度”
{
	{
		wxString tmp[]={U8S("左侧带冒号"),U8S("左侧"),U8S("右侧")};
		style_comment=new wxChoice(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,3,tmp);
	}
	{
		wxString tmp[]={U8S("自动计算"),U8S("手动输入")};
		style_width=new wxChoice(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,2,tmp);
	}
	
	wxFloatingPointValidator<double> validator(4,nullptr,wxNUM_VAL_NO_TRAILING_ZEROES);
	validator.SetMax(499);
	validator.SetRange(0,99);
	//wxWidgets bug: 同时使用hint和validator会引起段错误
	style_multi=new wxTextCtrl(this,wxID_ANY,wxString::Format("%.2f",PXLSX_WIDTH_MULTI_DEFAULT),wxDefaultPosition,wxDefaultSize,0,validator);
	//style_multi->SetHint(wxString::Format("%.2f",PXLSX_WIDTH_MULTI_DEFAULT));
	
	style_comment->SetSelection(0);
	style_width->SetSelection(0);
	
	auto sizer=new wxFlexGridSizer(2);
	sizer->Add(label_comment,0,wxALL,2);
	sizer->Add(style_comment,0,wxALL,2);
	sizer->Add(label_width,0,wxALL,2);
	sizer->Add(style_width,0,wxALL,2);
	sizer->Add(label_multi,0,wxALL,2);
	sizer->Add(style_multi,0,wxALL,2);
	
	SetSizerAndFit(sizer);
	
	style_comment->Bind(wxEVT_CHOICE,[this](wxCommandEvent&)
	{
		switch(style_comment->GetSelection())
		{
		case 0:style.comment=PXLSX_COMMENT_LCOLON;break;
		case 1:style.comment=PXLSX_COMMENT_LEFT;break;
		case 2:style.comment=PXLSX_COMMENT_RIGHT;break;
		}
	});
	style_width->Bind(wxEVT_CHOICE,[this](wxCommandEvent&)
	{
		//style_width的选项的更换会触发一系列动作，所以需要用width_auto检测是否更换了选项
		switch(style_width->GetSelection())
		{
		case 0:
			if(!width_auto)
			{
				width_auto=true;
				style.width=PXLSX_WIDTH_AUTO;
				label_multi->SetLabel(U8S("修正倍率"));
				auto s=style_multi->GetValue();
				//style_multi->ChangeValue("");
				//style_multi->SetHint(wxString::Format("%.2f",PXLSX_WIDTH_MULTI_DEFAULT));
				style_multi->SetValue(style_multi_old_text);
				style_multi_old_text=s;
				
			}
			break;
		case 1:
			if(width_auto)
			{
				width_auto=false;
				label_multi->SetLabel(U8S("输入列宽度"));
				auto s=style_multi->GetValue();
				//style_multi->ChangeValue("");
				//style_multi->SetHint(wxString::Format("%.2f",PXLSX_WIDTH_V0_1));
				style_multi->SetValue(style_multi_old_text);
				style_multi_old_text=s;
			}
			break;
		}
	});
	style_multi->Bind(wxEVT_TEXT,[this](wxCommandEvent&)
	{
		double d;
		if(style_multi->GetValue().IsEmpty())
		{
			if(width_auto)
				style.width_auto_multi=PXLSX_WIDTH_MULTI_DEFAULT;
			else
				style.width=PXLSX_WIDTH_V0_1;
		}
		if(style_multi->GetValue().ToDouble(&d))
		{
			if(width_auto)
				style.width_auto_multi=d;
			else
				style.width=d;
		}
	});
	style_multi->Bind(wxEVT_KILL_FOCUS,[this](wxFocusEvent&)
	{
		if(style_multi->GetValue().IsEmpty())
		{
			if(width_auto)
				style_multi->ChangeValue(wxString::Format("%.2f",PXLSX_WIDTH_MULTI_DEFAULT));
			else
				style_multi->ChangeValue(wxString::Format("%.1f",PXLSX_WIDTH_V0_1));
		}
	});
}
}

program_xlsx_style ExportExtra_style(wxWindow *window)
{
	auto p=dynamic_cast<ExportExtra*>(window);
	if(!p)return PXLSX_DEFAULT_STYLE;
	return p->style;
}

wxWindow* ExportExtraCreator(wxWindow *parent)
{
	return new ExportExtra(parent);
}
