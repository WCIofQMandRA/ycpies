# 安川控制器程序导入导出系统 Yaskawa Controller Program Import and Export System

Version 0.2.0

## 编译

需求：

1. 支持 C++ 17 的编译器；
2. CMake 3.4+；
3. Boost 1.73+（目前 Boost 必须作为系统库安装，即编译器可以直接使用 Boost，而无需加入额外的编译/连接目录）；
4. wxWidgets 3.0+（wxWidgets 必须能够通过CMake的`find_package(wxWidgets)`找到）；

命令：

```
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config=Release
```

## 更新记录

### v0.1.0.1 (20210830)

1. 在使用`MSVC`且编译动态库时，在`libcsv`的函数前加`__declspec(dllexport)`。
2. 在 Win32 平台下，添加连接参数`\subsystem:windows`或`-mwindows`；并在主函数结束前调用`FreeConsole()`以隐藏控制台。
3. 将`shared_variable.cpp`的`GetModuleFileNameA`改成`GetModuleFileNameW`，以解决编译错误（之前写作`GetModuleFileNameA`可能是手误）。

### v0.1.0.2 (20210830)

1. 删去了v0.1.0.1中加入的`FreeConsole()`，以解决在`Ubuntu`上编译时的编译错误（因为误以为使用`\subsystem:windows`后就不再需要`FreeConsole()`了）。

### v0.1.1.0 (20210831)

1. 修复了`Windows`下新建、打开等菜单选项快捷键缺失的问题。
2. 恢复了主函数结束处的`FreeConsole()`，并在`FreeConsole()`前加上了`SetProcessDPIAware()`，以解决`Windows`平台上窗口模糊的问题。
3. 在关于对话框中展示完整的许可证（如完整的LGPL v2.1）。
4. 更改了 CMakeLists.txt 中的输出目录，使打包更加方便。

### v0.1.2.0 (20210902)

1. 修复了`Windows`下打开文件时，索引始终从0开始的bug。
2. 将指令`MVS`的参数`WM7`的类型设置为`BIN`。

### v0.2.0.0 (20210904)

1. 在导入对话框中，可以选择重新指定所有程序的索引。
2. 改进了输入索引的对话框：
   - 现在会自动检查索引是否已被占用；
   - 增加“跳过该文件”按钮；
   - 修复了因`validator.SetMin(begin_index)`导致的在索引的起始值大于等于10时，清空编辑框后，无法重新输入数据的bug。
3. 重新导出时的 确认覆盖文件 对话框：
   - 将标签“是”改为“覆盖”，标签“否”改为“跳过”；
   - 可以选择“对之后的所有程序应用此操作”，即之后全选覆盖或之后全选跳过。
4. `program_t::save_xlsx`函数支持选择导出文件的风格：
   - 列宽：自动计算（可指定实际宽度与算出的宽度的比）/手动指定，默认值是自动计算，实际:算出=1.85；
   - 注释位置：左侧/右侧/左侧带冒号，默认值是左侧带冒号。
5. 现在在导出对话框中可自定义表格列宽和注释位置。

   

### v0.2.0.1 (20210907)

1. 修复bug.00001: 在`Windows`上导入文件时遇到未捕获的异常。
   - 成因：在构造`MainWindow::OnImport()`函数中的`paths`数组的成员时，没有使用`std::filesystem::u8path`，导致在编码页不是`65501`时构造`std::filesystem::path`对象时抛出异常。
   - 由于`Linux`下不存在此问题，本版本只发布`Windows`可执行文件。
2. 在之后的更新中，如果`data`或`external`目录未改动，则只在一个次要版本的首个版本的源代码中打包`data`或`external`目录。

### v0.2.1.0 (20211007)

1. 为编辑区的按钮添加快捷键。
