# Logger Quick Start Guide

## 最も基本的な使い方

### 1. インクルード

```cpp
#include "yvc_core/Logger.h"
```

### 2. ログ出力

```cpp
// シンプルなログ
LOG_INFO("処理を開始しました");
LOG_DEBUG("デバッグ情報");
LOG_WARN("警告メッセージ");
LOG_ERROR("エラーが発生しました");

// 値を含むログ（printfスタイル）
int count = 42;
float value = 3.14f;
LOG_INFOF("カウント: %d, 値: %.2f", count, value);

// ストリームスタイル
LOG_STREAM(LogLevel::INFO) << "Count: " << count << ", Value: " << value;
```

### 3. 設定（オプション）

```cpp
auto& logger = Logger::getInstance();

LoggerConfig config;
config.minLevel = LogLevel::DEBUG;  // DEBUGレベル以上を表示
config.enableFile = true;           // ファイル出力を有効化
config.logFilePath = "my_app.log";  // ログファイル名

logger.configure(config);
```

### 4. パフォーマンス計測

```cpp
void 重い処理() {
    LOG_SCOPE_TIMER("重い処理");
    
    // 処理内容...
    // スコープを抜ける時に自動的に実行時間をログ出力
}
```

## 実用例

### アプリケーション起動時

```cpp
int main() {
    auto& logger = Logger::getInstance();
    
    LoggerConfig config;
    config.minLevel = LogLevel::INFO;
    config.enableConsole = true;
    config.enableFile = true;
    config.logFilePath = "voivoi.log";
    
    logger.configure(config);
    
    LOG_INFO("アプリケーション開始");
    
    // メイン処理...
    
    LOG_INFO("アプリケーション終了");
    logger.shutdown();
    
    return 0;
}
```

### エラーハンドリング

```cpp
try {
    処理();
} catch (const std::exception& e) {
    LOG_ERRORF("例外が発生: %s", e.what());
    // エラー処理...
}
```

### 音声処理でのログ（プライバシー保護）

```cpp
// ❌NG: 音声データを直接ログに出力してはいけない
// LOG_DEBUG(audioBuffer);  // 絶対にしないこと！

// ✅OK: メトリクスのみをログ出力
float f0 = 220.5f;
float rms = -12.3f;
LOG_INFOF("メトリクス - F0: %.1f Hz, RMS: %.1f dB", f0, rms);

// ✅OK: メタデータのログ出力
LOG_DEBUGF("バッファ処理: %zu サンプル @ %.1f kHz", 
           bufferSize, sampleRate / 1000.0);
```

## デバッグビルドとリリースビルドの違い

### デバッグビルド
```cpp
#ifndef NDEBUG
    // 詳細なログを有効化
    config.minLevel = LogLevel::DEBUG;
    config.enableFile = true;
#endif
```

### リリースビルド
```cpp
#ifdef NDEBUG
    // 最小限のログのみ
    config.minLevel = LogLevel::WARN;
    config.enableFile = false;
#endif
```

## ログレベルの使い分け

| レベル | 用途 | 例 |
|--------|------|-----|
| TRACE | 非常に詳細なデバッグ | サンプルごとの処理内容 |
| DEBUG | 開発時のデバッグ情報 | 関数の呼び出し、変数の値 |
| INFO | 一般的な実行情報 | 処理の開始/終了、状態変化 |
| WARN | 警告（処理は継続） | バッファアンダーラン検出 |
| ERROR | エラー（機能に影響） | ファイルオープン失敗 |
| FATAL | 致命的エラー | メモリ不足 |

## よくある使い方パターン

### パターン1: 関数の開始/終了をログ

```cpp
void myFunction(int param) {
    LOG_DEBUGF("myFunction開始: param=%d", param);
    
    // 処理...
    
    LOG_DEBUG("myFunction終了");
}
```

### パターン2: スコープタイマーで性能測定

```cpp
void processAudio() {
    LOG_SCOPE_TIMER("音声処理");
    
    // 音声処理コード
    // 自動的に処理時間が記録される
}
```

### パターン3: 条件付きログ

```cpp
if (error) {
    LOG_ERROR("エラーが発生しました");
} else {
    LOG_DEBUG("正常に処理されました");
}
```

### パターン4: マルチスレッドでのログ

```cpp
void workerThread(int id) {
    LOG_INFOF("ワーカー %d 開始", id);
    
    // スレッドIDは自動的に記録される
    for (int i = 0; i < 10; ++i) {
        LOG_DEBUGF("ワーカー %d: アイテム %d 処理中", id, i);
    }
    
    LOG_INFOF("ワーカー %d 終了", id);
}

std::thread t1(workerThread, 1);
std::thread t2(workerThread, 2);
```

## トラブルシューティング

### Q: ログが出力されない

**A:** ログレベルを確認してください

```cpp
// 現在の設定を確認
auto& logger = Logger::getInstance();
logger.setMinLevel(LogLevel::DEBUG);  // DEBUGレベルまで表示
```

### Q: ファイルが作成されない

**A:** パスとファイル出力設定を確認

```cpp
LoggerConfig config;
config.enableFile = true;
config.logFilePath = "C:/Temp/myapp.log";  // 絶対パスを使用
logger.configure(config);
```

### Q: パフォーマンスへの影響が心配

**A:** リリースビルドではログレベルを上げる

```cpp
#ifdef NDEBUG
    logger.setMinLevel(LogLevel::WARN);  // WARNとERRORのみ
#endif
```

## さらに詳しく

詳細なドキュメントは [docs/LOGGER.md](../docs/LOGGER.md) を参照してください。
