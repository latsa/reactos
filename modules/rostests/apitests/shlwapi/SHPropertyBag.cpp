/*
 * PROJECT:     ReactOS api tests
 * LICENSE:     LGPL-2.1-or-later (https://spdx.org/licenses/LGPL-2.1-or-later)
 * PURPOSE:     Tests for SHPropertyBag Read/Write
 * COPYRIGHT:   Copyright 2023 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
 */

#include <apitest.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <shlwapi_undoc.h>

#include <pseh/pseh2.h>

static LPCWSTR s_pszPropNames[4] = { NULL, NULL, NULL, NULL };
static VARTYPE s_vt;
static INT s_cWrite = 0;

static void ResetTest(VARTYPE vt,
                  LPCWSTR pszName0 = NULL, LPCWSTR pszName1 = NULL,
                  LPCWSTR pszName2 = NULL, LPCWSTR pszName3 = NULL)
{
    s_vt = vt;
    s_cWrite = 0;
    s_pszPropNames[0] = pszName0;
    s_pszPropNames[1] = pszName1;
    s_pszPropNames[2] = pszName2;
    s_pszPropNames[3] = pszName3;
}

class CDummyWritePropertyBag : public IPropertyBag
{
public:
    CDummyWritePropertyBag()
    {
    }

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObject) override
    {
        ok_int(0, 1);
        return S_OK;
    }
    STDMETHODIMP_(ULONG) AddRef() override
    {
        ok_int(0, 1);
        return S_OK;
    }
    STDMETHODIMP_(ULONG) Release() override
    {
        ok_int(0, 1);
        return S_OK;
    }

    // IPropertyBag
    STDMETHODIMP Read(LPCWSTR pszPropName, VARIANT *pvari, IErrorLog *pErrorLog) override
    {
        ok_int(0, 1);
        return S_OK;
    }

    STDMETHODIMP Write(LPCWSTR pszPropName, VARIANT *pvari) override
    {
        ++s_cWrite;
        ok_int(s_vt, V_VT(pvari));
        for (size_t i = 0; i < _countof(s_pszPropNames); ++i)
        {
            if (s_pszPropNames[i])
            {
                ok_wstr(pszPropName, s_pszPropNames[i]);
                s_pszPropNames[i] = NULL;
                if (lstrcmpiW(pszPropName, L"RECTL2.bottom") == 0)
                {
                    s_vt = VT_EMPTY;
                    ZeroMemory(&s_pszPropNames, sizeof(s_pszPropNames));
                    s_pszPropNames[0] = L"RECTL2.right";
                    return E_FAIL;
                }
                break;
            }
        }
        return S_OK;
    }
};

static void SHPropertyBag_WriteTest(void)
{
    HRESULT hr;
    CDummyWritePropertyBag dummy;

    ResetTest(VT_EMPTY, L"EMPTY1");
    hr = SHPropertyBag_Delete(&dummy, s_pszPropNames[0]);
    ok_long(hr, S_OK);
    ok_int(s_cWrite, 1);

    ResetTest(VT_BOOL, L"BOOL1");
    hr = SHPropertyBag_WriteBOOL(&dummy, s_pszPropNames[0], TRUE);
    ok_long(hr, S_OK);
    ok_int(s_cWrite, 1);

    ResetTest(VT_UI2, L"SHORT1");
    hr = SHPropertyBag_WriteSHORT(&dummy, s_pszPropNames[0], 1);
    ok_long(hr, S_OK);
    ok_int(s_cWrite, 1);

    ResetTest(VT_I4, L"LONG1");
    hr = SHPropertyBag_WriteLONG(&dummy, s_pszPropNames[0], 1);
    ok_long(hr, S_OK);
    ok_int(s_cWrite, 1);

    ResetTest(VT_UI4, L"DWORD1");
    hr = SHPropertyBag_WriteDWORD(&dummy, s_pszPropNames[0], 1);
    ok_long(hr, S_OK);
    ok_int(s_cWrite, 1);

    ResetTest(VT_BSTR, L"Str1");
    hr = SHPropertyBag_WriteStr(&dummy, s_pszPropNames[0], L"1");
    ok_long(hr, S_OK);
    ok_int(s_cWrite, 1);

    ResetTest(VT_I4, L"POINTL1.x", L"POINTL1.y");
    POINTL ptl = { 0xEEEE, 0xDDDD };
    hr = SHPropertyBag_WritePOINTL(&dummy, L"POINTL1", &ptl);
    ok_long(hr, S_OK);
    ok_int(s_cWrite, 2);

    ResetTest(VT_I4, L"POINTS1.x", L"POINTS1.y");
    POINTS pts = { 0x2222, 0x3333 };
    hr = SHPropertyBag_WritePOINTS(&dummy, L"POINTS1", &pts);
    ok_long(hr, S_OK);
    ok_int(s_cWrite, 2);

    ResetTest(VT_I4, L"RECTL1.left", L"RECTL1.top", L"RECTL1.right", L"RECTL1.bottom");
    RECTL rcl = { 123, 456, 789, 101112 };
    hr = SHPropertyBag_WriteRECTL(&dummy, L"RECTL1", &rcl);
    ok_long(hr, S_OK);
    ok_int(s_cWrite, 4);

    ResetTest(VT_I4, L"RECTL2.left", L"RECTL2.top", L"RECTL2.right", L"RECTL2.bottom");
    hr = SHPropertyBag_WriteRECTL(&dummy, L"RECTL2", &rcl);
    ok_long(hr, S_OK);
    ok_int(s_cWrite, 5);

    GUID guid;
    ZeroMemory(&guid, sizeof(guid));
    ResetTest(VT_BSTR, L"GUID1");
    hr = SHPropertyBag_WriteGUID(&dummy, L"GUID1", &guid);
    ok_long(hr, S_OK);
    ok_int(s_cWrite, 1);
}

START_TEST(SHPropertyBag)
{
    SHPropertyBag_WriteTest();
}
