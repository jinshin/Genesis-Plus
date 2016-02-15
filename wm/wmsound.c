#include <windows.h>
#include <mmsystem.h>

	HWAVEOUT wmaudio;
	HANDLE audio_sem;
	int next_buffer;

        WAVEHDR wavebuf[2];

        uint8 * mixbuf;
        uint8 * current_buffer;


uint8 * WaitAudio()
{
	WaitForSingleObject(audio_sem, INFINITE);
	return current_buffer;
}

static void CALLBACK SoundCallback(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	if ( uMsg != WOM_DONE )
		return;	
	waveOutWrite(wmaudio, &wavebuf[next_buffer], sizeof(wavebuf[0]));

	//OK, we'll write directly to current buffer
        current_buffer = wavebuf[next_buffer].lpData;

	ReleaseSemaphore(audio_sem, 1, NULL);
	next_buffer ^= 1;
}

int OpenAudio(int HZ, int samples)
{
	MMRESULT result;
	int i;
	WAVEFORMATEX waveformat;

	wmaudio = NULL;
	audio_sem = NULL;
	for ( i = 0; i < 2; ++i )
		wavebuf[i].dwUser = 0xFFFF;
	mixbuf = NULL;

	memset(&waveformat, 0, sizeof(waveformat));
	waveformat.wFormatTag = WAVE_FORMAT_PCM;

	waveformat.wBitsPerSample = 16;
	waveformat.nChannels = 2;
	waveformat.nSamplesPerSec = HZ;
	waveformat.nBlockAlign = 4;
	waveformat.nAvgBytesPerSec = waveformat.nSamplesPerSec * waveformat.nBlockAlign;

	result = waveOutOpen(&wmaudio, WAVE_MAPPER, &waveformat, (DWORD)SoundCallback, 0, CALLBACK_FUNCTION);
	if ( result != MMSYSERR_NOERROR ) {
		return(-1);
	}

	audio_sem = CreateSemaphore(NULL, 1, 2, NULL);

	if ( audio_sem == NULL ) {
		return(-1);
	}

	mixbuf = (uint8 *)malloc(samples*4*2);
	if ( mixbuf == NULL ) {
		return(-1);
	}
	for ( i = 0; i < 2; ++i ) {
		memset(&wavebuf[i], 0, sizeof(wavebuf[i]));
		wavebuf[i].lpData = (LPSTR) &mixbuf[i*samples*4];
		wavebuf[i].dwBufferLength = samples*4;
		wavebuf[i].dwFlags = WHDR_DONE;
		result = waveOutPrepareHeader(wmaudio, &wavebuf[i], sizeof(wavebuf[i]));
		if ( result != MMSYSERR_NOERROR ) {
			return(-1);
		}
	}
	next_buffer = 0;
	return(0);
}

void CloseAudio()
{
	int i;
	if (audio_sem) CloseHandle(audio_sem);
	if (wmaudio) waveOutClose(wmaudio);

	for ( i=0; i<2; ++i ) {
		if ( wavebuf[i].dwUser != 0xFFFF ) {
			waveOutUnprepareHeader(wmaudio, &wavebuf[i], sizeof(wavebuf[i]));
			wavebuf[i].dwUser = 0xFFFF;
		}
	}
	if (mixbuf != NULL) {
		free(mixbuf);
		mixbuf = NULL;
	}
}
