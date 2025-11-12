# Visual Studio Development Guide for VoiVoi Analyzer

このガイドでは、Visual Studio 2022を使用してVoiVoi Analyzerを開発する方法について説明します。

## 📋 前提条件

### 必要なソフトウェア
- **Visual Studio 2022** (Community, Professional, または Enterprise)
- **C++ CMake tools for Visual Studio** (Visual Studioインストーラーで選択)
- **MSVC v143 コンパイラツールセット** (Visual Studioインストーラーで選択)
- **Windows 10 SDK** (最新版推奨)
- **CMake 3.20+** (Visual Studioに含まれる、または個別インストール)

### 推奨するVisual Studioワークロード
Visual Studioインストーラーで以下のワークロードを選択してください：
- ✅ **C++ によるデスクトップ開発**
- ✅ **C++ CMake tools for Visual Studio**
- ✅ **MSVC v143 - VS 2022 C++ x64/x86 build tools**
- ✅ **Windows 10 SDK (最新版)**

## 🚀 プロジェクトのセットアップ

### 1. リポジトリのクローン
```bash
git clone https://github.com/susuki-zzz/VoiVoiAnalyzer.git
cd VoiVoiAnalyzer
```

### 2. Visual Studioでプロジェクトを開く
以下のいずれかの方法でプロジェクトを開きます：

#### 方法A: フォルダーとして開く
1. Visual Studio 2022を起動
2. **ファイル** → **開く** → **フォルダー** を選択
3. VoiVoiAnalyzerフォルダーを選択

#### 方法B: CMakeプロジェクトとして開く
1. Visual Studio 2022を起動
2. **ファイル** → **開く** → **CMake** を選択
3. `CMakeLists.txt` ファイルを選択

#### 方法C: コマンドラインから
```bash
# プロジェクトディレクトリで
devenv .
```

## ⚙️ ビルド構成

### 利用可能なビルド構成
Visual Studioでは以下のビルド構成が利用できます：

| 構成名 | ジェネレーター | 説明 |
|--------|---------------|------|
| **x64-Debug** | Ninja | デバッグビルド（Ninja使用） |
| **x64-Release** | Ninja | リリースビルド（Ninja使用） |
| **VS2022-Debug** | Visual Studio 17 2022 | デバッグビルド（MSBuild使用） |
| **VS2022-Release** | Visual Studio 17 2022 | リリースビルド（MSBuild使用） |
| **Core-Only** | Ninja | コアライブラリのみ |

### 構成の選択方法
1. Visual Studioのツールバーで構成ドロップダウンを確認
2. **プロジェクト** → **CMake settings** から詳細設定が可能
3. CMakePresets.jsonで定義された構成を使用

## 🔨 ビルド方法

### Visual Studio IDE内でのビルド
1. **ビルド** → **すべてをビルド** (Ctrl+Shift+B)
2. または、ソリューションエクスプローラーで個別プロジェクトを右クリック → **ビルド**

### コマンドラインでのビルド
```bash
# Visual Studio構成でビルド
build.bat --vs-release

# Ninja構成でビルド
build.bat --release

# ビルド後にVisual Studioで開く
build.bat --vs-release --open
```

### CMake直接実行
```bash
# 構成
cmake --preset=vs2022-release

# ビルド
cmake --build --preset=vs2022-release --config Release
```

## 🐛 デバッグ設定

### launch.vs.jsonの構成
プロジェクトには以下のデバッグ構成が定義されています：
- **yvc_offline.exe (Debug)** - オフラインツールのデバッグ
- **yvc_offline.exe (Release)** - オフラインツールのリリース
- **Debug yvc_offline (Manual)** - 手動デバッグ設定
- **yvc_app.exe (GUI Debug)** - GUIアプリケーションのデバッグ

### デバッグの開始方法
1. **デバッグ** → **デバッグの開始** (F5)
2. または、ツールバーの▶️ボタンをクリック
3. 構成は上部のドロップダウンから選択

