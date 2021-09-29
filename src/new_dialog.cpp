//new_dialog.cpp
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
#include "new_dialog.hpp"
#include "wxpatch.hpp"
#include <wx/valnum.h>
#include <wx/sizer.h>

NewDialog::NewDialog(wxWindow *parent,const wxString &caption):
	wxDialog(parent,wxID_ANY,caption)
{
	wxIntegerValidator<unsigned> validator;
	validator.SetMax(499);
	m_edit_first=new wxTextCtrl(this,wxID_ANY,U8S("1"),wxDefaultPosition,wxDefaultSize,0,validator);
	m_edit_size=new wxTextCtrl(this,wxID_ANY,U8S("50"),wxDefaultPosition,wxDefaultSize,0,validator);
	m_label_first=new wxStaticText(this,wxID_ANY,U8S("首个程序的索引"));
	m_label_size=new wxStaticText(this,wxID_ANY,U8S("程序数量"));
	auto sizer1=new wxFlexGridSizer(2);
	sizer1->Add(m_label_first,0,wxALL,3);
	sizer1->Add(m_edit_first,0,wxALL,3);
	sizer1->Add(m_label_size,0,wxALL,3);
	sizer1->Add(m_edit_size,0,wxALL,3);
	auto sizer=new wxBoxSizer(wxVERTICAL);
	sizer->Add(sizer1,0,wxALIGN_CENTRE_HORIZONTAL);
	sizer->Add(CreateButtonSizer(wxOK|wxCANCEL),0,wxALIGN_CENTRE_HORIZONTAL);
	SetSizerAndFit(sizer);
	
	//固定窗口大小
	SetMaxSize(GetSize());
	SetMinSize(GetSize());
}

unsigned NewDialog::get_first()const
{
	unsigned long index=0;
	if(!m_edit_first->GetValue().ToCULong(&index))index=0;
	return static_cast<unsigned>(index);
}

unsigned NewDialog::get_size()const
{
	unsigned long index=0;
	if(!m_edit_size->GetValue().ToCULong(&index))index=0;
	return static_cast<unsigned>(index);
}
