﻿/*=============================================================================
*
*								ツールバーとメニュー
*
===============================================================================
/ Copyright (C) 1997-2007 Sota. All rights reserved.
/
/ Redistribution and use in source and binary forms, with or without 
/ modification, are permitted provided that the following conditions 
/ are met:
/
/  1. Redistributions of source code must retain the above copyright 
/     notice, this list of conditions and the following disclaimer.
/  2. Redistributions in binary form must reproduce the above copyright 
/     notice, this list of conditions and the following disclaimer in the 
/     documentation and/or other materials provided with the distribution.
/
/ THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR 
/ IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
/ OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
/ IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, 
/ INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
/ BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF 
/ USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
/ ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
/ (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF 
/ THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
/============================================================================*/

#include "common.h"

extern int SepaWidth;
extern int RemoteWidth;
extern int CancelFlg;
extern int DotFile;
extern char AsciiExt[ASCII_EXT_LEN+1];
extern int TransMode;
extern int ListType;
extern int LocalWidth;
extern char ViewerName[VIEWERS][FMAX_PATH+1];
extern int TransMode;
extern int SortSave;
extern int LocalKanjiCode;

static HWND hWndTbarMain = NULL;
static HWND hWndTbarLocal = NULL;
static HWND hWndTbarRemote = NULL;
static HWND hWndDirLocal = NULL;
static HWND hWndDirRemote = NULL;
static HWND hWndDirLocalEdit = NULL;
static HWND hWndDirRemoteEdit = NULL;
static int TmpTransMode;
static int TmpHostKanjiCode;
static int TmpHostKanaCnv;
// TODO: ローカルの漢字コードをShift_JIS以外にも対応
static int TmpLocalKanjiCode;
static int TmpLocalFileSort;
static int TmpLocalDirSort;
static int TmpRemoteFileSort;
static int TmpRemoteDirSort;
static int SyncMove = NO;
static int HideUI = 0;
static fs::path LocalCurDir;
static std::wstring RemoteCurDir;


/* メインのツールバー */
static constexpr TBBUTTON mainButtons[] = {
	{ .fsStyle = BTNS_SEP },
	{  0,  MENU_CONNECT,      TBSTATE_ENABLED, BTNS_BUTTON },
	{ 16, MENU_QUICK,         TBSTATE_ENABLED, BTNS_BUTTON },
	{  1,  MENU_DISCONNECT,   TBSTATE_ENABLED, BTNS_BUTTON },
	{ .fsStyle = BTNS_SEP },
	{  2,  MENU_DOWNLOAD,     TBSTATE_ENABLED, BTNS_BUTTON },
	{  3,  MENU_UPLOAD,       TBSTATE_ENABLED, BTNS_BUTTON },
	{ .fsStyle = BTNS_SEP },
	{ 24, MENU_MIRROR_UPLOAD, TBSTATE_ENABLED, BTNS_BUTTON },
	{ .fsStyle = BTNS_SEP },
	{  4,  MENU_DELETE,       TBSTATE_ENABLED, BTNS_BUTTON },
	{  5,  MENU_RENAME,       TBSTATE_ENABLED, BTNS_BUTTON },
	{  6,  MENU_MKDIR,        TBSTATE_ENABLED, BTNS_BUTTON },
	{ .fsStyle = BTNS_SEP },
	{  7,  MENU_TEXT,         TBSTATE_ENABLED, BTNS_CHECKGROUP },
	{  8,  MENU_BINARY,       TBSTATE_ENABLED, BTNS_CHECKGROUP },
	{ 17, MENU_AUTO,          TBSTATE_ENABLED, BTNS_CHECKGROUP },
	{ .fsStyle = BTNS_SEP },
	{ 27, MENU_L_KNJ_SJIS,    TBSTATE_ENABLED, BTNS_CHECKGROUP },
	{ 20, MENU_L_KNJ_EUC,     TBSTATE_ENABLED, BTNS_CHECKGROUP },
	{ 21, MENU_L_KNJ_JIS,     TBSTATE_ENABLED, BTNS_CHECKGROUP },
	{ 28, MENU_L_KNJ_UTF8N,   TBSTATE_ENABLED, BTNS_CHECKGROUP },
	{ 29, MENU_L_KNJ_UTF8BOM, TBSTATE_ENABLED, BTNS_CHECKGROUP },
	{ .fsStyle = BTNS_SEP },
	{ 27, MENU_KNJ_SJIS,      TBSTATE_ENABLED, BTNS_CHECKGROUP },
	{ 20, MENU_KNJ_EUC,       TBSTATE_ENABLED, BTNS_CHECKGROUP },
	{ 21, MENU_KNJ_JIS,       TBSTATE_ENABLED, BTNS_CHECKGROUP },
	{ 28, MENU_KNJ_UTF8N,     TBSTATE_ENABLED, BTNS_CHECKGROUP },
	{ 29, MENU_KNJ_UTF8BOM,   TBSTATE_ENABLED, BTNS_CHECKGROUP },
	{ 22, MENU_KNJ_NONE,      TBSTATE_ENABLED, BTNS_CHECKGROUP },
	{ .fsStyle = BTNS_SEP },
	{ 23, MENU_KANACNV,       TBSTATE_ENABLED, BTNS_CHECK },
	{ .fsStyle = BTNS_SEP },
	{ 15, MENU_REFRESH,       TBSTATE_ENABLED, BTNS_BUTTON },
	{ .fsStyle = BTNS_SEP },
	{ 18, MENU_LIST,          TBSTATE_ENABLED, BTNS_CHECKGROUP },
	{ 19, MENU_REPORT,        TBSTATE_ENABLED, BTNS_CHECKGROUP },
	{ .fsStyle = BTNS_SEP },
	{ 25, MENU_SYNC,          TBSTATE_ENABLED, BTNS_CHECK },
	{ .fsStyle = BTNS_SEP },
	{ 26, MENU_ABORT,         TBSTATE_ENABLED, BTNS_BUTTON },
};

