# PDFImporter
PDFを読み込むブループリントノードを追加するプラグインです。

動作にはGhostscriptが必要です。

https://www.ghostscript.com/download/gsdnld.html

# 関数
- OpenPDFDialog ... ファイルブラウザからPDFファイルを選択し、そのファイルパスを返します。

- OpenPDFDialogMultiple ... ファイルブラウザから複数のPDFファイルを選択し、そのファイルパスを返します。

- ConvertPDFtoTexture2D ... PDFのファイルパスとGhostscriptのexeファイルのファイルパス、解像度を指定してPDFファイルをテクスチャに変換します。
