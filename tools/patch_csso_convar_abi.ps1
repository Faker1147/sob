# Make the ConVar vtable layout match CS:SO's engine (CS:SO / Source Offensive).
#
# Why this exists
# ---------------
# The pinned HL2SDK's ConVar class and the one compiled into CS:SO's 64-bit
# engine disagree about the virtual function order.  The engine keeps calling
# our objects through its own layout, so a third-party cvar blows up as soon as
# the engine dispatches a slot we placed a different function in.  The most
# visible symptom is that printing a SOB cvar from the console (e.g. typing
# "sob_addbottime" with no value) kills the process with an access violation,
# while "sob_addbottime 7" works because the set path happens to line up.
#
# The 32-bit engines (legacy Source and CS:SO's win32 build) still use the
# stock SDK order, so the changes in this script are guarded by
# PLATFORM_64BITS and the win32 module is left exactly as it was.
#
# The engine's ConVar vtable (dumped from
# D:\Counter-Strike Source\win64\engine.dll, object vptr 0x1804efd10, and
# confirmed against engine's ConVar_PrintDescription at 0x180341150):
#
#     slot 0  ~ConVar
#     slot 1  IsCommand
#     slot 2  IsFlagSet
#     slot 3  AddFlags
#     slot 4  GetName
#     slot 5  GetHelpText
#     slot 6  IsRegistered
#     slot 7  GetDLLIdentifier
#     slot 8  CreateBase
#     slot 9  Init
#     slot 10 bool IsCompetitiveRestricted()        <-- absent from the SDK
#     slot 11 void SetValue/InternalSetValue(const char *)
#     slot 12 void SetValue/InternalSetFloatValue2(float, bool)
#     slot 13 void SetValue/InternalSetIntValue(int)
#     slot 14 bool ClampValue(float &)
#     slot 15 void ChangeStringValue(const char *, float)
#
# i.e. the engine has one extra bool virtual between Init and SetValue, and
# ClampValue/ChangeStringValue come *before* the InternalSet* helpers (the SDK
# places them after).  The engine's IConVar secondary vtable (object + 0x30,
# engine.dll+0x471200) is also ordered int, float, char* - while the SDK's
# iconvar.h declares char*, float, int.
#
# Why patching the header is safe
# -------------------------------
# The CSS module's tier1.lib is not prebuilt: SdkHelpers.ambuild builds it from
# hl2sdk-css/tier1/convar.cpp with these headers (see tier1/AMBuilder), so the
# whole ConVar/ConCommand implementation - and every internal virtual dispatch
# inside it - is recompiled with the corrected slot numbers.  Patching only the
# vtable bytes at runtime would not work: tier1.lib's SetValue(float) calls
# ClampValue through [vtable+0x80] (SDK slot 16), so shifting entries only in
# the object would send those internal calls to the wrong function.
#
# Slot 10 deliberately returns false (the same value the engine's own plain
# ConVar returns).  Returning true makes the engine's print path dynamic_cast
# style treat the object as a ConVar_ServerBounded and call GetFloat/GetInt at
# slots 16/17 - slots a plain ConVar does not have.
#
# Only hl2sdk-css is patched: that is the SDK the CS:SO module is built against.
# Other mods keep the stock SDK, whose engine layout is the SDK's own.
#
# Usage:
#     powershell -ExecutionPolicy Bypass -File tools\patch_csso_convar_abi.ps1 -Root alliedmodders
#
# The script is idempotent and exits non-zero if the expected declarations are
# not found, so CI fails loudly instead of shipping a plugin that crashes the
# engine.

param(
    [Parameter(Mandatory = $true)]
    [string]$Root
)

$ErrorActionPreference = 'Stop'

$sdkName = 'hl2sdk-css'
$convarRelative = 'public\tier1\convar.h'
$iconvarRelative = 'public\tier1\iconvar.h'
$marker = 'SOB: CS:SO ConVar ABI'

# ---------------------------------------------------------------------------
# Replacement blocks.  Written with LF here and converted to the file's own
# line endings by Patch-File.
# ---------------------------------------------------------------------------

# convar.h: the SetValue block at the start of the engine-visible virtuals.
$oldSetValue = @'
	// These just call into the IConCommandBaseAccessor to check flags and set the var (which ends up calling InternalSetValue).
	virtual void				SetValue( const char *value );
	virtual void				SetValue( float value );
	virtual void				SetValue( int value );
'@