/* ローカル側のツールバー */
static constexpr TBBUTTON localButtons[] = {
	{ .fsStyle = BTNS_SEP },
	{ 0, MENU_LOCAL_UPDIR, TBSTATE_ENABLED, BTNS_BUTTON },
	{ 1, MENU_LOCAL_CHDIR, TBSTATE_ENABLED, BTNS_BUTTON },
	{ .fsStyle = BTNS_SEP },
};

/* ホスト側のツールバー */
static constexpr TBBUTTON remoteButtons[] = {
	{ .fsStyle = BTNS_SEP },
	{ 0, MENU_REMOTE_UPDIR, TBSTATE_ENABLED, BTNS_BUTTON },
	{ 1, MENU_REMOTE_CHDIR, TBSTATE_ENABLED, BTNS_BUTTON },
	{ .fsStyle = BTNS_SEP },
};


static HBITMAP GetImage(int iamgeId) {
	HBITMAP resized = 0;
	if (auto original = (HBITMAP)LoadImageW(GetFtpInst(), MAKEINTRESOURCEW(iamgeId), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADMAP3DCOLORS)) {
		if (auto dc = GetDC(0)) {
			if (auto src = CreateCompatibleDC(dc)) {
				if (auto dest = CreateCompatibleDC(dc)) {
					if (BITMAP bitmap; GetObjectW(original, sizeof(BITMAP), &bitmap) > 0) {
						auto width = bitmap.bmWidth / 64 * CalcPixelX(16);
						auto height = bitmap.bmHeight / 64 * CalcPixelY(16);
						if (resized = CreateCompatibleBitmap(dc, width, height)) {
							auto hSrcOld = SelectObject(src, original);
							auto hDstOld = SelectObject(dest, resized);
							SetStretchBltMode(dest, HALFTONE);
							SetBrushOrgEx(dest, 0, 0, NULL);
							StretchBlt(dest, 0, 0, width, height, src, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY);
							SelectObject(src, hSrcOld);
							SelectObject(dest, hDstOld);
						}
					}
					DeleteDC(dest);
				}
				DeleteDC(src);
			}
			ReleaseDC(0, dc);
		}
		DeleteObject(original);
	}
	return resized;
}


static LRESULT CALLBACK IgnoreRightClick(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR) {
	switch (uMsg) {
	case WM_RBUTTONDBLCLK:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
		return TRUE;
	}
	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}


static LRESULT CALLBACK HistoryEdit(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR isLocal) {
	switch (uMsg) {
	case WM_CHAR:
		switch (wParam) {
		case 0x0D:			/* リターンキーが押された */
			if (isLocal) {
				DoLocalCWD(u8(GetText(hWnd)).c_str());
				GetLocalDirForWnd();
			} else {
				CancelFlg = NO;
				if (CheckClosedAndReconnect() == FFFTP_SUCCESS && DoCWD(u8(GetText(hWnd)).c_str(), YES, NO, YES) < FTP_RETRY)
					GetRemoteDirForWnd(CACHE_NORMAL, &CancelFlg);
			}
			return 0;
		case 0x09:			/* TABキーが押された */
			SetFocus(isLocal ? GetLocalHwnd() : GetRemoteHwnd());
			return 0;
		}
	}
	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}


