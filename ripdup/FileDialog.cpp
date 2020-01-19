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

    PWSTR pszFilePath;
    CHECKHR(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath));

    size_t size = wcslen(pszFilePath);
    file->resize(size + 1);
    wmemcpy(file->data(), pszFilePath, size + 1);
    CoTaskMemFree(pszFilePath);
    
    CoUninitialize();
    
    return file->size();
}


long long getOutputFile(std::vector<WCHAR>* file)
{
    file->clear();
    CHECKHR(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE));

    CComPtr<IFileOpenDialog> pFileSave;

    CHECKHR(CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL, IID_IFileSaveDialog, reinterpret_cast<void**>(&pFileSave)));

    CHECKHR(pFileSave->SetFileTypes(std::size(FileSpec), FileSpec));
    CHECKHR(pFileSave->SetDefaultExtension(L"iso"));
    CHECKHR(pFileSave->Show(NULL));

    CComPtr<IShellItem> pItem;

    CHECKHR(pFileSave->GetResult(&pItem));

    PWSTR pszFilePath;
    CHECKHR(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath));

    size_t size = wcslen(pszFilePath);
    file->resize(size + 1);
    wmemcpy(file->data(), pszFilePath, size + 1);
    CoTaskMemFree(pszFilePath);

    CoUninitialize();

    return file->size();
}
