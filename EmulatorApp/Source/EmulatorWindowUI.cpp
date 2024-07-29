#include "EmulatorWindowUI.h"

const PixieUI::Style defaultUIStyle{
	PixieUI::Color(0.03f, 0.09f, 0.13f),
	PixieUI::Color(0.03f, 0.09f, 0.13f),
	PixieUI::Color(0.88f, 0.97f, 0.82f),
	PixieUI::Color(0.88f, 0.97f, 0.82f),
	PixieUI::Color(0.88f, 0.97f, 0.82f),
	PixieUI::Color(0.88f, 0.97f, 0.82f),
	0,
};

const PixieUI::Style buttonStyle{
	PixieUI::Color(0.20f, 0.41f, 0.34f),
	PixieUI::Color(0.53f, 0.75f, 0.44f),
	PixieUI::Color(0.88f, 0.97f, 0.82f),
	PixieUI::Color(0.88f, 0.97f, 0.82f),
	PixieUI::Color(0.88f, 0.97f, 0.82f),
	PixieUI::Color(0.88f, 0.97f, 0.82f),
	1,
};

const PixieUI::Style containerStyle{
	PixieUI::Color(0.20f, 0.41f, 0.34f),
	PixieUI::Color(0.20f, 0.41f, 0.34f),
	PixieUI::Color(0.88f, 0.97f, 0.82f),
	PixieUI::Color(0.88f, 0.97f, 0.82f),
	PixieUI::Color(0.88f, 0.97f, 0.82f),
	PixieUI::Color(0.88f, 0.97f, 0.82f),
	1,
};

const PixieUI::Style windowOverlayStyle{
	PixieUI::Color(0.00f, 0.00f, 0.00f, 0.65f),
	PixieUI::Color(0.00f, 0.00f, 0.00f, 0.65f),
	PixieUI::Color(0.88f, 0.97f, 0.82f),
	PixieUI::Color(0.88f, 0.97f, 0.82f),
	PixieUI::Color(0.88f, 0.97f, 0.82f),
	PixieUI::Color(0.88f, 0.97f, 0.82f),
	0,
};

const PixieUI::Style emptyStyle{
	PixieUI::Color(0.00f, 0.00f, 0.00f, 0.00f),
	PixieUI::Color(0.00f, 0.00f, 0.00f, 0.00f),
	PixieUI::Color(0.00f, 0.00f, 0.00f, 0.00f),
	PixieUI::Color(0.00f, 0.00f, 0.00f, 0.00f),
	PixieUI::Color(0.00f, 0.00f, 0.00f, 0.00f),
	PixieUI::Color(0.00f, 0.00f, 0.00f, 0.00f),
	0,
};