static auto CreateToolbar(DWORD ws, UINT id, int bitmaps, HBITMAP image, const TBBUTTON* buttons, int size, int x, int y, int width, int height) {
	ws |= WS_CHILD | WS_VISIBLE | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT;
	auto toolbar = CreateWindowExW(0, TOOLBARCLASSNAMEW, nullptr, ws, 0, 0, 0, 0, GetMainHwnd(), (HMENU)UIntToPtr(id), 0, nullptr);
	if (toolbar) {
		TBADDBITMAP addbitmap{ 0, (UINT_PTR)image };
		SendMessageW(toolbar, TB_ADDBITMAP, bitmaps, (LPARAM)&addbitmap);
		SendMessageW(toolbar, TB_SETBITMAPSIZE, 0, MAKELPARAM(CalcPixelX(16), CalcPixelY(16)));
		SendMessageW(toolbar, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
		SendMessageW(toolbar, TB_ADDBUTTONSW, size, (LPARAM)buttons);
		SetWindowSubclass(toolbar, IgnoreRightClick, 0, 0);
		MoveWindow(toolbar, x, y, width, height, false);
	}
	return toolbar;
}


static std::tuple<HWND, HWND> CreateComboBox(HWND toolbar, DWORD style, int width, int menuId, bool isLocal, HFONT font) {
	style |= WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | CBS_DROPDOWN | CBS_AUTOHSCROLL;
	RECT rect;
	SendMessageW(toolbar, TB_GETITEMRECT, 3, (LPARAM)&rect);
	auto combobox = CreateWindowExW(WS_EX_CLIENTEDGE, WC_COMBOBOXW, nullptr, style, rect.right, rect.top, width - rect.right, CalcPixelY(200), toolbar, (HMENU)IntToPtr(menuId), GetFtpInst(), nullptr);
	HWND edit = 0;
	if (combobox) {
		if (COMBOBOXINFO ci{ sizeof(COMBOBOXINFO) }; SendMessageW(combobox, CB_GETCOMBOBOXINFO, 0, (LPARAM)&ci)) {
			__assume(ci.hwndItem);
			edit = ci.hwndItem;
			SetWindowSubclass(edit, HistoryEdit, 0, isLocal);
		}
		SendMessageW(combobox, WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));
		SendMessageW(combobox, CB_LIMITTEXT, FMAX_PATH, 0);
	}
	return { combobox, edit };
}


// ツールバーを作成する
bool MakeToolBarWindow() {
	auto mainImage = GetImage(main_toolbar_bmp);
	if (!mainImage)
		return false;
	auto remoteImage = GetImage(remote_toolbar_bmp);
	if (!mainImage)
		return false;
	RECT rect;
	GetClientRect(GetMainHwnd(), &rect);

	// main toolbar
	if (hWndTbarMain = CreateToolbar(BTNS_SEP, 1, 30, mainImage, mainButtons, size_as<int>(mainButtons), 0, 0, rect.right, AskToolWinHeight()); !hWndTbarMain)
		return false;

	// local toobar
	if (hWndTbarLocal = CreateToolbar(BTNS_GROUP, 2, 2, remoteImage, localButtons, size_as<int>(localButtons), 0, AskToolWinHeight(), LocalWidth, AskToolWinHeight()); !hWndTbarLocal)
		return false;
	SendMessageW(hWndTbarLocal, TB_GETITEMRECT, 3, (LPARAM)&rect);
	auto font = CreateFontW(rect.bottom - rect.top - CalcPixelY(8), 0, 0, 0, 0, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"MS Shell Dlg");
	std::tie(hWndDirLocal, hWndDirLocalEdit) = CreateComboBox(hWndTbarLocal, CBS_SORT, LocalWidth, COMBO_LOCAL, true, font);
	if (hWndDirLocal == NULL)
		return false;
	GetDrives(SetLocalDirHist);
	SendMessageW(hWndDirLocal, CB_SETCURSEL, 0, 0);

	// remote toolbar
	if (hWndTbarRemote = CreateToolbar(BTNS_GROUP, 3, 2, remoteImage, remoteButtons, size_as<int>(remoteButtons), LocalWidth + SepaWidth, AskToolWinHeight(), RemoteWidth, AskToolWinHeight()); !hWndTbarRemote)
		return false;
	std::tie(hWndDirRemote, hWndDirRemoteEdit) = CreateComboBox(hWndTbarRemote, 0, RemoteWidth, COMBO_REMOTE, false, font);
	if (hWndDirRemote == NULL)
		return false;
	SendMessageW(hWndDirRemote, CB_SETCURSEL, 0, 0);

	return true;
}


// ツールバーを削除
void DeleteToolBarWindow() {
	if (hWndTbarMain != NULL)
		DestroyWindow(hWndTbarMain);
	if (hWndTbarLocal != NULL)
		DestroyWindow(hWndTbarLocal);
	if (hWndTbarRemote != NULL)
		DestroyWindow(hWndTbarRemote);
	if (hWndDirLocal != NULL)
		DestroyWindow(hWndDirLocal);
	if (hWndDirRemote != NULL)
		DestroyWindow(hWndDirRemote);
}


// メインのツールバーのウインドウハンドルを返す
HWND GetMainTbarWnd() {
	return hWndTbarMain;
}


// ローカル側のヒストリウインドウのウインドウハンドルを返す
HWND GetLocalHistHwnd() {
	return hWndDirLocal;
}


// ホスト側のヒストリウインドウのウインドウハンドルを返す
HWND GetRemoteHistHwnd() {
	return hWndDirRemote;
}


// ローカル側のヒストリエディットのウインドウハンドルを返す
HWND GetLocalHistEditHwnd() {
	return hWndDirLocalEdit;
}


// ホスト側のヒストリエディットのウインドウハンドルを返す
HWND GetRemoteHistEditHwnd() {
	return hWndDirRemoteEdit;
}


// ローカル側のツールバーのウインドウハンドルを返す
HWND GetLocalTbarWnd() {
	return hWndTbarLocal;
}


// ホスト側のツールバーのウインドウハンドルを返す
HWND GetRemoteTbarWnd() {
	return hWndTbarRemote;
}


