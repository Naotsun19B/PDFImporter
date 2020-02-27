# PDFImporter  
![7](https://user-images.githubusercontent.com/51815450/74624947-b230c280-518d-11ea-84e4-31d4769b9e53.PNG)

このプラグインはPDFファイルをPDFアセットとしてエディタにインポートすることができ、専用のエディタで閲覧することができます。  
また、実行時にPDFファイルをPDFアセットとしてロードするBlueprintノードを追加します。

対象プラットフォームはWindowsです。  
動作確認済みのUE4バージョンは4.20~4.24です。

もし4.22以外のバージョンに正常に変更できない場合、ここでダウンロードできる「PDF_Plugin」のプロジェクトでバージョンを変更し、Pluginファイルを目的のプロジェクトにコピーしてみてください。

# ライセンス
このプラグインではPDFファイルからPDFアセットの変換にGhostscriptのAPIを利用させて頂きました。  
そのため、ライセンスについては以下をご確認ください。  
https://www.ghostscript.com/license.html

# 関数  
## OpenPDFDialog  
![2](https://user-images.githubusercontent.com/51815450/74625151-b14c6080-518e-11ea-844f-2dd12c10d7b9.PNG)  

ファイルピッカーからPDFファイルを選択し、そのファイルパスを返します。

## OpenPDFDialogMultiple  
![8](https://user-images.githubusercontent.com/51815450/74625171-c2956d00-518e-11ea-80f0-84c8a13cca9b.PNG)  

ファイルピッカーから複数のPDFファイルを選択し、そのファイルパスを返します。

## ConvertPDFtoPDFAsset  
![3](https://user-images.githubusercontent.com/51815450/74625179-c9bc7b00-518e-11ea-8ffe-a21548591460.PNG)  

PDFのファイルパスと解像度、ページ範囲を指定してPDFファイルをPDFアセットに変換します。

## GetPageTexture
![10](https://user-images.githubusercontent.com/51815450/74625727-a692cb00-5190-11ea-9f2a-bb328d88c60a.PNG)  

PDFアセットのメンバ関数で、指定したページのテクスチャを取得します。  
ページの指定は配列とは違い、1から始めます。

## GetPageCount
![11](https://user-images.githubusercontent.com/51815450/74625793-e8237600-5190-11ea-8323-b6584dde83db.PNG)

PDFアセットのメンバ関数で、PDFアセットの総ページ数を取得します。  
解像度とページ範囲は変数から取得できます。   

# インポートとエディタ
他のアセットと同様、エディタのインポートボタンか、ドラッグアンドドロップでインポートすることができます。 

![4](https://user-images.githubusercontent.com/51815450/74625309-36d01080-518f-11ea-9c0c-96b23590e091.PNG)  

ConvertPDFtoPDFAsset関数と同様にインポート時のオプション画面で解像度とページの範囲を指定することができます。  

![6](https://user-images.githubusercontent.com/51815450/74625428-962e2080-518f-11ea-8230-36f4fe697bbd.PNG)

エディタ画面は上の画像のような構成で、ツールバーの左右ボタンで1ページ前か後ろに捲ります。その横の入力欄にページを入力することで指定したページに移動することもできます。  
右側には閲覧しているページのテクスチャのステータスとPDFアセットのステータスを確認することができます。
