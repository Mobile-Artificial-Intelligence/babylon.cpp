# Variables
BUILD_DIR = build
BIN_DIR = bin
CMAKE_FLAGS = -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=$(CURDIR)/$(BIN_DIR)
CORES ?= 1

# Default target
all: release

# Release build
release:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake $(CMAKE_FLAGS) -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=ON ..
	@$(MAKE) -C $(BUILD_DIR) -j$(CORES)

# Source build
source:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake $(CMAKE_FLAGS) -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=ON -DBABYLON_BUILD_SOURCE=ON ..
	@$(MAKE) -C $(BUILD_DIR) -j$(CORES)

# android build
android:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake $(CMAKE_FLAGS) -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=$(ANDROID_NDK)/build/cmake/android.toolchain.cmake -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-21 ..
	@$(MAKE) -C $(BUILD_DIR) -j$(CORES)

# Debug build
debug:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake $(CMAKE_FLAGS) -DCMAKE_BUILD_TYPE=Debug ..
	@$(MAKE) -C $(BUILD_DIR) -j$(CORES)

# Clean build directory
clean:
	@$(RM) -r $(BUILD_DIR)
	@$(RM) -r $(BIN_DIR)
	@$(RM) -r $(CURDIR)/lib
	@$(RM) -r $(CURDIR)/download

.PHONY: all release source debug clean
