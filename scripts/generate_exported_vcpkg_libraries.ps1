# powershell script
# Set-ExecutionPolicy remotesigned -Force
# Set-ExecutionPolicy unrestricted -Force

# add an envionment variable
Write-Host "script to generate all vcpkg export libraries"
$env:ZZZ123_ENV = 'ZZZ123_VALUE'
#[System.Environment]::SetEnvironmentVariable('ZZZ_ENV','ZZZ_VALUE')
'hello'

#$current = $pwd

if ($null -eq $env:EXTERNALS_PATH)
{
    $exported_libraries_root_path = Join-Path $pwd ".."
    
    Write-Host "EXTERNALS_PATH environment variable does not exist"
    
    [Environment]::SetEnvironmentVariable('EXTERNALS_PATH','C:/Libraries/', 'User')
}
else
{
    Write-Host "zzz exists"
    $exported_libraries_root_path = $env:EXTERNALS_PATH
}

#[string]::Format(“installing libraries to {0}”,$exported_libraries_root_path)
Write-Host “installing libraries to $exported_libraries_root_path”
 
# export command can't be called from this direcory because it conflicts with the manifest file 

Push-Location -Path $exported_libraries_root_path
#cd $exported_libraries_root_path

$lib_target_path = Join-Path $exported_libraries_root_path "exported_libraries"
vcpkg export fmt:x64-windows-static-md --output-dir=$exported_libraries_root_path --output=exported_packages --raw --zip

Pop-Location

# "VCPKG_CHAINLOAD_TOOLCHAIN_FILE":  "C:/Libraries/exported_packages/scripts/buildsystems/vcpkg.cmake",