[CmdletBinding()]
param()
$ErrorActionPreference='Stop'
$pubRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$pubManifest=Get-Content -LiteralPath (Join-Path $pubRoot 'orb60/PUBLICATION_MANIFEST.json') -Raw | ConvertFrom-Json
foreach($entry in $pubManifest.files) {
    $pubPath=Join-Path $pubRoot $entry.path
    if((Get-FileHash -LiteralPath $pubPath).Hash.ToLowerInvariant() -ne $entry.sha256) {throw "ORB60 publication byte mismatch: $($entry.path)"}
}
$pubBit=Get-Content -LiteralPath (Join-Path $pubRoot 'orb60/artifacts/100mhz/artifact_manifest.json') -Raw | ConvertFrom-Json
if($pubBit.status -ne 'PASS' -or $pubBit.part -ne 'xc7z015clg485-2' -or $pubBit.frequency_mhz -ne 100) {throw 'Wrong bitstream publication'}
Write-Output "ORB60_PUBLICATION_PASS files=$($pubManifest.files.Count) part=$($pubBit.part) MHz=$($pubBit.frequency_mhz)"
Write-Output 'This checks publication integrity, not a new synthesis/RTL/physical-board test.'
