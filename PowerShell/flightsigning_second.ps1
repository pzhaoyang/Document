#Define some golbal varians
[String]$Global:RetailPath         = "NULL"
[String]$Global:UpdateHistoryPath  = "NULL"
[String]$Global:OemInputPath       = "NULL"
[String]$Global:OutputFilePath     = "NULL"
[String]$Global:SignedFilePath     = "NULL"

function CheckParam($InputParam) {
    if($InputParam.Count -ne 1){
       $host.UI.WriteErrorLine("Please Input ICD Project Path!")
       exit
    }

    $Global:RetailPath = $InputParam

    if(([io.Directory]::Exists($RetailPath)) -ne "True"){
      $host.UI.WriteErrorLine("The parameter is not a correct directory!")
      exit
    }

    if((([io.File]::Exists("$RetailPath\Flash.FFU")) -ne "True" ) -or (([io.File]::Exists("$RetailPath\Flash.cat")) -ne "True" ) -or (([io.File]::Exists("$RetailPath\Flash.UpdateHistory.xml")) -ne "True") -or (([io.Directory]::Exists("$RetailPath\ProcessedFiles")) -ne "True")){
        $host.UI.WriteErrorLine("This is a invalid directory!")
        exit
    }

    if(([io.File]::Exists("$RetailPath\UpdateHistory.xml")) -ne "True"){
      $DROP = copy $RetailPath\Flash.UpdateHistory.xml $RetailPath\UpdateHistory.xml
      $host.UI.WriteVerboseLine("copy $RetailPath\Flash.UpdateHistory.xml to $RetailPath\UpdateHistory.xml")
    }

    $Global:UpdateHistoryPath     =  $RetailPath
    $Global:OemInputPath          =  $RetailPath + "\ProcessedFiles"
    $Global:OutputFilePath        =  $RetailPath + "\OutputZip"
    $Global:SignedFilePath        =  $RetailPath + "\SignedZip"
}

function DownloadZip {
    if(([io.File]::Exists("$OutputFilePath\TicketID.txt")) -ne "True"){
        $host.UI.WriteErrorLine("TicketID.txt Can not found!")
        return "Flase"
    }

    [String]$TicketID = Get-Content $OutputFilePath\TicketID.txt
    if($TicketID.Equals("")){
        $host.UI.WriteErrorLine("TicketID.txt Is Empty")
        Remove-Item $OutputFilePath\TicketID.txt
        return "Flase"
    }

    #Create a directory if it's not exist
    if(([io.Directory]::Exists("$RetailPath\SignedZip")) -ne "True"){
        $Drop = New-Item "$RetailPath\SignedZip" -type directory -force
    }

    #show Download message.
    $host.UI.WriteVerboseLine("Get-SignedFirmwareSubmission -verbose -FirmwareSubmissionTicketId $TicketID -DownloadDirectory $SignedFilePath")

    $Result = Get-SignedFirmwareSubmission -verbose -FirmwareSubmissionTicketId $TicketID -DownloadDirectory $RetailPath\SignedZip
    
    #Save Zip File 
    [String]$ZipName = $Result.File.Name
    if($ZipName.Equals("")){
        exit
    }
    #show Downloaded ZipFile
    $host.UI.WriteVerboseLine("Zip File: $ZipName")
    
    #Save file name to file.
    echo $ZipName >> $SignedFilePath\ZipFile.txt

    return "True"
}

function UploadZip {

    if(([io.File]::Exists("$OutputFilePath\TicketID.txt")) -ne "True"){

        $ZipFile = Get-Content $OutputFilePath\ZipFile.txt

        #Check ZipFile is valid
        if($ZipFile.Equals("")){
            $host.UI.WriteErrorLine("ZipFile Content is Empty")
            exit
        }

        #show upload message
        $host.UI.WriteVerboseLine("New-FirmwareSubmission -verbose -path $OutputFilePath\$ZipFile")

        $Result = New-FirmwareSubmission -verbose -path $OutputFilePath\$ZipFile

        #get Result of cmdlet
        $TicketID = $Result.FirmwareSubmissionTicketId

        #Check TicketID is valid
        if($TicketID.Equals("")){
            exit
        }

        #show save tip
        $host.UI.WriteVerboseLine("Save TicketID: $TicketID")

        #Save Ticket to file
        echo $TicketID >> $OutputFilePath\TicketID.txt
    }
}


