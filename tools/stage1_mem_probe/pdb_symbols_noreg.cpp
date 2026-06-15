// External DIA/PDB symbol lookup helper.
// This stays outside PaRappaWin runtime and loads msdia*.dll without COM registration.

#include <windows.h>
#include <dia2.h>

#include <cwctype>
#include <cstdio>
#include <string>
#include <vector>

template <typename T>
static void ReleaseIf(T*& ptr) {
    if (ptr != nullptr) {
        ptr->Release();
        ptr = nullptr;
    }
}

static std::wstring ToLower(std::wstring value) {
    for (wchar_t& ch : value) {
        ch = static_cast<wchar_t>(std::towlower(ch));
    }
    return value;
}

static std::wstring BstrToString(BSTR value) {
    if (value == nullptr) {
        return L"";
    }
    std::wstring out(value, SysStringLen(value));
    SysFreeString(value);
    return out;
}

static std::wstring GetName(IDiaSymbol* sym) {
    BSTR name = nullptr;
    if (sym == nullptr || FAILED(sym->get_name(&name))) {
        return L"";
    }
    return BstrToString(name);
}

static std::wstring GetUndecoratedName(IDiaSymbol* sym) {
    BSTR name = nullptr;
    if (sym == nullptr || FAILED(sym->get_undecoratedName(&name))) {
        return L"";
    }
    return BstrToString(name);
}

static std::wstring GetTypeName(IDiaSymbol* sym) {
    IDiaSymbol* type = nullptr;
    if (sym == nullptr || FAILED(sym->get_type(&type)) || type == nullptr) {
        return L"";
    }
    std::wstring out = GetName(type);
    ReleaseIf(type);
    return out;
}

static ULONGLONG GetTypeLength(IDiaSymbol* sym) {
    IDiaSymbol* type = nullptr;
    ULONGLONG length = 0;
    if (sym != nullptr && SUCCEEDED(sym->get_type(&type)) && type != nullptr) {
        type->get_length(&length);
    }
    ReleaseIf(type);
    return length;
}

static const wchar_t* SymTagName(DWORD tag) {
    switch (tag) {
    case SymTagFunction:
        return L"Function";
    case SymTagData:
        return L"Data";
    case SymTagUDT:
        return L"UDT";
    case SymTagPublicSymbol:
        return L"Public";
    default:
        return L"Other";
    }
}

static bool Matches(IDiaSymbol* sym, const std::vector<std::wstring>& needles) {
    if (needles.empty()) {
        return true;
    }
    const std::wstring haystack =
        ToLower(GetName(sym) + L" " + GetUndecoratedName(sym) + L" " + GetTypeName(sym));
    for (const std::wstring& needle : needles) {
        if (haystack.find(needle) != std::wstring::npos) {
            return true;
        }
    }
    return false;
}

static void PrintSymbol(IDiaSymbol* sym) {
    DWORD tag = 0;
    DWORD rva = 0;
    DWORD locType = 0;
    DWORD dataKind = 0;
    LONG offset = 0;
    ULONGLONG length = 0;
    sym->get_symTag(&tag);
    sym->get_relativeVirtualAddress(&rva);
    sym->get_locationType(&locType);
    sym->get_dataKind(&dataKind);
    sym->get_offset(&offset);
    sym->get_length(&length);

    std::wstring name = GetName(sym);
    std::wstring undec = GetUndecoratedName(sym);
    std::wstring typeName = GetTypeName(sym);
    ULONGLONG typeLength = GetTypeLength(sym);
    std::wprintf(
        L"symbol tag=%s name=\"%ls\" undec=\"%ls\" rva=0x%08X loc=%u dataKind=%u "
        L"offset=%ld length=%llu type=\"%ls\" typeLength=%llu\n",
        SymTagName(tag),
        name.c_str(),
        undec.c_str(),
        rva,
        locType,
        dataKind,
        offset,
        length,
        typeName.c_str(),
        typeLength);
}