// ツールボタン／メニューのハイド処理
void MakeButtonsFocus() {
	if (HideUI == 0) {
		auto focus = GetFocus();
		auto connected = AskConnecting() == YES;
		auto selected = 0 < GetSelectedCount(focus == GetRemoteHwnd() ? WIN_REMOTE : WIN_LOCAL);
		auto transferable = connected && selected;
		auto operatable = focus == GetLocalHwnd() || connected;
		auto menu = GetMenu(GetMainHwnd());

		for (auto menuId : { MENU_BMARK_ADD, MENU_BMARK_ADD_LOCAL, MENU_BMARK_ADD_BOTH, MENU_BMARK_EDIT, MENU_DIRINFO, MENU_MIRROR_UPLOAD, MENU_MIRROR_DOWNLOAD, MENU_DOWNLOAD_NAME })
			EnableMenuItem(menu, menuId, connected ? MF_ENABLED : MF_GRAYED);
		SendMessageW(hWndTbarMain, TB_ENABLEBUTTON, MENU_MIRROR_UPLOAD, MAKELPARAM(connected, 0));
		if (focus == GetLocalHwnd()) {
			SendMessageW(hWndTbarMain, TB_ENABLEBUTTON, MENU_UPLOAD, MAKELPARAM(transferable, 0));
			EnableMenuItem(menu, MENU_UPLOAD, transferable ? MF_ENABLED : MF_GRAYED);
			EnableMenuItem(menu, MENU_UPLOAD_AS, transferable ? MF_ENABLED : MF_GRAYED);
			SendMessageW(hWndTbarMain, TB_ENABLEBUTTON, MENU_DOWNLOAD, MAKELPARAM(false, 0));
			EnableMenuItem(menu, MENU_SOMECMD, MF_GRAYED);
			EnableMenuItem(menu, MENU_DOWNLOAD, MF_GRAYED);
			EnableMenuItem(menu, MENU_DOWNLOAD_AS, MF_GRAYED);
			EnableMenuItem(menu, MENU_DOWNLOAD_AS_FILE, MF_GRAYED);
		} else if (focus == GetRemoteHwnd()) {
			EnableMenuItem(menu, MENU_SOMECMD, connected ? MF_ENABLED : MF_GRAYED);
			SendMessageW(hWndTbarMain, TB_ENABLEBUTTON, MENU_DOWNLOAD, MAKELPARAM(transferable, 0));
			EnableMenuItem(menu, MENU_DOWNLOAD, transferable ? MF_ENABLED : MF_GRAYED);
			EnableMenuItem(menu, MENU_DOWNLOAD_AS, transferable ? MF_ENABLED : MF_GRAYED);
			EnableMenuItem(menu, MENU_DOWNLOAD_AS_FILE, transferable ? MF_ENABLED : MF_GRAYED);
			SendMessageW(hWndTbarMain, TB_ENABLEBUTTON, MENU_UPLOAD, MAKELPARAM(false, 0));
			EnableMenuItem(menu, MENU_UPLOAD, MF_GRAYED);
			EnableMenuItem(menu, MENU_UPLOAD_AS, MF_GRAYED);
		}
		if (focus == GetLocalHwnd() || focus == GetRemoteHwnd()) {
			SendMessageW(hWndTbarMain, TB_ENABLEBUTTON, MENU_DELETE, MAKELPARAM(selected, 0));
			EnableMenuItem(menu, MENU_DELETE, selected ? MF_ENABLED : MF_GRAYED);
			SendMessageW(hWndTbarMain, TB_ENABLEBUTTON, MENU_RENAME, MAKELPARAM(selected, 0));
			EnableMenuItem(menu, MENU_RENAME, selected ? MF_ENABLED : MF_GRAYED);
			EnableMenuItem(menu, MENU_CHMOD, selected ? MF_ENABLED : MF_GRAYED);
			SendMessageW(hWndTbarMain, TB_ENABLEBUTTON, MENU_MKDIR, MAKELPARAM(operatable, 0));
			EnableMenuItem(menu, MENU_MKDIR, operatable ? MF_ENABLED : MF_GRAYED);
			EnableMenuItem(menu, MENU_FILESIZE, operatable ? MF_ENABLED : MF_GRAYED);
		} else {
			for (auto menuId : { MENU_UPLOAD, MENU_DOWNLOAD, MENU_DELETE, MENU_RENAME, MENU_MKDIR }) {
				SendMessageW(hWndTbarMain, TB_ENABLEBUTTON, menuId, MAKELPARAM(false, 0));
				EnableMenuItem(menu, menuId, MF_GRAYED);
			}
			for (auto menuId : { MENU_UPLOAD_AS, MENU_DOWNLOAD_AS, MENU_DOWNLOAD_AS_FILE, MENU_CHMOD, MENU_FILESIZE, MENU_SOMECMD })
				EnableMenuItem(menu, menuId, MF_GRAYED);
		}
	}
}


static void EnableMenu(HWND main, bool enabled) {
	auto menu = GetMenu(main);
	for (int i = 0, count = GetMenuItemCount(menu); i < count; i++)
		EnableMenuItem(menu, i, MF_BYPOSITION | (enabled ? MF_ENABLED : MF_GRAYED));
	DrawMenuBar(main);
}


