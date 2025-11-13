# Visual Studio でのクイックスタート

## 初回セットアップ

1. **Visual Studio 2019/2022** で `CMakeLists.txt` を開く
2. CMake設定を選択: `dev-debug` （推奨）
3. `プロジェクト` → `CMakeキャッシュの生成` を実行
4. `ビルド` → `すべてビルド` (Ctrl+Shift+B)

## デバッグ実行

### GUIアプリケーション (VoiVoi Analyzer)

1. デバッグターゲットで **`yvc_app (Debug - Default Target)`** を選択
2. **F5** キーを押す

### オフライン解析ツール

1. デバッグターゲットで **`yvc_offline.exe (Debug)`** を選択
2. **F5** キーを押す

## トラブルシューティング

### "実行可能ファイルが欠落しています" エラー

1. **ビルドを確認**:
   ```
   ビルド → すべてビルド (Ctrl+Shift+B)
   ```

2. **CMake設定とデバッグ設定を一致させる**:
   - CMake設定が `dev-debug` なら
   - デバッグターゲットは `VoiVoi Analyzer (dev-debug)` または `yvc_app (Debug - Default Target)`

3. **キャッシュをクリーンビルド**:
   ```
   プロジェクト → CMakeキャッシュのクリーン
   プロジェクト → CMakeキャッシュの生成
   ビルド → すべてビルド
   ```

詳細は [`docs/VISUAL_STUDIO_DEBUG.md`](docs/VISUAL_STUDIO_DEBUG.md) を参照してください。

## ビルド設定一覧

| 設定名 | 用途 | 出力先 |
|--------|------|--------|
| `dev-debug` | 開発用（ログ有効） | `out/build/dev-debug/` |
| `x64-Debug` | 標準デバッグ | `out/build/x64-Debug/` |
| `x64-Release` | リリース | `out/build/x64-Release/` |
| `VS2022-Debug` | VS生成デバッグ | `out/build/VS2022-Debug/` |
| `Core-Only` | コアライブラリのみ | `out/build/Core-Only/` |

## ドキュメント

- [ビルドガイド](docs/BUILD.md) - 詳細なビルド手順
- [Visual Studioデバッグ](docs/VISUAL_STUDIO_DEBUG.md) - デバッグ設定ガイド
- [改善サマリー](docs/IMPROVEMENTS_SUMMARY.md) - 最近の改善内容
