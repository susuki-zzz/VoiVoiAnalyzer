# VoiVoiAnalyzer 開発者向け API リファレンス

このドキュメントは、`yvc_core` と `yvc_app` に含まれる主要インターフェースの構造と拡張ポイントをまとめたものです。インターフェースを実装するときの注意点や、実アプリケーションへの組み込み例を紹介します。

## 全体構成

- `yvc_core` (MIT): 音声解析エンジン、メトリクス配送、AI コーチ向けデータサマリを提供します。
- `yvc_app` (GPLv3): JUCE ベースの GUI。`yvc_core::MetricsBus` からメトリクスを取得し、可視化と録音制御を担当します。

プロジェクトは Windows / macOS / Linux で共通の CMake 構成を使用し、GitHub Actions 上のマトリックスビルドで同一のユニットテストと GUI シナリオテストを実行します。プラットフォーム固有コードは `yvc_core/src/audio` と `yvc_app` の JUCE モジュール設定へ集約されており、共通部分はクロスプラットフォーム CI で継続的に検証されます。

`yvc_core/include/yvc_core/AnalyzerEngine.h` に記載されている `AnalyzerEngine` が、個別のアナライザーや VAD をまとめて `MetricsBus` へ書き込みます。GUI 側では `yvc_app/src/Main.cpp` 内で `MetricsBus` を生成し、`MainComponent` に渡して UI 更新を行っています。

## IAnalyzer: 解析プラグインの追加

- 定義ファイル: `yvc_core/include/yvc_core/IAnalyzer.h`
- 目的: 新しい解析処理を追加するための抽象クラスです。

```cpp
class IAnalyzer {
public:
    virtual ~IAnalyzer() = default;
    virtual void analyze(const float* mono, size_t n, double sr, double t0, AnalysisResults& out) = 0;
    virtual const char* name() const = 0;
};
```

### 実装のポイント
1. `analyze` ではモノラルの浮動小数点サンプルを入力し、結果を `AnalysisResults` へ書き込みます。既存の構造体は `yvc_core/include/yvc_core/Types.h` を参照してください。
2. スレッドセーフティ: `AnalyzerEngine::process` はオーディオスレッドから呼び出されるため、状態を持つ場合はロックフリー構造や明示的な保護が必要です。
3. メトリクスを追加する場合は `AnalysisResults` にフィールドを追加し、GUI 側の描画コードも拡張します。

### エンジンへの統合
- 既存の `AnalyzerEngine` は内部で `F0Detector` や `CPPAnalyzer` を直接保持しています (`yvc_core/src/AnalyzerEngine.cpp`)。新しい `IAnalyzer` を組み込む場合は、エンジンにメンバーを追加し、`process` ループ内で `analyze` を呼び出し `AnalysisResults` に書き込みます。
- カスタムビルドで柔軟性を高める場合は、`std::vector<std::unique_ptr<IAnalyzer>>` を保持し、`AnalyzerEngine` 初期化時に登録する形に変更するのが推奨です。

## PerformanceModeConfig とリアルタイム制御

- 定義ファイル: `yvc_core/include/yvc_core/PerformanceMode.h`
- 役割: FFT サイズ・ホップサイズ・最大レイテンシの組み合わせをプラットフォームに依存せず切り替えます。

`AnalyzerEngine::setPerformanceMode` を呼ぶと、内部の `AudioBuffer` が再初期化され、タイムスタンプと履歴が新しいウィンドウ幅に合わせてリセットされます。`yvc_core/tests/AnalyzerEngineTests.cpp` にはモード切り替え後のタイムスタンプが正しく再初期化される回帰テスト (`AnalyzerEnginePerformanceModeTest`) が追加されており、ハイレゾ FFT を使用する macOS / Linux ビルドでも同一の振る舞いが保証されます。

パフォーマンスモードごとの推奨値:

| モード | FFT サイズ | ホップサイズ | 主な用途 |
|--------|------------|--------------|----------|
| Light | 1024 | 512 | 低遅延練習、ラップトップ収録 |
| Standard | 2048 | 512 | デフォルト。応答性と解析精度のバランス |
| Diagnostic | 4096 | 1024 | 詳細なレポートやオフライン比較 |