$newSetValue = @'
#ifdef PLATFORM_64BITS
	// SOB: CS:SO ConVar ABI (64-bit engine only).
	//
	// The CS:SO engine's ConVar has an extra bool virtual at slot 10 and puts
	// ClampValue/ChangeStringValue at slots 14/15, before the InternalSet*
	// helpers.  The vtable order is an ABI contract: this is what lets the
	// engine call into third-party ConVars at all.
	//
	// A plain ConVar must answer false here.  The engine's print path treats a
	// true answer as "this is a ConVar_ServerBounded" and calls GetFloat/GetInt
	// at slots 16/17, which a plain ConVar does not have.
	virtual bool				IsCompetitiveRestricted() const { return false; }
#endif

	// These just call into the IConCommandBaseAccessor to check flags and set the var (which ends up calling InternalSetValue).
	virtual void				SetValue( const char *value );
	virtual void				SetValue( float value );
	virtual void				SetValue( int value );

#ifdef PLATFORM_64BITS
private:
	// CS:SO engine slots 14/15 - these must be declared before InternalSetValue.
	virtual bool				ClampValue( float& value );
	virtual void				ChangeStringValue( const char *tempVal, float flOldValue );
public:
#endif
'@

# convar.h: the old inline declaration further down the public section.  On
# 32-bit builds it stays (the stock layout needs it); the 64-bit build defines
# the method inside the class as the engine's slot 10 virtual instead.
$oldInlineDeclaration = @'
	FORCEINLINE_CVAR bool		IsCompetitiveRestricted() const;
'@

$newInlineDeclaration = @'
#ifndef PLATFORM_64BITS
	FORCEINLINE_CVAR bool		IsCompetitiveRestricted() const;
#endif
'@

