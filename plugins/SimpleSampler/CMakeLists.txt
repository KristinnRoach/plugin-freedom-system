cmake_minimum_required(VERSION 3.15)

# Plugin formats: VST3, AU, Standalone
juce_add_plugin(SimpleSampler
    COMPANY_NAME "YourCompany"
    PLUGIN_MANUFACTURER_CODE Manu
    PLUGIN_CODE Simp
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "SimpleSampler"
    IS_SYNTH TRUE           # REQUIRED for instruments
    NEEDS_MIDI_INPUT TRUE   # Explicit MIDI requirement
    NEEDS_WEB_BROWSER TRUE  # WebView UI support
)

# Source files
target_sources(SimpleSampler
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/SimpleSamplerSound.cpp
        Source/SimpleSamplerVoice.cpp
)

# Include paths
target_include_directories(SimpleSampler
    PRIVATE
        Source
)

# Required JUCE modules
target_link_libraries(SimpleSampler
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra  # Required for WebBrowserComponent
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Generate JuceHeader.h (JUCE 8 requirement - CRITICAL: after target_link_libraries)
juce_generate_juce_header(SimpleSampler)

# WebView UI Resources (commented out - will be uncommented by gui-agent in Stage 3)
# juce_add_binary_data(SimpleSampler_UIResources
#     SOURCES
#         Source/ui/public/index.html
#         Source/ui/public/js/juce/index.js
#         Source/ui/public/js/juce/check_native_interop.js
# )
#
# target_link_libraries(SimpleSampler
#     PRIVATE
#         SimpleSampler_UIResources
# )

# Compile definitions
target_compile_definitions(SimpleSampler
    PUBLIC
        JUCE_VST3_CAN_REPLACE_VST2=0
)

# WebView support (commented out - will be uncommented by gui-agent in Stage 3)
# target_compile_definitions(SimpleSampler
#     PUBLIC
#         JUCE_WEB_BROWSER=1
#         JUCE_USE_CURL=0
# )
