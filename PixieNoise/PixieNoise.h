#pragma once
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <queue>

namespace PixieNoise {

	extern ALCdevice* device;
	extern ALCcontext* context;

	class Source {
	public:
		bool Play() {
			alSourcePlay(m_source);
			if (!CheckALErrors()) {
				std::cout << "OpenAL failed to play source\n";
				return false;
			}
			return true;
		}

		bool Pause() {
			alSourcePause(m_source);
			if (!CheckALErrors()) {
				std::cout << "OpenAL failed to pause source\n";
				return false;
			}
			return true;
		}

		bool Stop() {
			alSourceStop(m_source);
			if (!CheckALErrors()) {
				std::cout << "OpenAL failed to stop source\n";
				return false;
			}
			return true;
		}

		bool IsPlaying() {
			ALint state;
			alGetSourcei(m_source, AL_SOURCE_STATE, &state);
			if (!CheckALErrors()) {
				std::cout << "OpenAL failed to retrieve source state.\n";
				return false;
			}
			return state == AL_PLAYING;
		}

		// Warning: errors can lead to infinite loop.
		bool WaitToFinish() {
			while (IsPlaying()) {}
			return true;
		}

		bool CheckALErrors() {
			ALenum error = alGetError();
			if (error != AL_NO_ERROR) {
				switch (error) {
				case AL_INVALID_NAME:
					std::cerr << "AL_INVALID_NAME: a bad name (ID) was passed to an OpenAL function";
					break;
				case AL_INVALID_ENUM:
					std::cerr << "AL_INVALID_ENUM: an invalid enum value was passed to an OpenAL function";
					break;
				case AL_INVALID_VALUE:
					std::cerr << "AL_INVALID_VALUE: an invalid value was passed to an OpenAL function";
					break;
				case AL_INVALID_OPERATION:
					std::cerr << "AL_INVALID_OPERATION: the requested operation is not valid";
					break;
				case AL_OUT_OF_MEMORY:
					std::cerr << "AL_OUT_OF_MEMORY: the requested operation resulted in OpenAL running out of memory";
					break;
				default:
					std::cerr << "UNKNOWN AL ERROR: " << error;
				}
				std::cerr << std::endl;
				return false;
			}
			return true;
		}

	protected:
		ALuint m_source;

		Source() {
			alGenSources((ALuint)1, &m_source);
			if (!CheckALErrors()) {
				std::cout << "OpenAL failed to create source.\n";
				return;
			}

			alSourcef(m_source, AL_PITCH, 1);
			if (!CheckALErrors()) {
				std::cout << "OpenAL failed to set source pitch.\n";
			}

			alSourcef(m_source, AL_GAIN, 1);
			if (!CheckALErrors()) {
				std::cout << "OpenAL failed to set source gain.\n";
			}

			alSource3f(m_source, AL_POSITION, 0, 0, 0);
			if (!CheckALErrors()) {
				std::cout << "OpenAL failed to set source position.\n";
			}

			alSource3f(m_source, AL_VELOCITY, 0, 0, 0);
			if (!CheckALErrors()) {
				std::cout << "OpenAL failed to set source velocity.\n";
			}

			alSourcei(m_source, AL_LOOPING, AL_FALSE);
			if (!CheckALErrors()) {
				std::cout << "OpenAL failed to set source loop.\n";
			}
		}

		~Source() {
			alDeleteSources(1, &m_source);
		}
	};

	class Player : public Source {
	public:
		Player() {
			alGenBuffers(1, &m_buffer);
			if (!CheckALErrors()) {
				std::cout << "OpenAL failed to create buffer.\n";
				return;
			}

			alSourcei(m_source, AL_BUFFER, m_buffer);
			if (!CheckALErrors()) {
				std::cout << "OpenAL failed to bind buffer to source.\n";
				return;
			}
		}

		~Player() {
			alDeleteBuffers(1, &m_buffer);
		}

		bool Play(int16_t* data, uint32_t samples, uint32_t sampleRate, bool waitToFinish = false) {
			if (waitToFinish) WaitToFinish();
			else Stop();

			alSourcei(m_source, AL_BUFFER, NULL);
			if (!CheckALErrors()) {
				std::cout << "OpenAL failed to unbind buffer from source.\n";
				return false;
			}

			alBufferData(m_buffer, AL_FORMAT_MONO16, data, samples, sampleRate);
			if (!CheckALErrors()) {
				std::cout << "OpenAL failed to load data.\n";
				return false;
			}

			alSourcei(m_source, AL_BUFFER, m_buffer);
			if (!CheckALErrors()) {
				std::cout << "OpenAL failed to bind buffer to source.\n";
				return false;
			}

			return Source::Play();
		}

	protected:
		ALuint m_buffer;
	};

	class Streamer : public Source {
	public:
		Streamer(uint32_t queueSize)
			: m_queueSize(queueSize) {
			m_buffers = new ALuint[queueSize];
			alGenBuffers(queueSize, m_buffers);
			if (!CheckALErrors()) {
				std::cout << "OpenAL failed to create buffers.\n";
				return;
			}
			for (uint32_t i = 0; i < queueSize; i++) {
				m_freeBuffers.push(m_buffers[i]);
			}
		}

