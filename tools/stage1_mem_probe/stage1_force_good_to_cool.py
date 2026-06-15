#!/usr/bin/env python3
"""
External Stage1 GOOD->COOL trigger sidecar.

This is intentionally not linked into PaRappaWin and should remain a detachable
debug aid. It writes a small set of PDB-verified direct-runtime facts into a
running PaRappaWin.exe process to force the same event-stream input shape that
the scorer publishes when GOOD commits to COOL.

Use only for reproducing/capturing the COOL transition when manual play is not
available. Do not treat a forced run as gameplay-authority evidence.
"""

from __future__ import annotations

import argparse
import csv
import ctypes
import ctypes.wintypes as wt
import json
import re
import struct
import subprocess
import sys
import time
from pathlib import Path
from typing import Any


TOOL_DIR = Path(__file__).resolve().parent
REPO_DIR = TOOL_DIR.parents[1]
sys.path.insert(0, str(REPO_DIR))

from debug_client import DebugClient  # noqa: E402


DEFAULT_PROCESS = "PaRappaWin.exe"

PROCESS_QUERY_INFORMATION = 0x0400
PROCESS_VM_OPERATION = 0x0008
PROCESS_VM_READ = 0x0010
PROCESS_VM_WRITE = 0x0020
TH32CS_SNAPMODULE = 0x00000008
TH32CS_SNAPMODULE32 = 0x00000010
INVALID_HANDLE_VALUE = wt.HANDLE(-1).value

S_STAGE1_NUMERIC_RUNTIME_RVA = 0x00540350
S_STAGE1_EVENT_STREAM_RUNTIME_RVA = 0x0052B390
S_STAGE1_SCENE_SUBMIT_RUNTIME_RVA = 0x00592928

OFF_ACTIVE = 0
OFF_QUERY_FRAME = 8
OFF_WORD9182A_ENABLED = 26
OFF_DESCRIPTOR_SUBSTATE50 = 27
OFF_DESCRIPTOR_FLAG_WORD_KNOWN = 28
OFF_DESCRIPTOR_FLAG_WORD = 30
OFF_BUCKET30_RESOLUTION_KNOWN = 32
OFF_BUCKET30_RESOLUTION_V22 = 33
OFF_BUCKET30_WRITEBACK_COMMITTED = 34
OFF_LAST_ROW_WRITE_KNOWN = 35
OFF_LAST_ROW_WRITE_QUERY_FRAME = 36
OFF_LAST_ROW_WRITE_PREV_ROW = 40
OFF_LAST_ROW_WRITE_RESOLVED_ROW = 41
OFF_LAST_ROW_WRITE_RESOLUTION_KNOWN = 42
OFF_LAST_ROW_WRITE_RESOLUTION_V22 = 43
OFF_LAST_ROW_WRITE_COMMITTED = 44
OFF_RIGHT_RANK_STATE = 72
OFF_RIGHT_RANK_ACTIVE_ROW = OFF_RIGHT_RANK_STATE + 0
OFF_RIGHT_RANK_BLINK_TARGET_ROW = OFF_RIGHT_RANK_STATE + 1
OFF_RIGHT_RANK_BLINK_ENABLED = OFF_RIGHT_RANK_STATE + 2
OFF_RIGHT_RANK_FOLLOW_UP_PHASE = 96
OFF_SCORER_PORT = 7352
OFF_SCORER_ACCEPTED_COUNT_91810 = OFF_SCORER_PORT + 0
OFF_SCORER_RECORDED_HIT_COUNT_91812 = OFF_SCORER_PORT + 2
OFF_SCORER_PENALTY_SIDE_COUNT_91814 = OFF_SCORER_PORT + 4
OFF_SCORER_ACCUMULATOR_91816 = OFF_SCORER_PORT + 8
OFF_SCORER_ADDITIVE_TERM_91822 = OFF_SCORER_PORT + 12
OFF_SCORER_RECORD_COMPANION_91824 = OFF_SCORER_PORT + 14
OFF_SCORER_ACCEPTED_MASK_91808 = OFF_SCORER_PORT + 16
OFF_SCORER_LAST_CLEARED_ACCEPTED_MASK_9180C = OFF_SCORER_PORT + 20
OFF_RIGHT_RANK_PHASE1_OWNER = 8920
OFF_PHASE1_BASELINE18 = OFF_RIGHT_RANK_PHASE1_OWNER + 0
OFF_PHASE1_SHORT_WINDOW_LATCH34_KNOWN = OFF_RIGHT_RANK_PHASE1_OWNER + 4
OFF_PHASE1_SHORT_WINDOW_LATCH34 = OFF_RIGHT_RANK_PHASE1_OWNER + 6
OFF_PHASE1_CACHED_CLASSIFIER36 = OFF_RIGHT_RANK_PHASE1_OWNER + 8
OFF_PHASE1_LATCH_ARMED38 = OFF_RIGHT_RANK_PHASE1_OWNER + 9
OFF_RIGHT_RANK_TIE_BREAKER = 8944
OFF_TIE_BREAKER_SNAPSHOT1A = OFF_RIGHT_RANK_TIE_BREAKER + 0
OFF_RIGHT_RANK_HELPER_SHADOW = 8956
OFF_HELPER_SNAPSHOT1C = OFF_RIGHT_RANK_HELPER_SHADOW + 0
OFF_HELPER_BUCKET0_CALL_WINDOW_OPEN = OFF_RIGHT_RANK_HELPER_SHADOW + 4
OFF_HELPER_BUCKET0_CTX118_WRITE_PULSE = OFF_RIGHT_RANK_HELPER_SHADOW + 5
OFF_HELPER_TIE_CARRY_LATCH1E = OFF_RIGHT_RANK_HELPER_SHADOW + 6
OFF_HELPER_NO_INPUT_COUNTER20 = OFF_RIGHT_RANK_HELPER_SHADOW + 8
OFF_HELPER_COOL_GAIN_LATCH26 = OFF_RIGHT_RANK_HELPER_SHADOW + 10
OFF_HELPER_COOL_GAIN_STREAK28 = OFF_RIGHT_RANK_HELPER_SHADOW + 12
OFF_RIGHT_RANK_BUCKET_CONTEXT = 8948
OFF_BUCKET_CTX54_PERMIT = OFF_RIGHT_RANK_BUCKET_CONTEXT + 0
OFF_BUCKET_CTX6A_CONSUMER_GATE = OFF_RIGHT_RANK_BUCKET_CONTEXT + 1
OFF_BUCKET_CTX7A = OFF_RIGHT_RANK_BUCKET_CONTEXT + 2
OFF_BUCKET_CTX74 = OFF_RIGHT_RANK_BUCKET_CONTEXT + 4
OFF_BUCKET_TRANSITION_ANIM18E = OFF_RIGHT_RANK_BUCKET_CONTEXT + 6
OFF_BUCKET30_OWNER_OBSERVER = 8972
OFF_BUCKET30_OWNER_SCORER_COMMIT_WINDOW_OPEN = OFF_BUCKET30_OWNER_OBSERVER + 1
OFF_BUCKET30_OWNER_KERNEL_OPEN = OFF_BUCKET30_OWNER_OBSERVER + 2
OFF_BUCKET30_OWNER_KERNEL_ENTERED = OFF_BUCKET30_OWNER_OBSERVER + 3
OFF_BUCKET30_OWNER_DESCRIPTOR_FLAG_WORD = OFF_BUCKET30_OWNER_OBSERVER + 6
OFF_BUCKET30_OWNER_ACTIVE_ROW = OFF_BUCKET30_OWNER_OBSERVER + 8
OFF_BUCKET30_OWNER_RESOLUTION_CALLED = OFF_BUCKET30_OWNER_OBSERVER + 198
OFF_BUCKET30_OWNER_RESOLUTION_KNOWN = OFF_BUCKET30_OWNER_OBSERVER + 199
OFF_BUCKET30_OWNER_RESOLUTION_INPUT_CLASSIFIER = OFF_BUCKET30_OWNER_OBSERVER + 200
OFF_BUCKET30_OWNER_RESOLUTION_V22 = OFF_BUCKET30_OWNER_OBSERVER + 201
OFF_BUCKET30_OWNER_GOOD_TO_COOL_COMMITTED = OFF_BUCKET30_OWNER_OBSERVER + 202
OFF_RIGHT_RANK_DIRECT_FOLLOW_UP = 9192
OFF_FOLLOW_UP_CTX72 = OFF_RIGHT_RANK_DIRECT_FOLLOW_UP + 0
OFF_FOLLOW_UP_ED24 = OFF_RIGHT_RANK_DIRECT_FOLLOW_UP + 4
OFF_FOLLOW_UP_ED00 = OFF_RIGHT_RANK_DIRECT_FOLLOW_UP + 8
OFF_FOLLOW_UP_ED14 = OFF_RIGHT_RANK_DIRECT_FOLLOW_UP + 12
OFF_FOLLOW_UP_ED0C = OFF_RIGHT_RANK_DIRECT_FOLLOW_UP + 16
OFF_FOLLOW_UP_FLAG2000 = OFF_RIGHT_RANK_DIRECT_FOLLOW_UP + 31
OFF_FOLLOW_UP_FLAG4000 = OFF_RIGHT_RANK_DIRECT_FOLLOW_UP + 32
OFF_RIGHT_RANK_FORCED_GOOD_EVENT_STREAM_DONE = 9268
OFF_PSX_EVENT_STREAM_FLAG_KNOWN = 9269
OFF_PSX_EVENT_STREAM_ACTIVE = 9270
OFF_PSX_EVENT_STREAM_ID_KNOWN = 9271
OFF_PSX_EVENT_STREAM_ID = 9272
OFF_PSX_CTX_FLAGS_KNOWN = 9273
OFF_PSX_CTX_FLAGS = 9274

