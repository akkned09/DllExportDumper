# DllExportDumper
### A tool to grab dll exports
## Usage
    DllExportDumper.exe [dll name] [mode] [output file]
Modes:
  - 0: one per line
  - 1: index, name
  - 2: c++ std::vector
  - 3: python list
  - 4: dll relative offset, name
  - 5: c++ std::map (name to dll relative offset)
