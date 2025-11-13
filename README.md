# VoiVoiAnalyzer

VoiVoiAnalyzer は、会話練習のためのリアルタイム音声トレーニングアプリケーションです (ローカルファースト)。

**対応プラットフォーム**: まずは Windows (JUCE 7) を優先し、将来的に Android / iOS への展開を予定しています。
**開発スタック**: C++20、CMake、JUCE 7、KissFFT (MIT)、Eigen (任意)
**プライバシー**: 音声データはデバイス外へ送信しません。オプションの AI コーチもメトリクスのみを共有します。

## 注意点
本プロジェクトはとりあえずAIにコーディングさせております。しばらくとっ散らかったままであることをご了承ください。

## 概要

VoiVoiAnalyzer は、厳格なプライバシー方針と高いリアルタイム性能を両立した音声解析機能を提供します。

### 音声メトリクス
- **F0 検出**: 基本周波数（ピッチ）を追跡します。
- **レベル解析**: RMS、ピーク、クレストファクタを測定します。
- **CPP**: 音声の明瞭度を示すケプストラムピークプロミネンスを算出します。
- **HNR**: 有声音の倍音成分を評価するハーモニクス対ノイズ比です。
- **スペクトルチルト**: 周波数バランスを分析します。
- **/s/ セントロイド**: 無声子音のエネルギーバランスを追跡します。
- **VAD**: 音声区間検出と発話速度・ポーズ比率の算出を行います。

### 可視化
- リアルタイムメトリクス表示
- ヒートマップ可視化
- 代表的なトレーニング用途向けプリセット管理

### パフォーマンスモード

UI 往復レイテンシを保証した 3 つのモードを備えています。

| モード | レイテンシ | FFT サイズ | ホップサイズ | 主な用途 |
|--------|------------|------------|--------------|----------|
| **ライト** | 40 ms 以下 | 1024 | 512 | 最小レイテンシでのリアルタイム練習 |
| **ノーマル** | 60 ms 以下 | 2048 | 512 | 応答性と詳細度のバランス |
| **ダイアグノスティック** | 80 ms 以下 | 4096 | 1024 | 詳細な評価やレポート作成 |

**自動段階的負荷軽減**: 描画 FPS を 60 → 45 → 30 Hz の順に下げ、オーディオ処理パラメータは自動変更しません。

### プライバシーアーキテクチャ

- **100% ローカル処理**: 音声をネットワーク送信しません。
- **RAM 専用ライブモード**: ライブ解析データは RAM のみで保持します。
- **ドライ信号解析**: プリプロセス前の信号で解析を行います。
- **前処理チェーン分離**: エフェクトチェーンはモニター／録音にのみ影響し、解析メーターには影響しません。
- **手動保存制御**: 手動保存ボタンと停止時の自動保存（任意）を提供します。
- **AI コーチ（任意）**: 有効化した場合でもメトリクスのみ送信し、生の音声は共有しません。
  - 既定では無効
  - API キーは安全に保存（Windows では DPAPI）
  - アドバイス要求にはレート制限を適用

詳細は [Privacy Policy](docs/PRIVACY.md) を参照してください。

## ドキュメント

- [ユーザー操作ガイド](docs/USER_GUIDE.md): 主要機能やトラブルシューティングをまとめたユーザー向け資料。
- [API リファレンス](docs/DEVELOPER_API_REFERENCE.md): yvc_core / yvc_app の主要インターフェース拡張方法を解説。
- [ARCHITECTURE](docs/ARCHITECTURE.md): 全体アーキテクチャとデータフローの説明。
- [BUILD](docs/BUILD.md): ビルド要件とプラットフォーム別の注意事項。
- [VISUAL_STUDIO_GUIDE](docs/VISUAL_STUDIO_GUIDE.md): Windows / Visual Studio 向けセットアップ手順。

## 達成すべき条件（ハード要件）

作者旧環境を基に仮設定。
基準ハードウェア（Ryzen 5 3600XT + RTX 2060 相当）での性能目標:

- **1 時間連続稼働**: XRuns = 0、p95 レイテンシ 60 ms 以下（ノーマルモード）
- **CPU 使用率**: 12% 未満で安定
- **RAM 使用量**: 1 時間録音で 1.0 GB 以下
- **サンプルレート**: 初回起動時は 48 kHz を自動設定、非対応の場合はデバイス SR（設定画面から変更可）
- **バッファサイズ**: OS 既定値（Windows=256、Android=バースト×2、iOS=128）で固定
- **ライブ時の RAM 保存**: 手動／自動保存が起動するまで永続ストレージに書き込みません

