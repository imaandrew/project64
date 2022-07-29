
#pragma once

class CHomeboyPage :
	public CSettingsPageImpl<CHomeboyPage>,
	public CSettingsPage
{

	BEGIN_MSG_MAP_EX(CHomeboyPage)
		COMMAND_ID_HANDLER_EX(IDC_ENABLEHOMEBOY,CheckBoxChanged)
		COMMAND_ID_HANDLER_EX(IDC_ENABLESD,CheckBoxChanged)
		COMMAND_ID_HANDLER_EX(IDC_SELECT_SDPATH,SelectSDPath)
		COMMAND_HANDLER_EX(IDC_SDPATH,EN_UPDATE,SDPathChanged)
		COMMAND_ID_HANDLER_EX(IDC_ENABLEFIFO,CheckBoxChanged)
		COMMAND_HANDLER_EX(IDC_FIFOPORT,EN_UPDATE,EditBoxChanged)
	END_MSG_MAP()

	enum { IDD = IDD_Settings_Homeboy };

public:
	CHomeboyPage(HWND hParent, const RECT & rcDispay );

	LanguageStringID PageTitle ( void ) { return TAB_HOMEBOY; }
	void HidePage ( void );
	void ShowPage ( void );
	void ApplySettings ( bool UpdateScreen );
	bool EnableReset ( void );
	void ResetPage ( void );

private:
	void SelectSDPath(UINT Code, int id, HWND ctl);
	void SDPathChanged(UINT Code, int id, HWND ctl);

	void UpdatePageSettings(void);
	CModifiedEditBox m_SDPath;

	bool m_InUpdateSettings;
};