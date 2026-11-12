# Host-side regression test for the 1-Wire analyzer's read-slot sample point. No Logic 2 required.
#   make check CAPTURE=/path/to/raw_capture.csv
# SDK headers: AnalyzerSDK checkout (default: the one CMake fetched, or ../../AnalyzerSDK).
SDK      ?= $(firstword $(wildcard ../build/_deps/analyzersdk-src ../../AnalyzerSDK))
# CAPTURE: Logic 2 raw edge export ("Time [s],bus") of the two-device bus; too large to check in. Optional ORACLE=decode_raw.py
# additionally compares every transaction against an independent edge-list decoder.
CAPTURE  ?= $(error set CAPTURE=/path/to/raw_capture.csv)
CXX      ?= c++
OUT      ?= out
CXXFLAGS += -std=c++11 -g -O1 -DLOGIC2 -Wno-deprecated-declarations -I$(SDK)/include -I../src
SRC      := ../src/OneWireAnalyzer.cpp ../src/OneWireAnalyzerSettings.cpp

all: $(OUT)/replay $(OUT)/settings_test

$(OUT)/replay: replay.cpp mock_sdk.cpp mock_sdk.h $(SRC)
	@mkdir -p $(OUT)
	$(CXX) $(CXXFLAGS) replay.cpp mock_sdk.cpp $(SRC) -o $@

$(OUT)/settings_test: settings_test.cpp mock_sdk.cpp mock_sdk.h $(SRC)
	@mkdir -p $(OUT)
	$(CXX) $(CXXFLAGS) settings_test.cpp mock_sdk.cpp $(SRC) -o $@

check: all
	$(OUT)/settings_test
	$(OUT)/replay $(CAPTURE) > $(OUT)/decoded.txt
	python3 -I check_capture.py $(OUT)/decoded.txt
	python3 -I gen_synthetic.py $(OUT)/synthetic.csv
	$(OUT)/replay $(OUT)/synthetic.csv > $(OUT)/synthetic.txt
	python3 -I check_synthetic.py $(OUT)/synthetic.txt

clean:
	rm -rf $(OUT)
