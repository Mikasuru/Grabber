/*
Manages audio settings
*/
#include "VolumeManager.hpp"
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#pragma comment(lib, "ole32.lib")

bool VolumeManager::setVolumeMax() {
    IMMDeviceEnumerator* deviceEnumerator = nullptr;
    IMMDevice* defaultDevice = nullptr;
    IAudioEndpointVolume* endpointVolume = nullptr;

    if (!initializeAudio(deviceEnumerator, defaultDevice, endpointVolume)) {
        return false;
    }

    bool success = (endpointVolume->SetMasterVolumeLevelScalar(1.0f, NULL) == S_OK);

    releaseAudio(deviceEnumerator, defaultDevice, endpointVolume);
    return success;
}

bool VolumeManager::setVolumeMin() {
    IMMDeviceEnumerator* deviceEnumerator = nullptr;
    IMMDevice* defaultDevice = nullptr;
    IAudioEndpointVolume* endpointVolume = nullptr;

    if (!initializeAudio(deviceEnumerator, defaultDevice, endpointVolume)) {
        return false;
    }

    bool success = (endpointVolume->SetMasterVolumeLevelScalar(0.0f, NULL) == S_OK);

    releaseAudio(deviceEnumerator, defaultDevice, endpointVolume);
    return success;
}

float VolumeManager::getCurrentVolume() {
    IMMDeviceEnumerator* deviceEnumerator = nullptr;
    IMMDevice* defaultDevice = nullptr;
    IAudioEndpointVolume* endpointVolume = nullptr;
    float currentVolume = 0.0f;

    if (initializeAudio(deviceEnumerator, defaultDevice, endpointVolume)) {
        endpointVolume->GetMasterVolumeLevelScalar(&currentVolume);
        releaseAudio(deviceEnumerator, defaultDevice, endpointVolume);
    }

    return currentVolume;
}

bool VolumeManager::initializeAudio(IMMDeviceEnumerator*& deviceEnumerator,
    IMMDevice*& defaultDevice,
    IAudioEndpointVolume*& endpointVolume) {
    if (FAILED(CoInitialize(NULL))) return false;

    if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL,
        CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
        (void**)&deviceEnumerator))) {
        CoUninitialize();
        return false;
    }

    if (FAILED(deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice))) {
        deviceEnumerator->Release();
        CoUninitialize();
        return false;
    }

    if (FAILED(defaultDevice->Activate(__uuidof(IAudioEndpointVolume),
        CLSCTX_ALL, NULL, (void**)&endpointVolume))) {
        defaultDevice->Release();
        deviceEnumerator->Release();
        CoUninitialize();
        return false;
    }

    return true;
}

void VolumeManager::releaseAudio(IMMDeviceEnumerator* deviceEnumerator,
    IMMDevice* defaultDevice,
    IAudioEndpointVolume* endpointVolume) {
    if (endpointVolume) endpointVolume->Release();
    if (defaultDevice) defaultDevice->Release();
    if (deviceEnumerator) deviceEnumerator->Release();
    CoUninitialize();
}