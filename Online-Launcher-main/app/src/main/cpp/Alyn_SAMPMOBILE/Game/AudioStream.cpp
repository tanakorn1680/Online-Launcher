#include <memory>
#include <unistd.h>
#include <pthread.h>

#include "AudioStream.h"
#include "Game.h"
#include "../Client.h"

extern Game* pGame;

HSTREAM bassStream = 0;
char g_szAudioStreamUrl[256 + 1];
float g_fAudioStreamX;
float g_fAudioStreamY;
float g_fAudioStreamZ;
float g_fAudioStreamRadius;
bool g_audioStreamUsePos;
bool g_bAudioStreamStop;
bool g_bAudioStreamThreadWorked;

void* audioStreamThread(void*)
{
	g_bAudioStreamThreadWorked = true;
	if (bassStream) {
		BASS_ChannelStop(bassStream);
		bassStream = 0;
	}

	bassStream = BASS_StreamCreateURL(g_szAudioStreamUrl, 0, 0x940000, 0);
	BASS_ChannelPlay(bassStream, 0);

	//BASS_ChannelSetSync(bassStream, 4, 0, 0, 0);
	//BASS_ChannelSetSync(bassStream, 12, 0, 0, 0);

	while (!g_bAudioStreamStop) {
		// ..
		usleep(2000);
	}

	BASS_ChannelStop(bassStream);
	bassStream = 0;
	g_bAudioStreamThreadWorked = false;
	pthread_exit(nullptr);
}

AudioStream::AudioStream()
{
	m_bInited = false;
	m_bBassTried = false;
}

// BASS is a second audio engine running next to GTA's OpenAL. It is only needed
// when the server plays an audio stream, so it is started on first use (Play)
// instead of at game start. No BASS_Free here: voice chat inits BASS itself.
bool AudioStream::InitBass()
{
	if (!BASS_Init || !BASS_Free) {
		spdlog::error("AudioStream disabled: BASS library is unavailable");
		return false;
	}

	if (!BASS_Init(-1, 44100, 0)) {
		if (BASS_ErrorGetCode() != 14) { // 14 = BASS_ERROR_ALREADY (already initialised, fine)
			spdlog::error("AudioStream: BASS_Init failed (code {})", BASS_ErrorGetCode());
			return false;
		}
	}

	BASS_SetConfigPtr(16, "SA-MP/0.3");
	//BASS_SetConfig(5, ) volume
	BASS_SetConfig(21, 1);            // BASS_CONFIG_NET_PLAYLIST
	BASS_SetConfig(11, 10000);        // BASS_CONFIG_NET_TIMEOUT

	m_bInited = true;
	spdlog::info("AudioStream: BASS initialised");
	return true;
}

bool AudioStream::Initialize()
{
	spdlog::info("AudioStream::Initialize(): BASS init deferred until the first audio stream");

	bassStream = 0;
	return true;
}

void AudioStream::Process()
{
	if (g_bAudioStreamThreadWorked) {
		// ..
		if (Game::IsGamePaused()) {
			BASS_SetConfig(5, 0);
		}
		else {
			BASS_SetConfig(5, 5000);
		}
	}
}

bool AudioStream::Play(const char* szUrl, float fX, float fY, float fZ, float fRadius, bool bUsePos)
{
	spdlog::info("Play: {}", szUrl);

	if (!m_bInited) {
		if (m_bBassTried) return false;
		m_bBassTried = true;
		if (!InitBass()) return false;
	}
	Stop(true);

	if (bassStream) {
		BASS_ChannelStop(bassStream);
		bassStream = 0;
	}

	memset(g_szAudioStreamUrl, 0, sizeof(g_szAudioStreamUrl));
	strncpy(g_szAudioStreamUrl, szUrl, 256);

	g_fAudioStreamX = fX;
	g_fAudioStreamY = fY;
	g_fAudioStreamZ = fZ;
	g_fAudioStreamRadius = fRadius;
	g_audioStreamUsePos = bUsePos;

	g_bAudioStreamStop = false;

	pthread_t thread;
	pthread_create(&thread, nullptr, audioStreamThread, nullptr);
	return true;
}

bool AudioStream::Stop(bool bWaitThread)
{
	spdlog::info("Stop: {}", g_szAudioStreamUrl);
	if (!m_bInited || !g_bAudioStreamThreadWorked) {
		return false;
	}

	g_bAudioStreamStop = true;

	if (bWaitThread) {
		while (g_bAudioStreamThreadWorked)
			usleep(200);
	}

	BASS_StreamFree(bassStream);
	bassStream = 0;
	return true;
}
