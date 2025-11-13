# Visual Studio Debug Configuration Guide

## Problem: "ターゲットに実行可能ファイルが欠落しています"

このエラーは、起動設定で指定されたパスにビルド済み実行ファイルが存在しない場合に発生します。

## 解決方法

### 1. ビルド設定の確認

Visual Studioで以下を確認：

1. **CMake設定の選択**
   - メニュー: `プロジェクト` → `CMakeの設定`
   - または上部ツールバーの設定ドロップダウン
   - 使用可能な設定：
     - `dev-debug` - 開発用デバッグ（ファイルログ有効）
     - `x64-Debug` - 標準デバッグ
     - `x64-Release` - リリースビルド
     - `VS2022-Debug` - Visual Studioジェネレータ使用
     - `VS2022-Release` - VSリリース

2. **CMakeキャッシュの生成**
   - メニュー: `プロジェクト` → `CMakeキャッシュの生成`
   - または `Ctrl+Shift+D` → `CMake` → `キャッシュの生成`

3. **ビルドの実行**
   - メニュー: `ビルド` → `すべてビルド` (Ctrl+Shift+B)
   - または特定ターゲットのみ: `yvc_app.exe` を選択してビルド

### 2. 起動設定の選択

`.vs/launch.vs.json`に以下の設定が含まれています：

#### yvc_offline (オフライン解析ツール)
- `yvc_offline.exe (Debug)` - デバッグビルド
- `yvc_offline.exe (Release)` - リリースビルド
- `Debug yvc_offline (Manual)` - ブレークポイント付き手動デバッグ

#### yvc_app (GUIアプリケーション)
- `yvc_app (Debug - Default Target)` - デフォルトターゲット（推奨）
- `VoiVoi Analyzer (x64-Debug)` - x64-Debug設定用
- `VoiVoi Analyzer (x64-Release)` - x64-Release設定用
- `VoiVoi Analyzer (dev-debug)` - dev-debug設定用

### 3. ビルド出力パス

各設定でのビルド出力先：

```
out/build/dev-debug/
├── yvc_app/
│   └── yvc_app_artefacts/
│       └── Debug/
│           └── VoiVoi Analyzer.exe    ← GUIアプリ
├── yvc_offline/
│   └── yvc_offline.exe                ← オフライン解析ツール
└── yvc_core/
    └── yvc_core.lib                   ← コアライブラリ

out/build/x64-Debug/
├── yvc_app/
│   └── yvc_app_artefacts/
│       └── Debug/
│           └── VoiVoi Analyzer.exe
└── ...

out/build/x64-Release/
├── yvc_app/
│   └── yvc_app_artefacts/
│       └── Release/
│           └── VoiVoi Analyzer.exe
└── ...
```

### 4. ステップバイステップ手順

#### VoiVoi Analyzer (GUIアプリ) を起動する場合：

1. CMake設定を選択（例: `dev-debug`）
2. `プロジェクト` → `CMakeキャッシュの生成`
3. `ビルド` → `すべてビルド`
4. デバッグメニューで起動設定を選択：
   - `yvc_app (Debug - Default Target)` を選択
   - または設定に応じて `VoiVoi Analyzer (dev-debug)` など
5. F5キーまたは `デバッグ` → `デバッグの開始`

#### yvc_offline (オフライン解析) を起動する場合：

1. CMake設定を選択（例: `dev-debug`）
2. `プロジェクト` → `CMakeキャッシュの生成`
3. `ビルド` → `すべてビルド`
4. デバッグメニューで `yvc_offline.exe (Debug)` を選択
5. F5キーまたは `デバッグ` → `デバッグの開始`

### 5. トラブルシューティング

#### ビルドが失敗する

```powershell
# 1. CMakeキャッシュをクリア
Remove-Item -Recurse -Force out\build\dev-debug

# 2. Visual Studioで再生成
# プロジェクト → CMakeキャッシュの生成

# 3. ビルド
# ビルド → すべてビルド
```

#### JUCEのダウンロードに失敗する

```powershell
# 手動でJUCEをクローン
git clone --depth 1 --branch 7.0.12 https://github.com/juce-framework/JUCE.git third_party/JUCE
```

#### 実行ファイルが見つからない

```powershell
# ビルド出力を確認
Get-ChildItem -Recurse out\build\dev-debug\yvc_app -Filter "*.exe"

# 起動設定のパスと一致するか確認
Get-Content .vs\launch.vs.json
```

#### 設定が表示されない

1. Visual Studioを再起動
2. `.vs`フォルダを削除して再生成
3. `launch.vs.json`を手動で編集

### 6. 推奨デバッグフロー

**開発時:**
1. 設定: `dev-debug`
2. ターゲット: `yvc_app (Debug - Default Target)`
3. ログファイル: `voivoi_app_debug.log` に自動出力

**リリーステスト:**
1. 設定: `x64-Release`
2. ターゲット: `VoiVoi Analyzer (x64-Release)`
3. ログ: コンソールのみ（ファイル出力なし）

**オフライン解析:**
1. 設定: `dev-debug`
2. ターゲット: `yvc_offline.exe (Debug)`
3. 引数: テストオーディオファイルと出力先を自動設定済み

### 7. よくある質問

**Q: "VoiVoi Analyzer.exe"と"yvc_app.exe"の違いは？**

A: 
- `VoiVoi Analyzer.exe` - JUCEがビルドする最終実行ファイル（スペース含む正式名）
- `yvc_app.exe` - CMakeターゲット名（内部参照用）

**Q: どの起動設定を使えばいい？**

A: 通常は `yvc_app (Debug - Default Target)` を使用してください。CMakeが自動的に正しいパスを解決します。

**Q: ビルドは成功するが実行できない**

A: 
1. 実行ファイルが実際に存在するか確認
2. 起動設定のビルド設定名（dev-debug, x64-Debug等）と一致しているか確認
3. セキュリティソフトがブロックしていないか確認

### 8. 参考ファイル

- `.vs/launch.vs.json` - デバッグ起動設定
- `CMakeSettings.json` - CMakeビルド設定
- `CMakeLists.txt` - ビルドスクリプト
- `yvc_app/CMakeLists.txt` - GUIアプリビルド設定
