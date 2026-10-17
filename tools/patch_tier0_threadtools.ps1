# Align CThreadFastMutex with the ABI used by modern Source engine builds.
#
# Why this exists
# ---------------
# The plugin imports a handful of symbols from the engine's tier0.dll.  One of
# them, CThreadFastMutex::Lock(), is an out-of-line member function whose
# mangled name depends on the type of its first parameter, and whose class
# layout depends on the type of its m_ownerID member.
#
# In the pinned HL2SDK that type is ThreadId_t, and ThreadId_t is:
#
#     #ifdef PLATFORM_64BITS
#     typedef uint64 ThreadId_t;
#     #else
#     typedef uint32 ThreadId_t;
#     #endif
#
# so a 64-bit build asks tier0.dll for
#
#     ?Lock@CThreadFastMutex@@AECAX_KI@Z   (unsigned __int64 threadId, unsigned)
#
# The engine that ships with CS:SO (and other modern 64-bit Source builds) uses
# 32-bit thread ids instead - exactly what SourceMod's sourcemod.2.csso.dll does
# - so its tier0.dll only exports
#
#     ?Lock@CThreadFastMutex@@AECAXII@Z    (unsigned int threadId, unsigned)
#
# Result: LoadLibrary() on our plugin fails with GetLastError() == 127
# (ERROR_PROC_NOT_FOUND, "The specified procedure could not be found") and
# Metamod lists the plugin as <FAILED>.  Renaming the import alone is not
# enough: the two class layouts differ (uint64 owner + int depth versus uint32
# owner + int depth), so the engine's Lock() would read and write the wrong
# offsets and the mutex would never be released.
#
# Rather than bump the whole SDK (a much bigger ABI jump), this script replaces
# only the CThreadFastMutex class with the 32-bit form that the engine and
# SourceMod both use.  The class is self-contained, so nothing else is affected:
#
#   * 64-bit builds now import ?Lock@CThreadFastMutex@@AECAXII@Z, which exists.
#   * 32-bit builds are unchanged in behaviour (ThreadId_t is already uint32
#     there, and ThreadInterlockedAssignIf is already the 32-bit CAS).
#   * The parameter casts are explicit, so the patched text also builds in the
#     POSIX/Clang branch of the header, where the helpers take int32.
#
# Usage:
#     powershell -ExecutionPolicy Bypass -File tools\patch_tier0_threadtools.ps1 -Root alliedmodders
#
# Every hl2sdk-*\public\tier0\threadtools.h that still contains the old class is
# patched.  The SDK used for the CSS/CS:SO module (hl2sdk-css) must be present
# and must end up aligned, otherwise the script exits non-zero so CI fails
# loudly instead of shipping a plugin that cannot load.

param(
    [Parameter(Mandatory = $true)]
    [string]$Root
)

$ErrorActionPreference = 'Stop'

$relative = 'public\tier0\threadtools.h'
$requiredSdk = 'hl2sdk-css'

$newClass = @'
class CThreadFastMutex
{
public:
	CThreadFastMutex()
	  :	m_ownerID( 0 ),
	  	m_depth( 0 )
	{
	}

private:
	FORCEINLINE bool TryLockInline( const uint32 threadId ) volatile
	{
		if ( threadId != m_ownerID && !ThreadInterlockedAssignIf( (int32 volatile *)&m_ownerID, (int32)threadId, 0 ) )
			return false;

		ThreadMemoryBarrier();
		++m_depth;
		return true;
	}

	bool TryLock( const uint32 threadId ) volatile
	{
		return TryLockInline( threadId );
	}