## アーキテクチャ

```
Audio I/O (WASAPI) → RAM RingBuffer (PCM float32 mono)
                                    ↓
                              Analyzer (dry signal)
                                    ↓
                              MetricsBus (double-buffer)
                                    ↓
                                   UI

                     ↘ PreprocessChain (monitor/record paths only)
```

VoiVoiAnalyzer は以下の 3 コンポーネントで構成されます。

1. **yvc_core** (MIT License): コア音声解析ライブラリ
   - Audio I/O（Windows では WASAPI）
   - ロックフリーリングバッファ（単一プロデューサ／コンシューマ）
   - 解析エンジン: F0、RMS / Peak / Crest、CPP、HNR、スペクトルチルト、/s/ セントロイド、VAD
   - MetricsBus（ダブルバッファ、スレッドセーフ）
   - 前処理インターフェース（IPreprocessor）
   - AI コーチインターフェース（ICoachProvider: メトリクスのみ）
   - 設定およびプリセット管理
   - ユーティリティ（FTZ / DAZ、ウィンドウ関数、NaN ガード）

2. **yvc_app** (GPLv3): GUI アプリケーション
   - JUCE フレームワークで構築
   - リアルタイム可視化:
     - 目標帯域オーバーレイ付き F0 ゲージ
     - CPP、HNR、スペクトルチルトのメーター
     - 発話速度とポーズ解析
     - ミニ波形表示
     - 時間×F0 および時間×RMS ヒートマップ
   - ステータスバー: `FPS | CPU | RAM | Recording Time Left`
   - 4 固定プリセット（MVP）: Natural Conversation、Phone Training、Resonance Focus、Diagnostic
   - 設定: サンプルレート、バッファサイズ、ライブ最大時間、自動保存トグル、前処理チェーン
   - 保存ダイアログ: WAV（32-bit float）+ メトリクス（CSV / Parquet）+ session.json
   - FPS リミッターと自動段階的負荷軽減（60→45→30 Hz）

3. **yvc_offline**: オフライン解析ツール
   - 最長 3 時間の音声ファイルを処理
   - 30 秒チャンクを 1 秒オーバーラップで解析
   - 出力: メトリクス CSV、サマリー JSON、異常検出 JSON、ヒートマップ CSV
   - リアルタイム版と同一の解析コードパスを使用（再現性確保）
   - ヒートマップ生成と異常ハイライト

## 主要インターフェース

### IAnalyzer
```cpp
class IAnalyzer {
public:
    virtual void analyze(const float* mono, size_t n, double sr,
                        double t0, AnalysisResults& out) = 0;
};
```

### IPreprocessor
```cpp
class IPreprocessor {
public:
    virtual void process(const float* in, float* out, size_t n) = 0;
    virtual int latency_samples() const { return 0; }
    virtual void setParams(const std::unordered_map<std::string, float>& kv) = 0;
};
```

### ICoachProvider
```cpp
struct SummarySnapshot { /* 集計済みメトリクスのみ - 音声データなし */ };

class ICoachProvider {
public:
    virtual void setApiKey(const std::string& key) = 0;
    virtual std::string advise(const SummarySnapshot& snapshot) = 0;
};
```

## プリセット（MVP: 固定）

4 種類の組み込みプリセットを用途別に最適化しています。

1. **Natural Conversation**: 日常会話向けのバランス設定。
   - メーター: F0 ゲージ、CPP、HNR、スペクトルチルト、発話速度、ポーズ
   - 目標 F0: 100–160 Hz / 170–230 Hz

2. **Phone Training**: 電話／VoIP での明瞭度向上を重視。
   - メーター: F0 ゲージ、タイミング、明瞭度、スペクトルチルト
   - 300–3400 Hz 帯域を重点評価

3. **Resonance Focus**: 共鳴バランスと声の響きに着目。
   - メーター: スペクトルチルト、/s/ セントロイド、F0 ゲージ、CPP

4. **Diagnostic**: 詳細評価やレポート作成向け。
   - メーター: F0、RMS、ピーク、クレスト、CPP、HNR、チルト、VAD などすべて
   - 倍音まで含むフルスペクトル表示

※ 将来的にユーザー定義プリセットやレイアウトエディターを提供予定です。

## 技術スタック

