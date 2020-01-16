# PDFImporter
PDFを読み込むブループリントノードとPDFアセットを追加するプラグインです。  
PDFファイルをエディタにインポートすることもできます。  

開発バージョンはUE4.22。  

対象プラットフォームはWindowsですが、GhostscriptのLinux用の動的リンクライブラリを用意し、FGhostscriptCoreのコンストラクタでロードするように変更するとLinuxでも使えるかもしれません。

# 関数
- OpenPDFDialog ... ファイルブラウザからPDFファイルを選択し、そのファイルパスを返します。

- OpenPDFDialogMultiple ... ファイルブラウザから複数のPDFファイルを選択し、そのファイルパスを返します。

- ConvertPDFtoPDFAsset ... PDFのファイルパスと解像度、ページ範囲を指定してPDFファイルをPDFアセットに変換します。また、ファイル名に応じてロケールを設定する必要があります。(例 : 日本語 -> ja)
