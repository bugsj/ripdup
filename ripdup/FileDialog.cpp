#include <vector>
#include <iterator>

#include <Windows.h>
#include <Shobjidl.h>

#include "FileDialog.h"

COMDLG_FILTERSPEC FileSpec[] =
{
    { L"ISO镜像", L"*.iso" },
    { L"所有文件", L"*.*" },
};

int getInputFile(std::vector<WCHAR>* file)
{
    file->clear();
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
        COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        IFileOpenDialog* pFileOpen;

        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
            IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

        if (SUCCEEDED(hr))
        {
            hr = pFileOpen->SetFileTypes(std::size(FileSpec), FileSpec);
            if (SUCCEEDED(hr))
            {
                hr = pFileOpen->SetDefaultExtension(L"iso");
                if (SUCCEEDED(hr))
                {
                    hr = pFileOpen->Show(NULL);

                    if (SUCCEEDED(hr))
                    {
                        IShellItem* pItem;
                        hr = pFileOpen->GetResult(&pItem);
                        if (SUCCEEDED(hr))
                        {
                            PWSTR pszFilePath;
                            hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                            if (SUCCEEDED(hr))
                            {
                                size_t size = wcslen(pszFilePath);
                                file->resize(size + 1);
                                wmemcpy(file->data(), pszFilePath, size + 1);
                                CoTaskMemFree(pszFilePath);
                            }
                            pItem->Release();
                        }
                    }
                    pFileOpen->Release();
                }
            }
        }
        CoUninitialize();
    }
    return file->size();
}


int getOutputFile(std::vector<WCHAR>* file)
{
    file->clear();
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
        COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        IFileOpenDialog* pFileSave;

        hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL,
            IID_IFileSaveDialog, reinterpret_cast<void**>(&pFileSave));

        if (SUCCEEDED(hr))
        {
            hr = pFileSave->SetFileTypes(std::size(FileSpec), FileSpec);
            if (SUCCEEDED(hr))
            {
                hr = pFileSave->SetDefaultExtension(L"iso");
                if (SUCCEEDED(hr))
                {
                    hr = pFileSave->Show(NULL);

                    if (SUCCEEDED(hr))
                    {
                        IShellItem* pItem;
                        hr = pFileSave->GetResult(&pItem);
                        if (SUCCEEDED(hr))
                        {
                            PWSTR pszFilePath;
                            hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                            if (SUCCEEDED(hr))
                            {
                                size_t size = wcslen(pszFilePath);
                                file->resize(size + 1);
                                wmemcpy(file->data(), pszFilePath, size + 1);
                                CoTaskMemFree(pszFilePath);
                            }
                            pItem->Release();
                        }
                    }
                    pFileSave->Release();
                }
            }
        }
        CoUninitialize();
    }
    return file->size();
}