新しいモードを追加する場合は `PerformanceModeConfig::applyToConfig` を拡張し、GUI 側のプリセットや設定画面の説明を更新してください。

## IPreprocessor と PreprocessChain: 音声エフェクトの拡張

- インターフェース: `yvc_core/include/yvc_core/IPreprocessor.h`
- チェーン制御: `yvc_core/include/yvc_core/PreprocessChain.h`

`IPreprocessor` はモニター出力や録音パスに挿入できるエフェクトの共通インターフェースです。解析エンジンは常にドライ信号を参照するため、加工してもメトリクスには影響しません。

### 実装の流れ
1. `class MyPreprocessor : public yvc::IPreprocessor` のように派生クラスを定義します。
2. `process` でサンプルを処理し、必要に応じて `latency_samples` や `setParams` / `getParams` を実装します。
3. `PreprocessChain::addProcessor` でチェーンに登録します。ターゲットには `PreprocessTarget::Monitor` / `PreprocessTarget::Record` / 複合ビットマスクが指定できます。

```cpp
yvc::PreprocessChain chain;
chain.addProcessor(std::make_shared<MyPreprocessor>(), yvc::PreprocessTarget::Monitor);
```

`PreprocessChain` は内部でミューテックス保護された `std::vector` を使用し、`process` 呼び出し時にターゲットと一致するノードだけを順次適用します (`yvc_core/src/PreprocessChain.cpp`)。

## AudioBackend: プラットフォーム抽象化

- 定義ファイル: `yvc_core/include/yvc_core/AudioBackend.h`
- 実装例: `yvc_core/src/audio/CoreAudioBackend.cpp`、`yvc_core/src/audio/AlsaAudioBackend.cpp`

`AudioBackend` は入出力ストリームのライフサイクルを管理する純粋仮想クラスです。Windows では WASAPI バックエンドを追加予定、macOS では CoreAudio、Linux では ALSA 実装が提供されています。新しいプラットフォームを追加する際は以下の点に注意してください。

1. `AudioBackendFactory` にプラットフォーム判定を追加し、未対応環境では `nullptr` を返して GUI 側でフォールバックメッセージを表示する。
2. リアルタイムスレッドで `AnalyzerEngine::process` を呼び出すため、コールバックからは余計なアロケーションを避ける。
3. CI では Linux/macOS のスモークテストとして ALSA/CoreAudio 実装がビルドされるため、`CMakeLists.txt` に必要なリンクオプションと `find_package` の条件分岐を追加する。

`yvc_core/tests/AudioBackendTests.cpp` ではバックエンドファクトリの分岐とサンプルレート設定が回帰テストされているため、挙動変更時はテストを更新してください。

## ICoachProvider: AI コーチ連携

- インターフェース: `yvc_core/include/yvc_core/ICoachProvider.h`
- サンプル実装: `yvc_core/include/yvc_core/MetricsOnlyCoachProvider.h`

`ICoachProvider` は解析結果の集計 (`SummarySnapshot`) を受け取り、アドバイス文字列を返すサービスです。既定の `MetricsOnlyCoachProvider` は API キーの保存 (`ApiKeyVault`) と、クールダウン時間の管理のみを行うスタブ実装です (`yvc_core/src/MetricsOnlyCoachProvider.cpp`)。

### 実装手順
1. `setApiKey` で秘密情報を取り扱う場合は OS 提供のセキュアストレージを利用してください。
2. `advise` は同期呼び出しとして設計されています。外部 API を呼ぶ場合はレート制限 (`setMinIntervalMs`) を尊重し、タイムアウト処理を実装します。
3. GUI からの利用を想定する際は、レスポンスを `MetricsBus` の更新とは独立したスレッドで扱う設計が推奨です。

## MetricsBus: スレッド間メトリクス配送

- 定義: `yvc_core/include/yvc_core/MetricsBus.h`
- 機能: ダブルバッファを用いてオーディオスレッド → UI スレッド間で `AnalysisResults` を受け渡します。