static void EnableToolbar(HWND toolbar, const TBBUTTON* buttons, int size, bool enabled) {
	for (int i = 0; i < size; i++)
		if ((buttons[i].fsState & TBSTATE_ENABLED) && buttons[i].idCommand != MENU_ABORT)
			SendMessageW(toolbar, TB_ENABLEBUTTON, buttons[i].idCommand, MAKELPARAM(enabled, 0));
}


// ユーザの操作を禁止する
void DisableUserOpe() {
	HideUI++;
	EnableMenu(GetMainHwnd(), false);
	EnableToolbar(hWndTbarMain, mainButtons, size_as<int>(mainButtons), false);
	EnableToolbar(hWndTbarLocal, localButtons, size_as<int>(localButtons), false);
	EnableToolbar(hWndTbarRemote, remoteButtons, size_as<int>(remoteButtons), false);
	EnableWindow(hWndDirLocal, false);
	EnableWindow(hWndDirRemote, false);
}


// ユーザの操作を許可する
void EnableUserOpe() {
	if (0 < HideUI)
		HideUI--;
	if (HideUI == 0) {
		EnableMenu(GetMainHwnd(), true);
		EnableToolbar(hWndTbarMain, mainButtons, size_as<int>(mainButtons), true);
		EnableToolbar(hWndTbarLocal, localButtons, size_as<int>(localButtons), true);
		EnableToolbar(hWndTbarRemote, remoteButtons, size_as<int>(remoteButtons), true);
		EnableWindow(hWndDirLocal, true);
		EnableWindow(hWndDirRemote, true);

		// 選択不可な漢字コードのボタンが表示されるバグを修正
		HideHostKanjiButton();
		HideLocalKanjiButton();

		SetFocus(0);
		SetFocus(GetMainHwnd());

		MakeButtonsFocus();
	}
}


// ユーザの操作が禁止されているかどうかを返す
bool AskUserOpeDisabled() {
	return 0 < HideUI;
}


/*===================================================
*			転送モード
*===================================================*/

// 転送モードを設定する
void SetTransferTypeImm(int Mode) {
	TmpTransMode = Mode;
	HideHostKanjiButton();
	HideLocalKanjiButton();
}


constexpr struct { int menu, code; } transfermap[]{
	{ MENU_TEXT, TYPE_A },
	{ MENU_BINARY, TYPE_I },
};


// メニューにより転送モードを設定する
void SetTransferType(int Type) {
	auto const it = std::find_if(std::begin(transfermap), std::end(transfermap), [Type](auto const& item) { return item.menu == Type; });
	TmpTransMode = it != std::end(transfermap) ? it->code : TYPE_X;
	HideHostKanjiButton();
	HideLocalKanjiButton();
}


// 転送モードにしたがってボタンを表示する
void DispTransferType() {
	auto const it = std::find_if(std::begin(transfermap), std::end(transfermap), [](auto const& item) { return item.code == TmpTransMode; });
	SendMessageW(hWndTbarMain, TB_CHECKBUTTON, it != std::end(transfermap) ? it->menu : MENU_AUTO, MAKELPARAM(true, 0));
}


// 設定上の転送モードを返す
int AskTransferType() {
	return TmpTransMode;
}


// 実際の転送モードを返す
int AskTransferTypeAssoc(char* Fname, int Type) {
	if (Type != TYPE_X)
		return Type;
	if (0 < StrMultiLen(AsciiExt)) {
		auto wName = u8(GetFileName(Fname));
		for (char* Pos = AsciiExt; *Pos != NUL; Pos += strlen(Pos) + 1)
			if (CheckFname(wName, u8(Pos)))
				return TYPE_A;
	}
	return TYPE_I;
}


// 現在の転送モードがレジストリに保存される
void SaveTransferType() {
	TransMode = TmpTransMode;
}


/*===================================================
*			漢字モード
*===================================================*/

// ホストの漢字モードをセットする
void SetHostKanjiCodeImm(int Mode) {
	TmpHostKanjiCode = Mode;
	DispHostKanjiCode();
	HideHostKanjiButton();
}


constexpr struct { int remoteMenu, localMenu, code; } kanjimap[]{
	{ MENU_KNJ_SJIS,    MENU_L_KNJ_SJIS,    KANJI_SJIS },
	{ MENU_KNJ_EUC,     MENU_L_KNJ_EUC,     KANJI_EUC },
	{ MENU_KNJ_JIS,     MENU_L_KNJ_JIS,     KANJI_JIS },
	{ MENU_KNJ_UTF8N,   MENU_L_KNJ_UTF8N,   KANJI_UTF8N },
	{ MENU_KNJ_UTF8BOM, MENU_L_KNJ_UTF8BOM, KANJI_UTF8BOM },
};


// メニューによりホストの漢字モードを設定する
void SetHostKanjiCode(int Type) {
	auto const it = std::find_if(std::begin(kanjimap), std::end(kanjimap), [Type](auto const& item) { return item.remoteMenu == Type; });
	TmpHostKanjiCode = it != std::end(kanjimap) ? it->code : KANJI_NOCNV;
	DispHostKanjiCode();
	HideHostKanjiButton();
}


