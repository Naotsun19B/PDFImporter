# PDFImporter  
![7](https://user-images.githubusercontent.com/51815450/74624947-b230c280-518d-11ea-84e4-31d4769b9e53.PNG)

This plugin allows you to import PDF files as PDF assets into the editor and view them in a dedicated editor.
Also add a Blueprint node that loads the PDF file as a PDF asset at runtime.

The target platform is Windows.  
UE4 versions that have been confirmed to work are 4.20 to 4.24.

If you can not change to a version other than 4.22, please change the version using the "PDF_Plugin" project which can be downloaded from here, and copy the plug-in file to the target project.  

# License
This plugin uses the Ghostscript API to convert PDF files to PDF assets.  
Therefore, check the following about the license.  
https://www.ghostscript.com/license.html

# Functions 
## OpenPDFDialog  
![2](https://user-images.githubusercontent.com/51815450/74625151-b14c6080-518e-11ea-844f-2dd12c10d7b9.PNG)  

Select a PDF file from the file picker and return its file path.  

## OpenPDFDialogMultiple  
![8](https://user-images.githubusercontent.com/51815450/74625171-c2956d00-518e-11ea-80f0-84c8a13cca9b.PNG)  

Select multiple PDF files from the file picker and return their file paths.  

## ConvertPDFtoPDFAsset  
![3](https://user-images.githubusercontent.com/51815450/74625179-c9bc7b00-518e-11ea-8ffe-a21548591460.PNG)  

Convert PDF file to PDF asset by specifying PDF file path, resolution and page range.  

## GetPageTexture
![10](https://user-images.githubusercontent.com/51815450/74625727-a692cb00-5190-11ea-9f2a-bb328d88c60a.PNG)  

A member function of the PDF asset that gets the texture of the specified page.  
The page specification starts from 1 unlike the array.  

## GetPageCount
![11](https://user-images.githubusercontent.com/51815450/74625793-e8237600-5190-11ea-8323-b6584dde83db.PNG)

It is a member function of PDF asset to get the total number of pages.  
Resolution and page range can be obtained from variables.  

# Import and dedicated editor  
Like other assets, you can import them using the import button in the editor, or by dragging and dropping.  

![4](https://user-images.githubusercontent.com/51815450/74625309-36d01080-518f-11ea-9c0c-96b23590e091.PNG)  

As with the ConvertPDFtoPDFAsset function, you can specify the resolution and page range on the option screen during import.    

![6](https://user-images.githubusercontent.com/51815450/74625428-962e2080-518f-11ea-8230-36f4fe697bbd.PNG)

The editor screen is configured as shown in the image above. Use the left and right buttons on the toolbar to move one page back and forth. You can also enter a page in the input field next to the left and right buttons to go to the specified page.  
On the right, you can see the texture status of the page you are viewing and the status of the PDF asset.
