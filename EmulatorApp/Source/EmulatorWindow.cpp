#include "EmulatorWindow.h"

const uint32_t layoutWidth = 480;
const uint32_t layoutHeight = 304;

EmulatorWindow::EmulatorWindow(uint32_t _width, uint32_t _height)
	: m_width(_width), m_height(_height) {
	if (glfwInit() != GL_TRUE) {
		std::cout << "GLFW failed to initialize\n";
		exit(1);
	}

	m_mainWindow = glfwCreateWindow(m_width, m_height, "Game Boy Color Emulator", NULL, NULL);
	glfwMakeContextCurrent(m_mainWindow);
	glfwSwapInterval(0);
	glfwSetInputMode(m_mainWindow, GLFW_STICKY_KEYS, GL_TRUE);

	if (!gladLoadGL()) {
		std::cout << "GLAD failed to initialize\n";
		exit(2);
	}

	if (!PixieNoise::Initialize()) {
		std::cout << "PixieNoise failed to initialize\n";
		exit(3);
	}
	PixieNoise::ListAudioDevices();

	glClearColor(0.03f, 0.09f, 0.13f, 1.0f);

	PixieUI::Init();
	PixieUI::SetCanvasSize(layoutWidth, layoutHeight);

	m_ui = new EmulatorWindowUI(*this, m_emulator, layoutWidth, layoutHeight);

	m_layoutFrameBuffer = new PixieUI::FrameBuffer(layoutWidth, layoutHeight);

	FileAccessState settingsState = LoadSettings();
	if (settingsState == FileAccessState::FileDoesntExist) {
		FileAccessState saveSettingsState = SaveSettings();
		if (saveSettingsState != FileAccessState::Ok) {
			std::cout << "Failed to create settings file.\n";
		}
		else {
			std::cout << "Created new settings file.\n";
		}
	}
	else if (settingsState == FileAccessState::CouldNotOpenFile) {
		std::cout << "Failed to load settings.\n";
	}
}

EmulatorWindow::~EmulatorWindow() {
	glfwDestroyWindow(m_mainWindow);
	glfwTerminate();
	delete m_layoutFrameBuffer;
	delete m_ui;
}

void EmulatorWindow::HandleResolutionChange(uint32_t width, uint32_t height) {
	m_width = width;
	m_height = height;
	glViewport(0, 0, width, height);
}

void EmulatorWindow::HandleMouseEvent(int32_t button, int32_t action) {
	if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {
		m_ui->Click();
	}
}

void EmulatorWindow::HandleMouseMove(double x, double y) {
	// Transform window space to layout space.
	double layoutAspect = (double)layoutWidth / layoutHeight;
	double viewportAspect = (double)m_width / m_height;
	double scaleX = 1.0, scaleY = 1.0;
	double posX, posY;
	double sizeX, sizeY;
	if (viewportAspect > layoutAspect) {
		sizeY = 1.0;
		sizeX = layoutAspect / viewportAspect;
		posX = (1.0 - sizeX) * 0.5;
		posY = 0.0;
		scaleX = viewportAspect / layoutAspect;
	}
	else {
		sizeX = 1.0;
		sizeY = viewportAspect / layoutAspect;
		posX = 0.0;
		posY = (1.0 - sizeY) * 0.5;
		scaleY = layoutAspect / viewportAspect;
	}
	int32_t layoutX = (int32_t)((x / m_width - posX) * layoutWidth * scaleX);
	int32_t layoutY = (int32_t)((y / m_height - posY) * layoutHeight * scaleY);

	// update layout based on cursor position
	m_ui->SetCursorPosition(layoutX, layoutY);
}

void EmulatorWindow::HandleKeyEvent(int32_t key, int32_t scancode, int32_t action) {
	if (action == GLFW_PRESS) {
		if (m_isRebindingKey) {
			m_isRebindingKey = false;
			if (key == GLFW_KEY_ESCAPE) return;
			m_buttonMap.mappings[(uint32_t)m_keyToRebind] = key;
			return;
		}

		if (key == m_buttonMap.mappings[(uint32_t)EmulatorButton::Reset]) {
			m_emulator.Reset();
		}

		if (!m_paused && key == m_buttonMap.mappings[(uint32_t)EmulatorButton::Pause]) {
			m_paused = true;
		}
		else if (m_paused && key == m_buttonMap.mappings[(uint32_t)EmulatorButton::Resume]) {
			m_paused = false;
		}

		if (key == m_buttonMap.mappings[(uint32_t)EmulatorButton::SaveState]) {
			SaveStateToFile();
		}
		else if (key == m_buttonMap.mappings[(uint32_t)EmulatorButton::LoadState]) {
			LoadStateFromFile();
		}
	}
}