OFF_EV_FLAGS40_2000_4000 = 0
OFF_EV_CTX72 = 8
OFF_EV_SELECTED_STREAM = 32
OFF_EV_G_EVENT_STREAM_FLAG = 34
OFF_EV_G_EVENT_STREAM_ID = 36
OFF_EV_G_EVENT_STREAM_DONE = 116
OFF_EV_DESC_CURSOR_ARRAY = 120
OFF_EV_EVENT_STREAM_FLAG_ACTIVE = 416
OFF_EV_EVENT_STREAM_FLAG_STREAM = 417
OFF_EV_ACTIVE_DISPATCH_STREAM = 418
OFF_EV_ACTIVE_DISPATCH_START_FRAME = 420
OFF_EV_ACTIVE_DISPATCH_START_SCRIPT_FRAME = 424
OFF_EV_ACTIVE_DISPATCH_END_LOCAL_FRAME = 428
OFF_EV_ACTIVE_DISPATCH_TERMINAL_END_LOCAL_FRAME = 432
OFF_EV_ACTIVE_DISPATCH_TERMINAL_PULSE_EMITTED = 436
OFF_EV_ACTIVE_DISPATCH_DONE_PULSE_EMITTED = 437
OFF_EV_DWORD801D3048 = 440
OFF_EV_LAST_STARTED_STREAM = 444
OFF_EV_LAST_STARTED_QUERY_FRAME = 448
OFF_EV_EVENT_STREAM_DONE_PENDING = 454
OFF_EV_EVENT_STREAM_DONE_PENDING_SOURCE_STREAM = 455
OFF_EV_EVENT_STREAM_DONE_PULSE = 456
OFF_EV_EVENT_STREAM_DONE_SOURCE_STREAM = 457
OFF_EV_FLAG2000_SCENE_FAMILY_ACTIVE = 472
OFF_EV_FLAG2000_SCENE_FAMILY_STREAM = 473
OFF_EV_FLAG2000_SCENE_FAMILY_START_FRAME = 476
OFF_EV_FLAG2000_SCENE_FAMILY_START_SCRIPT_FRAME = 480
OFF_EV_FLAGS2000_LAST_DISPATCHED_STREAM = 498
OFF_EV_PREV_FLAG2000 = 501

OFF_RUNTIME_IMPL_PTR = 0

OFF_IMPL_CAMERA_BEZ_PLAYER_801CBFDC = 20620
OFF_IMPL_CAMERA_AUTHORITY_801CBFDC = 34004
OFF_IMPL_CAMERA_PRESENTATION_801CBFDC = 34048
OFF_IMPL_CAMERA_BEZ_ID_801CBFDC = 34092
OFF_IMPL_CAMERA_SOURCE_EVENT_801CBFDC = 34096
OFF_IMPL_CAMERA_USING_NEUTRAL_801CBFDC = 34100
OFF_IMPL_CAMERA_ADVANCE_VALID_801CBFDC = 34101
OFF_IMPL_LAST_CAMERA_QUERY_FRAME_801CBFDC = 34104
OFF_IMPL_PSX801CBFDC = 258204

OFF_PSX801_DWORD_801D302C = 84
OFF_PSX801_DWORD_801D3034 = 88

OFF_CAMERA_BEZ_TOTAL_ENTRIES = 13312
OFF_CAMERA_BEZ_CURRENT_FRAME = 13316
OFF_CAMERA_BEZ_TICK_DIVISION = 13320
OFF_CAMERA_BEZ_POS_MODE = 13324
OFF_CAMERA_BEZ_TGT_MODE = 13328
OFF_CAMERA_BEZ_POS_VEL = 13332
OFF_CAMERA_BEZ_TGT_VEL = 13344
OFF_CAMERA_BEZ_OUT_POS = 13356
OFF_CAMERA_BEZ_OUT_TGT = 13368
OFF_CAMERA_BEZ_LOADED = 13380
OFF_CAMERA_BEZ_ACTIVE = 13381
OFF_CAMERA_BEZ_TRUE_CUBIC = 13382

OFF_CAMERA_STATE_VALID = 0
OFF_CAMERA_STATE_POS = 4
OFF_CAMERA_STATE_TARGET = 16
OFF_CAMERA_STATE_TWIST = 28
OFF_CAMERA_STATE_PROJECTION_DISTANCE = 32
OFF_CAMERA_STATE_SCREEN_CENTER_X = 36
OFF_CAMERA_STATE_SCREEN_CENTER_Y = 40


class MODULEENTRY32W(ctypes.Structure):
    _fields_ = [
        ("dwSize", wt.DWORD),
        ("th32ModuleID", wt.DWORD),
        ("th32ProcessID", wt.DWORD),
        ("GlblcntUsage", wt.DWORD),
        ("ProccntUsage", wt.DWORD),
        ("modBaseAddr", ctypes.POINTER(wt.BYTE)),
        ("modBaseSize", wt.DWORD),
        ("hModule", wt.HMODULE),
        ("szModule", wt.WCHAR * 256),
        ("szExePath", wt.WCHAR * 260),
    ]


kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)
kernel32.OpenProcess.argtypes = [wt.DWORD, wt.BOOL, wt.DWORD]
kernel32.OpenProcess.restype = wt.HANDLE
kernel32.CloseHandle.argtypes = [wt.HANDLE]
kernel32.CloseHandle.restype = wt.BOOL
kernel32.ReadProcessMemory.argtypes = [
    wt.HANDLE,
    wt.LPCVOID,
    wt.LPVOID,
    ctypes.c_size_t,
    ctypes.POINTER(ctypes.c_size_t),
]
kernel32.ReadProcessMemory.restype = wt.BOOL
kernel32.WriteProcessMemory.argtypes = [
    wt.HANDLE,
    wt.LPVOID,
    wt.LPCVOID,
    ctypes.c_size_t,
    ctypes.POINTER(ctypes.c_size_t),
]
kernel32.WriteProcessMemory.restype = wt.BOOL
kernel32.CreateToolhelp32Snapshot.argtypes = [wt.DWORD, wt.DWORD]
kernel32.CreateToolhelp32Snapshot.restype = wt.HANDLE
kernel32.Module32FirstW.argtypes = [wt.HANDLE, ctypes.POINTER(MODULEENTRY32W)]
kernel32.Module32FirstW.restype = wt.BOOL
kernel32.Module32NextW.argtypes = [wt.HANDLE, ctypes.POINTER(MODULEENTRY32W)]
kernel32.Module32NextW.restype = wt.BOOL


def _find_pids_by_image(image_name: str) -> list[int]:
    try:
        out = subprocess.check_output(
            ["tasklist", "/FI", f"IMAGENAME eq {image_name}", "/FO", "CSV", "/NH"],
            stderr=subprocess.DEVNULL,
        )
    except Exception:
        return []
    rows = csv.reader(out.decode("utf-8", errors="replace").splitlines())
    pids: list[int] = []
    for row in rows:
        if len(row) < 2:
            continue
        if row[0].strip('"').lower() != image_name.lower():
            continue
        try:
            pids.append(int(row[1].strip('"')))
        except ValueError:
            pass
    return pids


