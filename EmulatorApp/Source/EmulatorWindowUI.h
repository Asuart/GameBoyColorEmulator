#pragma once
#include <PixieUI/PixieUI.h>
#include "GBCEmulator.h"
#include "ButtonMap.h"
#include "EmulatorWindow.h"

class EmulatorWindow;

class EmulatorWindowUI {
public:
	EmulatorWindowUI(EmulatorWindow& parent, GBCEmulator& emulator, uint32_t width, uint32_t height);
	~EmulatorWindowUI();

	void UploadViewportTexture(uint32_t width, uint32_t height, void* data, GLenum format, GLenum type);
	void Draw();
	void PrintState();
	void SetCursorPosition(int32_t x, int32_t y);
	void Click();

protected:
	EmulatorWindow& m_parent;
	GBCEmulator& m_emulator;
	uint32_t m_width;
	uint32_t m_height;
	PixieUI::Layout* m_uiLayout;
	PixieUI::Texture* m_viewportTexture;
	PixieUI::Menu* m_menu;
	PixieUI::Element* m_sideArea;
	PixieUI::Window* m_settingsWindow;
	bool m_windowOpened = false;

	friend class EmulatorWindow;
};