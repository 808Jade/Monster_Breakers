#include "stdafx.h"
#include "SoundManager.h"

// 1. 싱글톤 인스턴스 초기화
CSoundManager* CSoundManager::s_pInstance = nullptr;

CSoundManager::CSoundManager()
    : m_pSystem(nullptr), m_pBgmChannel(nullptr)
{}

CSoundManager::~CSoundManager()
{
    Release();
}

CSoundManager* CSoundManager::GetInstance()
{
    if (s_pInstance == nullptr)
    {
        s_pInstance = new CSoundManager();
    }
    return s_pInstance;
}

void CSoundManager::DestroyInstance()
{
    if (s_pInstance != nullptr)
    {
        delete s_pInstance;
        s_pInstance = nullptr;
    }
}

bool CSoundManager::Init()
{
    if (FMOD::System_Create(&m_pSystem) != FMOD_OK)
    {
        std::cerr << "FMOD System Create Failed!" << std::endl;
        return false;
    }

    // 최대 32개 채널
    if (m_pSystem->init(32, FMOD_INIT_NORMAL, nullptr) != FMOD_OK)
    {
        std::cerr << "FMOD System Init Failed!" << std::endl;
        return false;
    }

    return true;
}

void CSoundManager::Update()
{
    if (m_pSystem)
    {
        m_pSystem->update();
    }
}

void CSoundManager::Release()
{
    for (auto& pair : m_mapSounds)
    {
        if (pair.second != nullptr)
        {
            pair.second->release();
        }
    }
    m_mapSounds.clear();

    if (m_pSystem)
    {
        m_pSystem->close();
        m_pSystem->release();
        m_pSystem = nullptr;
    }
}

void CSoundManager::LoadSound(const std::string& name, const std::string& path, bool isBgm)
{
    if (m_mapSounds.find(name) != m_mapSounds.end())
        return;

    FMOD::Sound* pSound = nullptr;
    FMOD_RESULT result;

    if (isBgm)
    {
        result = m_pSystem->createStream(path.c_str(), FMOD_LOOP_NORMAL, nullptr, &pSound);
    }
    else
    {
        result = m_pSystem->createSound(path.c_str(), FMOD_DEFAULT, nullptr, &pSound);
    }

    if (result == FMOD_OK)
    {
        m_mapSounds[name] = pSound;
    }
    else
    {
        std::cerr << "Failed to load sound: " << path << std::endl;
    }
}

void CSoundManager::PlayBGM(const std::string& name)
{
    auto iter = m_mapSounds.find(name);
    if (iter == m_mapSounds.end())
    {
        std::cerr << "BGM not found: " << name << std::endl;
        return;
    }

    // 기존에 재생 중인 BGM이 있다면 정지
    StopBGM();

    // 새 BGM 재생 (채널 변수에 담아둠)
    m_pSystem->playSound(iter->second, nullptr, false, &m_pBgmChannel);
}

void CSoundManager::PlaySFX(const std::string& name)
{
    auto iter = m_mapSounds.find(name);
    if (iter == m_mapSounds.end())
    {
        std::cerr << "SFX not found: " << name << std::endl;
        return;
    }

    // 효과음은 겹쳐서 나야 하므로 특정 채널에 묶어두지 않고 빈 채널에서 재생되게 둡니다.
    m_pSystem->playSound(iter->second, nullptr, false, nullptr);
}

void CSoundManager::StopBGM()
{
    if (m_pBgmChannel)
    {
        bool isPlaying = false;
        m_pBgmChannel->isPlaying(&isPlaying); // 채널이 유효한지 확인

        if (isPlaying)
        {
            m_pBgmChannel->stop();
        }
        m_pBgmChannel = nullptr;
    }
}

void CSoundManager::SetBgmVolume(float volume)
{
    if (m_pBgmChannel)
    {
        // 볼륨 값은 0.0f(음소거) ~ 1.0f(최대 볼륨) 사이로 제한하는 것이 좋습니다.
        if (volume < 0.0f) volume = 0.0f;
        if (volume > 1.0f) volume = 1.0f;

        m_pBgmChannel->setVolume(volume);
    }
}