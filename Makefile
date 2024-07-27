# Variables
BUILD_DIR = build
BIN_DIR = bin
CMAKE_FLAGS = -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=$(CURDIR)/$(BIN_DIR) -DBUILD_EXAMPLES=ON

# Default target
all: release

# Release build
release:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake $(CMAKE_FLAGS) -DCMAKE_BUILD_TYPE=Release ..
	@$(MAKE) -C $(BUILD_DIR)

# Debug build
debug:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake $(CMAKE_FLAGS) -DCMAKE_BUILD_TYPE=Debug ..
	@$(MAKE) -C $(BUILD_DIR)

# Clean build directory
clean:
	@$(RM) -r $(BUILD_DIR)
	@$(RM) -r $(BIN_DIR)
	@$(RM) -r $(CURDIR)/lib
	@$(RM) -r $(CURDIR)/download

.PHONY: all release debug clean