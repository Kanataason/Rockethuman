# ロケットにんげん

C++ と DirectX 11 で作った、2.5D の物理破壊アクションです。
大砲に詰めた人間(ラグドール)を撃ち出して、ステージにある的を飛ばします。
物理演算(ラグドール、ジョイント、衝突判定)は、ライブラリを使わず、自作しています。

<!-- ここにスクリーンショットや、動画(GIF)を貼る -->

## 遊び方

| 画面 | 操作 |
|---|---|
| タイトル | W / S: 選ぶ　スペース: 決定 |
| ステージ選択 | W / S: 選ぶ　スペース: 始める　Enter / ESC: もどる |
| キャラクター選択 | W / S: 選ぶ　スペース / Enter / ESC: 閉じる |
| ステージ中 | スペース: 角度を決める → スペース: 威力を決めて発射 |
| ステージ中 | ESC: ポーズ(つづける / やりなおす / タイトルへ) |

1. 大砲の角度が、左右に揺れます。スペースで止めます。
2. 威力のゲージが、上下します(緑: よわい / 黄: ふつう / 赤: つよい)。スペースで止めると、発射します。
3. 的(箱)に当てて、点数を稼ぎます。
4. 玉がなくなるか、的をすべて飛ばすと、結果が出ます。星は、最大3つです。

### スコア
- 的に当てると得点
- 前の的から 3 秒以内に次の的に当てると、コンボ(倍率は最大 3 倍)
- 頭が的に当たると +500 点
- 飛んだ距離 1m ごとに 10 点
- 的をすべて飛ばすと、残った玉の数に応じて、クリアボーナス

## ビルド

### 必要なもの
- Windows 10 / 11(64bit)
- Visual Studio 2022(「C++ によるデスクトップ開発」と、Windows SDK)
- [vcpkg](https://github.com/microsoft/vcpkg)(Assimp を入れるために使います)

### 手順

```bash
git clone <このリポジトリのURL>
```

vcpkg を、まだ入れていなければ、入れます(`integrate install` は、1回だけです)。

```bash
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
bootstrap-vcpkg.bat
vcpkg integrate install
```

1. `3DGame.sln` を、Visual Studio で開きます。
2. 構成を **x64** の **Debug** か **Release** にして、ビルドします(Ctrl + Shift + B)。
3. 実行します(F5)。

- 最初のビルドは、Assimp などを、ソースからビルドするので、**10〜20分**かかることがあります(`vcpkg.json` から、自動で入ります)。
- DirectXTK は、NuGet から、自動で、復元されます。
- ビルドの後処理で、`Asset` フォルダと、シェーダー(`*.hlsl`)が、実行ファイルの隣に、コピーされます。

## 自作した部分

- **ラグドールの物理**:カプセルの骨、ボール / ヒンジのジョイント、Swing-Twist 分解による、関節の角度制限
- **衝突判定**:カプセル・箱・球、Octree、スリープ
- **設計**:`GameScene` と `Title` を、役割ごとにクラスへ分けて、通知(`Event`)でつなぎました
  - ステージ中:`GameAudio`(音)、`GameHud`(表示)、`PowerGauge`(ゲージ)
  - タイトル:`TitleMenuPage`、`StageSelectPage`、`CharacterSelectPage`、共通の `MenuList`
- **データ駆動**:ステージ・キャラクター・音は、JSONに追記するだけで、増やせます

### ステージ・キャラクター・音を足す

| 足すもの | やること |
|---|---|
| ステージ | `Asset/Data/StageN.json` を作って、`Asset/Data/Stages.json` に1行足す |
| キャラクター | `Asset/Data/Characters.json` に追記(FBXを `Asset/Prefab` に置く) |
| 音 | `Asset/Data/Sound.json` に追記(WAVを `Asset/Sound` に置く) |

## 使った技術

- C++20、DirectX 11
- [DirectXTK](https://github.com/microsoft/DirectXTK)(音、フォント、テクスチャ)— MIT License
- [Assimp](https://github.com/assimp/assimp)(FBX の読み込み)— BSD 3-Clause License
- [nlohmann/json](https://github.com/nlohmann/json)(ステージやキャラクターの設定)— MIT License

## 素材について

<!-- 使った素材の出どころを書く -->

- キャラクター、アニメーション:[Mixamo](https://www.mixamo.com/)
- エルビス(`debu.fbx`):(出どころを書く)
- BGM、効果音:(出どころを書く)
- フォント:(出どころを書く)

## ライセンス

<!-- 自分のコードの、ライセンスを書く。素材は、それぞれの規約に従う -->