def _open_process(pid: int) -> int:
    access = (
        PROCESS_QUERY_INFORMATION
        | PROCESS_VM_READ
        | PROCESS_VM_WRITE
        | PROCESS_VM_OPERATION
    )
    handle = kernel32.OpenProcess(access, False, pid)
    if not handle:
        err = ctypes.get_last_error()
        raise OSError(err, f"OpenProcess failed for pid {pid}")
    return int(handle)


def _module_bases(pid: int) -> dict[str, int]:
    snap = kernel32.CreateToolhelp32Snapshot(
        TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32,
        pid,
    )
    if int(snap) == INVALID_HANDLE_VALUE:
        err = ctypes.get_last_error()
        raise OSError(err, f"CreateToolhelp32Snapshot failed for pid {pid}")
    try:
        entry = MODULEENTRY32W()
        entry.dwSize = ctypes.sizeof(MODULEENTRY32W)
        bases: dict[str, int] = {}
        ok = kernel32.Module32FirstW(snap, ctypes.byref(entry))
        while ok:
            name = str(entry.szModule).lower()
            bases[name] = ctypes.cast(entry.modBaseAddr, ctypes.c_void_p).value or 0
            ok = kernel32.Module32NextW(snap, ctypes.byref(entry))
        return bases
    finally:
        kernel32.CloseHandle(snap)


def _read_bytes(handle: int, address: int, size: int) -> bytes:
    buf = (ctypes.c_ubyte * size)()
    read = ctypes.c_size_t(0)
    ok = kernel32.ReadProcessMemory(
        wt.HANDLE(handle),
        wt.LPCVOID(address),
        ctypes.byref(buf),
        size,
        ctypes.byref(read),
    )
    if not ok or read.value != size:
        err = ctypes.get_last_error()
        raise OSError(err, f"ReadProcessMemory failed at 0x{address:X} size={size}")
    return bytes(buf)


def _write_bytes(handle: int, address: int, data: bytes) -> None:
    buf = (ctypes.c_ubyte * len(data)).from_buffer_copy(data)
    written = ctypes.c_size_t(0)
    ok = kernel32.WriteProcessMemory(
        wt.HANDLE(handle),
        wt.LPVOID(address),
        ctypes.byref(buf),
        len(data),
        ctypes.byref(written),
    )
    if not ok or written.value != len(data):
        err = ctypes.get_last_error()
        raise OSError(err, f"WriteProcessMemory failed at 0x{address:X} size={len(data)}")


def _read_u8(handle: int, address: int) -> int:
    return _read_bytes(handle, address, 1)[0]


def _read_u16(handle: int, address: int) -> int:
    return struct.unpack("<H", _read_bytes(handle, address, 2))[0]


def _read_i16(handle: int, address: int) -> int:
    return struct.unpack("<h", _read_bytes(handle, address, 2))[0]


def _read_i32(handle: int, address: int) -> int:
    return struct.unpack("<i", _read_bytes(handle, address, 4))[0]


def _read_u32(handle: int, address: int) -> int:
    return struct.unpack("<I", _read_bytes(handle, address, 4))[0]


def _read_f32(handle: int, address: int) -> float:
    return struct.unpack("<f", _read_bytes(handle, address, 4))[0]


def _read_ptr(handle: int, address: int) -> int:
    return struct.unpack("<Q", _read_bytes(handle, address, 8))[0]


def _write_u8(handle: int, address: int, value: int) -> None:
    _write_bytes(handle, address, struct.pack("<B", value & 0xFF))


def _write_u16(handle: int, address: int, value: int) -> None:
    _write_bytes(handle, address, struct.pack("<H", value & 0xFFFF))


def _write_i16(handle: int, address: int, value: int) -> None:
    _write_bytes(handle, address, struct.pack("<h", int(value)))


def _write_i32(handle: int, address: int, value: int) -> None:
    _write_bytes(handle, address, struct.pack("<i", int(value)))


def _write_u32(handle: int, address: int, value: int) -> None:
    _write_bytes(handle, address, struct.pack("<I", value & 0xFFFFFFFF))


