# Makefile for Juno60 SuperCollider plugin

# SuperCollider path - can be overridden with make SC_PATH=/path/to/sc
SC_PATH ?= $(shell pwd)/supercollider

# Plugin directory
PLUGIN_DIR = sc-plugin

# Build directory
BUILD_DIR = $(PLUGIN_DIR)/build

# CMake options
CMAKE_FLAGS = -DSC_PATH=$(SC_PATH)

.PHONY: all clean install uninstall sc-clone sc-setup test

all: sc-setup $(BUILD_DIR)/Makefile
	@echo "Building Juno60 plugin..."
	$(MAKE) -C $(BUILD_DIR)

$(BUILD_DIR)/Makefile:
	@echo "Configuring build with CMake..."
	@mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake .. $(CMAKE_FLAGS)

# Clone SuperCollider if it doesn't exist
sc-clone:
	@if [ ! -d "$(SC_PATH)" ]; then \
		echo "Cloning SuperCollider..."; \
		git clone --depth 1 --branch Version-3.14.0 https://github.com/supercollider/supercollider $(SC_PATH); \
	else \
		echo "SuperCollider already exists at $(SC_PATH)"; \
	fi

# Update submodules for SuperCollider
sc-setup: sc-clone
	@if [ -d "$(SC_PATH)/.git" ]; then \
		echo "Updating SuperCollider submodules..."; \
		cd $(SC_PATH) && git submodule update --init --recursive; \
	fi

install: all
	@echo "Installing Juno60 plugin..."
	$(MAKE) -C $(BUILD_DIR) install

uninstall:
	@echo "Uninstalling Juno60 plugin..."
	@if [ -d "$(BUILD_DIR)" ]; then \
		$(MAKE) -C $(BUILD_DIR) uninstall; \
	else \
		echo "Build directory not found. Creating it to uninstall..."; \
		mkdir -p $(BUILD_DIR); \
		cd $(BUILD_DIR) && cmake .. $(CMAKE_FLAGS); \
		$(MAKE) -C $(BUILD_DIR) uninstall; \
	fi
	@EXT_DIR="$(HOME)/.local/share/SuperCollider/Extensions"; \
	if [ -d "$$EXT_DIR/Juno60" ]; then \
		echo "Removing plugin from $$EXT_DIR/Juno60"; \
		rm -rf "$$EXT_DIR/Juno60"; \
	else \
		echo "Plugin not found in $$EXT_DIR/Juno60"; \
	fi

clean:
	@echo "Cleaning build files..."
	@rm -rf $(BUILD_DIR)

test: uninstall clean install
	timeout 6s sclang test.scd

help:
	@echo "Juno60 SuperCollider Plugin - Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  all        - Build the plugin (default)"
	@echo "  install    - Build and install the plugin to Extensions folder"
	@echo "  uninstall  - Remove the plugin from Extensions folder"
	@echo "  clean      - Remove build files"
	@echo "  sc-clone   - Clone SuperCollider repository"
	@echo "  sc-setup   - Setup SuperCollider (clone + submodules)"
	@echo "  test       - Show instructions for testing"
	@echo "  help       - Show this help message"
	@echo ""
	@echo "Variables:"
	@echo "  SC_PATH    - Path to SuperCollider source (default: ./supercollider)"
	@echo ""
	@echo "Examples:"
	@echo "  make"
	@echo "  make install"
	@echo "  make SC_PATH=/path/to/supercollider install"
	@echo "  make uninstall"
