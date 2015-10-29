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
    
    $ZipFile = Get-Content $OutputFilePath\ZipFile.txt

    if(([io.File]::Exists("$OutputFilePath\TicketID.txt")) -ne "True"){
        #show upload message
        $host.UI.WriteVerboseLine("New-FirmwareSubmission -verbose -path $OutputFilePath\$ZipFile")

        $Result = New-FirmwareSubmission -verbose -path $OutputFilePath\$ZipFile

        #get Result of cmdlet
        $TicketID = $Result.FirmwareSubmissionTicketId

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
        $host.UI.WriteVerboseLine("Initialize-FirmwareSubmission -TypeOfSubmission Image -TypeOfProduct WINDOWSPHONETHRESHOLD -UpdateHistoryPath $UpdateHistoryPath -OemInputPath $OemInputPath -OutputFilePath $OutputFilePath")

        $Result = Initialize-FirmwareSubmission -TypeOfSubmission Image -TypeOfProduct WINDOWSPHONETHRESHOLD -UpdateHistoryPath $UpdateHistoryPath -OemInputPath $OemInputPath -OutputFilePath $OutputFilePath

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
if(DownloadZip -ne "True"){
   echo "Retry."
   UploadZip
}

echo "=============End Flight Signing=================="