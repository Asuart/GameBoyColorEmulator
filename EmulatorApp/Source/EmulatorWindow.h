﻿#pragma once
#include <PixieUI/PixieUI.h>
#include <string>
#include <format>
#include <filesystem>
#include "GBCEmulator.h"
#include "ButtonMap.h"
#include "EmulatorWindowUI.h"

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

private:
	uint32_t m_width;
	uint32_t m_height;
	GBCEmulator m_emulator;
	PixieUI::FrameBuffer* m_layoutFrameBuffer;
	EmulatorWindowUI* m_ui;
	ButtonMap m_buttonMap;
	double m_targetFPS = 60.0;
	double m_targetFrameTime = 1.0 / m_targetFPS;

	void UpdateScreen();
	void UpdateKeyStates();
	bool ButtonIsPressed(EmulatorButton button);

	FileAccessState LoadSettings();
	FileAccessState SaveSettings();
	FileAccessState LoadState();
	FileAccessState SaveState();

	friend class EmulatorWindowUI;
};

