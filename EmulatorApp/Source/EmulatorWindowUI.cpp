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

	auto createRebindButton = [&](const std::string& text, EmulatorButton button, int32_t y) {
		controlsWindowContent->AddChild(new PixieUI::Text(text, 20, y, 0, 0, 111, defaultUIStyle));
		PixieUI::Element* btn = new PixieUI::Button(PixieUI::ButtonConfig("", [&, button](int32_t, int32_t) {
			m_parent.m_isRebindingKey = true;
			m_parent.m_keyToRebind = button;
			return true;
			}), 92, y, 115, buttonStyle);
		btn->ForceWidth(50);
		std::string btnText = GLFWKeyToString(m_parent.m_buttonMap.mappings[(uint32_t)button]);
		btn->AddChild(new PixieUI::DynamicText([&, button]() {
			return GLFWKeyToString(m_parent.m_buttonMap.mappings[(uint32_t)button]);
			}, 115 - (uint32_t)btnText.size() * 3, y, 0, 0, 120, defaultUIStyle));
		controlsWindowContent->AddChild(btn);
		};

	createRebindButton(" Start:", EmulatorButton::Start, 50);
	createRebindButton("Select:", EmulatorButton::Select, 70);
	createRebindButton("     A:", EmulatorButton::A, 90);
	createRebindButton("     B:", EmulatorButton::B, 110);
	createRebindButton("  Left:", EmulatorButton::Left, 130);
	createRebindButton(" Right:", EmulatorButton::Right, 150);
	createRebindButton("    Up:", EmulatorButton::Up, 170);
	createRebindButton("  Down:", EmulatorButton::Down, 190);

	createRebindButton(" Reset:", EmulatorButton::Reset, 210);
	createRebindButton(" Pause:", EmulatorButton::Pause, 230);
	createRebindButton("Resume:", EmulatorButton::Resume, 250);
	createRebindButton("  Step:", EmulatorButton::Step, 270);

	m_settingsWindow = new PixieUI::Window("Settings", m_width, m_height, controlsWindowContent, 100, windowOverlayStyle, containerStyle, buttonStyle);
	m_settingsWindow->SetShowCallback([&]() {
		m_windowOpened = true;
		});
	m_settingsWindow->SetHideCallback([&]() {
		m_windowOpened = false;
		m_parent.m_isRebindingKey = false;
		});
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

	if (m_parent.m_isRebindingKey) {
		std::string rebindText = "Press any key to set new binding.";
		PixieUI::Renderer::DrawText(rebindText, m_width / 2 - (int32_t)rebindText.size() * 3, m_height / 2 - 4, PixieUI::Color(255, 0, 0));
	}
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
	PixieUI::Renderer::DrawText("  IF: " + std::format("{:#04x}", m_emulator.cpu.IF), 323, 90, defaultUIStyle.fontColor);
	PixieUI::Renderer::DrawText("  IE: " + std::format("{:#04x}", m_emulator.cpu.IE), 398, 90, defaultUIStyle.fontColor);
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
