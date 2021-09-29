//open_extra.cpp
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
#include "open_extra.hpp"
#include "wxpatch.hpp"
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/valnum.h>
#include <wx/sizer.h>

namespace
{
//在主窗口的打开对话框上显示的额外信息，要求用户选择首条程序的索引
class OpenExtra: public wxWindow
{
public:
	OpenExtra(wxWindow *parent);
	wxTextCtrl *m_edit;
	wxStaticText *m_label;
	unsigned long m_value=1;
};

OpenExtra::OpenExtra(wxWindow *parent):
	wxWindow(parent,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxBORDER_NONE),
	m_edit(new wxTextCtrl(this,wxID_ANY,U8S("1"),wxDefaultPosition,wxDefaultSize,0,
	[this]
	{
		wxIntegerValidator<unsigned> x;
		x.SetMax(499);
		return x;
	}())),
	m_label(new wxStaticText(this,wxID_ANY,U8S("首个程序的索引")))
{
	auto sizer=new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(1,0,3);
	sizer->Add(m_label,0,wxALL,3);
	sizer->Add(1,0,1);
	sizer->Add(m_edit,0,wxALL,3);
	sizer->Add(1,0,3);
	SetSizerAndFit(sizer);
	
	m_edit->Bind(wxEVT_TEXT,[this](wxCommandEvent&)
	{
		unsigned long x;
		if(m_edit->GetValue().ToCULong(&x))m_value=x;//只在成功时更新
	});
}
}

//在原本的实现中，此时通过m_edit->GetValue()获取编辑框的值，但在wxMSW中，
//此时m_edit->GetValue()为空字符串（这应该是wxWidgets 3.0.5的bug），
//所以改成了每次收到wxEVT_TEXT都将当前编辑框的内容保存到m_value中
unsigned OpenExtra_first_index(wxWindow *window)
{
	auto p=dynamic_cast<OpenExtra*>(window);
	if(!p)return 0;
	return static_cast<unsigned>(p->m_value);
}

wxWindow* OpenExtraCreator(wxWindow *parent)
{
	return new OpenExtra(parent);
}
