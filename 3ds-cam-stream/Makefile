#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>/devkitARM")
endif

ifeq ($(strip $(DEVKITPRO)),)
DEVKITPRO := /opt/devkitpro
endif

SMDHTOOL := $(DEVKITPRO)/tools/bin/smdhtool$(EXEEXT)
3DSXTOOL := $(DEVKITPRO)/tools/bin/3dsxtool$(EXEEXT)
MAKEROM  := $(DEVKITPRO)/tools/bin/makerom$(EXEEXT)

include $(DEVKITARM)/3ds_rules

export CC CXX LD CPP AR STRIP OBJCOPY NM RANLIB

#---------------------------------------------------------------------------------
TARGET		:=	3ds-cam-stream
BUILD		:=	build
SOURCES		:=	source
INCLUDES	:=	include source
DATA		:=	data
GFX		:=	gfx

APP_TITLE	:=	3DS2SPOUT
APP_DESCRIPTION := Camera stream to Spout/Resolume
APP_AUTHOR	:=	Elisalien

# Icon + metadata for Homebrew Launcher (and CIA)
NO_SMDH		:=

# Release / CIA
VERSION		?=	1.0.0
RELEASE_PC_IP	?=	192.168.0.1
REPO_ROOT	:=	$(abspath $(TOPDIR)/..)
RELEASE_DIR	:=	$(TOPDIR)/release/3DS2SPOUT-$(VERSION)
RELEASE_ZIP	:=	$(TOPDIR)/release/3DS2SPOUT-$(VERSION).zip

CIA_TITLEID	:=	0x0100000003300001
CIA_PRODUCTCODE	:=	CTR-P-CAMS
CIA_RSF		:=	$(TOPDIR)/cia/cia.rsf

# Override at build/install time: make PC_IP=192.168.1.42 PC_PORT=5000
PC_IP		?=	10.129.63.189
PC_PORT		?=	5000

#---------------------------------------------------------------------------------
ARCH	:=	-march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft

ASFLAGS	:=	-g $(ARCH)
LDFLAGS	=	-specs=3dsx.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)

LIBS	:= -lctru -lm

#---------------------------------------------------------------------------------
LIBDIRS	:= $(CTRULIB) $(PORTLIBS)
#---------------------------------------------------------------------------------

ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)
export TOPDIR	:=	$(CURDIR)

ICONPNG		:=	$(TOPDIR)/gfx/icon.png

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
			$(foreach dir,$(DATA),$(CURDIR)/$(dir)) \
			$(foreach dir,$(GFX),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
PICAFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.v.pica)))
SHLISTFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.shlist)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*)))

export CFILES CPPFILES SFILES PICAFILES SHLISTFILES BINFILES
export OFILES	:=	$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export OUTPUT
export TOPDIR
export VPATH
export DEPSDIR
export APP_TITLE APP_DESCRIPTION APP_AUTHOR
export ICON NO_SMDH

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(BUILD)
export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)

CFLAGS	:=	-g -Wall -O2 -mword-relocations -fomit-frame-pointer -ffast-math $(ARCH)
CFLAGS	+=	$(INCLUDE) -D__3DS__ -DDEFAULT_PC_IP=\"$(PC_IP)\" -DDEFAULT_PC_PORT=$(PC_PORT)
CXXFLAGS	:= $(CFLAGS) -fno-rtti -fno-exceptions

export ARCH CFLAGS CXXFLAGS ASFLAGS LDFLAGS LIBS
export CC CXX LD
export LDSCRIPT	:= $(TOPDIR)/3dsx.ld

export SMDHFILE	:=	$(CURDIR)/$(TARGET).smdh

.PHONY: $(BUILD) clean all icon smdh bnr cia release release-zip

all: icon smdh $(BUILD)

icon:
	@if [ ! -f $(ICONPNG) ]; then \
		echo generating $(ICONPNG); \
		python3 $(TOPDIR)/gfx/generate-icon.py || python $(TOPDIR)/gfx/generate-icon.py; \
	fi

smdh: icon $(TARGET).smdh

