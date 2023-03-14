from __main__ import slicer

print('db dir',sys.argv[1])
print('input image dir',sys.argv[2])
# instantiate a new DICOM browser
slicer.util.selectModule("DICOM")
dicomBrowser = slicer.modules.DICOMWidget.dicomBrowser
dicomBrowser.databaseDirectory = sys.argv[1] 
# use dicomBrowser.ImportDirectoryCopy to make a copy of the files (useful for importing data from removable storage)
dicomFilesDirectory = sys.argv[2] 
dicomBrowser.importDirectory(dicomFilesDirectory, dicomBrowser.ImportDirectoryCopy)
# wait for import to finish before proceeding (optional, if removed then import runs in the background)
#dicomBrowser.waitForImportFinished()
sys.exit(0)
