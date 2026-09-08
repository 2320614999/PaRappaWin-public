# Self-contained build script for PaRappaWin
# Usage:
#   powershell -ExecutionPolicy Bypass -File build.ps1
#   powershell -ExecutionPolicy Bypass -File build.ps1 -Fast
#
# Default behavior is a full rebuild.
# This is intentional and recommended for this project: incremental builds have
# repeatedly left stale objects around and caused hard-to-trust runtime results.
# Use -Fast only when you explicitly want a quicker incremental build and accept
# that it is not recommended for bug verification.

param(
    [switch]$Fast,
    [switch]$Rebuild,
    [string]$MsvcPath = $env:VCToolsInstallDir,
    [string]$WindowsSdkPath = $env:WindowsSdkDir,
    [string]$WindowsSdkVersion = $env:WindowsSDKVersion
)

$ErrorActionPreference = 'Stop'

$fullRebuild = $true
if ($Fast) {
    $fullRebuild = $false
}

# Keep -Rebuild as a compatibility switch, but make full rebuild the default.
if ($Rebuild) {
    $fullRebuild = $true
}

function Wait-ForBuildArtifact {
    param(
        [string]$Path,
        [int]$TimeoutMs = 10000
    )
    $deadline = (Get-Date).AddMilliseconds($TimeoutMs)
    while ((Get-Date) -lt $deadline) {
        if (Test-Path $Path) {
            $item = Get-Item $Path -ErrorAction SilentlyContinue
            if ($null -ne $item -and $item.Length -gt 0) {
                return $true
            }
        }
        Start-Sleep -Milliseconds 200
    }
    if (Test-Path $Path) {
        $item = Get-Item $Path -ErrorAction SilentlyContinue
        return ($null -ne $item -and $item.Length -gt 0)
    }
    return $false
}

function Test-CompileArtifactHealthy {
    param(
        [string]$SrcFile,
        [string]$ObjFile,
        [string]$StdOutPath,
        [string]$StdErrPath
    )
    if (-not (Wait-ForBuildArtifact -Path $ObjFile)) {
        return $false
    }
    if (-not (Test-Path $ObjFile) -or -not (Test-Path $SrcFile)) {
        return $false
    }

    $objTime = (Get-Item $ObjFile).LastWriteTimeUtc
    $srcTime = (Get-Item $SrcFile).LastWriteTimeUtc
    if ($objTime -lt $srcTime) {
        return $false
    }

    $clOutText = if (Test-Path $StdOutPath) {
        Get-Content $StdOutPath -Raw -ErrorAction SilentlyContinue
    } else {
        ''
    }
    $clErrText = if (Test-Path $StdErrPath) {
        Get-Content $StdErrPath -Raw -ErrorAction SilentlyContinue
    } else {
        ''
    }

    if ($clOutText -match '(?i)fatal error|error C[0-9]+' -or
        $clErrText -match '(?i)fatal error|error C[0-9]+') {
        return $false
    }

    return $true
}

function Start-CompileProcess {
    param(
        [string]$CompilerPath,
        [string]$Arguments,
        [string]$StdOutPath,
        [string]$StdErrPath
    )
    for ($attempt = 0; $attempt -lt 2; $attempt++) {
        try {
            return Start-Process -FilePath $CompilerPath `
                -ArgumentList $Arguments `
                -NoNewWindow -PassThru `
                -RedirectStandardOutput $StdOutPath `
                -RedirectStandardError $StdErrPath
        } catch {
            if ($attempt -ge 1) {
                throw
            }
            Start-Sleep -Milliseconds 200
        }
    }
}

