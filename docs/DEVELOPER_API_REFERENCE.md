# VoiVoiAnalyzer 開発者向け API リファレンス

このドキュメントは、`yvc_core` と `yvc_app` に含まれる主要インターフェースの構造と拡張ポイントをまとめたものです。インターフェースを実装するときの注意点や、実アプリケーションへの組み込み例を紹介します。

## 全体構成

- `yvc_core` (MIT): 音声解析エンジン、メトリクス配送、AI コーチ向けデータサマリを提供します。
- `yvc_app` (GPLv3): JUCE ベースの GUI。`yvc_core::MetricsBus` からメトリクスを取得し、可視化と録音制御を担当します。

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

## yvc_app との連携ポイント

- `yvc_app/src/Main.cpp`: アプリ初期化時に `MetricsBus` を生成し、`MainComponent` に渡します。
- `yvc_app/src/MainComponent.cpp`: `timerCallback` で `metricsBus_.read` を呼び出し、`MetricsDisplay` やヒートマップに値を反映します。
- `yvc_app/src/SettingsDialog.cpp`: プリセットや AI コーチ設定を GUI から変更する際のハブになります。

新しいメトリクスを追加した場合は以下も更新してください。

1. `MetricsDisplay` (`yvc_app/src/MetricsComponents.cpp`) で表示ロジックを実装する。
2. プリセット (`yvc_app/src/PresetManager.cpp`) に新指標を登録し、既存の UI にターゲットレンジを表示する。
3. 言語リソース (`yvc_app/assets/i18n`) に表示テキストを追加する。

## テストと検証

- コアライブラリのユニットテストは `yvc_core/tests/` に配置され、`cmake --build . --target yvc_core_tests` でビルド、`ctest --output-on-failure` で実行できます。
- プリプロセッサやコーチプロバイダを追加した場合は、`yvc_core/tests` に専用のテストケースを追加し、既存の `PreprocessChainTests`・`MetricsOnlyCoachProviderTests` を参考にしてください。

## 参考資料

- アーキテクチャ全体像: [docs/ARCHITECTURE.md](ARCHITECTURE.md)
- ビルド手順: [docs/BUILD.md](BUILD.md)
- ロガー API: [docs/LOGGER.md](LOGGER.md)
