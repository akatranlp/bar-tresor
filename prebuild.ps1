#!/usr/bin/env powershell -File

$Folder = [Environment]::GetFolderPath("MyDocuments")

Copy-Item .\libraries\* $Folder\Arduino\libraries\ -Recurse -Force
Copy-Item .\externLibraries\* $Folder\Arduino\libraries\ -Recurse -Force
