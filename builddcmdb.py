from __main__ import slicer

print('import',sys.argv[1])
print('db dir',sys.argv[2])
print('input image dir',sys.argv[3])
# instantiate a new DICOM browser
slicer.util.selectModule("DICOM")
dicomBrowser = slicer.modules.DICOMWidget.dicomBrowser
dicomBrowser.databaseDirectory = sys.argv[2] 
dicomFilesDirectory = sys.argv[3]
if( 'link' == sys.argv[1]):
  dicomBrowser.importDirectory(dicomFilesDirectory, dicomBrowser.ImportDirectoryAddLink)
elif( 'copy' == sys.argv[1]):
  dicomBrowser.importDirectory(dicomFilesDirectory, dicomBrowser.ImportDirectoryCopy)
# wait for import to finish before proceeding (optional, if removed then import runs in the background)
#dicomBrowser.waitForImportFinished()
sys.exit(0)
