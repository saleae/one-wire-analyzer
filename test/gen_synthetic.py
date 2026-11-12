"""Synthetic edge capture ("Time [s],bus", 80 ns ticks = 12.5 MS/s) for the two sample-point cases that must hold
regardless of what a particular device on the bench does:

  standard speed:  Skip ROM, then a device-driven read byte where every 0 is held for exactly t_RDV + 1 us = 16 us
                   (legal: the device only has to hold the line low until 15 us after the falling edge)
  overdrive:       Overdrive Skip ROM, OD reset/presence, Skip ROM, then read bytes where every 0 is held 2.48 us
                   (seen on real devices whose legal read-0 hold is short)

Expected decode: std data byte A5, overdrive data bytes A5 3C. Pre-fix analyzer: std byte FF and OD bytes FF FF.
usage: python3 gen_synthetic.py out.csv
"""
import sys

TICK = 80e-9
ev = []          # (tick, level)
now = 0
level = 1

def us(x): return round(x * 1e-6 / TICK)
def low(width_us):
    """drive the bus low for width_us then release"""
    global now
    ev.append((now, 0)); now += us(width_us); ev.append((now, 1))
def idle(width_us):
    global now
    now += us(width_us)
def slot_write(bit, od):
    t_slot = 12.4 if od else 80.0
    w = (1.36 if bit else 7.52) if od else (6.0 if bit else 65.0)
    low(w); idle(t_slot - w)
def slot_read(bit, od):
    t_slot = 12.4 if od else 80.0
    master = 1.36 if od else 6.0
    hold = 2.48 if od else 16.0      # device-driven 0 (wired-OR with master pulse)
    low(master if bit else hold); idle(t_slot - (master if bit else hold))
def write_byte(b, od):
    for i in range(8): slot_write((b >> i) & 1, od)
def read_byte(b, od):
    for i in range(8): slot_read((b >> i) & 1, od)
def reset(od):
    low(70.16 if od else 480.16); idle(od and 3 or 30)
    low(18.4 if od else 120.0); idle(od and 12 or 300)

low(6); idle(100)   # lead-in pulse: the analyzer discards the first edge
reset(False); write_byte(0xCC, False); read_byte(0xA5, False)        # std: skip rom, device returns A5
idle(200)
reset(False); write_byte(0x3C, False)                                # std: overdrive skip rom
idle(100)
reset(True); write_byte(0xCC, True); read_byte(0xA5, True); read_byte(0x3C, True)
idle(50); low(1.36); idle(50)                                        # trailing edge so the decoder can finish the last slot

with open(sys.argv[1], "w") as f:
    f.write("Time [s],bus\n")
    f.write("-1.000000000,1\n")
    t0 = ev[0][0]
    last = 1
    for t, v in ev:
        if v == last: continue
        f.write(f"{(t - t0) * TICK:.9f},{v}\n"); last = v