static void PrintTypeFields(IDiaSymbol* udtOrTypedSymbol, const wchar_t* prefix) {
    IDiaSymbol* type = nullptr;
    DWORD tag = 0;
    udtOrTypedSymbol->get_symTag(&tag);
    if (tag == SymTagUDT) {
        type = udtOrTypedSymbol;
        type->AddRef();
    } else if (FAILED(udtOrTypedSymbol->get_type(&type)) || type == nullptr) {
        return;
    }

    std::wstring typeName = GetName(type);
    ULONGLONG typeLength = 0;
    type->get_length(&typeLength);
    std::wprintf(L"%ls type=\"%ls\" length=%llu\n", prefix, typeName.c_str(), typeLength);

    IDiaEnumSymbols* fields = nullptr;
    if (SUCCEEDED(type->findChildren(SymTagData, nullptr, nsNone, &fields)) && fields != nullptr) {
        IDiaSymbol* field = nullptr;
        ULONG fetched = 0;
        while (SUCCEEDED(fields->Next(1, &field, &fetched)) && fetched == 1) {
            DWORD dataKind = 0;
            LONG offset = 0;
            ULONGLONG length = 0;
            field->get_dataKind(&dataKind);
            field->get_offset(&offset);
            field->get_length(&length);
            std::wprintf(
                L"%ls field name=\"%ls\" offset=%ld dataKind=%u length=%llu type=\"%ls\" typeLength=%llu\n",
                prefix,
                GetName(field).c_str(),
                offset,
                dataKind,
                length,
                GetTypeName(field).c_str(),
                GetTypeLength(field));
            ReleaseIf(field);
        }
    }
    ReleaseIf(fields);
    ReleaseIf(type);
}

static bool CreateDiaSourceNoReg(const wchar_t* msdiaPath, IDiaDataSource** outSource) {
    *outSource = nullptr;
    HMODULE dia = LoadLibraryW(msdiaPath);
    if (dia == nullptr) {
        std::wprintf(L"LoadLibrary failed for %ls error=%lu\n", msdiaPath, GetLastError());
        return false;
    }

    using DllGetClassObjectFn = HRESULT(STDAPICALLTYPE*)(REFCLSID, REFIID, LPVOID*);
    auto getClassObject =
        reinterpret_cast<DllGetClassObjectFn>(GetProcAddress(dia, "DllGetClassObject"));
    if (getClassObject == nullptr) {
        std::wprintf(L"DllGetClassObject missing in %ls\n", msdiaPath);
        return false;
    }

    IClassFactory* factory = nullptr;
    HRESULT hr = getClassObject(__uuidof(DiaSource), __uuidof(IClassFactory),
                                reinterpret_cast<void**>(&factory));
    if (FAILED(hr) || factory == nullptr) {
        std::wprintf(L"DllGetClassObject(DiaSource) failed hr=0x%08X\n", hr);
        return false;
    }

    hr = factory->CreateInstance(nullptr, __uuidof(IDiaDataSource),
                                 reinterpret_cast<void**>(outSource));
    ReleaseIf(factory);
    if (FAILED(hr) || *outSource == nullptr) {
        std::wprintf(L"CreateInstance(IDiaDataSource) failed hr=0x%08X\n", hr);
        return false;
    }
    return true;
}

int wmain(int argc, wchar_t** argv) {
    if (argc < 4) {
        std::wprintf(L"usage: %ls <pdb> <msdia.dll> <query> [query...]\n", argv[0]);
        return 2;
    }

    std::vector<std::wstring> needles;
    for (int i = 3; i < argc; ++i) {
        needles.push_back(ToLower(argv[i]));
    }

    CoInitialize(nullptr);
    IDiaDataSource* source = nullptr;
    IDiaSession* session = nullptr;
    IDiaSymbol* global = nullptr;
    int rc = 1;

    if (!CreateDiaSourceNoReg(argv[2], &source)) {
        goto cleanup;
    }
    if (FAILED(source->loadDataFromPdb(argv[1]))) {
        std::wprintf(L"loadDataFromPdb failed for %ls\n", argv[1]);
        goto cleanup;
    }
    if (FAILED(source->openSession(&session)) || session == nullptr ||
        FAILED(session->get_globalScope(&global)) || global == nullptr) {
        std::wprintf(L"openSession/globalScope failed\n");
        goto cleanup;
    }

    {
        const enum SymTagEnum tags[] = {
            SymTagData,
            SymTagFunction,
            SymTagUDT,
            SymTagPublicSymbol,
        };
        for (const enum SymTagEnum tag : tags) {
            IDiaEnumSymbols* symbols = nullptr;
            if (FAILED(global->findChildren(tag, nullptr, nsNone, &symbols)) ||
                symbols == nullptr) {
                continue;
            }
            IDiaSymbol* sym = nullptr;
            ULONG fetched = 0;
            while (SUCCEEDED(symbols->Next(1, &sym, &fetched)) && fetched == 1) {
                if (Matches(sym, needles)) {
                    PrintSymbol(sym);
                    PrintTypeFields(sym, L"  typeinfo");
                }
                ReleaseIf(sym);
            }
            ReleaseIf(symbols);
        }
    }
    rc = 0;

cleanup:
    ReleaseIf(global);
    ReleaseIf(session);
    ReleaseIf(source);
    CoUninitialize();
    return rc;
}
