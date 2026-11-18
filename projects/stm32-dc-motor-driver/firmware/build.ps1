param([string]$ToolchainBin)

$ErrorActionPreference = 'Stop'

function Get-ArmTool([string]$Name) {
    if ($ToolchainBin) {
        return (Get-Item -LiteralPath (Join-Path $ToolchainBin "$Name.exe")).FullName
    }
    return (Get-Command "$Name.exe" -CommandType Application).Source
}

function Invoke-ArmTool([string]$Exe, [string[]]$ToolArgs) {
    & $Exe @ToolArgs
    if ($LASTEXITCODE -ne 0) { throw "$Exe exited with code $LASTEXITCODE" }
}

$gcc = Get-ArmTool 'arm-none-eabi-gcc'
$objcopy = Get-ArmTool 'arm-none-eabi-objcopy'
$size = Get-ArmTool 'arm-none-eabi-size'
Push-Location -LiteralPath $PSScriptRoot
try {
    New-Item -ItemType Directory -Force -Path build | Out-Null
    Invoke-ArmTool $gcc @(
        '-mcpu=cortex-m3', '-mthumb', '-std=c11', '-Os', '-g3',
        '-ffreestanding', '-ffunction-sections', '-fdata-sections',
        '-Wall', '-Wextra', '-Werror', '-nostdlib',
        '-T', 'stm32f103c8.ld', '-Wl,--gc-sections,-Map=build/motor.map',
        'startup.S', 'main.c', 'control.c', '-o', 'build/motor.elf'
    )
    Invoke-ArmTool $objcopy @('-O', 'ihex', 'build/motor.elf', 'build/motor.hex')
    Invoke-ArmTool $objcopy @('-O', 'binary', 'build/motor.elf', 'build/motor.bin')
    Invoke-ArmTool $size @('build/motor.elf')
} finally {
    Pop-Location
}