### ブレークポイントの設定
- コードエディターで行番号の左をクリック
- F9キーでブレークポイントの切り替え
- **デバッグ** → **ウィンドウ** → **ブレークポイント** で管理

## 🧪 テスト実行

### Visual Studio内でのテスト
1. **テスト** → **すべてのテストを実行** 
2. **テストエクスプローラー**で個別テストの実行が可能

### コマンドラインでのテスト
```bash
# Ninja構成でのテスト
ctest --preset=core-tests

# Visual Studio構成でのテスト
ctest --preset=vs-core-tests

# ビルド + テスト
build.bat --vs-debug --test
```

## 📁 出力ファイルの場所

### ビルド出力
```
out/build/
├── VS2022-Debug/           # Visual Studio Debug ビルド
│   ├── yvc_core/
│   ├── yvc_offline/
│   └── yvc_app/
├── VS2022-Release/         # Visual Studio Release ビルド
├── x64-Debug/              # Ninja Debug ビルド
└── x64-Release/            # Ninja Release ビルド
```

### 実行ファイル
```bash
# Visual Studio ビルド
out/build/VS2022-Release/yvc_offline/Release/yvc_offline.exe
out/build/VS2022-Release/yvc_app/Release/yvc_app.exe

# Ninja ビルド  
out/build/x64-Release/yvc_offline/yvc_offline.exe
out/build/x64-Release/yvc_app/yvc_app.exe
```

## 🛠️ 高度な設定

### CMakeSettings.json カスタマイズ
```json
{
  "configurations": [
    {
      "name": "Custom-Debug",
      "generator": "Visual Studio 17 2022",
      "configurationType": "Debug",
      "variables": [
        {
          "name": "BUILD_YVC_APP",
          "value": "ON",
          "type": "BOOL"
        }
      ]
    }
  ]
}
```

### プロパティシート使用
VoiVoiCommon.propsファイルが自動的に適用され、以下の設定を提供：
- C++20標準の設定
- 適切なインクルードパス
- 最適化フラグ
- Windows固有の定義

### パフォーマンス分析
1. **デバッグ** → **パフォーマンスプロファイラー**
2. RelWithDebInfo構成でビルド
3. CPUサンプリングまたはメモリ使用量を選択

## 🔧 トラブルシューティング

### よくある問題と解決方法

#### CMakeが見つからない
```
エラー: CMake executable not found
```
**解決方法**: Visual StudioインストーラーでC++ CMake toolsを追加インストール

#### C++20機能が使用できない
```
エラー: C++20 features not available
```
**解決方法**: プロジェクト設定でC++標準を確認
- **プロジェクト** → **プロパティ** → **C/C++** → **言語**
- **C++ 言語標準** を **ISO C++20 標準 (/std:c++20)** に設定

#### ビルドエラー: 未解決の外部シンボル
```
エラー: unresolved external symbol
```
**解決方法**: 
1. 全体の再ビルド (**ビルド** → **ソリューションの再ビルド**)
2. CMakeキャッシュのクリア (**プロジェクト** → **CMakeキャッシュの削除**)

### サポートリソース
- [Visual Studio CMake ドキュメント](https://docs.microsoft.com/ja-jp/cpp/build/cmake-projects-in-visual-studio)
- [CMakePresets.json リファレンス](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html)
- プロジェクトのGitHub Issues

## 🎯 開発ワークフロー

### 推奨する開発フロー
1. **VS2022-Debug** 構成で開発
2. コード変更後、**Ctrl+Shift+B** でビルド
3. **F5** でデバッグ実行
4. テスト実行でリグレッション確認
5. **VS2022-Release** 構成で最終確認

### Git統合
Visual Studioの組み込みGit機能を使用：
- **Git** → **変更** でコミット
- **Git** → **プッシュ** でリモートに送信
- **チームエクスプローラー** でブランチ管理

---

**Happy Coding! 🎉**

Visual Studioを使用したVoiVoi Analyzerの開発をお楽しみください。