	PLATFORM_CLASS void Lock( const uint32 threadId, unsigned nSpinSleepTime ) volatile;

public:
	bool TryLock() volatile
	{
#ifdef _DEBUG
		if ( m_depth == INT_MAX )
			DebuggerBreak();

		if ( m_depth < 0 )
			DebuggerBreak();
#endif
		return TryLockInline( (uint32)ThreadGetCurrentId() );
	}

#ifndef _DEBUG
	FORCEINLINE
#endif
	void Lock( unsigned int nSpinSleepTime = 0 ) volatile
	{
		const uint32 threadId = (uint32)ThreadGetCurrentId();

		if ( !TryLockInline( threadId ) )
		{
			ThreadPause();
			Lock( threadId, nSpinSleepTime );
		}
#ifdef _DEBUG
		if ( m_ownerID != (uint32)ThreadGetCurrentId() )
			DebuggerBreak();

		if ( m_depth == INT_MAX )
			DebuggerBreak();

		if ( m_depth < 0 )
			DebuggerBreak();
#endif
	}

#ifndef _DEBUG
	FORCEINLINE
#endif
	void Unlock() volatile
	{
#ifdef _DEBUG
		if ( m_ownerID != (uint32)ThreadGetCurrentId() )
			DebuggerBreak();

		if ( m_depth <= 0 )
			DebuggerBreak();
#endif

		--m_depth;
		if ( !m_depth )
		{
			ThreadMemoryBarrier();
			ThreadInterlockedExchange( (int32 volatile *)&m_ownerID, 0 );
		}
	}

#ifdef WIN32
	bool TryLock() const volatile							{ return (const_cast<CThreadFastMutex *>(this))->TryLock(); }
	void Lock(unsigned nSpinSleepTime = 1 ) const volatile	{ (const_cast<CThreadFastMutex *>(this))->Lock( nSpinSleepTime ); }
	void Unlock() const	volatile							{ (const_cast<CThreadFastMutex *>(this))->Unlock(); }
#endif
	// To match regular CThreadMutex:
	bool AssertOwnedByCurrentThread()	{ return true; }
	void SetTrace( bool )				{}

	uint32 GetOwnerId() const			{ return m_ownerID;	}
	int	GetDepth() const				{ return m_depth; }
private:
	volatile uint32 m_ownerID;
	int				m_depth;
};
'@

# The here-string above has a trailing newline; the class is inserted without
# one because the terminator's newline comes from the surrounding file.
if ($newClass.EndsWith("`r`n")) { $newClass = $newClass.Substring(0, $newClass.Length - 2) }
elseif ($newClass.EndsWith("`n")) { $newClass = $newClass.Substring(0, $newClass.Length - 1) }

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

# Status messages go to the console (Write-Host), not to the output stream:
# the caller assigns this function's output to a variable, which would swallow
# anything written with Write-Output.
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

    if ($block.Contains('const uint32 threadId')) {
        Write-Host '    already aligned'
        return 'already'
    }
    if (-not $block.Contains('const ThreadId_t threadId')) {
        Write-Host '    unrecognised CThreadFastMutex definition - not touching it'
        return 'skipped'
    }

    $replacement = $newClass
    if ($block.Contains("`r`n")) { $replacement = $newClass.Replace("`n", "`r`n") }

    $patched = $text.Substring(0, $start) + $replacement + $text.Substring($stop)
    [IO.File]::WriteAllText($Path, $patched)

    $oldLines = ($block -split "`n").Count
    $newLines = ($replacement -split "`n").Count
    Write-Host ("    patched ({0} lines -> {1} lines)" -f $oldLines, $newLines)
    return 'patched'
}

if (-not (Test-Path $Root)) {
    Write-Output "error: $Root is not a directory"
    exit 1
}

$results = @{}
Get-ChildItem -Path $Root -Directory -Filter 'hl2sdk-*' -ErrorAction SilentlyContinue | Sort-Object Name | ForEach-Object {
    $sdk = $_.Name
    $file = Join-Path (Join-Path $_.FullName 'public\tier0') 'threadtools.h'
    if (-not (Test-Path $file)) { return }
    Write-Output ("  {0}" -f $sdk)
    $results[$sdk] = Patch-File $file
}

if (-not $results.ContainsKey($requiredSdk)) {
    Write-Output ("error: {0} not found under {1} - the CSS module cannot be built against the right ABI" -f $requiredSdk, $Root)
    exit 1
}
if ($results[$requiredSdk] -eq 'skipped') {
    Write-Output ("error: {0} was not aligned - refusing to build a plugin that imports CThreadFastMutex::Lock(unsigned __int64)" -f $requiredSdk)
    exit 1
}

Write-Output ("OK: {0} is aligned with the engine ABI" -f $requiredSdk)
exit 0
