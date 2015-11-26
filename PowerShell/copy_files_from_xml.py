import sys
import os
import  xml.dom.minidom

SourceDir = sys.argv[1]
Sourcexml = sys.argv[2] #eg:E:\project\copy_files_from_xml\UpdateHistory.xml


dom = xml.dom.minidom.parse(Sourcexml)
Packages = dom.documentElement

Package = Packages.getElementsByTagName("Package")

for Pack in Package:
    PackageFile = Pack.getElementsByTagName("PackageFile")[0]
    #print "<PackageFile>: %s" % PackageFile.childNodes[0].data
    FileName = PackageFile.childNodes[0].data
    SourceFileName = SourceDir + '\\' + FileName.split('\\')[-1]
    #print SourceFileName
    PurposeDir = FileName.split('\\')[0:-1]

    PurposeDir = '\\'.join(PurposeDir)
    PurposeDir = PurposeDir.rstrip("\\") + '\\'
    #print PurposeDir

    os.system ("xcopy /y %s %s" % (SourceFileName, PurposeDir))

