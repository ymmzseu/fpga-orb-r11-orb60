[CmdletBinding()]
param()
$ErrorActionPreference='Stop'
$checkRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$checkManifest=Get-Content -LiteralPath (Join-Path $checkRoot 'orb60/INTEGRATED4_MANIFEST.json') -Raw | ConvertFrom-Json
foreach($entry in $checkManifest.files) {
    if((Get-FileHash -LiteralPath (Join-Path $checkRoot $entry.path)).Hash.ToLowerInvariant() -ne $entry.sha256) {throw "Publication mismatch: $($entry.path)"}
}
$checkArtifact=Get-Content -LiteralPath (Join-Path $checkRoot 'orb60/artifacts/integrated4_100mhz/artifact_manifest.json') -Raw | ConvertFrom-Json
if($checkArtifact.status -ne 'PASS' -or $checkArtifact.layer_ppc -ne 4 -or !$checkArtifact.joint_validation.engineering14ms_pass) {throw 'Missing accepted4PPC evidence'}
Write-Output "INTEGRATED4_PUBLICATION_PASS files=$($checkManifest.files.Count)"
Write-Output 'File integrity only; board/cache/videoP99 remains unverified.'