def _sample_numeric(handle: int, base: int) -> dict[str, Any]:
    root = base + S_STAGE1_NUMERIC_RUNTIME_RVA
    return {
        "active": _read_u8(handle, root + OFF_ACTIVE),
        "queryFrame": _read_u32(handle, root + OFF_QUERY_FRAME),
        "word9182AEnabled": _read_u8(handle, root + OFF_WORD9182A_ENABLED),
        "descriptorSubstate50": _read_u8(handle, root + OFF_DESCRIPTOR_SUBSTATE50),
        "rightRankDescriptorFlagWord08Known": _read_u8(
            handle, root + OFF_DESCRIPTOR_FLAG_WORD_KNOWN
        ),
        "rightRankDescriptorFlagWord08": _read_u16(
            handle, root + OFF_DESCRIPTOR_FLAG_WORD
        ),
        "bucket30ResolutionKnown": _read_u8(
            handle, root + OFF_BUCKET30_RESOLUTION_KNOWN
        ),
        "bucket30ResolutionV22": _read_u8(handle, root + OFF_BUCKET30_RESOLUTION_V22),
        "bucket30WritebackCommitted": _read_u8(
            handle, root + OFF_BUCKET30_WRITEBACK_COMMITTED
        ),
        "lastRowWriteKnown": _read_u8(handle, root + OFF_LAST_ROW_WRITE_KNOWN),
        "lastRowWriteQueryFrame": _read_i32(
            handle, root + OFF_LAST_ROW_WRITE_QUERY_FRAME
        ),
        "lastRowWritePrevRow": _read_u8(handle, root + OFF_LAST_ROW_WRITE_PREV_ROW),
        "lastRowWriteResolvedRow": _read_u8(
            handle, root + OFF_LAST_ROW_WRITE_RESOLVED_ROW
        ),
        "lastRowWriteResolutionKnown": _read_u8(
            handle, root + OFF_LAST_ROW_WRITE_RESOLUTION_KNOWN
        ),
        "lastRowWriteResolutionV22": _read_u8(
            handle, root + OFF_LAST_ROW_WRITE_RESOLUTION_V22
        ),
        "lastRowWriteCommitted": _read_u8(
            handle, root + OFF_LAST_ROW_WRITE_COMMITTED
        ),
        "rightRankActiveRow": _read_u8(handle, root + OFF_RIGHT_RANK_ACTIVE_ROW),
        "rightRankBlinkTargetRow": _read_u8(
            handle, root + OFF_RIGHT_RANK_BLINK_TARGET_ROW
        ),
        "rightRankBlinkEnabled": _read_u8(
            handle, root + OFF_RIGHT_RANK_BLINK_ENABLED
        ),
        "rightRankFollowUpPhase": _read_u8(handle, root + OFF_RIGHT_RANK_FOLLOW_UP_PHASE),
        "scorerAcceptedCount91810": _read_u16(
            handle, root + OFF_SCORER_ACCEPTED_COUNT_91810
        ),
        "scorerRecordedHitCount91812": _read_u16(
            handle, root + OFF_SCORER_RECORDED_HIT_COUNT_91812
        ),
        "scorerPenaltySideCount91814": _read_u16(
            handle, root + OFF_SCORER_PENALTY_SIDE_COUNT_91814
        ),
        "scorerAccumulator91816": _read_i32(
            handle, root + OFF_SCORER_ACCUMULATOR_91816
        ),
        "scorerAdditiveTerm91822": _read_i16(
            handle, root + OFF_SCORER_ADDITIVE_TERM_91822
        ),
        "scorerRecordCompanion91824": _read_u16(
            handle, root + OFF_SCORER_RECORD_COMPANION_91824
        ),
        "scorerAcceptedMask91808": _read_u32(
            handle, root + OFF_SCORER_ACCEPTED_MASK_91808
        ),
        "scorerLastClearedAcceptedMask9180C": _read_u32(
            handle, root + OFF_SCORER_LAST_CLEARED_ACCEPTED_MASK_9180C
        ),
        "phase1Baseline18": _read_i32(handle, root + OFF_PHASE1_BASELINE18),
        "phase1ShortWindowLatch34Known": _read_u8(
            handle, root + OFF_PHASE1_SHORT_WINDOW_LATCH34_KNOWN
        ),
        "phase1ShortWindowLatch34": _read_u16(
            handle, root + OFF_PHASE1_SHORT_WINDOW_LATCH34
        ),
        "phase1CachedClassifier36": _read_u8(
            handle, root + OFF_PHASE1_CACHED_CLASSIFIER36
        ),
        "phase1LatchArmed38": _read_u8(handle, root + OFF_PHASE1_LATCH_ARMED38),
        "tieBreakerSnapshot1A": _read_i32(
            handle, root + OFF_TIE_BREAKER_SNAPSHOT1A
        ),
        "helperSnapshot1C": _read_i32(handle, root + OFF_HELPER_SNAPSHOT1C),
        "helperBucket0CallWindowOpen": _read_u8(
            handle, root + OFF_HELPER_BUCKET0_CALL_WINDOW_OPEN
        ),
        "helperBucket0Ctx118WritePulse": _read_u8(
            handle, root + OFF_HELPER_BUCKET0_CTX118_WRITE_PULSE
        ),
        "helperTieCarryLatch1E": _read_u16(
            handle, root + OFF_HELPER_TIE_CARRY_LATCH1E
        ),
        "helperNoInputCounter20": _read_u16(
            handle, root + OFF_HELPER_NO_INPUT_COUNTER20
        ),
        "helperCoolGainLatch26": _read_u16(
            handle, root + OFF_HELPER_COOL_GAIN_LATCH26
        ),
        "helperCoolGainStreak28": _read_u16(
            handle, root + OFF_HELPER_COOL_GAIN_STREAK28
        ),
        "ctx72": _read_u16(handle, root + OFF_FOLLOW_UP_CTX72),
        "ed24": _read_i32(handle, root + OFF_FOLLOW_UP_ED24),
        "ed00": _read_i32(handle, root + OFF_FOLLOW_UP_ED00),
        "ed14": _read_i32(handle, root + OFF_FOLLOW_UP_ED14),
        "ed0c": _read_i32(handle, root + OFF_FOLLOW_UP_ED0C),
        "flag2000Pulse": _read_u8(handle, root + OFF_FOLLOW_UP_FLAG2000),
        "flag4000Pulse": _read_u8(handle, root + OFF_FOLLOW_UP_FLAG4000),
        "bucketCtx54Permit": _read_u8(handle, root + OFF_BUCKET_CTX54_PERMIT),
        "bucketCtx6AConsumerGate": _read_u8(
            handle, root + OFF_BUCKET_CTX6A_CONSUMER_GATE
        ),
        "bucketCtx7A": _read_u16(handle, root + OFF_BUCKET_CTX7A),
        "bucketCtx74": _read_u16(handle, root + OFF_BUCKET_CTX74),
        "bucketTransitionAnim18E": _read_u16(
            handle, root + OFF_BUCKET_TRANSITION_ANIM18E
        ),
        "bucket30OwnerScorerCommitWindowOpen": _read_u8(
            handle, root + OFF_BUCKET30_OWNER_SCORER_COMMIT_WINDOW_OPEN
        ),
        "bucket30OwnerKernelOpen": _read_u8(
            handle, root + OFF_BUCKET30_OWNER_KERNEL_OPEN
        ),
        "bucket30OwnerKernelEntered": _read_u8(
            handle, root + OFF_BUCKET30_OWNER_KERNEL_ENTERED
        ),
        "bucket30OwnerDescriptorFlagWord": _read_u16(
            handle, root + OFF_BUCKET30_OWNER_DESCRIPTOR_FLAG_WORD
        ),
        "bucket30OwnerActiveRow": _read_u8(
            handle, root + OFF_BUCKET30_OWNER_ACTIVE_ROW
        ),
        "bucket30OwnerResolutionCalled": _read_u8(
            handle, root + OFF_BUCKET30_OWNER_RESOLUTION_CALLED
        ),
        "bucket30OwnerResolutionKnown": _read_u8(
            handle, root + OFF_BUCKET30_OWNER_RESOLUTION_KNOWN
        ),
        "bucket30OwnerResolutionInputClassifier": _read_u8(
            handle, root + OFF_BUCKET30_OWNER_RESOLUTION_INPUT_CLASSIFIER
        ),
        "bucket30OwnerResolutionV22": _read_u8(
            handle, root + OFF_BUCKET30_OWNER_RESOLUTION_V22
        ),
        "bucket30OwnerGoodToCoolCommitted": _read_u8(
            handle, root + OFF_BUCKET30_OWNER_GOOD_TO_COOL_COMMITTED
        ),
        "rightRankForcedGoodEventStreamDone28": _read_u8(
            handle, root + OFF_RIGHT_RANK_FORCED_GOOD_EVENT_STREAM_DONE
        ),
        "psxEventStreamFlagKnown": _read_u8(
            handle, root + OFF_PSX_EVENT_STREAM_FLAG_KNOWN
        ),
        "psxEventStreamActive": _read_u8(handle, root + OFF_PSX_EVENT_STREAM_ACTIVE),
        "psxEventStreamIdKnown": _read_u8(
            handle, root + OFF_PSX_EVENT_STREAM_ID_KNOWN
        ),
        "psxEventStreamId": _read_u8(handle, root + OFF_PSX_EVENT_STREAM_ID),
        "psxCtxFlagsKnown": _read_u8(handle, root + OFF_PSX_CTX_FLAGS_KNOWN),
        "psxCtxFlags": _read_u16(handle, root + OFF_PSX_CTX_FLAGS),
    }


def _sample_event_runtime(handle: int, base: int) -> dict[str, Any]:
    root = base + S_STAGE1_EVENT_STREAM_RUNTIME_RVA
    return {
        "flags40_2000_4000": _read_u16(handle, root + OFF_EV_FLAGS40_2000_4000),
        "ctx72": _read_u16(handle, root + OFF_EV_CTX72),
        "selectedStream": _read_u8(handle, root + OFF_EV_SELECTED_STREAM),
        "gPrStageEventStreamFlag": _read_u16(handle, root + OFF_EV_G_EVENT_STREAM_FLAG),
        "gPrStageEventStreamId": _read_u16(handle, root + OFF_EV_G_EVENT_STREAM_ID),
        "gPrStageEventStreamDone": _read_u8(handle, root + OFF_EV_G_EVENT_STREAM_DONE),
        "eventStreamFlagActive": _read_u8(
            handle, root + OFF_EV_EVENT_STREAM_FLAG_ACTIVE
        ),
        "eventStreamFlagStream": _read_u8(handle, root + OFF_EV_EVENT_STREAM_FLAG_STREAM),
        "activeDispatchStream": _read_u8(handle, root + OFF_EV_ACTIVE_DISPATCH_STREAM),
        "activeDispatchStartFrame": _read_u32(
            handle, root + OFF_EV_ACTIVE_DISPATCH_START_FRAME
        ),
        "activeDispatchStartScriptFrame": _read_u32(
            handle, root + OFF_EV_ACTIVE_DISPATCH_START_SCRIPT_FRAME
        ),
        "activeDispatchEndLocalFrame": _read_u32(
            handle, root + OFF_EV_ACTIVE_DISPATCH_END_LOCAL_FRAME
        ),
        "activeDispatchTerminalEndLocalFrame": _read_u32(
            handle, root + OFF_EV_ACTIVE_DISPATCH_TERMINAL_END_LOCAL_FRAME
        ),
        "lastStartedStream": _read_u8(handle, root + OFF_EV_LAST_STARTED_STREAM),
        "lastStartedQueryFrame": _read_u32(
            handle, root + OFF_EV_LAST_STARTED_QUERY_FRAME
        ),
        "eventStreamDonePending": _read_u8(
            handle, root + OFF_EV_EVENT_STREAM_DONE_PENDING
        ),
        "eventStreamDonePendingSourceStream": _read_u8(
            handle, root + OFF_EV_EVENT_STREAM_DONE_PENDING_SOURCE_STREAM
        ),
        "eventStreamDonePulse": _read_u8(handle, root + OFF_EV_EVENT_STREAM_DONE_PULSE),
        "eventStreamDoneSourceStream": _read_u8(
            handle, root + OFF_EV_EVENT_STREAM_DONE_SOURCE_STREAM
        ),
        "flag2000SceneFamilyActive": _read_u8(
            handle, root + OFF_EV_FLAG2000_SCENE_FAMILY_ACTIVE
        ),
        "flag2000SceneFamilyStream": _read_u8(
            handle, root + OFF_EV_FLAG2000_SCENE_FAMILY_STREAM
        ),
        "flag2000SceneFamilyStartFrame": _read_u32(
            handle, root + OFF_EV_FLAG2000_SCENE_FAMILY_START_FRAME
        ),
        "flag2000SceneFamilyStartScriptFrame": _read_u32(
            handle, root + OFF_EV_FLAG2000_SCENE_FAMILY_START_SCRIPT_FRAME
        ),
        "flags2000LastDispatchedStream": _read_u8(
            handle, root + OFF_EV_FLAGS2000_LAST_DISPATCHED_STREAM
        ),
        "prevFlag2000": _read_u8(handle, root + OFF_EV_PREV_FLAG2000),
    }


