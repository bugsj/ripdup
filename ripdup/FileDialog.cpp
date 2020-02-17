/*
    ripdup - find and remove duplicate files in iso image file

    Copyright(c) 2020 Luo Jie

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this softwareand associated documentation files(the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions :

    The above copyright noticeand this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
 */

#include <vector>
#include <iterator>

#include <Windows.h>
#include <Shobjidl.h>
#include <atlbase.h>

#include "FileDialog.h"

COMDLG_FILTERSPEC FileSpec[] =
{
    { L"ISO镜像", L"*.iso" },
    { L"所有文件", L"*.*" },
};

#define CHECKHR(expr) do { if (!SUCCEEDED(expr)) { return -1; } } while(false)

long long getInputFile(std::vector<WCHAR>* file)
{
    file->clear();
    CHECKHR(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE));

    CComPtr<IFileOpenDialog> pFileOpen;
    CHECKHR(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen)));

    CHECKHR(pFileOpen->SetFileTypes(std::size(FileSpec), FileSpec));
    CHECKHR(pFileOpen->SetDefaultExtension(L"iso"));
    CHECKHR(pFileOpen->Show(NULL));

    CComPtr<IShellItem> pItem;
    CHECKHR(pFileOpen->GetResult(&pItem));

    CComHeapPtr<WCHAR> pszFilePath;
    CHECKHR(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath));

    size_t size = wcslen(pszFilePath);
    file->resize(size + 1);
    wmemcpy(file->data(), pszFilePath, size + 1);
    
    CoUninitialize();
    
    return file->size();
}


long long getOutputFile(std::vector<WCHAR>* file)
{
    file->clear();
    CHECKHR(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE));

    CComPtr<IFileSaveDialog> pFileSave;
    CHECKHR(CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL, IID_IFileSaveDialog, reinterpret_cast<void**>(&pFileSave)));

    CHECKHR(pFileSave->SetFileTypes(std::size(FileSpec), FileSpec));
    CHECKHR(pFileSave->SetDefaultExtension(L"iso"));
    CHECKHR(pFileSave->Show(NULL));

    CComPtr<IShellItem> pItem;
    CHECKHR(pFileSave->GetResult(&pItem));

    CComHeapPtr<WCHAR> pszFilePath;
    CHECKHR(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath));

    size_t size = wcslen(pszFilePath);
    file->resize(size + 1);
    wmemcpy(file->data(), pszFilePath, size + 1);

    CoUninitialize();

    return file->size();
}