EmulatorWindowUI::EmulatorWindowUI(EmulatorWindow& parent, GBCEmulator& emulator, uint32_t width, uint32_t height)
	: m_parent(parent), m_width(width), m_height(height), m_emulator(emulator) {

	const PixieUI::MenuConfig menuConfig = PixieUI::MenuConfig({
		PixieUI::MenuButtonConfig(
			"Commands", nullptr,
			PixieUI::ButtonListConfig({
				PixieUI::ButtonConfig("Reset", [&](int32_t, int32_t) {
					m_emulator.Reset();
					return true;
				}),
			})
		),
		PixieUI::MenuButtonConfig(
			"Settings",
			[&](int32_t, int32_t) {
				m_settingsWindow->Show();
				return true;
			}
		)
		});

	m_uiLayout = new PixieUI::Layout(m_width, m_height);

	m_viewportTexture = new PixieUI::Texture(0, 16, 320, 288);
	m_uiLayout->AttachElement(m_viewportTexture);

	m_sideArea = new PixieUI::Element(321, 16, 158, 288, 0, containerStyle);
	m_uiLayout->AttachElement(m_sideArea);

	m_menu = new PixieUI::Menu(menuConfig, 0, 0, m_width, 0, defaultUIStyle, buttonStyle, containerStyle, buttonStyle);
	m_uiLayout->AttachElement(m_menu);

	PixieUI::Element* controlsWindowContent = new PixieUI::Element(30, 40, m_width - 40, m_height - 50, 110, emptyStyle);
	controlsWindowContent->AddChild(new PixieUI::Text("Keyboard", 90, 30, 0, 0, 111, defaultUIStyle));

	controlsWindowContent->AddChild(new PixieUI::Text(" Start:", 20, 50, 0, 0, 111, defaultUIStyle));
	PixieUI::Element* startBtn = new PixieUI::Button(PixieUI::ButtonConfig("", [&](int32_t, int32_t) {
		return true;
		}), 92, 50, 115, buttonStyle);
	startBtn->ForceWidth(50);
	std::string startBtnText = GLFWKeyToString(m_parent.m_buttonMap.mappings[(uint32_t)EmulatorButton::Start]);
	startBtn->AddChild(new PixieUI::Text(startBtnText, 115 - (uint32_t)startBtnText.size() * 3, 50, 0, 0, 120, defaultUIStyle));
	controlsWindowContent->AddChild(startBtn);

	controlsWindowContent->AddChild(new PixieUI::Text("Select:", 20, 70, 0, 0, 111, defaultUIStyle));
	PixieUI::Element* selectBtn = new PixieUI::Button(PixieUI::ButtonConfig("", [&](int32_t, int32_t) {
		return true;
		}), 92, 70, 115, buttonStyle);
	selectBtn->ForceWidth(50);
	std::string selectBtnText = GLFWKeyToString(m_parent.m_buttonMap.mappings[(uint32_t)EmulatorButton::Select]);
	selectBtn->AddChild(new PixieUI::Text(selectBtnText, 115 - (uint32_t)selectBtnText.size() * 3, 70, 0, 0, 120, defaultUIStyle));
	controlsWindowContent->AddChild(selectBtn);

	controlsWindowContent->AddChild(new PixieUI::Text("     A:", 20, 90, 0, 0, 111, defaultUIStyle));
	PixieUI::Element* aBtn = new PixieUI::Button(PixieUI::ButtonConfig("", [&](int32_t, int32_t) {
		return true;
		}), 92, 90, 115, buttonStyle);
	aBtn->ForceWidth(50);
	std::string aBtnText = GLFWKeyToString(m_parent.m_buttonMap.mappings[(uint32_t)EmulatorButton::A]);
	aBtn->AddChild(new PixieUI::Text(aBtnText, 115 - (uint32_t)aBtnText.size() * 3, 90, 0, 0, 120, defaultUIStyle));
	controlsWindowContent->AddChild(aBtn);

	controlsWindowContent->AddChild(new PixieUI::Text("     B:", 20, 110, 0, 0, 111, defaultUIStyle));
	PixieUI::Element* bBtn = new PixieUI::Button(PixieUI::ButtonConfig("", [&](int32_t, int32_t) {
		return true;
		}), 92, 110, 115, buttonStyle);
	bBtn->ForceWidth(50);
	std::string bBtnText = GLFWKeyToString(m_parent.m_buttonMap.mappings[(uint32_t)EmulatorButton::B]);
	bBtn->AddChild(new PixieUI::Text(bBtnText, 115 - (uint32_t)bBtnText.size() * 3, 110, 0, 0, 120, defaultUIStyle));
	controlsWindowContent->AddChild(bBtn);

	controlsWindowContent->AddChild(new PixieUI::Text("  Left:", 20, 130, 0, 0, 111, defaultUIStyle));
	PixieUI::Element* leftBtn = new PixieUI::Button(PixieUI::ButtonConfig("", [&](int32_t, int32_t) {
		return true;
		}), 92, 130, 115, buttonStyle);
	leftBtn->ForceWidth(50);
	std::string leftBtnText = GLFWKeyToString(m_parent.m_buttonMap.mappings[(uint32_t)EmulatorButton::Left]);
	leftBtn->AddChild(new PixieUI::Text(leftBtnText, 115 - (uint32_t)leftBtnText.size() * 3, 130, 0, 0, 120, defaultUIStyle));
	controlsWindowContent->AddChild(leftBtn);

	controlsWindowContent->AddChild(new PixieUI::Text(" Right:", 20, 150, 0, 0, 111, defaultUIStyle));
	PixieUI::Element* rightBtn = new PixieUI::Button(PixieUI::ButtonConfig("", [&](int32_t, int32_t) {
		return true;
		}), 92, 150, 115, buttonStyle);
	rightBtn->ForceWidth(50);
	std::string rightBtnText = GLFWKeyToString(m_parent.m_buttonMap.mappings[(uint32_t)EmulatorButton::Right]);
	rightBtn->AddChild(new PixieUI::Text(rightBtnText, 115 - (uint32_t)rightBtnText.size() * 3, 150, 0, 0, 120, defaultUIStyle));
	controlsWindowContent->AddChild(rightBtn);

	controlsWindowContent->AddChild(new PixieUI::Text("    Up:", 20, 170, 0, 0, 111, defaultUIStyle));
	PixieUI::Element* upBtn = new PixieUI::Button(PixieUI::ButtonConfig("", [&](int32_t, int32_t) {
		return true;
		}), 92, 170, 115, buttonStyle);
	upBtn->ForceWidth(50);
	std::string upBtnText = GLFWKeyToString(m_parent.m_buttonMap.mappings[(uint32_t)EmulatorButton::Up]);
	upBtn->AddChild(new PixieUI::Text(upBtnText, 115 - (uint32_t)upBtnText.size() * 3, 170, 0, 0, 120, defaultUIStyle));
	controlsWindowContent->AddChild(upBtn);

	controlsWindowContent->AddChild(new PixieUI::Text("  Down:", 20, 190, 0, 0, 111, defaultUIStyle));
	PixieUI::Element* downBtn = new PixieUI::Button(PixieUI::ButtonConfig("", [&](int32_t, int32_t) {
		return true;
		}), 92, 190, 115, buttonStyle);
	downBtn->ForceWidth(50);
	std::string downBtnText = GLFWKeyToString(m_parent.m_buttonMap.mappings[(uint32_t)EmulatorButton::Down]);
	downBtn->AddChild(new PixieUI::Text(downBtnText, 115 - (uint32_t)downBtnText.size() * 3, 190, 0, 0, 120, defaultUIStyle));
	controlsWindowContent->AddChild(downBtn);

	controlsWindowContent->AddChild(new PixieUI::Text(" Reset:", 20, 210, 0, 0, 111, defaultUIStyle));
	PixieUI::Element* resetBtn = new PixieUI::Button(PixieUI::ButtonConfig("", [&](int32_t, int32_t) {
		return true;
		}), 92, 210, 115, buttonStyle);
	resetBtn->ForceWidth(50);
	std::string resetBtnText = GLFWKeyToString(m_parent.m_buttonMap.mappings[(uint32_t)EmulatorButton::Reset]);
	resetBtn->AddChild(new PixieUI::Text(resetBtnText, 115 - (uint32_t)resetBtnText.size() * 3, 210, 0, 0, 120, defaultUIStyle));
	controlsWindowContent->AddChild(resetBtn);

	controlsWindowContent->AddChild(new PixieUI::Text(" Pause:", 20, 230, 0, 0, 111, defaultUIStyle));
	PixieUI::Element* pauseBtn = new PixieUI::Button(PixieUI::ButtonConfig("", [&](int32_t, int32_t) {
		return true;
		}), 92, 230, 115, buttonStyle);
	pauseBtn->ForceWidth(50);
	std::string pauseBtnText = GLFWKeyToString(m_parent.m_buttonMap.mappings[(uint32_t)EmulatorButton::Pause]);
	pauseBtn->AddChild(new PixieUI::Text(pauseBtnText, 115 - (uint32_t)pauseBtnText.size() * 3, 230, 0, 0, 120, defaultUIStyle));
	controlsWindowContent->AddChild(pauseBtn);

	controlsWindowContent->AddChild(new PixieUI::Text("Resume:", 20, 250, 0, 0, 111, defaultUIStyle));
	PixieUI::Element* resumeBtn = new PixieUI::Button(PixieUI::ButtonConfig("", [&](int32_t, int32_t) {
		return true;
		}), 92, 250, 115, buttonStyle);
	resumeBtn->ForceWidth(50);
	std::string resumeBtnText = GLFWKeyToString(m_parent.m_buttonMap.mappings[(uint32_t)EmulatorButton::Resume]);
	resumeBtn->AddChild(new PixieUI::Text(resumeBtnText, 115 - (uint32_t)resumeBtnText.size() * 3, 250, 0, 0, 120, defaultUIStyle));
	controlsWindowContent->AddChild(resumeBtn);

	controlsWindowContent->AddChild(new PixieUI::Text("  Step:", 20, 270, 0, 0, 111, defaultUIStyle));
	PixieUI::Element* stepBtn = new PixieUI::Button(PixieUI::ButtonConfig("", [&](int32_t, int32_t) {
		return true;
		}), 92, 270, 115, buttonStyle);
	stepBtn->ForceWidth(50);
	std::string stepBtnText = GLFWKeyToString(m_parent.m_buttonMap.mappings[(uint32_t)EmulatorButton::Step]);
	stepBtn->AddChild(new PixieUI::Text(stepBtnText, 115 - (uint32_t)stepBtnText.size() * 3, 270, 0, 0, 120, defaultUIStyle));
	controlsWindowContent->AddChild(stepBtn);

	m_settingsWindow = new PixieUI::Window("Settings", m_width, m_height, controlsWindowContent, 100, windowOverlayStyle, containerStyle, buttonStyle);
	m_uiLayout->AttachElement(m_settingsWindow);
}