- **C++20**: 最新の C++ 機能を活用
- **CMake**: クロスプラットフォームビルドシステム
- **JUCE 7**: プロ向けオーディオアプリケーションフレームワーク（GUI）
- **KissFFT**: 軽量高速な FFT ライブラリ
- **Eigen**（任意）: 行列演算ライブラリ

## ビルド

### 要件
- CMake 3.20 以上
- C++20 対応コンパイラ（MSVC 2019+、GCC 10+、Clang 12+）
- Windows 10 / 11（優先ターゲットプラットフォーム）

### ビルド手順

```bash
# リポジトリを取得
git clone https://github.com/susuki-zzz/VoiVoiAnalyzer.git
cd VoiVoiAnalyzer

# ビルド用ディレクトリを作成
mkdir build
cd build

# 構成を実行
cmake ..

# ビルドを実行
cmake --build . --config Release

# コアライブラリのみビルドしたい場合
cmake .. -DBUILD_YVC_APP=OFF -DBUILD_YVC_OFFLINE=OFF

# Eigen サポートを有効化して構成する場合
cmake .. -DUSE_EIGEN=ON
```

#### GUI（`yvc_app`）のビルド

GUI は [JUCE](https://juce.com/) に依存しており、`BUILD_YVC_APP=ON`（既定）で有効になります。JUCE を CMake に知らせるには次のいずれかの方法を取ります。

1. **リポジトリ同梱**: `third_party/JUCE` にサブモジュールとして追加します。
2. **外部ソースツリー**: `-DYVC_JUCE_PATH="/path/to/JUCE"` を渡すか、環境変数 `JUCE_DIR` を設定します。
3. **自動取得**: `-DYVC_FETCH_JUCE=ON` を指定して構成し、ビルド時に `FetchContent` で JUCE をダウンロードします。

JUCE を自動取得して GUI 実行ファイルをビルドする例:

```bash
cmake -S .. -B build/gui -DYVC_FETCH_JUCE=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build/gui --target yvc_app --config Release
```

単一構成ジェネレータ（Ninja や Unix Makefiles など）の場合は、ビルドコマンドで `--config Release` を省略します。

## メンテナンス

- **依存関係の更新**: サブモジュールや `third_party/` を更新した際は `cmake --build .` を再実行し、警告の有無を確認します。
- **自動テスト**: コアライブラリを変更したら `cmake --build . --target yvc_core_tests` を実行し、続けて `ctest --output-on-failure` でユニットテストを確認します。
- **メトリクス整合性チェック**: `yvc_offline` で既知のサンプルを解析し、`docs/IMPLEMENTATION_SUMMARY.md` のベンチマークと比較します。
- **ドキュメント更新**: ユーザー体験や API に影響する変更があれば `docs/USER_GUIDE.md` と `docs/DEVELOPER_API_REFERENCE.md` を更新してからリリースします。

### ビルド成果物
- `yvc_core.lib`: コア解析ライブラリ
- `yvc_app.exe`: GUI アプリケーション（JUCE 利用時）
- `yvc_offline.exe`: オフライン解析ツール

## オーディオ設定

- **既定サンプルレート**: 初回起動時に 48 kHz を自動選択
- **バッファサイズ**: OS 既定値を使用（低レイテンシを維持）
- **チャンネル**: モノラル解析
- **サンプルレート**: 設定画面で 44.1 kHz、48 kHz などに変更可能

## パフォーマンス

オーディオ処理性能を確保するため、必要に応じて描画 FPS を段階的に低下させます。
- 目標: 60 FPS
- 軽減時: 45 FPS
- 最低: 30 FPS

オーディオ処理レイテンシは常にモードごとの目標値内に収まるよう管理されます。

## ロガーの主な特徴

### 1. スレッドセーフ
- 複数スレッドから同時にログ出力可能
- 低競合なロックベース同期
- 適切な構成によりリアルタイムオーディオスレッドでも安全

### 2. 性能重視
- ログを無効化した際のオーバーヘッドを最小化
- コンパイル時のログレベルフィルタリング
- ファイル出力向けのバッファリングを選択可能
- 自動ローテーション対応

### 3. プライバシー重視
- **本番ビルドでは音声データをログに記録しません**
- メトリクスやメタデータのみをログ化
- プライバシー設定をカスタマイズ可能
- VoiVoi のプライバシーアーキテクチャに準拠

### 4. 柔軟な構成
- TRACE / DEBUG / INFO / WARN / ERROR / FATAL の複数レベル
- コンソールおよびファイル出力をサポート
- タイムスタンプ・スレッド ID・ソース位置を含む書式を調整可能
- 実行時に設定変更が可能

### 5. 開発者フレンドリー
- マクロベース API により簡潔に利用可能
- ストリームスタイルのログ記述をサポート
- パフォーマンス測定用のスコープタイマーを提供
- printf 形式のフォーマットに対応

詳細な利用方法は [Logger Documentation](docs/LOGGER.md) を参照してください。

## ライセンス

- **yvc_core**: MIT License（制限の少ないライセンス）
- **yvc_app**: GPLv3（JUCE フレームワークの OSS ライセンスに基づく）
- **yvc_offline**: GPLv3

詳細は [LICENSE](LICENSE) を参照してください。

## コントリビュート

↓未確認。まとめていません。
メンテナンスフローやコードスタイルは [CONTRIBUTING.md](CONTRIBUTING.md) にまとめています。Pull Request を送る前に以下を確認してください。

1. 作業ブランチに最新の `main` / `develop` を取り込み、ビルドとテストを通過させる。
2. 変更内容に合わせてドキュメントを更新し、`docs/` 配下のガイドやリンクが最新であることを確認する。
3. コミットメッセージと PR 概要に目的と検証方法を明記する。

## ロードマップ

### JUCE GUI の完成度向上
- [ ] ダッシュボードビュー（F0 ゲージ、ヒートマップ、ステータスバー）のレイアウトを確定
- [ ] MetricsBus の更新を JUCE コンポーネントへダブルバッファで受け渡す
- [ ] プリセットセレクタ実装
- [ ] 設定ダイアログ実装
- [ ] FPS リミッター表示と自動負荷軽減時のメッセージフックを追加

### オーディオファイル入出力（オフラインツール）
- [x] 30 秒チャンク＋ 1 秒オーバーラップのバッチ処理パイプライン
- [x] メトリクス CSV とセッションサマリー JSON のシリアライズ
- [x] リアルタイムアナライザーと揃えたヒートマップ／異常レポート生成
- [ ] WASAPI以外のオーディオバックエンド

### 高度な可視化オプション
- [ ] ヒートマップ操作（ズーム、時間範囲スクラブ、解像度切り替え）の拡張
- [ ] メトリクスパネルのセッション比較表示
- [ ] 注釈付きスナップショット画像のエクスポート機能

### 追加音声メトリクス
- [ ] フォルマント追跡(F1, F2, F3)

### プリセット共有（メトリクスのみ）
- [ ] 共有可能なプリセットスキーマ（メトリクス、目標、レイアウトメタデータ）を定義
- [ ] インポート／エクスポートダイアログ付きローカルプリセットライブラリを実装
- [ ] 生の音声や個人情報が含まれないよう検証機構を追加

### 多言語対応
- [ ] UTF-8 リソースバンドルによる UI 文字列の外部化
- [ ] MVP フロー向け日本語＋英語翻訳を提供
- [ ] 永続化付きランタイム言語切り替えを追加

### Android / iOS 対応
- [ ] JUCE モバイルプロジェクトテンプレートを統合
- [ ] タッチ操作向け UI 最適化

### 歌声分析機能の追加
- [ ] 音域測定
- [ ] ピッチ安定度
- [ ] ビブラート検出
- [ ] 発声持続時間

### macOS / Linux 対応
- [x] CoreAudio / ALSA をサポートするようオーディオバックエンドを抽象化
- [ ] プラットフォーム別ビルドプリセットと CI スモークビルドを統合
- [ ] 代表的なハードウェアで性能目標を検証

| プラットフォーム | バックエンド | 状態 | 備考 |
|------------------|--------------|------|------|
| macOS 14.4 | CoreAudio | ✅ | AudioQueue コールバックによる既定入力のスモークテストを確認済み |
| Ubuntu 22.04 | ALSA | ✅ | 既定 PCM デバイスの float32 キャプチャループを自動テストハーネスで確認 |

## サポート

不具合や質問はこちらへ:
- GitHub Issues: https://github.com/susuki-zzz/VoiVoiAnalyzer/issues
- プライバシーに関する問い合わせ: [Privacy Policy](docs/PRIVACY.md)

---

**注意**: 本ソフトウェアはボイストレーニング用途です。声帯の健康に関する専門的な助言が必要な場合は、言語聴覚士やボイストレーナーなどの専門家に相談してください。