```cpp
yvc::MetricsBus bus;
// オーディオスレッド側
bus.write(results);
// UI スレッド側
if (bus.read(latest)) {
    // 新しいデータが取得できた
}
```

履歴は最大 10,000 サンプルまで保持され、`getHistory` を使ってヒートマップなどのビジュアライゼーションに利用できます。

### 実装メモ

- `write` はミューテックス保護済みですが、リアルタイムスレッドでの使用を想定しているため、呼び出し回数を最小限に抑えるために `AnalyzerEngine` 側でホップサイズ単位にまとめてバッチ処理しています。
- `getHistory(max_count)` を呼ぶと末尾から最大 `max_count` 件のコピーを返します。GUI 側では 5,000 件程度に制限するとメモリ使用量を抑えられます。
- `yvc_core/tests/VADAnalyzerWindowTests.cpp` では音声活動履歴が 5 秒ウィンドウに収まることを確認し、`MetricsBus` の履歴が過剰に肥大化しないよう回帰テストを追加しています。

## yvc_app との連携ポイント

- `yvc_app/src/Main.cpp`: アプリ初期化時に `MetricsBus` を生成し、`MainComponent` に渡します。
- `yvc_app/src/MainComponent.cpp`: `timerCallback` で `metricsBus_.read` を呼び出し、`MetricsDisplay` やヒートマップに値を反映します。
- `yvc_app/src/SettingsDialog.cpp`: プリセットや AI コーチ設定を GUI から変更する際のハブになります。

新しいメトリクスを追加した場合は以下も更新してください。

1. `MetricsDisplay` (`yvc_app/src/MetricsComponents.cpp`) で表示ロジックを実装する。
2. プリセット (`yvc_app/src/PresetManager.cpp`) に新指標を登録し、既存の UI にターゲットレンジを表示する。
3. 言語リソース (`yvc_app/assets/i18n`) に表示テキストを追加する。
4. GUI シナリオテスト (`yvc_app/tests/VisualizationComponentScenarioTests.cpp`) にケースを追加し、ヒートマップやスペクトラム操作を自動検証する。

## テストと検証

- コアライブラリのユニットテストは `yvc_core/tests/` に配置され、`cmake --build . --target yvc_core_tests` でビルド、`ctest --output-on-failure` で実行できます。
- プリプロセッサやコーチプロバイダを追加した場合は、`yvc_core/tests` に専用のテストケースを追加し、既存の `PreprocessChainTests`・`MetricsOnlyCoachProviderTests` を参考にしてください。
- GUI テストは `cmake --build . --target yvc_app_gui_tests` でビルドでき、`ctest -R yvc_app_gui_scenarios` から自動化シナリオを実行します。JUCE 初期化を含むため、CI ではヘッドレス環境でも動作するよう Xvfb などの依存を追加してください。
- `cmake --build . --target check` はコア・GUI 双方のテストを一括実行します。GitHub Actions でも同ターゲットを利用しており、変更時はローカルで同じコマンドを走らせることを推奨します。

## 継続的インテグレーション

- ワークフロー定義: `.github/workflows/ci.yml`
- 対象 OS: `ubuntu-latest` / `macos-latest` / `windows-latest`

CI では次のステップを共通で実行します。

1. 必要なプラットフォーム依存パッケージ（ALSA、X11、JUCE、Ninja など）をインストールする。
2. `cmake -S . -B build -DYVC_FETCH_JUCE=ON -DBUILD_TESTING=ON` で構成し、Windows では Visual Studio 17 ジェネレーター、他プラットフォームでは Ninja を使用する。
3. `cmake --build build --config Release` でビルドし、`ctest --output-on-failure` を実行する。

CI 成果物としてバイナリを保存していないため、パッケージ生成や署名が必要な場合はローカルで `cpack` を使用してください。依存パッケージを追加した際は該当 OS のセットアップセクションへ追記し、レビュー時に環境差異が再現できるようにします。

## 参考資料

- アーキテクチャ全体像: [docs/ARCHITECTURE.md](ARCHITECTURE.md)
- ビルド手順: [docs/BUILD.md](BUILD.md)
- ロガー API: [docs/LOGGER.md](LOGGER.md)
