# Make CThreadFastMutex::Lock() engine-independent (CS:SO / modern Source builds).
#
# Why this exists
# ---------------
# KeyValues (and a few other SDK classes) lock a CThreadFastMutex, and in the
# pinned HL2SDK that lock's slow path is an *imported* member function:
#
#     PLATFORM_CLASS void Lock( const ThreadId_t threadId, unsigned nSpinSleepTime ) volatile;
#
# The mangled name of that import depends on ThreadId_t, and ThreadId_t is:
#
#     #ifdef PLATFORM_64BITS
#     typedef uint64 ThreadId_t;      ->  ?Lock@CThreadFastMutex@@AECAX_KI@Z
#     #else
#     typedef uint32 ThreadId_t;      ->  ?Lock@CThreadFastMutex@@AECAXII@Z
#     #endif
#
# The engine that ships with CS:SO is a modern 64-bit Source build and uses
# 32-bit thread ids - SourceMod's working sourcemod.2.csso.dll imports exactly
# that uint32 form - so its tier0.dll exports
#
#     ?Lock@CThreadFastMutex@@AECAXII@Z   (unsigned int, unsigned)
#
# but not the uint64 form.  A 64-bit build of our plugin therefore gets
#
#     [SOB] Loader: FAILED to open: ... (GetLastError() == 127,
#                                       ERROR_PROC_NOT_FOUND)
#
# from LoadLibrary() and Metamod lists the plugin as <FAILED>.
#
# Why not simply rename the import (or bump the whole SDK)?
# --------------------------------------------------------
#   * Renaming it in the binary is unsafe: the two class layouts differ
#     (uint64 owner + int depth versus uint32 owner + int depth), so the
#     engine's Lock() would read and write the wrong offsets and the mutex
#     would never be released.
#   * Declaring the uint32 form in the header does not link either: the SDK's
#     prebuilt import library (hl2sdk-css\lib\public\x64\tier0.lib) still only
#     provides the uint64 symbol, so the link fails with
#
#       error LNK2019: unresolved external symbol
#         __imp_?Lock@CThreadFastMutex@@AECAXII@Z  (KeyValues.obj)
#
# Instead, this script defines the slow path *inline* in the class, so the
# plugin no longer imports the engine's Lock() at all:
#
#   * nothing else about the class changes - same layout, same inline fast path
#     (TryLockInline) and same inline Unlock() that the SDK already had, so the
#     mutex stays entirely inside our own module and remains self-consistent;
#   * the import disappears, which is what makes the plugin loadable on CS:SO;
#   * it works for the 32-bit build too (there the engine's type and ours are
#     both uint32; we simply stop calling into the engine as well);
#   * the only extra dependency is ThreadSleep(), which every tier0 has - the
#     SDK's own prebuilt tier0.lib for css contains it, and so does the
#     engine's tier0.dll (check it with: pe-check.ps1 -Exports <...>\tier0.dll).
#
# Only hl2sdk-css is patched: that is the SDK the CSS/CS:SO module is built and
# linked against (see the link line for sob.2.css.dll, which references
# hl2sdk-css\lib/public/x64/tier0.lib).  Other mods keep the stock SDK, whose
# import name matches their own 32-bit engines.
#
# Usage:
#     powershell -ExecutionPolicy Bypass -File tools\patch_tier0_threadtools.ps1 -Root alliedmodders
#
# The script is idempotent and exits non-zero if the expected class is not
# found, so CI fails loudly instead of shipping a plugin that cannot load.

param(
    [Parameter(Mandatory = $true)]
    [string]$Root
)

$ErrorActionPreference = 'Stop'

$relative = 'public\tier0\threadtools.h'
$requiredSdk = 'hl2sdk-css'

# The out-of-line declaration we are replacing, exactly as it appears in the
# pinned HL2SDK (tab-indented, one line).
$oldDeclaration = "`tPLATFORM_CLASS void Lock( const ThreadId_t threadId, unsigned nSpinSleepTime ) volatile;"

# Our replacement: the same signature, defined inline instead of imported.
$newDefinition = @'
	// SOB: the engine's tier0.dll is not guaranteed to export this out-of-line
	// helper. Modern 64-bit Source builds (CS:SO builds; SourceMod's
	// sourcemod.2.csso.dll imports the same) use 32-bit thread ids, so their
	// tier0 exports Lock(unsigned int, unsigned) while a 64-bit build of this
	// SDK asks for Lock(unsigned __int64, unsigned). The import then cannot be
	// resolved and LoadLibrary() fails with ERROR_PROC_NOT_FOUND (127), which
	// Metamod reports as <FAILED>. Defining the slow path here keeps the mutex
	// inside our own module and its layout private to us.
	void Lock( const ThreadId_t threadId, unsigned nSpinSleepTime ) volatile
	{
		for ( unsigned nSpin = 0; !TryLock( threadId ); ++nSpin )
		{
			if ( nSpin < 1000 )
			{
				ThreadPause();
			}
			else
			{
				ThreadSleep( nSpinSleepTime ? nSpinSleepTime : 1 );
				nSpin = 0;
			}
		}
	}
'@

$alreadyMarker = 'Defining the slow path here keeps the mutex'

function Find-ClassStart([string]$Text) {
    $marker = 'class CThreadFastMutex'
    $pos = 0
    while ($true) {
        $pos = $Text.IndexOf($marker, $pos)
        if ($pos -lt 0) { return -1 }
        $next = $Text.Substring($pos + $marker.Length, 1)
        if ($next -eq "`r" -or $next -eq "`n") { return $pos }
        $pos += $marker.Length
    }
}

function Patch-File([string]$Path) {
    $text = [IO.File]::ReadAllText($Path)

    $start = Find-ClassStart $text
    if ($start -lt 0) {
        Write-Host '    no CThreadFastMutex class found'
        return 'skipped'
    }

    # The class ends at the first "};" that starts a line.
    $end = $text.IndexOf("`n};", $start)
    if ($end -lt 0) {
        Write-Host '    class terminator not found'
        return 'skipped'
    }
    $stop = $end + 3
    $block = $text.Substring($start, $stop - $start)

    if ($block.Contains($alreadyMarker)) {
        Write-Host '    already patched'
        return 'already'
    }
    if (-not $block.Contains($oldDeclaration)) {
        Write-Host '    the expected out-of-line Lock() declaration is not there - not touching this file'
        return 'skipped'
    }

    $replacement = $newDefinition
    if ($block.Contains("`r`n")) { $replacement = $newDefinition.Replace("`n", "`r`n") }

    # Replace only that one declaration, inside the class.
    $patched = $text.Substring(0, $start) + $block.Replace($oldDeclaration, $replacement) + $text.Substring($stop)
    [IO.File]::WriteAllText($Path, $patched)

    Write-Host '    patched: CThreadFastMutex::Lock() is now defined inline, no engine import'
    return 'patched'
}

if (-not (Test-Path $Root)) {
    Write-Host "error: $Root is not a directory"
    exit 1
}

$file = Join-Path (Join-Path $Root $requiredSdk) $relative
Write-Host "  $requiredSdk"
if (-not (Test-Path $file)) {
    Write-Host "error: $file not found - the CSS/CS:SO module is built against hl2sdk-css"
    exit 1
}

$status = Patch-File $file
if ($status -eq 'skipped') {
    Write-Host 'error: hl2sdk-css was not patched - refusing to build a plugin that imports CThreadFastMutex::Lock(unsigned __int64)'
    exit 1
}

Write-Host 'OK: the CSS/CS:SO module will not import CThreadFastMutex::Lock() from the engine'
exit 0
