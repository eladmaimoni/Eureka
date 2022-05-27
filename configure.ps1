<#
.SYNOPSIS 
script for exporting all 3rd party dependencies using vcpkg

.DESCRIPTION
script for exporting rgo 3rd party dependencies using vcpkg

.PARAMETER preset
sepcifices the architecture, platform-os, compiler, and high-level compilation flags.
supported options are:
-
-
.EXAMPLE
./expoty_rgo_vcpkg_libraries.ps1 -preset "windows-x64-debug"

#>

param (
    [Parameter(Mandatory=$true)]
    [ValidateSet("windows-x64-debug", "windows-x64-release")]
    [string]
    $preset
 )


if ($null -eq $env:VCPKG_ROOT)
{    
    Write-Host "VCPKG_ROOT environment variable does not exist"
}

cmake --preset=${preset}