// ホストの漢字モードにしたがってボタンを表示する
void DispHostKanjiCode() {
	auto const it = std::find_if(std::begin(kanjimap), std::end(kanjimap), [](auto const& item) { return item.code == TmpHostKanjiCode; });
	SendMessageW(hWndTbarMain, TB_CHECKBUTTON, it != std::end(kanjimap) ? it->remoteMenu : MENU_KNJ_NONE, MAKELPARAM(true, 0));
}


// ホストの漢字モードを返す
int AskHostKanjiCode() {
	return TmpHostKanjiCode;
}


// 漢字モードボタンのハイド処理を行う
void HideHostKanjiButton() {
	auto ascii = TmpTransMode != TYPE_I;
	for (auto menuId : { MENU_KNJ_SJIS, MENU_KNJ_EUC, MENU_KNJ_JIS, MENU_KNJ_UTF8N, MENU_KNJ_UTF8BOM, MENU_KNJ_NONE })
		SendMessageW(hWndTbarMain, TB_ENABLEBUTTON, menuId, MAKELPARAM(ascii, 0));

	// 現在カナ変換はShift_JIS、JIS、EUC間でのみ機能する
	SendMessageW(hWndTbarMain, TB_ENABLEBUTTON, MENU_KANACNV, MAKELPARAM(false, 0));
	if (ascii)
		switch (TmpHostKanjiCode) {
		case KANJI_SJIS:
		case KANJI_JIS:
		case KANJI_EUC:
			switch (TmpLocalKanjiCode) {
			case KANJI_SJIS:
			case KANJI_JIS:
			case KANJI_EUC:
				SendMessageW(hWndTbarMain, TB_ENABLEBUTTON, MENU_KANACNV, MAKELONG(TRUE, 0));
				break;
			}
			break;
		}
}


// ローカルの漢字コード
// テキストモード転送時に使用
// ホスト側が無変換の時はローカルも無変換
void SetLocalKanjiCodeImm(int Mode) {
	TmpLocalKanjiCode = Mode;
	DispLocalKanjiCode();
	HideLocalKanjiButton();
}


void SetLocalKanjiCode(int Type) {
	auto const it = std::find_if(std::begin(kanjimap), std::end(kanjimap), [Type](auto const& item) { return item.localMenu == Type; });
	assert(it != std::end(kanjimap));
	TmpLocalKanjiCode = it->code;
	DispLocalKanjiCode();
	HideLocalKanjiButton();
}


void DispLocalKanjiCode() {
	auto const it = std::find_if(std::begin(kanjimap), std::end(kanjimap), [](auto const& item) { return item.code == TmpLocalKanjiCode; });
	assert(it != std::end(kanjimap));
	SendMessageW(hWndTbarMain, TB_CHECKBUTTON, it->localMenu, MAKELPARAM(true, 0));
}


int AskLocalKanjiCode() {
	return TmpLocalKanjiCode;
}


void HideLocalKanjiButton() {
	auto ascii = TmpTransMode != TYPE_I;
	for (auto menuId : { MENU_L_KNJ_SJIS, MENU_L_KNJ_EUC, MENU_L_KNJ_JIS, MENU_L_KNJ_UTF8N, MENU_L_KNJ_UTF8BOM })
		SendMessageW(hWndTbarMain, TB_ENABLEBUTTON, menuId, MAKELPARAM(ascii, 0));
	if (ascii) {
		// 現在カナ変換はShift_JIS、JIS、EUC間でのみ機能する
		SendMessageW(hWndTbarMain, TB_ENABLEBUTTON, MENU_KANACNV, MAKELPARAM(false, 0));
		switch (TmpHostKanjiCode) {
		case KANJI_SJIS:
		case KANJI_JIS:
		case KANJI_EUC:
			switch (TmpLocalKanjiCode) {
			case KANJI_SJIS:
			case KANJI_JIS:
			case KANJI_EUC:
				SendMessageW(hWndTbarMain, TB_ENABLEBUTTON, MENU_KANACNV, MAKELPARAM(true, 0));
				break;
			}
			break;
		}
	}
}


void SaveLocalKanjiCode() {
	LocalKanjiCode = TmpLocalKanjiCode;
}


/*===================================================
*			半角変換モード
*===================================================*/

// ホストの半角変換モードを設定する
void SetHostKanaCnvImm(int Mode) {
	TmpHostKanaCnv = Mode;
	DispHostKanaCnv();
}


// ホストの半角変換モードを反転する
void SetHostKanaCnv() {
	SetHostKanaCnvImm(TmpHostKanaCnv ^ 1);
}


// ホストの半角変換モードにしたがってボタンを表示する
void DispHostKanaCnv() {
	SendMessageW(hWndTbarMain, TB_CHECKBUTTON, MENU_KANACNV, MAKELONG(TmpHostKanaCnv != 0, 0));
}


// ホストの半角変換モードを返す
int AskHostKanaCnv() {
	return TmpHostKanaCnv;
}


/*===================================================
*			ソート方法
*===================================================*/