def _read_i32_array(handle: int, address: int, count: int) -> list[int]:
    return [_read_i32(handle, address + i * 4) for i in range(count)]


def _read_f32_array(handle: int, address: int, count: int) -> list[float]:
    return [_read_f32(handle, address + i * 4) for i in range(count)]


def _read_i16_array(handle: int, address: int, count: int) -> list[int]:
    return [_read_i16(handle, address + i * 2) for i in range(count)]


def _sample_camera_state(handle: int, address: int) -> dict[str, Any]:
    return {
        "valid": _read_u8(handle, address + OFF_CAMERA_STATE_VALID),
        "pos": _read_f32_array(handle, address + OFF_CAMERA_STATE_POS, 3),
        "target": _read_f32_array(handle, address + OFF_CAMERA_STATE_TARGET, 3),
        "twist": _read_i32(handle, address + OFF_CAMERA_STATE_TWIST),
        "projectionDistance": _read_i32(
            handle, address + OFF_CAMERA_STATE_PROJECTION_DISTANCE
        ),
        "screenCenterX": _read_i32(handle, address + OFF_CAMERA_STATE_SCREEN_CENTER_X),
        "screenCenterY": _read_i32(handle, address + OFF_CAMERA_STATE_SCREEN_CENTER_Y),
    }


def _sample_camera_bez_control(handle: int, root: int, item_index: int) -> dict[str, Any] | None:
    if item_index < 0 or item_index >= 256:
        return None
    coeff_offset = item_index * 4 * 2
    return {
        "item": item_index,
        "posX": _read_i16_array(handle, root + coeff_offset, 4),
        "posY": _read_i16_array(handle, root + 2048 + coeff_offset, 4),
        "posZ": _read_i16_array(handle, root + 4096 + coeff_offset, 4),
        "tgtX": _read_i16_array(handle, root + 6144 + coeff_offset, 4),
        "tgtY": _read_i16_array(handle, root + 8192 + coeff_offset, 4),
        "tgtZ": _read_i16_array(handle, root + 10240 + coeff_offset, 4),
    }


def _sample_camera_bez_player(handle: int, address: int) -> dict[str, Any]:
    total_entries = _read_i32(handle, address + OFF_CAMERA_BEZ_TOTAL_ENTRIES)
    current_frame = _read_i32(handle, address + OFF_CAMERA_BEZ_CURRENT_FRAME)
    control_samples: dict[str, Any] = {
        "first": _sample_camera_bez_control(handle, address, 0),
        "current": _sample_camera_bez_control(handle, address, current_frame),
        "next": _sample_camera_bez_control(handle, address, current_frame + 1),
    }
    return {
        "loaded": _read_u8(handle, address + OFF_CAMERA_BEZ_LOADED),
        "active": _read_u8(handle, address + OFF_CAMERA_BEZ_ACTIVE),
        "useTrueCubic": _read_u8(handle, address + OFF_CAMERA_BEZ_TRUE_CUBIC),
        "totalEntries": total_entries,
        "currentFrame": current_frame,
        "tickDivision": _read_i32(handle, address + OFF_CAMERA_BEZ_TICK_DIVISION),
        "posMode": _read_i32(handle, address + OFF_CAMERA_BEZ_POS_MODE),
        "tgtMode": _read_i32(handle, address + OFF_CAMERA_BEZ_TGT_MODE),
        "posVel": _read_i32_array(handle, address + OFF_CAMERA_BEZ_POS_VEL, 3),
        "tgtVel": _read_i32_array(handle, address + OFF_CAMERA_BEZ_TGT_VEL, 3),
        "outPos": _read_i32_array(handle, address + OFF_CAMERA_BEZ_OUT_POS, 3),
        "outTgt": _read_i32_array(handle, address + OFF_CAMERA_BEZ_OUT_TGT, 3),
        "control": control_samples,
    }


def _sample_scene_submit_camera(handle: int, base: int) -> dict[str, Any]:
    runtime_addr = base + S_STAGE1_SCENE_SUBMIT_RUNTIME_RVA
    impl_addr = _read_ptr(handle, runtime_addr + OFF_RUNTIME_IMPL_PTR)
    out: dict[str, Any] = {
        "runtimeAddr": f"0x{runtime_addr:X}",
        "implAddr": f"0x{impl_addr:X}",
    }
    if impl_addr == 0:
        out["initialized"] = 0
        return out

    psx_root = impl_addr + OFF_IMPL_PSX801CBFDC
    out.update(
        {
            "initialized": 1,
            "cameraBezId": _read_i16(
                handle, impl_addr + OFF_IMPL_CAMERA_BEZ_ID_801CBFDC
            ),
            "cameraSourceEventPsxAddr": _read_u32(
                handle, impl_addr + OFF_IMPL_CAMERA_SOURCE_EVENT_801CBFDC
            ),
            "cameraUsingNeutral": _read_u8(
                handle, impl_addr + OFF_IMPL_CAMERA_USING_NEUTRAL_801CBFDC
            ),
            "cameraAdvanceFrameValid": _read_u8(
                handle, impl_addr + OFF_IMPL_CAMERA_ADVANCE_VALID_801CBFDC
            ),
            "lastCameraQueryFrame": _read_u32(
                handle, impl_addr + OFF_IMPL_LAST_CAMERA_QUERY_FRAME_801CBFDC
            ),
            "dword_801D302C": _read_u32(
                handle, psx_root + OFF_PSX801_DWORD_801D302C
            ),
            "dword_801D3034": _read_u32(
                handle, psx_root + OFF_PSX801_DWORD_801D3034
            ),
            "bez": _sample_camera_bez_player(
                handle, impl_addr + OFF_IMPL_CAMERA_BEZ_PLAYER_801CBFDC
            ),
            "authority": _sample_camera_state(
                handle, impl_addr + OFF_IMPL_CAMERA_AUTHORITY_801CBFDC
            ),
            "presentation": _sample_camera_state(
                handle, impl_addr + OFF_IMPL_CAMERA_PRESENTATION_801CBFDC
            ),
        }
    )
    return out


def _attach_scene_camera(
    sample: dict[str, Any],
    handle: int,
    base: int,
    enabled: bool,
) -> dict[str, Any]:
    if not enabled:
        return sample
    try:
        sample["sceneCamera"] = _sample_scene_submit_camera(handle, base)
    except OSError as exc:
        sample["sceneCameraError"] = str(exc)
    return sample


def _emit(sample: dict[str, Any], out_file: Path | None) -> None:
    text = json.dumps(sample, ensure_ascii=False, separators=(",", ":"))
    print(text, flush=True)
    if out_file is not None:
        out_file.parent.mkdir(parents=True, exist_ok=True)
        with out_file.open("a", encoding="utf-8") as f:
            f.write(text + "\n")


