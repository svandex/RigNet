[CmdletBinding()]
param (
    [string] $InstallModuleName,
    [string] $UninstallModuleName,
    [string] $ModulePath
)

Import-Module WebAdministration

# check if script is executed in role of administrator
$isAdmin = (New-Object System.Security.Principal.WindowsPrincipal([System.Security.Principal.WindowsIdentity]::GetCurrent())).IsInRole([System.Security.Principal.WindowsBuiltInRole]::Administrator)

if ($isAdmin -eq $false) {
    Write-Output("[ps] The scirpt is not executed in Administrator role.")
    return
} 

if ($UninstallModuleName -ne "") {
    $ModuleList = Get-IISConfigSection -SectionPath "system.webServer/modules" | Get-IISConfigCollection

    foreach ($Module in $ModuleList) {
        if ($Module.RawAttributes.Count -ne 0 -and $Module.RawAttributes.name.CompareTo($UninstallModuleName) -eq 0) {
            Write-Output("[ps] Module exist. Uninstalling module...")
            $command = $env:SystemRoot + "\System32\inetsrv\appcmd uninstall module " + $UninstallModuleName+" |Out-Null"
            Invoke-Expression $command
            return
        }
    }

    Write-Output("[ps] No Module exist.")
}

if ($InstallModuleName -ne "" -and $ModulePath -ne "") {
    $command = $env:SystemRoot + "\System32\inetsrv\appcmd install module /name:" + $InstallModuleName + " /image:" + $ModulePath+" /preCondition:bitness64"# |Out-Null"
    Invoke-Expression $command
    Write-Output("[ps] Module Installed.")
}