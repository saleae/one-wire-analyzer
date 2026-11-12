"""usage: python3 -I check_synthetic.py decoded.txt   (replay output for gen_synthetic.py's capture)"""
import sys
lines = [l.split() for l in open(sys.argv[1])]
ok = len(lines) == 3 and lines[0][-2:] == ["cmd=skip:cc", "a5"] and lines[2][-3:] == ["cmd=skip:cc", "a5", "3c"]
print("std 16 us device-driven 0s decode as A5; overdrive 2.48 us 0s decode as A5 3C:", "PASS" if ok else "FAIL\n" + "".join(" ".join(l) + "\n" for l in lines))
sys.exit(0 if ok else 1)