# --- MSVC Environment ---
if (-not $MsvcPath) {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (-not (Test-Path -LiteralPath $vswhere)) {
        throw 'Install Visual Studio C++ Build Tools, or provide -MsvcPath.'
    }
    $installation = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (-not $installation) { throw 'Visual Studio x64 C++ tools were not found.' }
    $MsvcPath = (Get-ChildItem -LiteralPath (Join-Path $installation 'VC\Tools\MSVC') -Directory |
        Sort-Object { [version]$_.Name } -Descending | Select-Object -First 1).FullName
}
if (-not $WindowsSdkPath) {
    $WindowsSdkPath = Join-Path ${env:ProgramFiles(x86)} 'Windows Kits\10'
}
if (-not $WindowsSdkVersion) {
    $WindowsSdkVersion = (Get-ChildItem -LiteralPath (Join-Path $WindowsSdkPath 'Include') -Directory |
        Where-Object { $_.Name -match '^10\.\d+\.\d+\.\d+$' } |
        Sort-Object { [version]$_.Name } -Descending | Select-Object -First 1).Name
}
$msvc = $MsvcPath.TrimEnd('\')
$sdk = $WindowsSdkPath.TrimEnd('\')
$sdkVer = $WindowsSdkVersion.TrimEnd('\')
foreach ($required in @("$msvc\bin\Hostx64\x64\cl.exe", "$sdk\Include\$sdkVer\um\Windows.h")) {
    if (-not (Test-Path -LiteralPath $required)) { throw "Missing toolchain component: $required" }
}

$env:PATH    = "$msvc\bin\Hostx64\x64;$sdk\bin\$sdkVer\x64;$env:PATH"
$env:INCLUDE = "$msvc\include;$sdk\Include\$sdkVer\ucrt;$sdk\Include\$sdkVer\shared;$sdk\Include\$sdkVer\um;$sdk\Include\$sdkVer\winrt"
$env:LIB     = "$msvc\lib\x64;$sdk\Lib\$sdkVer\ucrt\x64;$sdk\Lib\$sdkVer\um\x64"

# --- Paths ---
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$src       = "$scriptDir\src"
$objDir    = "$scriptDir\build\PaRappaWin.dir\RelWithDebInfo"
$productDir = Join-Path $scriptDir 'build\product'
New-Item -ItemType Directory -Path $productDir -Force | Out-Null
$outExe = Join-Path $productDir 'PaRappaWin.exe'
$outPdb = Join-Path $productDir 'PaRappaWin.pdb'

# Do not terminate games or compilers belonging to another checkout.
# Close this checkout's executable before building if Windows reports a lock.

# Ensure obj dir
if (!(Test-Path $objDir)) { New-Item -ItemType Directory -Path $objDir -Force | Out-Null }

# Full rebuild is the default because stale incremental outputs have caused
# misleading "fixed in code but still broken in runtime" situations before.
if ($fullRebuild) {
    $resolvedObjDir = (Resolve-Path -LiteralPath $objDir).Path
    $expectedObjDir = [IO.Path]::GetFullPath((Join-Path $scriptDir 'build\PaRappaWin.dir\RelWithDebInfo'))
    if ($resolvedObjDir -ne $expectedObjDir) { throw 'Refusing to clean an unexpected object directory.' }
    Get-ChildItem -LiteralPath $resolvedObjDir -Force | ForEach-Object {
        Remove-Item -LiteralPath $_.FullName -Recurse -Force
    }
} else {
    Write-Host '[BUILD] FAST incremental mode enabled (not recommended for bug verification).' -ForegroundColor Yellow
}

# --- Source files ---
$sources = @(
  'main.cpp','app_config.cpp','d3d11_renderer.cpp','tim_decoder.cpp',
  'boot_logo.cpp','int_loader.cpp','resource_manager.cpp','menu_scene.cpp',
  'pr\pr_main.cpp','pr\pr_event.cpp','pr\pr_sqevs1.cpp','pr\pr_card.cpp','pr\pr_memcard_backend.cpp','pr\pr_ss0_card_image_storage_direct.cpp',
  'pr\pr_overlay_loader.cpp','pr\pr_scenes.cpp','pr\pr_stage1_scorer_host.cpp','pr\pr_stage1_scorer_host_direct.cpp','pr\pr_stage1_scorer_direct.cpp','pr\pr_stage1_rating_presentation_direct.cpp','pr\pr_stage1_hud_presentation_direct.cpp','pr\pr_stage1_lifecycle_direct.cpp','pr\pr_stage1_lifecycle_executor_direct.cpp','pr\pr_stage1_bootstrap_cd_request_direct.cpp','pr\pr_stage1_lifecycle_host_adapter_801c81ec.cpp','pr\pr_stage1_loader_direct.cpp','pr\pr_stage1_loader_producer_adapter.cpp','pr\pr_stage1_lower_cd_producer_direct.cpp','pr\pr_stage1_loader_memory_direct.cpp','pr\pr_stage1_loader_cd_hal.cpp','pr\pr_stage1_loader_gpu_hal.cpp','pr\pr_stage1_loader_spu_hal.cpp','pr\pr_stage1_fail_prompt_direct.cpp','pr\pr_stage1_overlay_script_text_direct.cpp','pr\pr_stage1_rail_cursor_event_direct.cpp','pr\pr_stage1_script_event_runtime_direct.cpp','pr\pr_stage_event_direct.cpp','pr\pr_stage1_runtime_slots_direct.cpp','pr\pr_stage1_compact_rail_80024744_direct.cpp','pr\pr_stage1_xa_cd_direct.cpp','pr\pr_stage1_camera_motion_direct.cpp','pr\pr_stage1_tod_cursor_direct.cpp','pr\pr_stage_scene_submit_backend.cpp','pr\pr_stage_scene_submit_direct.cpp','pr\pr_stage_payload_bank_direct.cpp','pr\pr_stage_status_bank_direct.cpp','pr\pr_stage_status_bank_host_bridge_direct.cpp','pr\pr_psx_event_frame_direct.cpp','pr\pr_psx_vsync_direct.cpp','pr\pr_psx_fast_sprite_submit_direct.cpp','pr\pr_psx_gte_direct.cpp','pr\pr_psx_gs_sprite_submit_direct.cpp','pr\pr_psx_tmd_submit_direct.cpp','pr\pr_psx_graph_owner_direct.cpp','pr\pr_psx_clear_image_direct.cpp','pr\pr_psx_dma_submit_direct.cpp','pr\pr_psx_pad_direct.cpp','pr\pr_stage1_save_card_hal_direct.cpp','pr\pr_stage1_save_ui_directory_carrier_direct.cpp','pr\pr_stage1_save_ui_host_bridge_direct.cpp','pr\pr_stage1_save_ui_direct.cpp','pr\pr_stage1_movie_text_direct.cpp','pr\pr_stage1_movie_text_outer_loop_direct.cpp','pr\pr_ss0_direct.cpp','pr\pr_ss0_scene0_runtime_direct.cpp','pr\pr_ss0_scene0_global_bindings_direct.cpp','pr\pr_ss0_scene0_resource_ingress_direct.cpp','pr\pr_ss0_scene0_int_load_direct.cpp','pr\pr_ss0_scene0_int_side_effect_direct.cpp','pr\pr_ss0_scene0_int_spu_direct.cpp','pr\pr_ss0_scene0_int_gpu_direct.cpp','pr\pr_ss0_scene0_shared_event_predispatch_direct.cpp','pr\pr_ss0_state16_runtime_one_shot_direct.cpp','pr\pr_ss0_state16_runtime_envelope_import_direct.cpp','pr\pr_ss0_transition_direct.cpp','pr\pr_ss0_directory_dispatcher_direct.cpp','pr\pr_ss0_resource_audio_direct.cpp','pr\pr_ss0_title_render_direct.cpp','pr\pr_ss0_title_tmd_backend.cpp','pr\pr_ss0_str_lifecycle_direct.cpp','pr\pr_ss0_title_entry_prefix_direct.cpp','pr\pr_ss0_title_hud_events_direct.cpp','pr\pr_ss0_card_memcard_handoff_direct.cpp','pr\pr_scene_drawbuffer_direct.cpp','pr\pr_scene_boot_worklist_direct.cpp','pr\pr_scene_bootstrap_direct.cpp','pr\pr_scene_entry_direct.cpp','pr\pr_scene_entry_card_feedback_direct.cpp','pr\pr_scene_entry_feedback_adapter_direct.cpp','pr\pr_scene_entry_executor_direct.cpp','pr\pr_scene1_entry_original_disc_direct.cpp','pr\pr_movie_segment_direct.cpp','pr\pr_stage1_movie_segment_direct.cpp','pr\pr_stage1_scene1_movie1_direct.cpp','pr\pr_stage1_scene1_draw_backend.cpp','pr\pr_stage1_vtext_direct.cpp','pr\pr_stage1_live_hud.cpp','pr\pr_stage1_p2_scorer_hud.cpp','pr\pr_stage1_hd_subtitles.cpp','pr\pr_stage1_texture_replacements.cpp','pr\pr_stage_runner.cpp','pr\pr_stage_runner_direct.cpp',
  'pr\pr_ss0_mdec_vlc_direct.cpp',
  'pr\pr_ss0_mdec_output_direct.cpp',
  'pr\pr_ss0_scene0_int_renderer_direct.cpp',
  'pr\pr_ss0_title_initial_clock_direct.cpp',
  'pr\pr_ss0_word800916f0_direct.cpp',
  'pr\pr_ss0_title_transform_direct.cpp',
  'pr\pr_ss0_title_packet_work_direct.cpp',
  'pr\pr_ss0_title_draw_backend.cpp',
  'pr\pr_ss0_title_draw_desc_direct.cpp',
  'pr\pr_ss0_title_packet_render_direct.cpp',
  'pr\pr_ss0_title_packet_commit_direct.cpp',
  'pr\pr_ss0_title_packet_plan_direct.cpp',
  'pr\pr_ss0_title_primitive_group_direct.cpp',
  'pr\pr_ss0_stage_progress_bank_direct.cpp',
  'pr\pr_ss0_event_frame_loop_direct.cpp',
  'pr\pr_ss0_event_text_direct.cpp',
  'pr\pr_ss0_practice_lifecycle_direct.cpp',
  'pr\pr_ss0_directory_pages_render_direct.cpp',
  'pr\pr_ss0_event_backdrop_render_direct.cpp',
  'pr\pr_ss0_card_info_render_direct.cpp',
  'pr\pr_ss0_card_io_banner_render_direct.cpp',
  'pr\pr_ss0_hiscore_render_direct.cpp',
  'pr\pr_ss0_prompt_card_render_direct.cpp',
  'pr\pr_ss0_event4_prompt_render_direct.cpp',
  'pr\pr_pad.cpp','pr\pr_timer.cpp','pr\pr_cd.cpp','pr\pr_mem.cpp',
  'pr\pr_beat_chart.cpp','pr\pr_psx_sprite_template_render.cpp','pr\pr_ui_overlay.cpp','pr\pr_tmd.cpp',
  'pr\pr_tmd_renderer.cpp','pr\scene_event_parser.cpp','pr\pr_stage1_overlay_parser.cpp','pr\face_event_processor.cpp',
  'pr\pr_vram_atlas.cpp','pr\pr_mime.cpp','pr\pr_transition.cpp',
  'pr\pr_sfx.cpp','pr\pr_vtext.cpp',
  'debug_server.cpp','str_player.cpp','str_parser.cpp','mdec_decoder.cpp',
  'xa_decoder.cpp','xa1_player.cpp','audio_output.cpp','wasapi_sink.cpp',
  'audio_engine.cpp','vab_player.cpp'
)

# --- Compile ---
Write-Host ("[BUILD] Mode: " + ($(if ($fullRebuild) { 'FULL rebuild (recommended)' } else { 'FAST incremental' }))) -ForegroundColor Cyan
Write-Host '[BUILD] Compiling...' -ForegroundColor Cyan
$fail = $false
# The translated PSX graph owner is intentionally large and can exceed five
# minutes on a cold full rebuild.  Keep a bounded timeout, but do not classify
# a CPU-active full compile as failed before that source has a chance to finish.
$timeoutMs = 900000
foreach ($s in $sources) {
    $srcFile = "$src\$s"
    $objName = [System.IO.Path]::GetFileNameWithoutExtension($s)
    $objFile = "$objDir\$objName.obj"
    $clOutFile = "$objDir\cl_out_$objName.txt"
    $clErrFile = "$objDir\cl_err_$objName.txt"
    Write-Host "  $s" -NoNewline

    if (-not $fullRebuild -and (Test-Path $objFile) -and (Test-Path $srcFile)) {
        $srcTime = (Get-Item $srcFile).LastWriteTimeUtc
        $objTime = (Get-Item $objFile).LastWriteTimeUtc
        if ($objTime -ge $srcTime) {
            Write-Host ' [SKIP]' -ForegroundColor DarkGray
            continue
        }
    }

    if (Test-Path $objFile) {
        Remove-Item $objFile -Force -ErrorAction SilentlyContinue
    }
    if (Test-Path $clOutFile) {
        Remove-Item $clOutFile -Force -ErrorAction SilentlyContinue
    }
    if (Test-Path $clErrFile) {
        Remove-Item $clErrFile -Force -ErrorAction SilentlyContinue
    }

    $p = Start-CompileProcess `
        -CompilerPath "$msvc\bin\Hostx64\x64\cl.exe" `
        -Arguments "/nologo /c /std:c++17 /utf-8 /EHsc /O2 /Z7 /DNDEBUG /DWIN32 /D_WINDOWS /W3 /wd4819 /MD /I`"$src`" `"$srcFile`" /Fo`"$objFile`"" `
        -StdOutPath $clOutFile `
        -StdErrPath $clErrFile

    if (-not $p.WaitForExit($timeoutMs)) {
        Write-Host ' [TIMEOUT]' -ForegroundColor Red
        Stop-Process -Id $p.Id -Force -ErrorAction SilentlyContinue
        Stop-Process -Name 'mspdbsrv' -Force -ErrorAction SilentlyContinue
        $fail = $true
        continue
    }

    $p.Refresh()
    $exitCode = $p.ExitCode
    if ($p.HasExited -and $null -eq $exitCode) { $exitCode = 0 }

    $compileArtifactHealthy = Test-CompileArtifactHealthy `
        -SrcFile $srcFile `
        -ObjFile $objFile `
        -StdOutPath $clOutFile `
        -StdErrPath $clErrFile
    if ($exitCode -eq 0) {
        if (-not $compileArtifactHealthy) {
            $exitCode = 1
        }
    } elseif ($compileArtifactHealthy) {
        $exitCode = 0
    }

    if ($exitCode -ne 0) {
        $clOut = Get-Content $clOutFile -Raw -ErrorAction SilentlyContinue
        if ($clOut -match 'Permission denied') {
            Stop-Process -Name 'cl' -Force -ErrorAction SilentlyContinue
            Stop-Process -Name 'mspdbsrv' -Force -ErrorAction SilentlyContinue
            if (Test-Path $objFile) {
                cmd /c "attrib -R \"$objFile\"" | Out-Null
                Remove-Item $objFile -Force -ErrorAction SilentlyContinue
            }
            Start-Sleep -Milliseconds 200

            $p = Start-CompileProcess `
                -CompilerPath "$msvc\bin\Hostx64\x64\cl.exe" `
                -Arguments "/nologo /c /std:c++17 /utf-8 /EHsc /O2 /Z7 /DNDEBUG /DWIN32 /D_WINDOWS /W3 /wd4819 /MD /I`"$src`" `"$srcFile`" /Fo`"$objFile`"" `
                -StdOutPath $clOutFile `
                -StdErrPath $clErrFile

            if (-not $p.WaitForExit($timeoutMs)) {
                Write-Host ' [TIMEOUT]' -ForegroundColor Red
                Stop-Process -Id $p.Id -Force -ErrorAction SilentlyContinue
                Stop-Process -Name 'mspdbsrv' -Force -ErrorAction SilentlyContinue
                $fail = $true
                continue
            }

            $p.Refresh()
            $exitCode = $p.ExitCode
            if ($p.HasExited -and $null -eq $exitCode) { $exitCode = 0 }
            $compileArtifactHealthy = Test-CompileArtifactHealthy `
                -SrcFile $srcFile `
                -ObjFile $objFile `
                -StdOutPath $clOutFile `
                -StdErrPath $clErrFile
            if ($exitCode -eq 0) {
                if (-not $compileArtifactHealthy) {
                    $exitCode = 1
                }
            } elseif ($compileArtifactHealthy) {
                $exitCode = 0
            }
        }

        if ($exitCode -ne 0) {
            Write-Host ' [FAILED]' -ForegroundColor Red
            if (Test-Path $clOutFile) {
                Get-Content $clOutFile -Tail 40 | ForEach-Object { Write-Host "    $_" -ForegroundColor DarkGray }
            }
            if (Test-Path $clErrFile) {
                Get-Content $clErrFile -Tail 80 | ForEach-Object { Write-Host "    $_" -ForegroundColor Red }
            }
            $fail = $true
        } else {
            Write-Host ' [OK]' -ForegroundColor Green
        }
    } else {
        Write-Host ' [OK]' -ForegroundColor Green
    }
}

if ($fail) {
    Write-Host '[BUILD] COMPILE FAILED' -ForegroundColor Red
    exit 1
}

# --- Link ---
Write-Host '[BUILD] Linking...' -ForegroundColor Cyan
# The user or tools may relaunch PaRappaWin while the full rebuild is still
# compiling. Kill it again immediately before link so writing the root exe
# doesn't fail on a stale handle.
Stop-Process -Name 'PaRappaWin' -Force -ErrorAction SilentlyContinue
Start-Sleep -Milliseconds 500
$expectedObjPaths = foreach ($s in $sources) {
    $objName = [System.IO.Path]::GetFileNameWithoutExtension($s)
    Join-Path $objDir "$objName.obj"
}
$linkInputWaitDeadline = (Get-Date).AddSeconds(10)
while ($true) {
    $missingObjPaths = @($expectedObjPaths | Where-Object { -not (Test-Path $_) })
    if ($missingObjPaths.Count -eq 0) {
        break
    }
    if ((Get-Date) -ge $linkInputWaitDeadline) {
        Write-Host '[BUILD] LINK INPUTS INCOMPLETE' -ForegroundColor Red
        $missingObjPaths | ForEach-Object { Write-Host "  missing: $_" -ForegroundColor Red }
        exit 1
    }
    Start-Sleep -Milliseconds 250
}
$objFiles = $expectedObjPaths | Sort-Object | ForEach-Object { "`"$_`"" }
$linkArgs = @(
    '/NOLOGO',
    "/OUT:`"$outExe`"",
    "/PDB:`"$outPdb`"",
    '/SUBSYSTEM:WINDOWS',
    '/MACHINE:X64',
    '/STACK:8388608',
    '/DEBUG',
    '/OPT:REF',
    '/OPT:ICF'
) + $objFiles + @(
    'd3d11.lib',
    'dxgi.lib',
    'd3dcompiler.lib',
    'shell32.lib',
    'windowscodecs.lib',
    'ws2_32.lib',
    'kernel32.lib',
    'user32.lib',
    'gdi32.lib',
    'winspool.lib',
    'ole32.lib',
    'oleaut32.lib',
    'uuid.lib',
    'comdlg32.lib',
    'advapi32.lib'
)

$linkRsp = "$objDir\\link.rsp"
$linkOut = "$objDir\\link_out.txt"
$linkErr = "$objDir\\link_err.txt"
if (Test-Path $linkRsp) {
    Remove-Item $linkRsp -Force -ErrorAction SilentlyContinue
}
if (Test-Path $linkOut) {
    Remove-Item $linkOut -Force -ErrorAction SilentlyContinue
}
if (Test-Path $linkErr) {
    Remove-Item $linkErr -Force -ErrorAction SilentlyContinue
}
[System.IO.File]::WriteAllLines($linkRsp, $linkArgs)

function Invoke-LinkStep {
    param(
        [string]$RspPath,
        [string]$OutPath,
        [string]$ErrPath
    )
    return Start-Process -FilePath "$msvc\bin\Hostx64\x64\link.exe" `
        -ArgumentList "@`"$RspPath`"" -NoNewWindow -Wait -PassThru `
        -RedirectStandardOutput $OutPath `
        -RedirectStandardError $ErrPath
}

$p = Invoke-LinkStep -RspPath $linkRsp -OutPath $linkOut -ErrPath $linkErr

if ($p.ExitCode -ne 0) {
    $linkOutText = if (Test-Path $linkOut) {
        Get-Content $linkOut -Raw -ErrorAction SilentlyContinue
    } else {
        ''
    }
    $linkErrText = if (Test-Path $linkErr) {
        Get-Content $linkErr -Raw -ErrorAction SilentlyContinue
    } else {
        ''
    }
    $retryLinkForFileLock =
        ($linkOutText -match 'LNK1104' -or $linkErrText -match 'LNK1104')
    if ($retryLinkForFileLock) {
        Stop-Process -Name 'PaRappaWin' -Force -ErrorAction SilentlyContinue
        Stop-Process -Name 'cl' -Force -ErrorAction SilentlyContinue
        Stop-Process -Name 'mspdbsrv' -Force -ErrorAction SilentlyContinue
        Start-Sleep -Milliseconds 800
        if (Test-Path $linkOut) {
            Remove-Item $linkOut -Force -ErrorAction SilentlyContinue
        }
        if (Test-Path $linkErr) {
            Remove-Item $linkErr -Force -ErrorAction SilentlyContinue
        }
        $p = Invoke-LinkStep -RspPath $linkRsp -OutPath $linkOut -ErrPath $linkErr
        if ($p.ExitCode -eq 0) {
            $linkOutText = ''
            $linkErrText = ''
        } else {
            $linkOutText = if (Test-Path $linkOut) {
                Get-Content $linkOut -Raw -ErrorAction SilentlyContinue
            } else {
                ''
            }
            $linkErrText = if (Test-Path $linkErr) {
                Get-Content $linkErr -Raw -ErrorAction SilentlyContinue
            } else {
                ''
            }
        }
    }
    if ($p.ExitCode -ne 0) {
        Write-Host '[BUILD] LINK FAILED' -ForegroundColor Red
        if ($linkOutText) {
            $linkOutText -split "`r?`n" | Where-Object { $_ -ne '' } | ForEach-Object { Write-Host $_ }
        }
        if ($linkErrText) {
            $linkErrText -split "`r?`n" | Where-Object { $_ -ne '' } | ForEach-Object { Write-Host $_ }
        }
        exit 1
    }
}

# --- Copy to bin ---
$binDir = "$scriptDir\bin"
if (!(Test-Path $binDir)) { New-Item -ItemType Directory -Path $binDir -Force | Out-Null }
Copy-Item $outExe "$binDir\" -Force -ErrorAction SilentlyContinue
Copy-Item $outPdb "$binDir\" -Force -ErrorAction SilentlyContinue

Write-Host ''
Write-Host "[BUILD] === SUCCESS === Output: $outExe" -ForegroundColor Green
exit 0
