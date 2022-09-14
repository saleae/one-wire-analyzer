# Saleae 1-Wire Analyzer

Saleae 1-Wire Analyzer

## Getting Started

### MacOS

Dependencies:
- XCode with command line tools
- CMake 3.13+

Installing command line tools after XCode is installed:
```
xcode-select --install
```

Then open XCode, open Preferences from the main menu, go to locations, and select the only option under 'Command line tools'.

Installing CMake on MacOS:

1. Download the binary distribution for MacOS, `cmake-*-Darwin-x86_64.dmg`
2. Install the usual way by dragging into applications.
3. Open a terminal and run the following:
```
/Applications/CMake.app/Contents/bin/cmake-gui --install
```
*Note: Errors may occur if older versions of CMake are installed.*

Building the analyzer:
```
mkdir build
cd build
cmake ..
cmake --build .
```

### Ubuntu 16.04

Dependencies:
- CMake 3.13+
- gcc 4.8+

Misc dependencies:

```
sudo apt-get install build-essential
```

Building the analyzer:
```
mkdir build
cd build
cmake ..
cmake --build .
```

### Windows

Dependencies:
- Visual Studio 2015 Update 3
- CMake 3.13+

**Visual Studio 2015**

*Note - newer versions of Visual Studio should be fine.*

Setup options:
- Programming Languages > Visual C++ > select all sub-components.

Note - if CMake has any problems with the MSVC compiler, it's likely a component is missing.

**CMake**

Download and install the latest CMake release here.
https://cmake.org/download/

Building the analyzer:
```
mkdir build
cd build -A x64
cmake ..
```

Then, open the newly created solution file located here: `build\one_wire_analyzer.sln`


## Output Frame Format
  
### Frame Type: `"reset"`

| Property | Type | Description |
| :--- | :--- | :--- |


Reset pulse

### Frame Type: `"presence"`

| Property | Type | Description |
| :--- | :--- | :--- |


Presence Pulse

### Frame Type: `"rom_command"`

| Property | Type | Description |
| :--- | :--- | :--- |
| `description` | str | read, skip, search, or match |
| `rom_command` | bytes | The command byte |

ROM command. This is the first command issued by the master after a presence pulse

### Frame Type: `"crc"`

| Property | Type | Description |
| :--- | :--- | :--- |
| `crc` | bytes | The CRC byte |

8 bit CRC, last part of the 64 bit identifier

### Frame Type: `"family_code"`

| Property | Type | Description |
| :--- | :--- | :--- |
| `family` | bytes | The family code, which is the first part of the 64 bit identifier |

The family code of the device ID

### Frame Type: `"id"`

| Property | Type | Description |
| :--- | :--- | :--- |
| `id` | int | 48 bit integer, taken from the center of the 64 bit identifier |

The 48 bit device identifier

### Frame Type: `"data"`

| Property | Type | Description |
| :--- | :--- | :--- |
| `data` | bytes | A single data byte |

Data byte after the ROM command and identifier

### Frame Type: `"invalid_rom_command"`

| Property | Type | Description |
| :--- | :--- | :--- |
| `rom_command` | bytes | The ROM command byte |

Unknown ROM command

### Frame Type: `"alarm"`

| Property | Type | Description |
| :--- | :--- | :--- |
| `rom_command` | bytes | The command byte |

Alarm search command