// ソート方法をセットする
//   LFsort : ローカル側のファイルのソート方法 (SORT_xxx)
//   LDsort : ローカル側のディレクトリのソート方法 (SORT_xxx)
//   RFsort : ホスト側のファイルのソート方法 (SORT_xxx)
//   RDsort : ホスト側のディレクトリのソート方法 (SORT_xxx)
void SetSortTypeImm(int LFsort, int LDsort, int RFsort, int RDsort) {
	TmpLocalFileSort = LFsort;
	TmpLocalDirSort = LDsort;
	TmpRemoteFileSort = RFsort;
	TmpRemoteDirSort = RDsort;
}


// リストビューのタブクリックによるソート方法のセット
void SetSortTypeByColumn(int Win, int Tab) {
	if (Win == WIN_LOCAL) {
		TmpLocalFileSort = (TmpLocalFileSort & SORT_MASK_ORD) == Tab ? TmpLocalFileSort ^ SORT_GET_ORD : Tab;
		TmpLocalDirSort = Tab == SORT_NAME || Tab == SORT_DATE ? TmpLocalFileSort : SORT_NAME;
	} else if (Tab != 4) {
		TmpRemoteFileSort = (TmpRemoteFileSort & SORT_MASK_ORD) == Tab ? TmpRemoteFileSort ^ SORT_GET_ORD : Tab;
		TmpRemoteDirSort = Tab == SORT_NAME || Tab == SORT_DATE ? TmpRemoteFileSort : SORT_NAME;
	}
}


// ソート方法を返す
int AskSortType(int Name) {
	switch (Name) {
	case ITEM_LFILE:
		return TmpLocalFileSort;
	case ITEM_LDIR:
		return TmpLocalDirSort;
	case ITEM_RFILE:
		return TmpRemoteFileSort;
	case ITEM_RDIR:
		return TmpRemoteDirSort;
	}
	return TmpLocalFileSort;
}


// ホストごとにソートを保存するかどうかをセットする
void SetSaveSortToHost(int Sw) {
	SortSave = Sw;
}


// ホストごとにソートを保存するかどうかを返す
int AskSaveSortToHost() {
	return SortSave;
}


/*===================================================
*			リストモード
*===================================================*/