EmulatorWindowUI::~EmulatorWindowUI() {
	delete m_uiLayout;
}

void EmulatorWindowUI::UploadViewportTexture(uint32_t width, uint32_t height, void* data, GLenum format, GLenum type) {
	m_viewportTexture->UploadTexture(
		m_emulator.ppu.frameBuffers[!m_emulator.ppu.activeFrame].width,
		m_emulator.ppu.frameBuffers[!m_emulator.ppu.activeFrame].height,
		m_emulator.ppu.frameBuffers[!m_emulator.ppu.activeFrame].pixels.data(),
		GL_RGB, GL_FLOAT
	);
}

void EmulatorWindowUI::Draw() {
	m_uiLayout->Draw();
}

void EmulatorWindowUI::PrintState() {
	if (!m_settingsWindow->IsHidden()) return;
	PixieUI::Renderer::DrawText("Emulator State:", 323, 20, defaultUIStyle.fontColor);
	PixieUI::Renderer::DrawText("CPU :", 323, 30, defaultUIStyle.fontColor);
	PixieUI::Renderer::DrawText("  PC: " + std::format("{:#06x}", m_emulator.cpu.PC), 323, 40, defaultUIStyle.fontColor);
	PixieUI::Renderer::DrawText("  SP: " + std::format("{:#06x}", m_emulator.cpu.SP), 398, 40, defaultUIStyle.fontColor);
	PixieUI::Renderer::DrawText("   A: " + std::format("{:#04x}", m_emulator.cpu.A), 323, 50, defaultUIStyle.fontColor);
	PixieUI::Renderer::DrawText("   F: " + std::format("{:#04x}", m_emulator.cpu.F), 398, 50, defaultUIStyle.fontColor);
	PixieUI::Renderer::DrawText("   B: " + std::format("{:#04x}", m_emulator.cpu.B), 323, 60, defaultUIStyle.fontColor);
	PixieUI::Renderer::DrawText("   C: " + std::format("{:#04x}", m_emulator.cpu.C), 398, 60, defaultUIStyle.fontColor);
	PixieUI::Renderer::DrawText("   D: " + std::format("{:#04x}", m_emulator.cpu.D), 323, 70, defaultUIStyle.fontColor);
	PixieUI::Renderer::DrawText("   E: " + std::format("{:#04x}", m_emulator.cpu.E), 398, 70, defaultUIStyle.fontColor);
	PixieUI::Renderer::DrawText("   H: " + std::format("{:#04x}", m_emulator.cpu.H), 323, 80, defaultUIStyle.fontColor);
	PixieUI::Renderer::DrawText("   L: " + std::format("{:#04x}", m_emulator.cpu.L), 398, 80, defaultUIStyle.fontColor);
	PixieUI::Renderer::DrawText("  IF: " + std::format("{:#04x}", m_emulator.bus.IF), 323, 90, defaultUIStyle.fontColor);
	PixieUI::Renderer::DrawText("  IE: " + std::format("{:#04x}", m_emulator.bus.IE), 398, 90, defaultUIStyle.fontColor);
	PixieUI::Renderer::DrawText(" IME: " + (m_emulator.cpu.IME ? std::string("true") : std::string("false")), 323, 100, defaultUIStyle.fontColor);
	PixieUI::Renderer::DrawText("HALT: " + (m_emulator.cpu.isHalting ? std::string("true") : std::string("false")), 398, 100, defaultUIStyle.fontColor);

	PixieUI::Renderer::DrawText("PPU :", 323, 120, defaultUIStyle.fontColor);
	PixieUI::Renderer::DrawText("MODE: " + std::to_string(m_emulator.ppu.mode), 323, 130, defaultUIStyle.fontColor);
	PixieUI::Renderer::DrawText("ISEL: " + std::format("{:#04x}", m_emulator.ppu.mode >> 2), 398, 130, defaultUIStyle.fontColor);
	PixieUI::Renderer::DrawText("  LX: " + std::to_string(m_emulator.ppu.dot), 323, 140, defaultUIStyle.fontColor);
	PixieUI::Renderer::DrawText("  LY: " + std::to_string(m_emulator.ppu.LY), 398, 140, defaultUIStyle.fontColor);
	PixieUI::Renderer::DrawText("OBJE: " + std::to_string(m_emulator.ppu.OBJEnable), 323, 150, defaultUIStyle.fontColor);
	PixieUI::Renderer::DrawText("WINE: " + std::to_string(m_emulator.ppu.windowEnable), 398, 150, defaultUIStyle.fontColor);
	PixieUI::Renderer::DrawText(" SCX: " + std::to_string(m_emulator.ppu.SCX), 323, 160, defaultUIStyle.fontColor);
	PixieUI::Renderer::DrawText(" SCY: " + std::to_string(m_emulator.ppu.SCY), 398, 160, defaultUIStyle.fontColor);
	PixieUI::Renderer::DrawText("  WX: " + std::to_string(m_emulator.ppu.WX), 323, 170, defaultUIStyle.fontColor);
	PixieUI::Renderer::DrawText("  WY: " + std::to_string(m_emulator.ppu.WY), 398, 170, defaultUIStyle.fontColor);
}

void EmulatorWindowUI::SetCursorPosition(int32_t x, int32_t y) {
	m_uiLayout->SetCursorPosition(x, y);
}

void EmulatorWindowUI::Click() {
	m_uiLayout->Click();
}
