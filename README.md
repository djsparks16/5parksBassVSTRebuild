# Backside Bass v0.2.1 Fixed

This is a corrected rebuild of the Backside Bass foundation pack.

- kept the project **VST3-only**
- kept the **VST3/VST2 replacement conflict fixes** in CMake
- removed earlier header/API mismatches from the working Badline path
- added missing standard library include for `std::array`
- kept parameter reads on `getRawParameterValue(...)->load()`
- kept JUCE DSP filter calls on `processSample(0, x)`
- kept a page-based editor shell and macro strip
- kept the richer multi-osc Blackside voice architecture