def _write_force_facts(
    handle: int,
    base: int,
    *,
    ctx72: int | None,
    ed24: int | None,
    ed00: int | None,
    ed14: int | None,
    ed0c: int | None,
    phase: int | None,
    commit_shape: bool,
    tick96: int,
) -> None:
    root = base + S_STAGE1_NUMERIC_RUNTIME_RVA
    query_frame = _read_u32(handle, root + OFF_QUERY_FRAME)
    previous_ctx72 = _read_u16(handle, root + OFF_FOLLOW_UP_CTX72)
    write_ctx72 = previous_ctx72 if ctx72 is None else ctx72
    write_ed24 = 0 if ed24 is None else ed24
    write_ed00 = 0 if ed00 is None else ed00
    write_ed14 = 0 if ed14 is None else ed14
    write_ed0c = 0 if ed0c is None else ed0c
    write_phase = 1 if phase is None else phase
    if commit_shape:
        write_ctx72 = (
            ((previous_ctx72 + 1) & 0xFFFF) if ctx72 is None else (ctx72 & 0xFFFF)
        )
        delay_tick96 = 0x0310 if write_ctx72 >= 2 else 0x0610
        write_ed24 = 1 if ed24 is None else ed24
        write_ed00 = 7 if ed00 is None else ed00
        write_ed14 = 1 if ed14 is None else ed14
        write_ed0c = tick96 + delay_tick96 if ed0c is None else ed0c
        write_phase = 1 if phase is None else phase

    _write_u8(handle, root + OFF_DESCRIPTOR_SUBSTATE50, 1 if commit_shape else 0)
    _write_u8(handle, root + OFF_BUCKET30_RESOLUTION_KNOWN, 1 if commit_shape else 0)
    _write_u8(handle, root + OFF_BUCKET30_RESOLUTION_V22, 1 if commit_shape else 2)
    _write_u8(
        handle,
        root + OFF_BUCKET30_WRITEBACK_COMMITTED,
        1 if commit_shape else 0,
    )
    _write_u8(handle, root + OFF_LAST_ROW_WRITE_KNOWN, 1 if commit_shape else 0)
    _write_i32(handle, root + OFF_LAST_ROW_WRITE_QUERY_FRAME, int(query_frame))
    _write_u8(handle, root + OFF_LAST_ROW_WRITE_PREV_ROW, 1)
    _write_u8(handle, root + OFF_LAST_ROW_WRITE_RESOLVED_ROW, 0)
    _write_u8(
        handle,
        root + OFF_LAST_ROW_WRITE_RESOLUTION_KNOWN,
        1 if commit_shape else 0,
    )
    _write_u8(handle, root + OFF_LAST_ROW_WRITE_RESOLUTION_V22, 1 if commit_shape else 2)
    _write_u8(handle, root + OFF_LAST_ROW_WRITE_COMMITTED, 1 if commit_shape else 0)
    _write_u8(handle, root + OFF_RIGHT_RANK_ACTIVE_ROW, 0)
    _write_u8(handle, root + OFF_RIGHT_RANK_BLINK_TARGET_ROW, 0)
    _write_u8(handle, root + OFF_RIGHT_RANK_BLINK_ENABLED, 0)
    _write_u8(handle, root + OFF_RIGHT_RANK_FOLLOW_UP_PHASE, write_phase)
    _write_u16(handle, root + OFF_FOLLOW_UP_CTX72, write_ctx72)
    _write_i32(handle, root + OFF_FOLLOW_UP_ED24, write_ed24)
    _write_i32(handle, root + OFF_FOLLOW_UP_ED00, write_ed00)
    _write_i32(handle, root + OFF_FOLLOW_UP_ED14, write_ed14)
    _write_i32(handle, root + OFF_FOLLOW_UP_ED0C, write_ed0c)
    _write_u8(handle, root + OFF_BUCKET_CTX54_PERMIT, 0)
    _write_u8(handle, root + OFF_BUCKET_CTX6A_CONSUMER_GATE, 0)
    _write_u16(handle, root + OFF_BUCKET_CTX7A, 0)
    _write_u16(handle, root + OFF_BUCKET_TRANSITION_ANIM18E, 5 if commit_shape else 0)
    _write_u8(handle, root + OFF_BUCKET30_OWNER_SCORER_COMMIT_WINDOW_OPEN, 1)
    _write_u8(handle, root + OFF_BUCKET30_OWNER_KERNEL_OPEN, 1)
    _write_u8(handle, root + OFF_BUCKET30_OWNER_KERNEL_ENTERED, 1)
    _write_u16(handle, root + OFF_BUCKET30_OWNER_DESCRIPTOR_FLAG_WORD, 0x0004)
    _write_u8(handle, root + OFF_BUCKET30_OWNER_ACTIVE_ROW, 1)
    _write_u8(handle, root + OFF_BUCKET30_OWNER_RESOLUTION_CALLED, 1)
    _write_u8(handle, root + OFF_BUCKET30_OWNER_RESOLUTION_KNOWN, 1)
    _write_u8(handle, root + OFF_BUCKET30_OWNER_RESOLUTION_INPUT_CLASSIFIER, 1)
    _write_u8(handle, root + OFF_BUCKET30_OWNER_RESOLUTION_V22, 1)
    _write_u8(
        handle,
        root + OFF_BUCKET30_OWNER_GOOD_TO_COOL_COMMITTED,
        1 if commit_shape else 0,
    )
    _write_u8(handle, root + OFF_FOLLOW_UP_FLAG4000, 0)
    _write_u8(handle, root + OFF_FOLLOW_UP_FLAG2000, 1)


def _natural_bucket30_commit_seen(sample: dict[str, Any]) -> bool:
    return bool(
        sample.get("bucket30OwnerGoodToCoolCommitted")
        or (
            sample.get("rightRankActiveRow") == 0
            and sample.get("ed00") in (1, 7)
            and sample.get("ed14") == 1
        )
        or (
            sample.get("lastRowWriteKnown")
            and sample.get("lastRowWritePrevRow") == 1
            and sample.get("lastRowWriteResolvedRow") == 0
            and sample.get("lastRowWriteResolutionKnown")
            and sample.get("lastRowWriteResolutionV22") == 1
            and sample.get("lastRowWriteCommitted")
        )
    )


def _write_natural_bucket30_good_to_cool_preconditions(
    handle: int,
    base: int,
    *,
    accumulator: int,
    descriptor_flags: int,
) -> bool:
    root = base + S_STAGE1_NUMERIC_RUNTIME_RVA
    current = _sample_numeric(handle, base)
    if not current["active"] or _natural_bucket30_commit_seen(current):
        return False

    if current["rightRankActiveRow"] != 1:
        return False

    current_flags = current["rightRankDescriptorFlagWord08"]
    merged_flags = (current_flags | descriptor_flags) & 0xFFFF

    # Feed the direct scorer the same preconditions that a strong second-loop
    # GOOD measure should provide. This does not publish row-write mirrors,
    # event-stream state, ctx flag pulses, or camera facts; the next real
    # bucket30 owner call must still resolve and emit GOOD->COOL by itself.
    _write_u8(handle, root + OFF_WORD9182A_ENABLED, 1)
    _write_u8(handle, root + OFF_DESCRIPTOR_FLAG_WORD_KNOWN, 1)
    _write_u16(handle, root + OFF_DESCRIPTOR_FLAG_WORD, merged_flags)
    _write_u8(handle, root + OFF_DESCRIPTOR_SUBSTATE50, 0)

    _write_i32(handle, root + OFF_SCORER_ACCUMULATOR_91816, accumulator)
    _write_i32(handle, root + OFF_PHASE1_BASELINE18, 0)
    _write_u8(handle, root + OFF_PHASE1_SHORT_WINDOW_LATCH34_KNOWN, 1)
    _write_u16(handle, root + OFF_PHASE1_SHORT_WINDOW_LATCH34, 0)
    _write_u8(handle, root + OFF_PHASE1_CACHED_CLASSIFIER36, 1)
    _write_u8(handle, root + OFF_PHASE1_LATCH_ARMED38, 0)
    _write_i32(handle, root + OFF_TIE_BREAKER_SNAPSHOT1A, 0)
    _write_i32(handle, root + OFF_HELPER_SNAPSHOT1C, 0)
    _write_u16(handle, root + OFF_HELPER_TIE_CARRY_LATCH1E, 0)
    _write_i32(handle, root + OFF_FOLLOW_UP_ED24, 0)
    _write_i32(handle, root + OFF_FOLLOW_UP_ED00, 0)
    _write_i32(handle, root + OFF_FOLLOW_UP_ED14, 0)
    _write_u8(handle, root + OFF_FOLLOW_UP_FLAG2000, 0)
    _write_u8(handle, root + OFF_FOLLOW_UP_FLAG4000, 0)
    return True


def _parse_handoff_int(raw: str, key: str) -> int | None:
    match = re.search(rf"(?:^|\\s){re.escape(key)}=(-?\\d+)", raw)
    if not match:
        return None
    try:
        return int(match.group(1), 10)
    except ValueError:
        return None


