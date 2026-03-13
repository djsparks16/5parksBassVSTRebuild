cmake_minimum_required(VERSION 3.22)

project(BlacksideBass VERSION 0.2.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(${JUCE_DIR} JUCE)

juce_add_plugin(BlacksideBass
    COMPANY_NAME "Blackside Audio"
    BUNDLE_ID "com.blacksideaudio.blacksidebass"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE BLKS
    PLUGIN_CODE BSBS
    FORMATS VST3
    VST3_CAN_REPLACE_VST2 FALSE
    PRODUCT_NAME "Blackside Bass"
)

target_sources(BlacksideBass PRIVATE
    source/PluginProcessor.cpp
    source/PluginEditor.cpp
)

target_include_directories(BlacksideBass PRIVATE
    source
)

target_compile_definitions(BlacksideBass
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(BlacksideBass
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
