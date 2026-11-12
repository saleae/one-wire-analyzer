# Read-slot sample point regression test

Host-only (no Logic 2): `mock_sdk.cpp` implements the few out-of-line SDK members the analyzer uses, over an edge list, and
`replay` runs the real `OneWireAnalyzer::WorkerThread()` on it.

    make -C test check SDK=/path/to/AnalyzerSDK \
        CAPTURE=/path/raw_capture.csv [ORACLE=/path/decode_raw.py]

* `check_capture.py` – device-independent checks on a raw edge capture of an overdrive bus: ROM CRCs valid, device
  responses do not decode as runs of FF, and (optionally, `ORACLE=`) every transaction matches an independent decoder.
  Pre-fix, a device whose legal read-0 hold is shorter than the old 3 us sample point decodes as all-FF.
* `gen_synthetic.py` / `check_synthetic.py` – self-contained: a 16 us standard-speed 0 (legal, t_RDV = 15 us, but earlier
  than the old 19 us sample point) and a 2.48 us overdrive 0.
* `settings_test.cpp` – new settings round-trip; settings strings saved by older versions still load.

`replay <csv> --std-ns N --ovd-ns N` overrides the sample points; on the reference capture the overdrive decode is
correct for 1400–2300 ns (default 1900), and the standard decode for 10000+ ns (default 13000).
