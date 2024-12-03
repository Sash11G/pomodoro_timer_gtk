#!/bin/bash

# Paths
SRC_DIR="src"           # Source code directory
BUILD_DIR="build"       # Build output directory
OUTPUT_FILE="pomodoro_gui_copy" # Output binary name


# Compiler and flags
CC="gcc"                                   # Compiler
CFLAGS="$(pkg-config --cflags gtk+-3.0)"  # GTK include flags
LDFLAGS="$(pkg-config --libs gtk+-3.0)"   # GTK library flags
EXTRA_FLAGS="-Wall -Wextra -g"            # Additional warnings and debug info


# Source file
SRC_FILE="$SRC_DIR/pomodoro_gui_copy.c"        # Update the source file path

# Ensure build directory exists
mkdir -p "$BUILD_DIR"

# Compile the program
$CC $SRC_FILE -o "$BUILD_DIR/$OUTPUT_FILE" $CFLAGS $LDFLAGS $EXTRA_FLAGS

# Check if compilation was successful
if [ $? -eq 0 ]; then
    echo "Compilation successful. Binary: $BUILD_DIR/$OUTPUT_FILE"
else
    echo "Compilation failed."
fi