function PacketSPKGSZip {
    if(([io.Directory]::Exists($OutputFilePath)) -eq "True"){
        if(([io.File]::Exists($OutputFilePath+"\ZipFile.txt")) -ne "True"){
           $host.ui.WriteErrorLine("Saved ZipFileName Can Not Be Found!")
           exit
        }
    }else{
        $host.UI.WriteVerboseLine("Initialize-FirmwareSubmission -TypeOfSubmission FFUCATALOG -TypeOfProduct WINDOWSPHONETHRESHOLD -FfuPath $RetailPath\Flash.FFU -UpdateHistoryPath $UpdateHistoryPath -OemInputPath $OemInputPath -OutputFilePath $OutputFilePath")

        $Result = Initialize-FirmwareSubmission -TypeOfSubmission FFUCATALOG -TypeOfProduct WINDOWSPHONETHRESHOLD -FfuPath $RetailPath\Flash.FFU -UpdateHistoryPath $UpdateHistoryPath -OemInputPath $OemInputPath -OutputFilePath $OutputFilePath

        #get cmdlet result
        [String]$ZipFile = $Result.name
    
        if($ZipFile.Equals("")){
          exit
        }
        #show Packeted ZipFile name
        $host.UI.WriteVerboseLine("Zip File: $ZipFile")
    
        #Save file name to file.
        echo $ZipFile >> $OutputFilePath\ZipFile.txt
    }
}
function Unzip {
    if([io.file]::Exists("$SignedFilePath\ZipFile.txt") -ne "True"){
        return "False"
    }

    [String]$ZipFile = Get-Content "$SignedFilePath\ZipFile.txt"
    if($ZipFile.Equals("")){
        return "False"
    }
    [String]$BaseName = Get-Item "$SignedFilePath\$ZipFile" |%{$_.BaseName}

    if($BaseName.Equals("")){
        return "False"
    }
    
    if([io.Directory]::Exists("$SignedFilePath\$BaseName")){
        $host.ui.WriteWarningLine("Zip File has Unziped")
        return "True"
    }
    [String]$drop = unzip "$SignedFilePath\$ZipFile" "$SignedFilePath\$BaseName"

    if($drop.Equals("")){
        exit
        #return "False"
    }

    return "True"
}
function CopySPKGS {
    $xmldata = [xml](Get-Content "$RetailPath\UpdateHistory.xml")
    $doc = $xmldata.DocumentElement.GetElementsByTagName("Package")
    $filefolder = Get-ChildItem $SrcDir -recurse |%{$_.DirectoryName}
    
    if((Unzip) -eq "False"){
        return "False"
    }
    
    foreach($i in $doc){
        [String]$SpkgPath = $i.PackageFile
        $Name = $SpkgPath.Substring($SpkgPath.LastIndexOf("\")+1,$SpkgPath.Length - $SpkgPath.LastIndexOf("\") - 1)
        $DestDir = $SpkgPath.Substring(0,$SpkgPath.LastIndexOf("\"))
        
        foreach($f in $filefolder){
            if(([io.File]::Exists("$f\$Name")) -eq "True"){

                if(([io.Directory]::Exists("$DestDir")) -ne "True"){
                    $drop = New-Item -type directory $DestDir
                }

                Copy-Item -Path "$f\$Name" -Destination $DestDir -Verbose -Recurse -Force
                break
            }
        }
    }

    return "True"
}

function CreateFFU ($ffutype){
    Invoke-Expression -Command "$env:BSPROOT\create_ffu.bat $ffutype"
}

function BuildSubSystem {
    $bspdir = $env:BSPROOT + "\..\..\..\"
    if(([io.File]::Exists("$bspdir\SubSystemBuiltFlag")) -ne "True"){
        Copy-Item "$bspdir\int_tools\build-wp8909-1074.cmd" "$bspdir" -Force
        $drop = New-Item  -type File "$bspdir\SubSystemBuiltFlag" -Value "Don't Delete this File, unless you will build subsystem!"
        Invoke-Expression -Command "$bspdir\build-wp8909-1074.cmd -ALL"
    }
}

echo "=============Start Build SubSystem================="
#Build SubSystem
#BuildSubSystem
#end BuildSystem


echo "=============Start Build FFU First================="
#Build FFU First
#CreateFFU retail


echo "=============Start Flight Signing=================="

#Check Input Param
CheckParam $args
echo "end CheckParam"

#Packet SPKGS to Zip
PacketSPKGSZip
echo "end PacketSPKGSZip"

#Upload Packeted Zip to MSFT
UploadZip
echo "end UploadZip"

#Download Packeted Zip from MSFT
while((DownloadZip) -ne "True"){
   $host.ui.WriteWarningLine("******************* Download Failed 5 Seconds Later Retry UploadZip **************")
   Start-Sleep -Seconds 5
   UploadZip
}
#Copy Signed spkg to project
#CopySPKGS
echo "end CopySPKGS"

#Build FFU Second
#CreateFFU retail

echo "=============End Flight Signing=================="