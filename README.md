# Blackside Bass v0.2.1 Fixed

This is a corrected rebuild of the Blackside Bass foundation pack.

## Corrections applied
- kept the project **VST3-only**
- kept the **VST3/VST2 replacement conflict fixes** in CMake
- removed earlier header/API mismatches from the working Badline path
- added missing standard library include for `std::array`
- kept parameter reads on `getRawParameterValue(...)->load()`
- kept JUCE DSP filter calls on `processSample(0, x)`
- kept a page-based editor shell and macro strip
- kept the richer multi-osc Blackside voice architecture

## Build on GitHub
1. Create a new GitHub repo.
2. Upload every file from this folder.
3. Confirm `.github/workflows/build.yml` exists.
4. Wait for **Build Blackside Bass** in Actions.
5. Download the `BlacksideBass-build` artifact.

## Install in Reason
Copy the built `.vst3` into:

`C:\Program Files\Common Files\VST3\`

Then in Reason:

`Edit -> Preferences -> Plug-ins -> Rescan`

## Notes
This is still a **foundation build**, not the final flagship. It is intended to be a cleaner, more reliable base for the next Blackside Bass upgrades.
