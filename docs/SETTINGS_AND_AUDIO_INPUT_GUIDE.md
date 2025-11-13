# VoiVoiAnalyzer - Settings & Audio Input Usage Guide

## 概要

このドキュメントは、設定ウィンドウとオーディオ入力マネージャーの使用方法を説明します。

## 設定ウィンドウ (SettingsComponent)

### 機能

#### 1. オーディオデバイス選択
- **入力デバイス選択**: システムで利用可能な全オーディオ入力デバイスを列挙
- **デフォルトデバイス自動選択**: OS のデフォルト入力デバイスを推奨
- **デバイス情報表示**: チャネル数、既定サンプルレートを表示

#### 2. サンプルレート設定
- **Auto (48 kHz preferred)**: デバイスと自動ネゴシエーション、48kHz 優先
- **44100 Hz**: CD 品質固定
- **48000 Hz**: プロオーディオ標準固定
- **88200 Hz / 96000 Hz**: ハイレゾ（オプション）

#### 3. バッファサイズ
- **OS 既定値固定**: 最適レイテンシのため変更不可
- **Windows**: 通常 256 または 512 サンプル
- **表示のみ**: 実際の値はデバイスオープン時に取得

#### 4. パフォーマンスモード
- **Light**: ≤40ms レイテンシ、FFT 1024、Hop 512
- **Normal**: ≤60ms レイテンシ、FFT 2048、Hop 512（推奨）
- **Diagnostic**: ≤80ms レイテンシ、FFT 4096、Hop 1024

#### 5. ライブ録音制限
- **範囲**: 5〜120 分（5分刻み）
- **Desktop 既定**: 60 分
- **Mobile 既定**: 15 分（将来実装）

#### 6. 自動保存オプション
- **Enable auto-save**: 自動保存機能の有効/無効
- **Save on stop**: 停止時に自動保存
- **Save on limit reached**: 制限時間到達時に自動保存

### 使用例

```cpp
#include "SettingsComponent.h"

// Settings ウィンドウを開く
auto* settingsWindow = new yvc::app::SettingsWindow();
auto* settingsComponent = settingsWindow->getSettingsComponent();

// コールバック設定
settingsComponent->onSettingsChanged = [](const auto& settings) {
    // 設定変更時の処理
    std::cout << "Device: " << settings.inputDeviceName << std::endl;
    std::cout << "Sample Rate: " << settings.sampleRate << " Hz" << std::endl;
    std::cout << "Performance Mode: " << static_cast<int>(settings.performanceMode) << std::endl;
    
    // オーディオデバイスを再オープン
    // audioInputManager->openDevice(settings.inputDeviceId, settings.sampleRate, settings.bufferSize);
};

// 既存設定をロード
yvc::app::SettingsComponent::Settings currentSettings;
currentSettings.sampleRate = 48000;
currentSettings.performanceMode = yvc::PerformanceMode::Normal;
currentSettings.liveMaxMinutes = 60;
currentSettings.autoSaveEnabled = true;
settingsComponent->setSettings(currentSettings);
```

## オーディオ入力マネージャー (AudioInputManager)

### 機能

#### 1. デバイス管理
- **自動バックエンド選択**: Windows (WASAPI), macOS (CoreAudio), Linux (ALSA)
- **デバイス列挙**: 利用可能な全入力デバイス取得
- **デバイスオープン**: ID 指定またはデフォルトデバイス
- **自動パラメータネゴシエーション**: サンプルレート、バッファサイズ

#### 2. リアルタイムオーディオ処理
- **ロックフリーリングバッファ**: 2秒分（96000サンプル × 2）
- **スレッドセーフ**: オーディオスレッド → 解析スレッド
- **XRun 検出**: バッファオーバーフロー自動検出
- **統計情報**: コールバック回数、サンプル数、レベル

#### 3. パフォーマンス監視
- **平均コールバック時間**: 直近 100 回の平均
- **XRun カウント**: バッファオーバーラン回数
- **Peak / RMS レベル**: 入力信号レベル監視

### 使用例

```cpp
#include "AudioInputManager.h"

// AudioInputManager を作成
yvc::app::AudioInputManager audioManager;

// デフォルトデバイスをオープン（48kHz, 512サンプルバッファ）
if (audioManager.openDefaultDevice(48000, 512)) {
    std::cout << "Device: " << audioManager.getDeviceName() << std::endl;
    std::cout << "Actual SR: " << audioManager.getActualSampleRate() << " Hz" << std::endl;
    std::cout << "Actual Buffer: " << audioManager.getActualBufferSize() << " samples" << std::endl;
}

// コールバック設定（オプション）
audioManager.onXRun = []() {
    std::cout << "WARNING: Audio XRun detected!" << std::endl;
};

audioManager.onDeviceError = []() {
    std::cout << "ERROR: Audio device error!" << std::endl;
};

// オーディオ入力開始
audioManager.start();

// 解析スレッドでデータを読み取り
std::vector<float> audioBuffer(2048);
while (running) {
    size_t available = audioManager.getAvailableSamples();
    
    if (available >= audioBuffer.size()) {
        size_t read = audioManager.readAudioData(audioBuffer.data(), audioBuffer.size());
        
        // 解析処理
        // analyzer.analyze(audioBuffer.data(), read, audioManager.getActualSampleRate());
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

// 統計情報取得
auto stats = audioManager.getStatistics();
std::cout << "Total Samples: " << stats.totalSamplesReceived << std::endl;
std::cout << "Total Callbacks: " << stats.totalCallbacks << std::endl;
std::cout << "XRuns: " << stats.xruns << std::endl;
std::cout << "Avg Callback Time: " << stats.averageCallbackTime * 1000.0 << " ms" << std::endl;
std::cout << "Peak Level: " << stats.peakLevel << std::endl;
std::cout << "RMS Level: " << stats.rmsLevel << std::endl;

// 停止とクローズ
audioManager.stop();
audioManager.closeDevice();
```

