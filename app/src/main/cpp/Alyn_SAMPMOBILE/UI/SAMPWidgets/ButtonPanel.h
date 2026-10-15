#pragma once

class ButtonPanel : public Layout {
public:
	ButtonPanel();

	bool extended() { return m_extended; }

	void minimize();
	void maximize();

private:
	bool m_extended;

	Button* m_extend;
	Button* m_esc;
	Button* m_tab;
	Button* m_alt;
	Button* m_spc;
	Button* m_f;
	Button* m_h;
	Button* m_2;
	Button* m_y;
	Button* m_n;
	Button* m_c;
	//Button* m_g;

public:
	class PassengerButton : public Image {
	public:
		PassengerButton();

		void draw(ImGuiRenderer* renderer) override;
		void touchPopEvent() override;
	};

	// Hold-to-guard button: while the finger is down the local ped keeps the
	// fight/guard (targeting) stance. Shown only on foot and alive.
	class GuardButton : public Widget {
	public:
		GuardButton();

		void draw(ImGuiRenderer* renderer) override;
		void focuseEvent(bool focus) override;
	};
};