// リストモードにしたがってボタンを表示する
void DispListType() {
	auto list = ListType == LVS_LIST;
	auto menu = GetMenu(GetMainHwnd());
	SendMessageW(hWndTbarMain, TB_CHECKBUTTON, list ? MENU_LIST : MENU_REPORT, MAKELPARAM(true, 0));
	CheckMenuItem(menu, MENU_LIST, list ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(menu, MENU_REPORT, list ? MF_UNCHECKED : MF_CHECKED);
}


/*===================================================
*			フォルダ同時移動モード
*===================================================*/

// 転送モードを設定する
void SetSyncMoveMode(int Mode) {
	SyncMove = Mode;
	DispSyncMoveMode();
}


// フォルダ同時移動モードを切り替える
void ToggleSyncMoveMode() {
	SetSyncMoveMode(SyncMove ^ 1);
}


// フォルダ同時移動を行うかどうかをによってメニュー／ボタンを表示
void DispSyncMoveMode() {
	auto sync = SyncMove != 0;
	SendMessageW(hWndTbarMain, TB_CHECKBUTTON, MENU_SYNC, MAKELPARAM(sync, 0));
	CheckMenuItem(GetMenu(GetMainHwnd()), MENU_SYNC, sync ? MF_CHECKED : MF_UNCHECKED);
}


// フォルダ同時移動モードを返す
int AskSyncMoveMode() {
	return SyncMove;
}


/*===================================================
*			ディレクトリヒストリ
*===================================================*/

// ホスト側のヒストリ一覧ウインドウに登録
void SetRemoteDirHist(std::wstring const& path) {
	auto index = SendMessageW(hWndDirRemote, CB_FINDSTRINGEXACT, 0, (LPARAM)path.c_str());
	if (index != CB_ERR)
		SendMessageW(hWndDirRemote, CB_DELETESTRING, index, 0);
	index = SendMessageW(hWndDirRemote, CB_ADDSTRING, 0, (LPARAM)path.c_str());
	SendMessageW(hWndDirRemote, CB_SETCURSEL, index, 0);
	RemoteCurDir = path;
}


// ローカル側のヒストリ一覧ウインドウに登録
void SetLocalDirHist(fs::path const& path) {
	auto index = SendMessageW(hWndDirLocal, CB_FINDSTRINGEXACT, 0, (LPARAM)path.c_str());
	if (index == CB_ERR)
		index = SendMessageW(hWndDirLocal, CB_ADDSTRING, 0, (LPARAM)path.c_str());
	SendMessageW(hWndDirLocal, CB_SETCURSEL, index, 0);
	LocalCurDir = path;
}


// ローカルのカレントディレクトリを返す
fs::path const& AskLocalCurDir() {
	return LocalCurDir;
}


// ホストのカレントディレクトリを返す
std::wstring const& AskRemoteCurDir() {
	return RemoteCurDir;
}


// カレントディレクトリを設定する
void SetCurrentDirAsDirHist() {
	std::error_code ec;
	fs::current_path(LocalCurDir, ec);
}


/*===================================================
*			メニュー
*===================================================*/

// ドットファイルを表示するかどうかをメニューに表示する
void DispDotFileMode() {
	CheckMenuItem(GetMenu(GetMainHwnd()), MENU_DOTFILE, DotFile == YES ? MF_CHECKED : MF_UNCHECKED);
}


// 右ボタンメニューを表示
void ShowPopupMenu(int Win, int Pos) {
	if (HideUI != 0)
		return;
	auto selectCount = GetSelectedCount(Win);
	auto connecting = AskConnecting() == YES;
	auto selecting = 0 < selectCount;
	auto canOpen = selectCount == 1 && (Win == WIN_LOCAL || connecting);
	auto menu = LoadMenuW(GetFtpInst(), MAKEINTRESOURCEW(popup_menu));
	auto submenu = GetSubMenu(menu, Win);
	EnableMenuItem(submenu, MENU_OPEN, canOpen ? 0 : MF_GRAYED);
	constexpr UINT MenuID[VIEWERS] = { MENU_OPEN1, MENU_OPEN2, MENU_OPEN3 };
	for (int i = VIEWERS - 1; i >= 0; i--) {
		if (strlen(ViewerName[i]) != 0) {
			static auto const format = GetString(IDS_OPEN_WITH);
			wchar_t text[FMAX_PATH + 1];
			swprintf_s(text, format.c_str(), fs::u8path(ViewerName[i]).filename().c_str(), i + 1);
			MENUITEMINFOW mii{ sizeof(MENUITEMINFOW), MIIM_FTYPE | MIIM_STATE | MIIM_ID | MIIM_STRING, MFT_STRING, UINT(canOpen ? 0 : MFS_GRAYED), MenuID[i] };
			mii.dwTypeData = text;
			InsertMenuItemW(submenu, 1, true, &mii);
		}
	}
	if (Win == WIN_LOCAL) {
		EnableMenuItem(submenu, MENU_UPLOAD, selecting && connecting ? 0 : MF_GRAYED);
		EnableMenuItem(submenu, MENU_UPLOAD_AS, selecting && connecting ? 0 : MF_GRAYED);
		EnableMenuItem(submenu, MENU_UPLOAD_ALL, connecting ? 0 : MF_GRAYED);
		EnableMenuItem(submenu, MENU_DELETE, selecting ? 0 : MF_GRAYED);
		EnableMenuItem(submenu, MENU_RENAME, selecting ? 0 : MF_GRAYED);
	} else {
		EnableMenuItem(submenu, MENU_DOWNLOAD, selecting && connecting ? 0 : MF_GRAYED);
		EnableMenuItem(submenu, MENU_DOWNLOAD_AS, selecting && connecting ? 0 : MF_GRAYED);
		EnableMenuItem(submenu, MENU_DOWNLOAD_AS_FILE, selecting && connecting ? 0 : MF_GRAYED);
		EnableMenuItem(submenu, MENU_DOWNLOAD_ALL, connecting ? 0 : MF_GRAYED);
		EnableMenuItem(submenu, MENU_DELETE, selecting && connecting ? 0 : MF_GRAYED);
		EnableMenuItem(submenu, MENU_RENAME, selecting && connecting ? 0 : MF_GRAYED);
#if defined(HAVE_TANDEM)
		/* HP NonStop Server では CHMOD の仕様が異なるため使用不可 */
		if (AskRealHostType() == HTYPE_TANDEM)
			RemoveMenu(submenu, MENU_CHMOD, MF_BYCOMMAND);
		else
#endif
			EnableMenuItem(submenu, MENU_CHMOD, selecting && connecting ? 0 : MF_GRAYED);
		EnableMenuItem(submenu, MENU_MKDIR, connecting ? 0 : MF_GRAYED);
		EnableMenuItem(submenu, MENU_URL_COPY, selecting && connecting ? 0 : MF_GRAYED);
#if defined(HAVE_TANDEM)
		/* OSS モードのときに表示されるように AskRealHostType() を使用する */
		if (AskRealHostType() == HTYPE_TANDEM)
			EnableMenuItem(submenu, MENU_SWITCH_OSS, connecting ? 0 : MF_GRAYED);
		else
#endif
			RemoveMenu(submenu, MENU_SWITCH_OSS, MF_BYCOMMAND);
		EnableMenuItem(submenu, MENU_REMOTE_MOVE_UPDIR, selecting && connecting ? 0 : MF_GRAYED);
		EnableMenuItem(submenu, MENU_FILESIZE, connecting ? 0 : MF_GRAYED);
		EnableMenuItem(submenu, REFRESH_REMOTE, connecting ? 0 : MF_GRAYED);
	}
	POINT point;
	if (Pos == 0)
		GetCursorPos(&point);
	else {
		RECT Rect;
		GetWindowRect(Win == WIN_LOCAL ? GetLocalHwnd() : GetRemoteHwnd(), &Rect);
		point.x = Rect.left + 20;
		point.y = Rect.top + 20;
	}
	TrackPopupMenu(submenu, TPM_LEFTBUTTON | TPM_RIGHTBUTTON, point.x, point.y, 0, GetMainHwnd(), NULL);
	DestroyMenu(menu);
}
