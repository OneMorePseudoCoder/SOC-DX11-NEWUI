#pragma once

#include "UIStatic.h"
#include "../script_export_space.h"


class CUIButton : public CUIStatic
{
private:
	typedef			CUIStatic				inherited;
public:
	CUIButton();
	virtual			~CUIButton()			{};

	virtual bool	OnMouseAction(float x, float y, EUIMessages mouse_action);
	virtual void	OnClick();

	//���������� ����
	virtual void	DrawTexture();
	virtual void	DrawText();
	virtual void	DrawHighlightedText();

	virtual void	Update();
	virtual void	Enable(bool status);
	virtual bool	OnKeyboardAction(int dik, EUIMessages keyboard_action);
	virtual void	OnFocusLost();

	//��������� � ������� ��������� ������
	typedef enum{
		BUTTON_NORMAL, //������ ����� �� �������������
		BUTTON_PUSHED,			//� ������� ��������
		BUTTON_UP				//��� ������������ ������ ���� 
	} E_BUTTON_STATE;


	//������ ����������� ���������
	virtual void	Reset();

	//��������� �� ����� �� ������
	// �������������� ���������
	virtual void	HighlightItem(bool bHighlight)			{ m_bCursorOverWindow = bHighlight; }

	// ��������� ��������� ������: ��������, �� ��������
	void				SetButtonState(E_BUTTON_STATE eBtnState)	{ m_eButtonState = eBtnState; }
	E_BUTTON_STATE		GetButtonState() const					{ return m_eButtonState; }

	// ��������� ������ ��� ������������� ����������� ���� ������ � ������ NORMAL_PRESS
	void				SetButtonAsSwitch(bool bAsSwitch)			{ m_bIsSwitch = bAsSwitch; }

	// ������ � �������������
	// ��� ������������ ������� �� ����� dinput.h, �� DirectX SDK.
	// ��������: ������ A - ��� 0x1E(DIK_A)
	void				SetAccelerator(int iAccel, int idx);
	const int			GetAccelerator(int idx) const;
	bool				IsAccelerator(int iAccel) const;

	void				SetPushOffset(const Fvector2& offset)	{ m_PushOffset = offset; }
	Fvector2			GetPushOffset()				{ return m_PushOffset; }
	void				SetShadowOffset(const Fvector2& offset) { m_ShadowOffset = offset; }

	shared_str			m_hint_text;
protected:

	E_BUTTON_STATE		m_eButtonState;
	s16					m_uAccelerator[4];
	bool				m_bIsSwitch;
	Fvector2			m_PushOffset;
	Fvector2			m_ShadowOffset;

	DECLARE_SCRIPT_REGISTER_FUNCTION
};



add_to_type_list(CUIButton)
#undef script_type_list
#define script_type_list save_type_list(CUIButton)
