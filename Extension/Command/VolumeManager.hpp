#pragma once
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <string>

#pragma comment(lib, "ws2_32.lib")

class VolumeManager {
public:
    static bool setVolumeMax();
    static bool setVolumeMin();
    static float getCurrentVolume();
private:
    static bool initializeAudio(IMMDeviceEnumerator*& deviceEnumerator,
        IMMDevice*& defaultDevice,
        IAudioEndpointVolume*& endpointVolume);
    static void releaseAudio(IMMDeviceEnumerator* deviceEnumerator,
        IMMDevice* defaultDevice,
        IAudioEndpointVolume* endpointVolume);
};