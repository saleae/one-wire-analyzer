"""Device-independent sanity checks on the replay output for a raw edge capture of an overdrive bus.

usage: python3 -I check_capture.py decoded.txt [min_transactions]
optional env: ORACLE=/path/decode_raw.py CAPTURE=/path/raw.csv  -> also compare every transaction's bytes
              against an independent edge-list decoder (decode(path) -> [{'bytes': [...]}, ...]).

The pre-fix symptom is a device whose read-0 slots are shorter than the old sample point: every data byte it
returns decodes as FF. The checks below are written to catch that without knowing anything about the devices.
"""
import importlib.util, os, re, sys

def parse(path):
    txns = []
    for line in open(path):
        t = line.split()
        d = {"line": line.rstrip(), "cmd": None, "fam": None, "id": None, "crc": None, "data": []}
        for tok in t[2:]:
            if tok.startswith("cmd="): d["cmd"] = tok[4:]
            elif tok.startswith("fam="): d["fam"] = tok[4:]
            elif tok.startswith("id="): d["id"] = tok[3:]
            elif tok.startswith("crc="): d["crc"] = tok[4:]
            elif re.fullmatch(r"[0-9a-f]{2}", tok): d["data"].append(int(tok, 16))
        txns.append(d)
    return txns

fails = []
def check(cond, msg):
    print(("ok   " if cond else "FAIL ") + msg)
    if not cond: fails.append(msg)

def crc8(b):  # Dallas/Maxim CRC-8
    c = 0
    for x in b:
        for _ in range(8):
            c = ((c >> 1) ^ 0x8C) if (c ^ x) & 1 else c >> 1
            x >>= 1
    return c

T = parse(sys.argv[1])
check(len(T) >= int(sys.argv[2] if len(sys.argv) > 2 else 1), f"{len(T)} transactions decoded")

roms = {(t["fam"], t["id"], t["crc"]) for t in T if t["id"]}
check(bool(roms), f"{len(roms)} distinct ROM(s) seen")
bad_crc = [r for r in roms if crc8(bytes([int(r[0], 16)]) + int(r[1], 16).to_bytes(6, "little")) != int(r[2], 16)]
check(not bad_crc, f"every ROM has a valid CRC-8 ({len(bad_crc)} invalid)")

# A device answering reads must not decode as a run of FF (an idle/released bus reads FF; real data essentially never
# is 4+ FF bytes in a row at the start of a response).
all_ff = [t for t in T if t["cmd"] and t["cmd"].startswith("match") and len(t["data"]) > 5 and all(b == 0xFF for b in t["data"][2:])]
check(not all_ff, f"no transaction with a device response decoding as all-FF ({len(all_ff)} found)")
total = sum(1 for t in T if len(t["data"]) > 5)
ffy = sum(1 for t in T if len(t["data"]) > 5 and t["data"][-4:] == [0xFF] * 4)
check(ffy * 2 < max(total, 1), f"fewer than half of data transactions end in 4+ FF bytes ({ffy}/{total})")

oracle, cap = os.environ.get("ORACLE"), os.environ.get("CAPTURE")
if oracle and cap:
    spec = importlib.util.spec_from_file_location("decode_raw", oracle); m = importlib.util.module_from_spec(spec); spec.loader.exec_module(m)
    ref = m.decode(cap)
    bad = 0
    for t, r in zip(T, ref):
        if t["cmd"] and t["cmd"].startswith("search"): continue  # search triplets are not a plain byte stream
        mine = []
        if t["cmd"]: mine.append(int(t["cmd"].split(":")[1], 16))
        if t["id"]: mine += [int(t["fam"], 16)] + list(int(t["id"], 16).to_bytes(6, "little")) + [int(t["crc"], 16)]
        mine += t["data"]
        if mine != r["bytes"]: bad += 1; print("  differs:", t["line"][:100], "| oracle:", bytes(r["bytes"][:12]).hex())
    check(bad == 0 and len(ref) == len(T), f"every non-search transaction matches the independent decoder ({bad} differ)")

print("PASS" if not fails else f"{len(fails)} FAILED")
sys.exit(1 if fails else 0)