		~Streamer() {
			alDeleteBuffers(4, m_buffers);
			delete[] m_buffers;
		}

		bool QueueSamples(int16_t* data, uint32_t samples, uint32_t sampleRate, bool waitToQueue = false) {
			samples -= samples % 4;

			ALint processedBuffers = 0;
			alGetSourcei(m_source, AL_BUFFERS_PROCESSED, &processedBuffers);
			if (!CheckALErrors()) {
				std::cout << "OpenAL failed to retrieve number of processed buffers.\n";
				return false;
			}

			if (waitToQueue && m_freeBuffers.size() == 0) {
				while (IsPlaying() && processedBuffers == 0) {
					alGetSourcei(m_source, AL_BUFFERS_PROCESSED, &processedBuffers);
					if (!CheckALErrors()) {
						std::cout << "OpenAL failed to retrieve number of processed buffers.\n";
						return false;
					}
				}
			}

			while (processedBuffers > 0) {
				ALuint buffer;
				alSourceUnqueueBuffers(m_source, 1, &buffer);
				if (!CheckALErrors()) {
					std::cout << "OpenAL failed to unqueu buffer.\n";
					return false;
				}
				m_freeBuffers.push(buffer);
				processedBuffers--;
			}

			if (m_freeBuffers.size() == 0) {
				return false;
			}

			ALuint buffer = m_freeBuffers.front();
			m_freeBuffers.pop();

			alBufferData(buffer, AL_FORMAT_MONO16, data, samples * sizeof(int16_t), sampleRate);
			if (!CheckALErrors()) {
				std::cout << "OpenAL failed to load data.\n";
				return false;
			}

			alSourceQueueBuffers(m_source, 1, &buffer);
			if (!CheckALErrors()) {
				std::cout << "OpenAL failed to queue buffers.\n";
				return false;
			}

			if (!IsPlaying()) {
				return Play();
			}

			return true;
		}

	protected:
		uint32_t m_queueSize;
		ALuint* m_buffers;
		std::queue<ALuint> m_freeBuffers;
	};

	static bool Initialize() {
		device = alcOpenDevice(NULL);
		if (!device) {
			std::cout << "OpenAL failed to create device.\n";
			return false;
		}

		context = alcCreateContext(device, NULL);
		if (!alcMakeContextCurrent(context)) {
			std::cout << "OpenAL failed to set context.\n";
			return false;
		}

		device = alcGetContextsDevice(context);
		
		return true;
	}

	static void Destroy() {
		alcMakeContextCurrent(NULL);
		alcDestroyContext(context);
		alcCloseDevice(device);
		context = nullptr;
		device = nullptr;
	}

	 static void ListAudioDevices() {
		const ALCchar* devices = alcGetString(NULL, ALC_DEVICE_SPECIFIER);
		const ALCchar* device = devices, * next = devices + 1;
		size_t len = 0;
		fprintf(stdout, "Devices list:\n");
		fprintf(stdout, "----------\n");
		while (device && *device != '\0' && next && *next != '\0') {
			fprintf(stdout, "%s\n", device);
			len = strlen(device);
			device += (len + 1);
			next += (len + 2);
		}
		fprintf(stdout, "----------\n");
	}

	 static int16_t* GenerateSine(uint32_t samples, float frequency, uint32_t sampleRate) {
		 int16_t* data = new int16_t[samples];
		 const double dt = 1.0 / sampleRate;
		 const int16_t scale = SHRT_MAX / 4;
		 for (uint32_t i = 0; i < samples; i++) {
			 data[i] = (int16_t)(scale * std::sin(frequency * 3.1417 * 2.0 * i * dt));
		 }
		 return data;
	 }

	 static int16_t* GenerateSineTransitionLinear(uint32_t samples, float frequencyFrom, float frequencyTo, uint32_t sampleRate) {
		 int16_t* data = new int16_t[samples];
		 const double dt = 1.0 / sampleRate;
		 const int16_t scale = SHRT_MAX / 4;
		 for (uint32_t i = 0; i < samples; i++) {
			 float t = (float)i / samples;
			 data[i] = (int16_t)(scale * std::sin(((1 - t) * frequencyFrom + t * frequencyTo) * 3.1417 * 2.0 * i * dt));
		 }
		 return data;
	 }

	 static int16_t* GenerateSineTransitionQuadratic(uint32_t samples, float frequencyFrom, float frequencyTo, uint32_t sampleRate) {
		 int16_t* data = new int16_t[samples];
		 const double dt = 1.0 / sampleRate;
		 const int16_t scale = SHRT_MAX / 4;
		 for (uint32_t i = 0; i < samples; i++) {
			 float t = (float)i / samples;
			 data[i] = (int16_t)(scale * std::sin(((1 - t * t) * frequencyFrom + t * t * frequencyTo) * 3.1417 * 2.0 * i * dt));
		 }
		 return data;
	 }

	 static int16_t* GenerateNoise(uint32_t samples, uint32_t sampleRate) {
		 int16_t* data = new int16_t[samples];
		 const double dt = 1.0 / sampleRate;
		 const int16_t scale = SHRT_MAX / 4;
		 for (uint32_t i = 0; i < samples; i++) {
			 data[i] = (int16_t)(scale * std::sin(rand()));
		 }
		 return data;
	 }
}