# convar.h: the private ClampValue/ChangeStringValue declarations that now
# move up into the public section above.
$oldPrivateClamp = @'
	// For CVARs marked FCVAR_NEVER_AS_STRING
	virtual void				InternalSetFloatValue( float fNewValue );
	virtual void				InternalSetIntValue( int nValue );

	virtual bool				ClampValue( float& value );
	virtual void				ChangeStringValue( const char *tempVal, float flOldValue );

	virtual void				Create_Vtbl( const char *pName, const char *pDefaultValue, int flags = 0,
'@

$newPrivateClamp = @'
	// For CVARs marked FCVAR_NEVER_AS_STRING
	virtual void				InternalSetFloatValue( float fNewValue );
	virtual void				InternalSetIntValue( int nValue );

#ifndef PLATFORM_64BITS
	// SOB: 32-bit engines still use the stock SDK virtual order.
	virtual bool				ClampValue( float& value );
	virtual void				ChangeStringValue( const char *tempVal, float flOldValue );
#endif

	virtual void				Create_Vtbl( const char *pName, const char *pDefaultValue, int flags = 0,
'@

# convar.h: the old out-of-class inline implementation.  The virtual is now
# defined inside the class so that it occupies engine slot 10.
$oldInlineDefinition = @'
//-----------------------------------------------------------------------------
// Purpose: Return whether this convar is restricted for competitive play.
// Output : bool
//-----------------------------------------------------------------------------
FORCEINLINE_CVAR bool ConVar::IsCompetitiveRestricted() const
{
	const int nFlags = m_pParent->m_nFlags;

	const bool bHasCompSettings = m_pParent->m_bHasCompMin || m_pParent->m_bHasCompMax;
	const bool bClientCanAdjust = ( nFlags & ( FCVAR_ARCHIVE | FCVAR_ALLOWED_IN_COMPETITIVE ) ) != 0;
	const bool bInternalUseOnly = ( nFlags & ( FCVAR_HIDDEN | FCVAR_DEVELOPMENTONLY | FCVAR_INTERNAL_USE | FCVAR_GAMEDLL | FCVAR_REPLICATED | FCVAR_CHEAT ) ) != 0;

	return bHasCompSettings || !( bClientCanAdjust || bInternalUseOnly );
}
'@

$newInlineDefinition = @'
#ifndef PLATFORM_64BITS
//-----------------------------------------------------------------------------
// Purpose: Return whether this convar is restricted for competitive play.
// Output : bool
//-----------------------------------------------------------------------------
FORCEINLINE_CVAR bool ConVar::IsCompetitiveRestricted() const
{
	const int nFlags = m_pParent->m_nFlags;

	const bool bHasCompSettings = m_pParent->m_bHasCompMin || m_pParent->m_bHasCompMax;
	const bool bClientCanAdjust = ( nFlags & ( FCVAR_ARCHIVE | FCVAR_ALLOWED_IN_COMPETITIVE ) ) != 0;
	const bool bInternalUseOnly = ( nFlags & ( FCVAR_HIDDEN | FCVAR_DEVELOPMENTONLY | FCVAR_INTERNAL_USE | FCVAR_GAMEDLL | FCVAR_REPLICATED | FCVAR_CHEAT ) ) != 0;

	return bHasCompSettings || !( bClientCanAdjust || bInternalUseOnly );
}
#endif
'@

# iconvar.h: match the engine's IConVar vtable order (int, float, char*).
$oldIConVar = @'
	// Value set
	virtual void SetValue( const char *pValue ) = 0;
	virtual void SetValue( float flValue ) = 0;
	virtual void SetValue( int nValue ) = 0;
'@

$newIConVar = @'
#ifdef PLATFORM_64BITS
	// SOB: CS:SO ConVar ABI (64-bit engine only) - the engine's IConVar vtable
	// is ordered int, float, char* (engine.dll+0x471200); order is part of the ABI.
	virtual void SetValue( int nValue ) = 0;
	virtual void SetValue( float flValue ) = 0;
	virtual void SetValue( const char *pValue ) = 0;
#else
	// Value set
	virtual void SetValue( const char *pValue ) = 0;
	virtual void SetValue( float flValue ) = 0;
	virtual void SetValue( int nValue ) = 0;
#endif
'@

function Patch-File([string]$Path, [string[]]$OldBlocks, [string[]]$NewBlocks) {
    $bytes = [IO.File]::ReadAllBytes($Path)
    $hadBom = $bytes.Length -ge 3 -and $bytes[0] -eq 0xEF -and $bytes[1] -eq 0xBB -and $bytes[2] -eq 0xBF
    $text = [Text.Encoding]::UTF8.GetString($bytes)
    if ($hadBom) { $text = $text.Substring(1) }

    if ($text.Contains($marker)) {
        Write-Host '    already patched'
        return 'already'
    }

    $crlf = $text.Contains("`r`n")
    if ($crlf) { $text = $text.Replace("`r`n", "`n") }

    if ($OldBlocks.Count -ne $NewBlocks.Count) {
        throw 'internal error: block lists differ in length'
    }

    for ($i = 0; $i -lt $OldBlocks.Count; $i++) {
        # The script itself may have been checked out with CRLF line endings
        # (git core.autocrlf on Windows), while $text has been normalised to
        # LF above.  Normalise the expected/new blocks to LF too; the file is
        # converted back to CRLF at the end if that is what it used.
        $old = $OldBlocks[$i].Replace("`r`n", "`n")
        $new = $NewBlocks[$i].Replace("`r`n", "`n")
        $first = $text.IndexOf($old, [StringComparison]::Ordinal)
        if ($first -lt 0) {
            Write-Host "    expected block not found:"
            Write-Host ($old -split "`n" | Select-Object -First 3 | ForEach-Object { "      $_" })
            return 'skipped'
        }
        if ($text.IndexOf($old, $first + 1, [StringComparison]::Ordinal) -ge 0) {
            Write-Host '    expected block is not unique'
            return 'skipped'
        }
        $text = $text.Substring(0, $first) + $new + $text.Substring($first + $old.Length)
    }

    if ($crlf) { $text = $text.Replace("`n", "`r`n") }

    $out = New-Object Text.UTF8Encoding($hadBom)
    [IO.File]::WriteAllText($Path, $text, $out)
    return 'patched'
}

if (-not (Test-Path $Root)) {
    Write-Host "error: $Root is not a directory"
    exit 1
}

$sdk = Join-Path $Root $sdkName
if (-not (Test-Path $sdk)) {
    Write-Host "error: $sdk not found - the CSS/CS:SO module is built against hl2sdk-css"
    exit 1
}

$failed = $false

Write-Host '  hl2sdk-css/public/tier1/convar.h'
$convar = Join-Path $sdk $convarRelative
if (-not (Test-Path $convar)) {
    Write-Host "error: $convar not found"
    exit 1
}
$status = Patch-File $convar `
    @($oldSetValue, $oldInlineDeclaration, $oldPrivateClamp, $oldInlineDefinition) `
    @($newSetValue, $newInlineDeclaration, $newPrivateClamp, $newInlineDefinition)
if ($status -eq 'skipped') { $failed = $true }

Write-Host '  hl2sdk-css/public/tier1/iconvar.h'
$iconvar = Join-Path $sdk $iconvarRelative
if (-not (Test-Path $iconvar)) {
    Write-Host "error: $iconvar not found"
    exit 1
}
$status = Patch-File $iconvar @($oldIConVar) @($newIConVar)
if ($status -eq 'skipped') { $failed = $true }

if ($failed) {
    Write-Host 'error: hl2sdk-css was not patched - refusing to build a plugin whose ConVar vtable does not match CS:SO'
    exit 1
}

Write-Host 'OK: the CSS/CS:SO module will use the engine-matching ConVar vtable'
exit 0
