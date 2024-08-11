#pragma once
#include <PixieUI/PixieUI.h>
#include <string>
#include <format>
#include <filesystem>
#include "GBCEmulator.h"
#include "ButtonMap.h"
#include "EmulatorWindowUI.h"
#include "../../PixieNoise/PixieNoise.h"

class EmulatorWindowUI;

enum class FileAccessState {
	Ok = 0,
	CouldNotOpenFile,
	FileDoesntExist
};

class EmulatorWindow {
public:
	GLFWwindow* m_mainWindow = nullptr;

	EmulatorWindow(uint32_t width, uint32_t height);
	~EmulatorWindow();

	bool LoadROM(const std::string& romPath);
	void Start();
	void HandleResolutionChange(uint32_t width, uint32_t height);
	void HandleMouseEvent(int32_t button, int32_t action);
	void HandleMouseMove(double x, double y);
	void HandleKeyEvent(int32_t key, int32_t scancode, int32_t action);
	bool IsPaused();
	uint32_t GetWidth();
	uint32_t GetHeight();
	FileAccessState SaveSettings();
	float GetVolume();
	void SetVolume(float value);

protected:
	uint32_t m_width;
	uint32_t m_height;
	GBCEmulator m_emulator;
	PixieUI::FrameBuffer* m_layoutFrameBuffer;
	EmulatorWindowUI* m_ui;
	ButtonMap m_buttonMap;
	double m_targetFPS = 60.0;
	double m_targetFrameTime = 1.0 / m_targetFPS;
	bool m_isRebindingKey = false;
	EmulatorButton m_keyToRebind = EmulatorButton::A;
	bool m_paused = false;
	float m_volume = 100.0f;

	void UpdateScreen();
	void UpdateKeyStates();
	bool ButtonIsPressed(EmulatorButton button);

	FileAccessState LoadSettings();
	FileAccessState LoadStateFromFile();
	FileAccessState SaveStateToFile();

	bool WriteSaveStateFile(SaveState*);
	SaveState* ReadSaveStateFile();

	friend class EmulatorWindowUI;
};

