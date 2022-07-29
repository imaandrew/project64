
#include "stdafx.h"

#include "SettingsPage.h"

CHomeboyPage::CHomeboyPage(HWND hParent, const RECT & rcDispay)
{
    if (!Create(hParent, rcDispay))
    {
        return;
    }

    AddModCheckBox(GetDlgItem(IDC_ENABLEHOMEBOY), Homeboy_Enable);

    AddModCheckBox(GetDlgItem(IDC_ENABLESD), Homeboy_SDCard_Enable);
    m_SDPath.Attach(GetDlgItem(IDC_SDPATH));
    m_SDPath.SetTextField(GetDlgItem(IDC_SDPATH_TXT));

    AddModCheckBox(GetDlgItem(IDC_ENABLEFIFO), Homeboy_FIFO_Enable);
    CModifiedEditBox * TxtBox = AddModTextBox(GetDlgItem(IDC_FIFOPORT), (SettingID)Homeboy_FIFO_Port, false);
    TxtBox->SetTextField(GetDlgItem(IDC_FIFOPORT_TXT));

    UpdatePageSettings();
}

void CHomeboyPage::HidePage()
{
    ShowWindow(SW_HIDE);
}

void CHomeboyPage::ShowPage()
{
    ShowWindow(SW_SHOW);
}

void CHomeboyPage::ApplySettings(bool UpdateScreen)
{
    if (m_SDPath.IsChanged())
    {
        stdstr file = GetCWindowText(m_SDPath);
        g_Settings->SaveString(Homeboy_SDCard_Path, file.c_str());
    }

    if (m_SDPath.IsReset())
    {
        g_Settings->DeleteSetting(Homeboy_SDCard_Path);
    }

    CSettingsPageImpl<CHomeboyPage>::ApplySettings(UpdateScreen);
}

bool CHomeboyPage::EnableReset(void)
{
    if (CSettingsPageImpl<CHomeboyPage>::EnableReset()) { return true; }
    return false;
}

void CHomeboyPage::ResetPage()
{
    CSettingsPageImpl<CHomeboyPage>::ResetPage();
}

void CHomeboyPage::SelectSDPath(UINT /*Code*/, int /*id*/, HWND /*ctl*/)
{
    const char * Filter = "Raw Disk Image (*.img, *.raw, *.bin)\0*.img;*.raw;*.bin;\0All files (*)\0*\0";

    CPath FileName;
    if (FileName.SelectFile(m_hWnd, g_Settings->LoadStringVal(Cmd_BaseDirectory).c_str(), Filter, true))
    {
        m_SDPath.SetChanged(true);
        m_SDPath.SetWindowText(((stdstr)(std::string)FileName).ToUTF16().c_str());
        SendMessage(GetParent(), PSM_CHANGED, (WPARAM)m_hWnd, 0);
    }
}

void CHomeboyPage::SDPathChanged(UINT /*Code*/, int /*id*/, HWND /*ctl*/)
{
    if (m_InUpdateSettings) { return; }
    m_SDPath.SetChanged(true);
    SendMessage(GetParent(), PSM_CHANGED, (WPARAM)m_hWnd, 0);
}

void CHomeboyPage::UpdatePageSettings(void)
{
    m_InUpdateSettings = true;
    CSettingsPageImpl<CHomeboyPage>::UpdatePageSettings();

    stdstr File;
    g_Settings->LoadStringVal(Homeboy_SDCard_Path, File);
    m_SDPath.SetWindowText(File.ToUTF16().c_str());

    m_InUpdateSettings = false;
}