bool EmulatorWindow::LoadROM(const std::string& romPath) {
	std::cout << "Loading rom: \"" << romPath << "\"\n";
	std::ifstream reader(romPath, std::ios::in | std::ifstream::binary | std::fstream::ate);
	if (!reader) {
		std::cout << "Could not open ROM file: \"" << romPath << "\"\n";
		return false;
	}

	uint32_t romSize = (uint32_t)reader.tellg();
	uint8_t* ROMdata = new uint8_t[romSize];
	reader.seekg(0, reader.beg);
	reader.read((char*)ROMdata, romSize);
	reader.close();

	bool loaded = m_emulator.LoadROM(ROMdata, romSize);
	m_emulator.SetName(std::filesystem::path(romPath).filename().string());

	delete[] ROMdata;

	return loaded;
}

FileAccessState EmulatorWindow::LoadSettings() {
	std::ifstream reader = std::ifstream("settings.ini", std::ios::in | std::ifstream::binary | std::fstream::ate);

	if (!reader.is_open()) {
		if (std::filesystem::exists("settings.ini")) return FileAccessState::CouldNotOpenFile;
		else return FileAccessState::FileDoesntExist;
	}

	uint32_t size = (uint32_t)reader.tellg();
	uint8_t* data = new uint8_t[size + 1];
	reader.seekg(0, reader.beg);
	reader.read((char*)data, size);
	data[size] = '\0';
	reader.close();

	std::string settingsContet = std::string((char*)data);
	delete[] data;

	auto getValue = [&](const std::string& name) {
		size_t start = settingsContet.find(name);
		if (start == std::string::npos) return std::string();
		size_t end = settingsContet.find('\n', start);
		if (end == std::string::npos) end = settingsContet.size();
		return settingsContet.substr(start + name.size() + 1, end - start - name.size() - 1);
		};

	std::string windowWidthStr = getValue("windowWidth");
	std::string windowHeightStr = getValue("windowHeight");
	if (windowHeightStr != "" && windowWidthStr != "") {
		try {
			int32_t windowWidth = std::stoi(windowWidthStr);
			int32_t windowHeight = std::stoi(windowHeightStr);
			if (windowWidth < 0 || windowHeight < 0)  throw new std::exception("Window size in settings is not properly set.");

			int32_t monitorsCount;
			GLFWmonitor** monitors = glfwGetMonitors(&monitorsCount);
			if (monitorsCount == 0) throw new std::exception("GLFW could not recognize monitors.");
			int32_t monitorX, monitorY, monitorWidth, monitorHeight;
			glfwGetMonitorWorkarea(monitors[0], &monitorX, &monitorY, &monitorWidth, &monitorHeight);
			if (monitorWidth == NULL || monitorHeight == NULL) throw new std::exception("GLFW could not retrieve monitor size.");

			if (windowWidth > monitorWidth) windowWidth = monitorWidth;
			if (windowHeight > monitorHeight) windowHeight = monitorHeight;

			glfwSetWindowSize(m_mainWindow, windowWidth, windowHeight);
			HandleResolutionChange(windowWidth, windowHeight);
		}
		catch (std::exception e) {
			std::cout << "Failed to load window size from settings file.\n";
			std::cout << e.what() << "\n";
		}
	}

	std::string targetFPSStr = getValue("targetFPS");
	if (targetFPSStr != "") {
		try {
			float targetFPS = std::stof(targetFPSStr);
			m_targetFPS = targetFPS;
			if (targetFPS == 0.0f) m_targetFrameTime = 0.0f;
			else m_targetFrameTime = 1.0f / targetFPS;
		}
		catch (std::exception e) {
			std::cout << "Failed to load target fps from settings file.\n";
			std::cout << e.what() << "\n";
		}
	}

	auto loadKeyMapping = [&](const std::string& name, EmulatorButton button) {
		std::string value = getValue(name);
		if (value == "") return;
		try {
			int32_t key = std::stoi(value);
			if (key == 0) {
				std::cout << "Failed to load key mapping for button: " << name << ". Value is negative.\n";
				return;
			}
			m_buttonMap.mappings[(uint32_t)button] = key;
		}
		catch (std::exception e) {
			std::cout << "Failed to load button mapping from settings file. Button: " << name << "\n";
			std::cout << e.what() << "\n";
		}
		};

	loadKeyMapping("buttonA", EmulatorButton::A);
	loadKeyMapping("buttonB", EmulatorButton::B);
	loadKeyMapping("buttonStart", EmulatorButton::Start);
	loadKeyMapping("buttonSelect", EmulatorButton::Select);
	loadKeyMapping("buttonRight", EmulatorButton::Right);
	loadKeyMapping("buttonLeft", EmulatorButton::Left);
	loadKeyMapping("buttonUp", EmulatorButton::Up);
	loadKeyMapping("buttonDown", EmulatorButton::Down);

	loadKeyMapping("buttonReset", EmulatorButton::Reset);
	loadKeyMapping("buttonPause", EmulatorButton::Pause);
	loadKeyMapping("buttonResume", EmulatorButton::Resume);
	loadKeyMapping("buttonStep", EmulatorButton::Step);

	return FileAccessState::Ok;
}

