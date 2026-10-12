[CmdletBinding()]
param(
    [ValidateSet('baseline','optimized')][string]$Variant='baseline',
    [ValidateSet('front','front2','layer2','descriptor','resize','gaussian','all')][string]$Kernel='all',
    [ValidateRange(5,20)][double]$Period=10,
    [switch]$Cosim,
    [switch]$CsimOnly,
    [string]$DescriptorVectors='',
    [string]$JobFile='',
    [string]$SourceJob=''
)
. (Join-Path $PSScriptRoot 'environment.ps1')
if ($Cosim -and $CsimOnly) { throw 'Choose CsimOnly or Cosim' }
$jobName = '{0}_{1}_{2}' -f (Get-Date -Format 'yyyyMMdd_HHmmss'),$Kernel,([guid]::NewGuid().ToString('N').Substring(0,8))
$job = Join-Path $Orb60Root "build/$Variant/hls/$jobName"
$inputRoot = Join-Path $job 'inputs'
New-Item -ItemType Directory -Path $inputRoot -Force | Out-Null
# Every job compiles a frozen snapshot, including the independent reference.
if($SourceJob) {
    $replayRoot=[IO.Path]::GetFullPath($SourceJob)
    $replayState=Get-Content -LiteralPath (Join-Path $replayRoot 'status.json') -Raw | ConvertFrom-Json
    if($replayState.State -ne 'SUCCEEDED' -or !$replayState.Cosim -or $replayState.Kernel -ne $Kernel -or
       $replayState.Variant -ne $Variant -or $replayState.Part -ne 'xc7z015clg485-2') {throw 'SourceJob must be a paired accepted HLS snapshot'}
    foreach($entry in (Get-Content -LiteralPath (Join-Path $replayRoot 'source_manifest.json') -Raw | ConvertFrom-Json)) {
        if((Get-FileHash -LiteralPath (Join-Path $replayRoot ('inputs/'+$entry.Path))).Hash -ne $entry.SHA256) {throw 'Replay source hash changed'}
    }
    foreach($item in Get-ChildItem -LiteralPath (Join-Path $replayRoot 'inputs')) {
        Copy-Item -LiteralPath $item.FullName -Destination $inputRoot -Recurse
    }
} else {
Copy-Item -LiteralPath (Join-Path $Orb60Root "$Variant/FPGA_ORB_HLS") -Destination $inputRoot -Recurse
Copy-Item -LiteralPath (Join-Path $Orb60Root "$Variant/FPGA_ORB_SHARED_v1.0") -Destination $inputRoot -Recurse
$vectors = Join-Path $Orb60Root "build/$Variant/host/generated/front_vectors.inc"
if (!(Test-Path -LiteralPath $vectors)) { throw 'Run host.ps1 first to generate independent front vectors' }
Copy-Item -LiteralPath $vectors -Destination (Join-Path $inputRoot 'front_vectors.inc')
if ($Kernel -eq 'front2') {
    Copy-Item -LiteralPath (Join-Path $Orb60Root "build/$Variant/host/generated/front2_vectors.inc") -Destination (Join-Path $inputRoot 'front2_vectors.inc')
}
if ($Kernel -eq 'layer2') {
    Copy-Item -LiteralPath (Join-Path $Orb60Root "build/$Variant/host/generated/layer2_vectors.inc") -Destination (Join-Path $inputRoot 'layer2_vectors.inc')
}
}
if ($Kernel -eq 'descriptor') {
    if (!$DescriptorVectors -or !(Test-Path -LiteralPath (Join-Path $DescriptorVectors 'vectors.json'))) {throw 'Requires independent descriptor vectors'}
    Copy-Item -LiteralPath $DescriptorVectors -Destination (Join-Path $job 'descriptor_vectors') -Recurse
}
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'hls.tcl') -Destination $job
$hashes = Get-ChildItem -LiteralPath $inputRoot -Recurse -File | ForEach-Object {
    [ordered]@{Path=$_.FullName.Substring($inputRoot.Length+1); SHA256=(Get-FileHash -LiteralPath $_.FullName).Hash}
}
$hashes | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath (Join-Path $job 'source_manifest.json') -Encoding UTF8
$state = [ordered]@{State='RUNNING';Variant=$Variant;Kernel=$Kernel;PeriodNs=$Period;Cosim=[bool]$Cosim;StartedUtc=[DateTime]::UtcNow.ToString('o');Part='xc7z015clg485-2';Tool=$VitisHlsBat;Job=$job;SourceJob=$SourceJob}
$state | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $job 'status.json') -Encoding UTF8
if($JobFile) {$job | Set-Content -LiteralPath $JobFile -Encoding UTF8}
$env:ORB60_HLS_JOB=$job.Replace('\','/')
$env:ORB60_HLS_KERNEL=$Kernel
$env:ORB60_HLS_PERIOD=$Period.ToString([Globalization.CultureInfo]::InvariantCulture)
$env:ORB60_HLS_COSIM=[int][bool]$Cosim
$env:ORB60_HLS_CSIM_ONLY=[int][bool]$CsimOnly
Write-Output "ORB60_HLS_JOB=$job"
Push-Location $job
try {
    if ($Kernel -eq 'layer2') {
        & $ProjectPython (Join-Path $Orb60Root 'tests/prepare_layer2.py') $job (Join-Path $job 'oracle_vectors')
        if ($LASTEXITCODE -ne 0) {throw 'Layer2 independent Python oracle failed'}
    }
    # Use the native no-log option: AMD's Windows launcher otherwise invokes
    # the unrelated MSYS tee on PATH. We retain the complete log ourselves.
    $ErrorActionPreference='Continue'
    & $VitisHlsBat -nolog -f (Join-Path $job 'hls.tcl') 2>&1 | Tee-Object -FilePath (Join-Path $job 'tool.log')
    $toolExit=$LASTEXITCODE
    $ErrorActionPreference='Stop'
    if ($toolExit -ne 0 -or !(Test-Path -LiteralPath (Join-Path $job 'success.stamp'))) { throw "HLS failed (exit $toolExit)" }
    $state.State='SUCCEEDED'
} catch {
    $state.State='FAILED'; $state['Error']=$_.Exception.Message
    throw
} finally {
    Pop-Location
    $state['FinishedUtc']=[DateTime]::UtcNow.ToString('o')
    $state | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $job 'status.json') -Encoding UTF8
}