$(TARGET).smdh: $(ICONPNG)
	@echo creating $(notdir $@)
	@$(SMDHTOOL) --create "$(APP_TITLE)" "$(APP_DESCRIPTION)" "$(APP_AUTHOR)" $< $@

bnr: icon $(TARGET).bnr

$(TARGET).bnr: $(TARGET).smdh
	@echo creating $(notdir $@)
	@cp $< $@

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

cia: all bnr $(TARGET).cia

$(TARGET).cia: $(TARGET).elf $(TARGET).smdh $(TARGET).bnr
	@if [ ! -x "$(MAKEROM)" ] && [ ! -f "$(MAKEROM)" ]; then \
		echo "makerom not found — install 3ds-dev tools or skip: make cia"; exit 1; \
	fi
	@echo creating $(notdir $@)
	@$(MAKEROM) -f cia -o $@ -elf $(TARGET).elf -icon $(TARGET).smdh -banner $(TARGET).bnr \
		-target t -rsf $(CIA_RSF) -titleid $(CIA_TITLEID) -exefslogo

release: icon smdh
	@echo "=== Release $(VERSION) (PC_IP=$(RELEASE_PC_IP)) ==="
	@$(MAKE) --no-print-directory PC_IP=$(RELEASE_PC_IP) $(BUILD)
	@rm -rf $(RELEASE_DIR)
	@mkdir -p $(RELEASE_DIR)
	@cp $(TARGET).3dsx $(RELEASE_DIR)/
	@cp $(TOPDIR)/example.cfg $(RELEASE_DIR)/3ds-cam-stream.cfg.example
	@cp $(TOPDIR)/release/README.txt $(RELEASE_DIR)/README.txt
	@cp $(REPO_ROOT)/README.md $(REPO_ROOT)/README.en.md $(REPO_ROOT)/LICENSE $(REPO_ROOT)/run-bridge.cmd $(REPO_ROOT)/install-buildtools.bat $(RELEASE_DIR)/
	@cp -r $(REPO_ROOT)/install $(REPO_ROOT)/docs $(REPO_ROOT)/resolume $(RELEASE_DIR)/
	@mkdir -p $(RELEASE_DIR)/pc-bridge
	@cp -r $(REPO_ROOT)/pc-bridge/src $(REPO_ROOT)/pc-bridge/vendor $(RELEASE_DIR)/pc-bridge/
	@cp $(REPO_ROOT)/pc-bridge/receiver.py $(REPO_ROOT)/pc-bridge/requirements.txt $(REPO_ROOT)/pc-bridge/config.toml $(REPO_ROOT)/pc-bridge/Cargo.toml $(REPO_ROOT)/pc-bridge/README.md $(RELEASE_DIR)/pc-bridge/ 2>/dev/null || true
	@rm -f $(RELEASE_DIR)/pc-bridge/vendor/*.dll
	@rm -f $(RELEASE_ZIP)
	@(cd $(TOPDIR)/release && zip -r $(notdir $(RELEASE_ZIP)) $(notdir $(RELEASE_DIR)))
	@echo ""
	@echo "OK: $(RELEASE_ZIP)"
	@echo "     + $(RELEASE_DIR)/"

release-zip: release

clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET).3dsx $(TARGET).smdh $(TARGET).bnr $(TARGET).cia $(TARGET).elf
	@rm -fr $(TOPDIR)/release/3DS2SPOUT-*

else

DEPENDS	:=	$(CFILES:.c=.d) $(CPPFILES:.cpp=.d) $(SFILES:.s=.d)

all: $(OUTPUT).3dsx

ICONOBJS :=

$(OUTPUT).elf: $(OFILES) $(GFXFILES) $(BINFILES) $(ICONOBJS)
	@echo linking $(notdir $@)
	@$(CXX) $(LDFLAGS) $(OFILES) $(GFXFILES) $(BINFILES) $(ICONOBJS) -o $@ $(LIBPATHS) $(LIBS)

%.3dsx: %.elf
	@echo creating $(notdir $@)
	@$(3DSXTOOL) $< $@ --smdh=$(SMDHFILE)

-include $(DEPENDS)

endif