FileAccessState EmulatorWindow::SaveSettings() {
	std::ofstream writer = std::ofstream("settings.ini", std::ios::out);

	if (!writer.is_open()) {
		if (std::filesystem::exists("settings.ini")) return FileAccessState::CouldNotOpenFile;
		else return FileAccessState::FileDoesntExist;
	}

	auto writeLine = [&](const std::string& line) {
		const std::string l = line + "\n";
		writer.write(l.c_str(), l.size());
		};

	writeLine("windowWidth " + std::to_string(m_width));
	writeLine("windowHeight " + std::to_string(m_height));
	writeLine("targetFPS " + std::to_string(m_targetFPS));

	writeLine("buttonA " + std::to_string(m_buttonMap.mappings[(uint32_t)EmulatorButton::A]));
	writeLine("buttonB " + std::to_string(m_buttonMap.mappings[(uint32_t)EmulatorButton::B]));
	writeLine("buttonSelect " + std::to_string(m_buttonMap.mappings[(uint32_t)EmulatorButton::Select]));
	writeLine("buttonStart " + std::to_string(m_buttonMap.mappings[(uint32_t)EmulatorButton::Start]));
	writeLine("buttonRight " + std::to_string(m_buttonMap.mappings[(uint32_t)EmulatorButton::Right]));
	writeLine("buttonLeft " + std::to_string(m_buttonMap.mappings[(uint32_t)EmulatorButton::Left]));
	writeLine("buttonUp " + std::to_string(m_buttonMap.mappings[(uint32_t)EmulatorButton::Up]));
	writeLine("buttonDown " + std::to_string(m_buttonMap.mappings[(uint32_t)EmulatorButton::Down]));

	writeLine("buttonReset " + std::to_string(m_buttonMap.mappings[(uint32_t)EmulatorButton::Reset]));
	writeLine("buttonPause " + std::to_string(m_buttonMap.mappings[(uint32_t)EmulatorButton::Pause]));
	writeLine("buttonResume " + std::to_string(m_buttonMap.mappings[(uint32_t)EmulatorButton::Resume]));
	writeLine("buttonStep " + std::to_string(m_buttonMap.mappings[(uint32_t)EmulatorButton::Step]));

	writer.close();

	return FileAccessState::Ok;
}

FileAccessState EmulatorWindow::LoadStateFromFile() {
	if (!std::filesystem::exists("saves")) return FileAccessState::FileDoesntExist;
	SaveState* state = ReadSaveStateFile();
	m_emulator.SetSaveState(state);
	m_emulator.LoadSaveState();
	return FileAccessState::Ok;
}

FileAccessState EmulatorWindow::SaveStateToFile() {
	if (!std::filesystem::exists("saves")) {
		std::filesystem::create_directories("saves");
	}
	SaveState* saveState = m_emulator.CreateSaveState();
	WriteSaveStateFile(saveState);
	return FileAccessState::Ok;
}

