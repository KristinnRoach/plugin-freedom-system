#!/bin/bash
set -e

# Batch install all Working plugins to system library
# Usage: ./install-all-working.sh

PLUGINS=("TapeAge" "DriveVerb" "FlutterVerb" "OrganicHats" "DrumRoulette" "Scatter" "AutoClip" "Drum808" "LushPad")
DATE=$(date +%Y-%m-%d)
INSTALLED_COUNT=0
FAILED_COUNT=0
FAILED_PLUGINS=()

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Batch Plugin Installation"
echo "Installing ${#PLUGINS[@]} plugins to /Library/Audio/Plug-Ins/"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

for PLUGIN in "${PLUGINS[@]}"; do
  echo "[$((INSTALLED_COUNT + FAILED_COUNT + 1))/${#PLUGINS[@]}] Processing $PLUGIN..."

  # Extract product name from CMakeLists.txt
  PRODUCT_NAME=$(grep 'PRODUCT_NAME' "plugins/$PLUGIN/CMakeLists.txt" | head -1 | cut -d'"' -f2)

  if [ -z "$PRODUCT_NAME" ]; then
    echo "  ❌ Failed: Could not extract product name"
    FAILED_COUNT=$((FAILED_COUNT + 1))
    FAILED_PLUGINS+=("$PLUGIN")
    continue
  fi

  echo "  Product name: $PRODUCT_NAME"

  # Check if Release build exists
  VST3_BUILD="build/plugins/$PLUGIN/${PLUGIN}_artefacts/Release/VST3/${PRODUCT_NAME}.vst3"
  AU_BUILD="build/plugins/$PLUGIN/${PLUGIN}_artefacts/Release/AU/${PRODUCT_NAME}.component"

  if [ ! -d "$VST3_BUILD" ] || [ ! -d "$AU_BUILD" ]; then
    echo "  Building $PLUGIN in Release mode..."
    cmake --build build --target ${PLUGIN}_VST3 ${PLUGIN}_AU --config Release

    if [ $? -ne 0 ]; then
      echo "  ❌ Failed: Build error"
      FAILED_COUNT=$((FAILED_COUNT + 1))
      FAILED_PLUGINS+=("$PLUGIN")
      continue
    fi
  else
    echo "  ✓ Release build exists"
  fi

  # Remove old versions
  if [ -d "/Library/Audio/Plug-Ins/VST3/${PRODUCT_NAME}.vst3" ]; then
    echo "  Removing old VST3..."
    rm -rf "/Library/Audio/Plug-Ins/VST3/${PRODUCT_NAME}.vst3"
  fi

  if [ -d "/Library/Audio/Plug-Ins/Components/${PRODUCT_NAME}.component" ]; then
    echo "  Removing old AU..."
    sudo rm -rf "/Library/Audio/Plug-Ins/Components/${PRODUCT_NAME}.component"
  fi

  # Copy to system folders
  echo "  Copying to system folders..."
  cp -R "$VST3_BUILD" /Library/Audio/Plug-Ins/VST3/
  sudo cp -R "$AU_BUILD" /Library/Audio/Plug-Ins/Components/

  # Set permissions
  chmod -R 755 "/Library/Audio/Plug-Ins/VST3/${PRODUCT_NAME}.vst3"
  sudo chmod -R 755 "/Library/Audio/Plug-Ins/Components/${PRODUCT_NAME}.component"

  # Verify installation
  if [ ! -d "/Library/Audio/Plug-Ins/VST3/${PRODUCT_NAME}.vst3" ] || [ ! -d "/Library/Audio/Plug-Ins/Components/${PRODUCT_NAME}.component" ]; then
    echo "  ❌ Failed: Installation verification failed"
    FAILED_COUNT=$((FAILED_COUNT + 1))
    FAILED_PLUGINS+=("$PLUGIN")
    continue
  fi

  # Update PLUGINS.md
  sed -i '' "s/| $PLUGIN | ✅ Working |/| $PLUGIN | 📦 Installed |/" PLUGINS.md
  sed -i '' "s/| $PLUGIN | 📦 Installed | \(.*\) | \(.*\) | .* |/| $PLUGIN | 📦 Installed | \1 | \2 | $DATE |/" PLUGINS.md

  # Create or update NOTES.md
  NOTES_FILE="plugins/$PLUGIN/NOTES.md"
  if [ ! -f "$NOTES_FILE" ]; then
    # Get version from PLUGINS.md
    VERSION=$(grep "^| $PLUGIN |" PLUGINS.md | awk -F'|' '{print $4}' | xargs)
    TYPE=$(grep "^| $PLUGIN |" PLUGINS.md | awk -F'|' '{print $5}' | xargs)

    cat > "$NOTES_FILE" <<EOF
# $PLUGIN Notes

## Status
- **Current Status:** 📦 Installed
- **Version:** $VERSION
- **Type:** $TYPE

## Lifecycle Timeline

- **$DATE:** Installed to system folders (VST3 + AU)

## Known Issues

None

## Additional Notes

**Installation Locations:**
- VST3: \`/Library/Audio/Plug-Ins/VST3/${PRODUCT_NAME}.vst3\`
- AU: \`/Library/Audio/Plug-Ins/Components/${PRODUCT_NAME}.component\`

**Formats:** VST3, AU, Standalone
EOF
  else
    # Update existing NOTES.md
    sed -i '' "s/^- \*\*Current Status:\*\* .*$/- **Current Status:** 📦 Installed/" "$NOTES_FILE"

    # Add timeline entry if not already present
    if ! grep -q "$DATE.*Installed to system folders" "$NOTES_FILE"; then
      sed -i '' "/^## Lifecycle Timeline$/a\\
- **$DATE:** Installed to system folders (VST3 + AU)
" "$NOTES_FILE"
    fi

    # Update installation locations if present
    if grep -q "Installation Locations:" "$NOTES_FILE"; then
      sed -i '' "s|VST3: \`.*${PRODUCT_NAME}.vst3\`|VST3: \`/Library/Audio/Plug-Ins/VST3/${PRODUCT_NAME}.vst3\`|" "$NOTES_FILE"
      sed -i '' "s|AU: \`.*${PRODUCT_NAME}.component\`|AU: \`/Library/Audio/Plug-Ins/Components/${PRODUCT_NAME}.component\`|" "$NOTES_FILE"
    fi
  fi

  echo "  ✓ $PLUGIN installed successfully"
  INSTALLED_COUNT=$((INSTALLED_COUNT + 1))
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
echo "✓ Installed: $INSTALLED_COUNT plugins"

if [ $FAILED_COUNT -gt 0 ]; then
  echo "❌ Failed: $FAILED_COUNT plugins"
  echo ""
  echo "Failed plugins:"
  for FAILED in "${FAILED_PLUGINS[@]}"; do
    echo "  - $FAILED"
  done
  echo ""
fi

echo "Installed plugins are now in:"
echo "  - /Library/Audio/Plug-Ins/VST3/"
echo "  - /Library/Audio/Plug-Ins/Components/"
echo ""
echo "Next steps:"
echo "  1. Open your DAW"
echo "  2. Rescan plugins (or restart DAW)"
echo "  3. Test your newly installed plugins"
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