def _write_force_event_runtime(
    handle: int,
    base: int,
    *,
    stream_id: int,
    ctx72: int,
    query_frame: int,
    script_frame: int,
    end_local_frame: int,
    terminal_end_local_frame: int,
) -> None:
    root = base + S_STAGE1_EVENT_STREAM_RUNTIME_RVA
    stream = stream_id & 0xFF
    _write_u16(handle, root + OFF_EV_FLAGS40_2000_4000, 0x2000)
    _write_u16(handle, root + OFF_EV_CTX72, ctx72)
    _write_u8(handle, root + OFF_EV_SELECTED_STREAM, stream)
    _write_u16(handle, root + OFF_EV_G_EVENT_STREAM_FLAG, 1)
    _write_u16(handle, root + OFF_EV_G_EVENT_STREAM_ID, stream)
    _write_u8(handle, root + OFF_EV_G_EVENT_STREAM_DONE, 0)
    if stream < 9:
        _write_u32(handle, root + OFF_EV_DESC_CURSOR_ARRAY + stream * 4, 0)
    _write_u8(handle, root + OFF_EV_EVENT_STREAM_FLAG_ACTIVE, 1)
    _write_u8(handle, root + OFF_EV_EVENT_STREAM_FLAG_STREAM, 0)
    _write_u8(handle, root + OFF_EV_ACTIVE_DISPATCH_STREAM, stream)
    _write_u32(handle, root + OFF_EV_ACTIVE_DISPATCH_START_FRAME, query_frame)
    _write_u32(
        handle,
        root + OFF_EV_ACTIVE_DISPATCH_START_SCRIPT_FRAME,
        script_frame,
    )
    _write_u32(
        handle,
        root + OFF_EV_ACTIVE_DISPATCH_END_LOCAL_FRAME,
        end_local_frame,
    )
    _write_u32(
        handle,
        root + OFF_EV_ACTIVE_DISPATCH_TERMINAL_END_LOCAL_FRAME,
        terminal_end_local_frame,
    )
    _write_u8(handle, root + OFF_EV_ACTIVE_DISPATCH_TERMINAL_PULSE_EMITTED, 0)
    _write_u8(handle, root + OFF_EV_ACTIVE_DISPATCH_DONE_PULSE_EMITTED, 0)
    _write_u32(handle, root + OFF_EV_DWORD801D3048, script_frame)
    _write_u8(handle, root + OFF_EV_LAST_STARTED_STREAM, stream)
    _write_u32(handle, root + OFF_EV_LAST_STARTED_QUERY_FRAME, query_frame)
    _write_u8(handle, root + OFF_EV_EVENT_STREAM_DONE_PENDING, 0)
    _write_u8(handle, root + OFF_EV_EVENT_STREAM_DONE_PENDING_SOURCE_STREAM, 0)
    _write_u8(handle, root + OFF_EV_EVENT_STREAM_DONE_PULSE, 0)
    _write_u8(handle, root + OFF_EV_EVENT_STREAM_DONE_SOURCE_STREAM, 0)
    _write_u8(handle, root + OFF_EV_FLAG2000_SCENE_FAMILY_ACTIVE, 1)
    _write_u8(handle, root + OFF_EV_FLAG2000_SCENE_FAMILY_STREAM, stream)
    _write_u32(
        handle,
        root + OFF_EV_FLAG2000_SCENE_FAMILY_START_FRAME,
        query_frame,
    )
    _write_u32(
        handle,
        root + OFF_EV_FLAG2000_SCENE_FAMILY_START_SCRIPT_FRAME,
        script_frame,
    )
    _write_u8(handle, root + OFF_EV_FLAGS2000_LAST_DISPATCHED_STREAM, stream)
    _write_u8(handle, root + OFF_EV_PREV_FLAG2000, 1)

    numeric = base + S_STAGE1_NUMERIC_RUNTIME_RVA
    _write_u8(handle, numeric + OFF_PSX_EVENT_STREAM_FLAG_KNOWN, 1)
    _write_u8(handle, numeric + OFF_PSX_EVENT_STREAM_ACTIVE, 1)
    _write_u8(handle, numeric + OFF_PSX_EVENT_STREAM_ID_KNOWN, 1)
    _write_u8(handle, numeric + OFF_PSX_EVENT_STREAM_ID, stream)
    _write_u8(handle, numeric + OFF_PSX_CTX_FLAGS_KNOWN, 1)
    _write_u16(handle, numeric + OFF_PSX_CTX_FLAGS, 0x2000)


def _write_force_event_done(
    handle: int,
    base: int,
    *,
    stream_id: int,
) -> None:
    root = base + S_STAGE1_EVENT_STREAM_RUNTIME_RVA
    stream = stream_id & 0xFF
    _write_u8(handle, root + OFF_EV_G_EVENT_STREAM_DONE, 1)
    _write_u8(handle, root + OFF_EV_EVENT_STREAM_DONE_PENDING, 1)
    _write_u8(handle, root + OFF_EV_EVENT_STREAM_DONE_PENDING_SOURCE_STREAM, stream)
    _write_u8(handle, root + OFF_EV_EVENT_STREAM_DONE_PULSE, 1)
    _write_u8(handle, root + OFF_EV_EVENT_STREAM_DONE_SOURCE_STREAM, stream)
    _write_u8(handle, root + OFF_EV_ACTIVE_DISPATCH_DONE_PULSE_EMITTED, 1)
    _write_u8(handle, root + OFF_EV_EVENT_STREAM_FLAG_ACTIVE, 1)
    _write_u8(handle, root + OFF_EV_EVENT_STREAM_FLAG_STREAM, 1)

    numeric = base + S_STAGE1_NUMERIC_RUNTIME_RVA
    _write_u8(handle, numeric + OFF_RIGHT_RANK_FORCED_GOOD_EVENT_STREAM_DONE, 1)


def _send_debug_commands(client: DebugClient, commands: list[str]) -> dict[str, str]:
    out: dict[str, str] = {}
    for cmd in commands:
        out[cmd] = client.send_full(cmd)
    return out


