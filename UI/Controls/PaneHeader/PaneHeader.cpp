#include <StdAfx.h>
#include <UI/Controls/PaneHeader/PaneHeader.h>
#include <UI/UIFunctions.h>
#include <core/utility/strings.h>
#include <core/utility/output.h>

namespace controls
{
	// Draw our collapse button and label, if needed.
	// Draws everything to GetFixedHeight()
	void PaneHeader::DeferWindowPos(_In_ HDWP hWinPosInfo, const _In_ int x, const _In_ int y, const _In_ int width)
	{
		const auto labelHeight = GetFixedHeight();
		auto curX = x;
		if (m_bCollapsible)
		{
			StyleButton(
				m_CollapseButton.m_hWnd, m_bCollapsed ? ui::uiButtonStyle::UpArrow : ui::uiButtonStyle::DownArrow);
			::DeferWindowPos(
				hWinPosInfo, m_CollapseButton.GetSafeHwnd(), nullptr, curX, y, width, labelHeight, SWP_NOZORDER);
			curX += m_iButtonHeight;
		}

		output::DebugPrint(
			output::dbgLevel::Draw,
			L"PaneHeader::DeferWindowPos x:%d width:%d labelpos:%d labelwidth:%d \n",
			x,
			width,
			curX,
			m_iLabelWidth);

		::DeferWindowPos(hWinPosInfo, GetSafeHwnd(), nullptr, curX, y, m_iLabelWidth, labelHeight, SWP_NOZORDER);
	}

	void PaneHeader::Initialize(_In_ CWnd* pParent, _In_opt_ HDC hdc, _In_ bool bCollapsible, _In_ UINT nidParent)
	{
		m_bCollapsible = bCollapsible;
		if (pParent) m_hWndParent = pParent->m_hWnd;
		// Assign a nID to the collapse button that is IDD_COLLAPSE more than the control's nID
		m_nIDCollapse = nidParent + IDD_COLLAPSE;

		// TODO: validate this nid
		EC_B_S(
			Create(WS_CHILD | WS_CLIPSIBLINGS | ES_READONLY | WS_VISIBLE, CRect(0, 0, 0, 0), pParent, nidParent + 1));
		::SetWindowTextW(m_hWnd, m_szLabel.c_str());
		ui::SubclassLabel(m_hWnd);

		if (m_bCollapsible)
		{
			StyleLabel(m_hWnd, ui::uiLabelStyle::PaneHeaderLabel);

			EC_B_S(m_CollapseButton.Create(
				nullptr,
				WS_TABSTOP | WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
				CRect(0, 0, 0, 0),
				pParent,
				m_nIDCollapse));
		}

		const auto sizeText = ui::GetTextExtentPoint32(hdc, m_szLabel);
		m_iLabelWidth = sizeText.cx;
		output::DebugPrint(
			output::dbgLevel::Draw,
			L"PaneHeader::Initialize m_iLabelWidth:%d \"%ws\"\n",
			m_iLabelWidth,
			m_szLabel.c_str());
	}

	bool PaneHeader::HandleChange(UINT nID)
	{
		// Collapse buttons have a nID IDD_COLLAPSE higher than nID of the pane they toggle.
		// So if we get asked about one that matches, we can assume it's time to toggle our collapse.
		if (m_nIDCollapse == nID)
		{
			OnToggleCollapse();
			return true;
		}

		return false;
	}

	void PaneHeader::OnToggleCollapse()
	{
		m_bCollapsed = !m_bCollapsed;

		// Trigger a redraw
		::PostMessage(m_hWndParent, WM_COMMAND, IDD_RECALCLAYOUT, NULL);
	}

	void PaneHeader::SetMargins(
		int iLabelHeight, // Height of the label
		int iButtonHeight) // Height of buttons below the control
	{
		m_iLabelHeight = iLabelHeight;
		m_iButtonHeight = iButtonHeight;
	}
} // namespace controls