void EmulatorWindow::Start() {
	double lastTime = glfwGetTime();
	PixieNoise::Streamer* streamer = new PixieNoise::Streamer(4);
	while (!glfwWindowShouldClose(m_mainWindow)) {
		glfwPollEvents();
		UpdateKeyStates();

		if (!IsPaused()) {
			m_emulator.Run(GBCEmulator::FrameCycles);
		}

		UpdateScreen();

		if (m_emulator.IsFrameReady()) {
			m_emulator.ResetFrameReadyFlag();
		}

		streamer->QueueSamples(m_emulator.spu.GetSamples(), m_emulator.spu.GetSamplesCount(), spuSampleRate, true);
		m_emulator.spu.ClearSamples();

		double newTime = glfwGetTime();
		double deltaTime = (newTime - lastTime);
		while (deltaTime < m_targetFrameTime) {
			newTime = glfwGetTime();
			deltaTime = (newTime - lastTime);
		}
		lastTime = newTime;
		std::string windowTitle = "GBC Emulator: " + m_emulator.GetROMName() + " (" + std::to_string(deltaTime * 1000) + "ms)";
		glfwSetWindowTitle(m_mainWindow, windowTitle.c_str());
	}

	delete streamer;
	PixieNoise::Destroy();
}

void EmulatorWindow::UpdateScreen() {
	// Upload last frame rendered by emulator.
	m_ui->UploadViewportTexture(
		m_emulator.ppu.frameBuffers[!m_emulator.ppu.activeFrame].width,
		m_emulator.ppu.frameBuffers[!m_emulator.ppu.activeFrame].height,
		m_emulator.ppu.frameBuffers[!m_emulator.ppu.activeFrame].pixels.data(),
		GL_RGB, GL_FLOAT
	);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// Draw layout to texture
	m_layoutFrameBuffer->Bind();
	glViewport(0, 0, layoutWidth, layoutHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_ui->Draw();
	m_ui->PrintState();
	m_layoutFrameBuffer->Unbind();
	
	// Draw layout texture, scaled to fit window
	glViewport(0, 0, m_width, m_height);
	PixieUI::Renderer::DrawScreenPlane(m_layoutFrameBuffer->m_colorAttachment, layoutWidth, layoutHeight, m_width, m_height);

	glfwSwapBuffers(m_mainWindow);
}

void EmulatorWindow::UpdateKeyStates() {
	m_emulator.joypad1.buttons[(uint32_t)Button::A] = ButtonIsPressed(EmulatorButton::A);
	m_emulator.joypad1.buttons[(uint32_t)Button::B] = ButtonIsPressed(EmulatorButton::B);
	m_emulator.joypad1.buttons[(uint32_t)Button::Select] = ButtonIsPressed(EmulatorButton::Select);
	m_emulator.joypad1.buttons[(uint32_t)Button::Start] = ButtonIsPressed(EmulatorButton::Start);
	m_emulator.joypad1.buttons[(uint32_t)Button::Right] = ButtonIsPressed(EmulatorButton::Right);
	m_emulator.joypad1.buttons[(uint32_t)Button::Left] = ButtonIsPressed(EmulatorButton::Left);
	m_emulator.joypad1.buttons[(uint32_t)Button::Up] = ButtonIsPressed(EmulatorButton::Up);
	m_emulator.joypad1.buttons[(uint32_t)Button::Down] = ButtonIsPressed(EmulatorButton::Down);
}

bool EmulatorWindow::ButtonIsPressed(EmulatorButton button) {
	return glfwGetKey(m_mainWindow, m_buttonMap.mappings[(uint32_t)button]) == GLFW_PRESS;
}

bool EmulatorWindow::IsPaused() {
	return m_paused || m_ui->m_windowOpened;
}

bool EmulatorWindow::WriteSaveStateFile(SaveState* state) {
	if (!state) return false;
	std::ofstream writer("saves/" + m_emulator.GetROMName() + ".sav", std::ios::out | std::ios::binary);
	if (!writer.is_open()) return false;
	writer.write((char*)state->data.data(), state->data.size());
	writer.close();
	return true;
}

SaveState* EmulatorWindow::ReadSaveStateFile() {
	std::ifstream reader("saves/" + m_emulator.GetROMName() + ".sav", std::ios::in | std::ios::binary | std::ios::ate);
	if (!reader.is_open()) return nullptr;
	uint32_t size = (uint32_t)reader.tellg();
	reader.seekg(std::ios::beg);
	SaveState* state = new SaveState(m_emulator.GetROMName());
	state->data.resize(size);
	reader.read((char*)state->data.data(), size);
	reader.close();
	return state;
}

uint32_t EmulatorWindow::GetWidth() {
	return m_width;
}

uint32_t EmulatorWindow::GetHeight() {
	return m_height;
}