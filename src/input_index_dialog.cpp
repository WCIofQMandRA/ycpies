//input_index_dialog.cpp
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
#include "input_index_dialog.hpp"
#include <wx/stattext.h>
#include <wx/valnum.h>
#include <wx/sizer.h>
#include <algorithm>
#include "wxpatch.hpp"

InputIndexDialog::InputIndexDialog(wxWindow *parent,const wxString &caption,
	const wxString &message,unsigned begin_index,unsigned end_index,const std::vector<unsigned> &program_list,unsigned default_index):
	wxDialog(parent,wxID_ANY,caption,wxDefaultPosition,wxDefaultSize,wxCAPTION),
	program_list(program_list),begin_index(begin_index),end_index(end_index),current_index(default_index)
{
	auto label=new wxStaticText(this,wxID_ANY,message,wxDefaultPosition,wxDefaultSize,wxALIGN_LEFT);
	label->Wrap(1000);
	wxIntegerValidator<unsigned> validator;
//	validator.SetMin(begin_index); //否则会导致编辑框的内容被清空后无法再次输入
	validator.SetMax(end_index-1);
	if(current_index==10000)current_index=find_first_unused_index();
	m_edit=new wxTextCtrl(this,wxID_ANY,std::to_string(current_index),wxDefaultPosition,wxDefaultSize,0,validator);
	auto sizer=new wxBoxSizer(wxVERTICAL);
	sizer->Add(label,0,wxALL|wxALIGN_CENTRE_HORIZONTAL,3);
	sizer->Add(m_edit,0,wxALL|wxALIGN_CENTRE_HORIZONTAL,3);
	auto button_sizer=new wxBoxSizer(wxHORIZONTAL);
	if(contains(current_index))
		button_import=new wxButton(this,wxID_OK,U8S("导入并覆盖"));
	else
		button_import=new wxButton(this,wxID_OK,U8S("导入"));
	button_sizer->Add(button_import,0,wxALL,3);
	button_sizer->Add(new wxButton(this,wxID_CANCEL,U8S("跳过该文件")),0,wxALL,3);
	sizer->Add(button_sizer,0,wxALIGN_CENTRE_HORIZONTAL);
	SetSizerAndFit(sizer);
	
	//固定窗口大小
	SetMaxSize(GetSize());
	SetMinSize(GetSize());
	
	m_edit->Bind(wxEVT_TEXT,[this](wxCommandEvent&)
	{
		if(m_edit->GetValue().IsEmpty())
		{
			button_import->Disable();
		}
		else
		{
			unsigned long x;
			m_edit->GetValue().ToCULong(&x);
			if(x<this->begin_index||x>=this->end_index)
			{
				button_import->Disable();
			}
			else
			{
				button_import->Enable();
				current_index=static_cast<unsigned>(x);
				if(contains(current_index))
					button_import->SetLabel(U8S("导入并覆盖"));
				else
					button_import->SetLabel(U8S("导入"));
			}
		}
	});
}

bool InputIndexDialog::contains(unsigned index)const
{
	auto p=std::lower_bound(program_list.begin(),program_list.end(),index);
	return p!=program_list.end()&&*p==index;
}

unsigned InputIndexDialog::find_first_unused_index()const
{
	unsigned i,j;
	for(i=begin_index,j=0;i<end_index&&j<program_list.size();++i,++j)
	{
		if(program_list[j]!=i)return i;
	}
	if(i==end_index)i=begin_index;
	return i;
}

unsigned InputIndexDialog::index()const
{
	return current_index;
}