def _maybe_send_debug_commands(
    client: DebugClient,
    commands: list[str],
    *,
    sample_index: int,
    every: int,
) -> dict[str, str]:
    if every <= 0 or sample_index % every != 0:
        return {}
    return _send_debug_commands(client, commands)


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--process", default=DEFAULT_PROCESS)
    ap.add_argument("--pid", type=int)
    ap.add_argument("--host", default="127.0.0.1")
    ap.add_argument("--port", type=int, default=19790)
    ap.add_argument("--duration", type=float, default=0.25)
    ap.add_argument("--interval", type=float, default=1.0 / 60.0)
    ap.add_argument("--wait-active", action="store_true")
    ap.add_argument("--wait-timeout", type=float, default=60.0)
    ap.add_argument(
        "--min-query-frame",
        type=int,
        default=0,
        help=(
            "Wait until Stage1 numeric queryFrame reaches this value before "
            "writing the forced GOOD->COOL facts."
        ),
    )
    ap.add_argument("--ctx72", type=lambda s: int(s, 0), default=None)
    ap.add_argument("--ed24", type=int, default=None)
    ap.add_argument("--ed00", type=int, default=None)
    ap.add_argument("--ed14", type=int, default=None)
    ap.add_argument("--ed0c", type=int, default=None)
    ap.add_argument("--phase", type=int, default=None)
    ap.add_argument(
        "--commit-shape",
        action="store_true",
        help=(
            "Write the scorer-side GOOD->COOL commit shape too: ctx72+1, "
            "ed24=1, ed00=7, ed14=1, ed0c=tick96+delay, row-write mirrors, "
            "transitionAnim18E=5."
        ),
    )
    ap.add_argument(
        "--natural-bucket30-good-to-cool",
        action="store_true",
        help=(
            "Only feed preconditions for the next real bucket30 scorer owner "
            "to resolve GOOD->COOL. This does not write event-runtime state, "
            "row-write mirrors, ctx flag pulses, or camera facts."
        ),
    )
    ap.add_argument(
        "--natural-accumulator",
        type=int,
        default=2000,
        help=(
            "Accumulator value used by --natural-bucket30-good-to-cool to make "
            "phase1 growth and tie-break delta positive."
        ),
    )
    ap.add_argument(
        "--natural-descriptor-flags",
        type=lambda s: int(s, 0),
        default=0x0006,
        help=(
            "Descriptor flag bits ORed into rightRankDescriptorFlagWord08 for "
            "the natural bucket30 route. Defaults to owner+resolver bits."
        ),
    )
    ap.add_argument(
        "--force-event-runtime",
        action="store_true",
        help=(
            "Also write the Stage1 801C9094 event-stream runtime directly. "
            "This is a detachable reproduction aid, not gameplay evidence."
        ),
    )
    ap.add_argument("--event-stream-id", type=int, default=6)
    ap.add_argument("--event-end-local-frame", type=int, default=4096)
    ap.add_argument("--event-terminal-end-local-frame", type=int, default=4096)
    ap.add_argument(
        "--force-event-done-after-query-delta",
        type=int,
        default=-1,
        help=(
            "After the forced write queryFrame plus this delta, keep pulsing "
            "the event-stream done carrier. Negative disables it."
        ),
    )
    ap.add_argument("--post-samples", type=int, default=180)
    ap.add_argument(
        "--debug-sample-every",
        type=int,
        default=1,
        help="Collect debug-server text every N post samples. 0 disables post debug text.",
    )
    ap.add_argument(
        "--skip-final-debug",
        action="store_true",
        help=(
            "Do not query debug-server text for the forced/natural summary "
            "sample. Useful when frame-dense camera traces matter."
        ),
    )
    ap.add_argument(
        "--scene-camera-trace",
        action="store_true",
        help=(
            "Also sample the owned 801CBFDC scene-submit camera BEZ state from "
            "PDB-verified offsets. Read-only; no runtime code is patched."
        ),
    )
    ap.add_argument("--out", type=Path)
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--wait-process", action="store_true")
    ap.add_argument("--wait-process-timeout", type=float, default=60.0)
    args = ap.parse_args(argv)

    if args.natural_bucket30_good_to_cool and (
        args.commit_shape
        or args.force_event_runtime
        or args.force_event_done_after_query_delta >= 0
    ):
        print(
            "ERROR: --natural-bucket30-good-to-cool is mutually exclusive with "
            "--commit-shape, --force-event-runtime, and "
            "--force-event-done-after-query-delta.",
            file=sys.stderr,
        )
        return 2

    pid = args.pid
    if pid is None:
        start_wait_process = time.time()
        while True:
            pids = _find_pids_by_image(args.process)
            if pids:
                pid = pids[0]
                break
            if (
                not args.wait_process
                or time.time() - start_wait_process > args.wait_process_timeout
            ):
                print(f"ERROR: process not found: {args.process}", file=sys.stderr)
                return 2
            time.sleep(0.1)

    handle = _open_process(pid)
    try:
        modules = _module_bases(pid)
        base = modules.get(args.process.lower())
        if base is None:
            print(f"ERROR: module not found: {args.process}", file=sys.stderr)
            return 2

        def reattach_after_process_loss() -> bool:
            nonlocal pid, handle, base
            try:
                kernel32.CloseHandle(handle)
            except Exception:
                pass
            start_wait = time.time()
            while time.time() - start_wait <= args.wait_process_timeout:
                pids = _find_pids_by_image(args.process)
                if not pids:
                    time.sleep(0.1)
                    continue
                try:
                    next_handle = _open_process(pids[0])
                    next_modules = _module_bases(pids[0])
                    next_base = next_modules.get(args.process.lower())
                    if next_base is None:
                        kernel32.CloseHandle(next_handle)
                        time.sleep(0.1)
                        continue
                    pid = pids[0]
                    handle = next_handle
                    base = next_base
                    return True
                except OSError:
                    time.sleep(0.1)
            return False

        client = DebugClient(host=args.host, port=args.port, timeout=0.5)
        start = time.time()
        while args.wait_active:
            try:
                current = _sample_numeric(handle, base)
            except OSError:
                if reattach_after_process_loss():
                    continue
                print("ERROR: lost process while waiting for Stage1 numeric runtime", file=sys.stderr)
                return 2
            if current["active"]:
                break
            if time.time() - start > args.wait_timeout:
                print("ERROR: timed out waiting for Stage1 numeric runtime", file=sys.stderr)
                return 3
            time.sleep(max(0.01, args.interval))

        start_query_wait = time.time()
        while args.min_query_frame > 0:
            try:
                current = _sample_numeric(handle, base)
            except OSError:
                if reattach_after_process_loss():
                    continue
                print("ERROR: lost process while waiting for min queryFrame", file=sys.stderr)
                return 2
            if current["active"] and current["queryFrame"] >= args.min_query_frame:
                break
            if time.time() - start_query_wait > args.wait_timeout:
                print(
                    "ERROR: timed out waiting for "
                    f"queryFrame >= {args.min_query_frame}",
                    file=sys.stderr,
                )
                return 3
            time.sleep(max(0.01, args.interval))

        before = _sample_numeric(handle, base)
        before_debug = _send_debug_commands(client, ["status", "stage1ovl handoff"])
        before_handoff = before_debug.get("stage1ovl handoff", "")
        event_script_frame = _parse_handoff_int(before_handoff, "tick96")
        if event_script_frame is None:
            event_script_frame = int(before["queryFrame"]) * 6
        _emit(
            _attach_scene_camera(
                {
                "ts": time.time(),
                "pid": pid,
                "phase": "before",
                "base": f"0x{base:X}",
                "numeric": before,
                "eventRuntime": _sample_event_runtime(handle, base),
                "debug": before_debug,
                },
                handle,
                base,
                args.scene_camera_trace,
            ),
            args.out,
        )

        writes = 0
        natural_commit_seen = False
        end = time.time() + max(0.0, args.duration)
        while time.time() <= end:
            if not args.dry_run:
                current = _sample_numeric(handle, base)
                if args.natural_bucket30_good_to_cool:
                    if _natural_bucket30_commit_seen(current):
                        natural_commit_seen = True
                        break
                    if _write_natural_bucket30_good_to_cool_preconditions(
                        handle,
                        base,
                        accumulator=args.natural_accumulator,
                        descriptor_flags=args.natural_descriptor_flags,
                    ):
                        writes += 1
                    time.sleep(max(0.001, args.interval))
                    continue

                _write_force_facts(
                    handle,
                    base,
                    ctx72=args.ctx72,
                    ed24=args.ed24,
                    ed00=args.ed00,
                    ed14=args.ed14,
                    ed0c=args.ed0c,
                    phase=args.phase,
                    commit_shape=args.commit_shape,
                    tick96=int(event_script_frame),
                )
                writes += 1
                if args.force_event_runtime:
                    write_ctx72 = current["ctx72"] if args.ctx72 is None else args.ctx72
                    _write_force_event_runtime(
                        handle,
                        base,
                        stream_id=args.event_stream_id,
                        ctx72=write_ctx72,
                        query_frame=int(current["queryFrame"]),
                        script_frame=int(event_script_frame),
                        end_local_frame=args.event_end_local_frame,
                        terminal_end_local_frame=args.event_terminal_end_local_frame,
                    )
            time.sleep(max(0.001, args.interval))

        _emit(
            _attach_scene_camera(
                {
                "ts": time.time(),
                "pid": pid,
                "phase": (
                    "dry-run"
                    if args.dry_run
                    else (
                        "natural-bucket30-committed"
                        if natural_commit_seen
                        else (
                            "natural-bucket30-fed"
                            if args.natural_bucket30_good_to_cool
                            else "forced"
                        )
                    )
                ),
                "writes": writes,
                "naturalCommitSeen": natural_commit_seen,
                "numeric": _sample_numeric(handle, base),
                "eventRuntime": _sample_event_runtime(handle, base),
                "debug": (
                    {}
                    if args.skip_final_debug
                    else _send_debug_commands(client, ["status", "stage1ovl handoff"])
                ),
                },
                handle,
                base,
                args.scene_camera_trace,
            ),
            args.out,
        )

        for i in range(max(0, args.post_samples)):
            time.sleep(max(0.001, args.interval))
            current = _sample_numeric(handle, base)
            if (
                not args.dry_run
                and args.force_event_runtime
                and args.force_event_done_after_query_delta >= 0
                and current["queryFrame"]
                >= before["queryFrame"] + args.force_event_done_after_query_delta
            ):
                _write_force_event_done(
                    handle,
                    base,
                    stream_id=args.event_stream_id,
                )
            _emit(
                _attach_scene_camera(
                    {
                    "ts": time.time(),
                    "pid": pid,
                    "phase": "post",
                    "index": i,
                    "numeric": _sample_numeric(handle, base),
                    "eventRuntime": _sample_event_runtime(handle, base),
                    "debug": _maybe_send_debug_commands(
                        client,
                        ["status", "stage1ovl handoff"],
                        sample_index=i,
                        every=args.debug_sample_every,
                    ),
                    },
                    handle,
                    base,
                    args.scene_camera_trace,
                ),
                args.out,
            )
    finally:
        kernel32.CloseHandle(handle)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
