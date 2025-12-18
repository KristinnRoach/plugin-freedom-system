#!/bin/bash
set -e

# Batch build and install all remaining Working plugins
# Uses official build-and-install.sh for proper compilation

PLUGINS=("DriveVerb" "FlutterVerb" "OrganicHats" "DrumRoulette" "Scatter" "AutoClip" "Drum808" "LushPad" "AngelGrain")
DATE=$(date +%Y-%m-%d)
INSTALLED=()
FAILED=()

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Batch Build & Install"
echo "Processing ${#PLUGINS[@]} plugins"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

for PLUGIN in "${PLUGINS[@]}"; do
  echo "[$((${#INSTALLED[@]} + ${#FAILED[@]} + 1))/${#PLUGINS[@]}] Processing $PLUGIN..."

  # Phase 1: Build using official script (with --no-install to avoid permission issues)
  echo "  Building..."
  if ! ./scripts/build-and-install.sh "$PLUGIN" --no-install > /dev/null 2>&1; then
    echo "  ❌ Build failed"
    FAILED+=("$PLUGIN")
    continue
  fi

  # Phase 2: Verify binaries exist
  VST3_BIN="build/plugins/$PLUGIN/${PLUGIN}_artefacts/Release/VST3/${PLUGIN}.vst3/Contents/MacOS/${PLUGIN}"
  AU_BIN="build/plugins/$PLUGIN/${PLUGIN}_artefacts/Release/AU/${PLUGIN}.component/Contents/MacOS/${PLUGIN}"

  if [ ! -f "$VST3_BIN" ] || [ ! -f "$AU_BIN" ]; then
    echo "  ❌ Binaries missing after build"
    FAILED+=("$PLUGIN")
    continue
  fi

  echo "  ✓ Build complete (VST3: $(du -h "$VST3_BIN" | awk '{print $1}'), AU: $(du -h "$AU_BIN" | awk '{print $1}'))"

  # Phase 3: Install
  echo "  Installing..."

  # Remove old versions
  sudo rm -rf "/Library/Audio/Plug-Ins/Components/${PLUGIN}.component" 2>/dev/null || true
  rm -rf "/Library/Audio/Plug-Ins/VST3/${PLUGIN}.vst3" 2>/dev/null || true

  # Install new versions
  sudo cp -R "build/plugins/$PLUGIN/${PLUGIN}_artefacts/Release/AU/${PLUGIN}.component" /Library/Audio/Plug-Ins/Components/
  cp -R "build/plugins/$PLUGIN/${PLUGIN}_artefacts/Release/VST3/${PLUGIN}.vst3" /Library/Audio/Plug-Ins/VST3/

  # Phase 4: Code sign
  echo "  Code signing..."
  codesign --force --deep --sign - "/Library/Audio/Plug-Ins/VST3/${PLUGIN}.vst3" > /dev/null 2>&1
  sudo codesign --force --deep --sign - "/Library/Audio/Plug-Ins/Components/${PLUGIN}.component" > /dev/null 2>&1

  # Phase 5: Verify installation
  if [ ! -f "/Library/Audio/Plug-Ins/VST3/${PLUGIN}.vst3/Contents/MacOS/${PLUGIN}" ] || \
     [ ! -f "/Library/Audio/Plug-Ins/Components/${PLUGIN}.component/Contents/MacOS/${PLUGIN}" ]; then
    echo "  ❌ Installation verification failed"
    FAILED+=("$PLUGIN")
    continue
  fi

  # Phase 6: Update PLUGINS.md
  sed -i '' "s/| $PLUGIN | ✅ Working |/| $PLUGIN | 📦 Installed |/" PLUGINS.md
  sed -i '' "s/| $PLUGIN | 📦 Installed | \(.*\) | \(.*\) | .* |/| $PLUGIN | 📦 Installed | \1 | \2 | $DATE |/" PLUGINS.md

  echo "  ✓ $PLUGIN installed successfully"
  INSTALLED+=("$PLUGIN")
  echo ""
done

# Clear DAW caches
echo "Clearing DAW caches..."
rm -rf ~/Library/Caches/AudioUnitCache/* 2>/dev/null || true
killall AudioComponentRegistrar 2>/dev/null || true
echo "✓ Caches cleared"
echo ""

# Final report
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Installation Complete"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "✓ Installed: ${#INSTALLED[@]} plugins"

if [ ${#INSTALLED[@]} -gt 0 ]; then
  for PLUGIN in "${INSTALLED[@]}"; do
    echo "  - $PLUGIN"
  done
  echo ""
fi

if [ ${#FAILED[@]} -gt 0 ]; then
  echo "❌ Failed: ${#FAILED[@]} plugins"
  for PLUGIN in "${FAILED[@]}"; do
    echo "  - $PLUGIN"
  done
  echo ""
fi

echo "All plugins installed to:"
echo "  - /Library/Audio/Plug-Ins/VST3/"
echo "  - /Library/Audio/Plug-Ins/Components/"
echo ""
echo "Next steps:"
echo "  1. Open Ableton"
echo "  2. Click 'Rescan' in Plug-In Sources"
echo "  3. All plugins should now appear!"
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
