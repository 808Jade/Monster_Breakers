#pragma once
#include <fmod.hpp>
#include <string>
#include <unordered_map>

// 싱글톤 객체
class CSoundManager
{
private:
    static CSoundManager* s_pInstance;

    FMOD::System* m_pSystem;

    // 로드된 사운드를 보관할 컨테이너
    std::unordered_map<std::string, FMOD::Sound*> m_mapSounds;

    FMOD::Channel* m_pBgmChannel;

    // 싱글톤을 위해 생성자와 소멸자는 private으로 은닉
    CSoundManager();
    ~CSoundManager();

public:
    // 싱글톤 접근 및 해제 메서드
    static CSoundManager* GetInstance();
    static void DestroyInstance();

    // 매니저 생명주기
    bool Init();
    void Update();
    void Release();

    // isBgm이 true이면 스트리밍 방식(createStream), false면 메모리 상주 방식(createSound)으로 로드
    void LoadSound(const std::string& name, const std::string& path, bool isBgm);

    void PlayBGM(const std::string& name);
    void PlaySFX(const std::string& name);
    void StopBGM();

    // 볼륨 조절 (0.0f ~ 1.0f)
    void SetBgmVolume(float volume);
};