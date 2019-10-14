# PDFImporter
PDFを読み込むブループリントノードを追加するプラグインです。

動作にはGhostscriptが必要です。

64bitバージョンなら設定無しで動くので64bitバージョンのインストールをお勧めします。32bitバージョンやデフォルトのインストール場所ではない場合はConvertPDFtoTexture2D関数の引数でファイルパスを指定する必要があります。

https://www.ghostscript.com/download/gsdnld.html

# 関数
- OpenPDFDialog ... ファイルブラウザからPDFファイルを選択し、そのファイルパスを返します。

- OpenPDFDialogMultiple ... ファイルブラウザから複数のPDFファイルを選択し、そのファイルパスを返します。

- ConvertPDFtoTexture2D ... PDFのファイルパスとGhostscriptのexeファイルのファイルパス、解像度を指定してPDFファイルをテクスチャに変換します。