## MainComponent での統合例

```cpp
class MainComponent : public juce::Component, private juce::Timer {
public:
    MainComponent() {
        // Settings ボタン
        settingsButton_.setButtonText("Settings");
        settingsButton_.onClick = [this] { openSettings(); };
        addAndMakeVisible(settingsButton_);
        
        // Audio Input Manager 初期化
        audioInputManager_ = std::make_unique<yvc::app::AudioInputManager>();
        
        // デフォルトデバイスで開始
        if (audioInputManager_->openDefaultDevice(48000, 512)) {
            audioInputManager_->start();
            startTimer(33);  // 30 FPS for UI updates
        }
    }
    
    void openSettings() {
        if (settingsWindow_) {
            settingsWindow_->toFront(true);
            return;
        }
        
        settingsWindow_ = std::make_unique<yvc::app::SettingsWindow>();
        auto* settingsComp = settingsWindow_->getSettingsComponent();
        
        // 現在の設定を反映
        auto currentSettings = getCurrentSettings();
        settingsComp->setSettings(currentSettings);
        
        // 設定変更コールバック
        settingsComp->onSettingsChanged = [this](const auto& settings) {
            applySettings(settings);
        };
    }
    
    void applySettings(const yvc::app::SettingsComponent::Settings& settings) {
        // オーディオ停止
        audioInputManager_->stop();
        audioInputManager_->closeDevice();
        
        // 新しい設定でデバイスオープン
        if (!settings.inputDeviceId.empty()) {
            audioInputManager_->openDevice(
                settings.inputDeviceId,
                settings.sampleRate,
                settings.bufferSize
            );
        } else {
            audioInputManager_->openDefaultDevice(
                settings.sampleRate,
                settings.bufferSize
            );
        }
        
        // パフォーマンスモード適用
        performanceMode_ = settings.performanceMode;
        
        // ライブ制限適用
        liveMaxSeconds_ = settings.liveMaxMinutes * 60;
        
        // 自動保存設定
        autoSaveEnabled_ = settings.autoSaveEnabled;
        autoSaveOnStop_ = settings.autoSaveOnStop;
        autoSaveOnLimit_ = settings.autoSaveOnLimit;
        
        // オーディオ再開
        audioInputManager_->start();
    }
    
    void timerCallback() override {
        // オーディオデータ読み取りと解析
        const size_t analyzeSize = 2048;
        std::vector<float> buffer(analyzeSize);
        
        size_t available = audioInputManager_->getAvailableSamples();
        if (available >= analyzeSize) {
            size_t read = audioInputManager_->readAudioData(buffer.data(), analyzeSize);
            
            // 解析処理
            // analyzerEngine_->analyze(buffer.data(), read, ...);
        }
        
        // 統計情報更新（UI）
        auto stats = audioInputManager_->getStatistics();
        if (stats.xruns > lastXRunCount_) {
            // XRun 警告表示
            lastXRunCount_ = stats.xruns;
        }
    }

private:
    std::unique_ptr<yvc::app::AudioInputManager> audioInputManager_;
    std::unique_ptr<yvc::app::SettingsWindow> settingsWindow_;
    juce::TextButton settingsButton_;
    
    yvc::PerformanceMode performanceMode_ = yvc::PerformanceMode::Normal;
    int liveMaxSeconds_ = 3600;
    bool autoSaveEnabled_ = true;
    bool autoSaveOnStop_ = true;
    bool autoSaveOnLimit_ = true;
    uint32_t lastXRunCount_ = 0;
};
```

## パフォーマンス考慮事項

### オーディオスレッド
- **ロックフリー**: `LockFreeRingBuffer` 使用で待機なし
- **最小処理**: コールバック内は書き込みのみ
- **統計更新**: 軽量、ロックは最小限

### 解析スレッド
- **ポーリング**: `readAudioData()` で非ブロッキング読み取り
- **バッチ処理**: 十分なサンプルが溜まってから処理
- **Timer 駆動**: JUCE Timer で定期的にチェック（30-60 FPS）

### メモリ管理
- **固定バッファ**: リングバッファは事前確保
- **動的確保回避**: オーディオスレッドで `new`/`delete` なし
- **再利用**: 解析バッファは使い回し

## トラブルシューティング

### デバイスが見つからない
```cpp
// バックエンドの確認
if (yvc::audio::hasWasapiBackend()) {
    std::cout << "WASAPI available" << std::endl;
}

// デバイス列挙
auto backend = yvc::audio::createPlatformBackend();
auto devices = backend->enumerateInputDevices();
for (const auto& dev : devices) {
    std::cout << dev.name << " (ID: " << dev.id << ")" << std::endl;
}
```

### XRun 頻発
- バッファサイズを増やす（設定で変更不可の場合は OS 設定）
- パフォーマンスモードを Light に変更
- 他のアプリケーションを終了

### レイテンシが高い
- パフォーマンスモードを Light に変更
- バッファサイズを確認（OS 既定値使用）
- オーディオドライバを最新に更新

---

**実装日**: 2024-12  
**対象バージョン**: v0.1.0+  
**次のステップ**: MetricsBus 統合、FPS リミッター実装
