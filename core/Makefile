BIN_DIR := bin
OBJ_DIR := $(BIN_DIR)/objs
TOL_DIR := $(BIN_DIR)/tools
TES_DIR := $(BIN_DIR)/tests

# Common sources
STB_Src    := \
		$(OBJ_DIR)/ext/stb/image_read.c.o \
		$(OBJ_DIR)/ext/stb/image_write.c.o \
		$(OBJ_DIR)/ext/stb/truetype.c.o \
		$(OBJ_DIR)/ext/stb/rectpack.c.o \
		$(OBJ_DIR)/ext/stb/local.c.o

# Flags
CFLAGS := -Werror -Wall -std=c23 -MMD -MP -I./main -I./ext
TFLAGS := -O0 -g -I./tests -I./tools
OFLAGS := -O3 -I./tools
LDFLAGS := -lm -lc

define PROG =
$(1): CFLAGS += $(2)
$(1): $$(OBJ_DIR)/main/common.c.o $(3)
	@mkdir -p $$(@D)
	@$(CC) $$^ -o $$@ $$(LDFLAGS)
	@chmod +x $$@
endef

# non file aliases
.PHONY: all tests tools clean

# all
all: tests tools
	@echo "Do all"

# install all tools
tools: $(addprefix $(TOL_DIR)/, file_rewrite image_pack)
	@echo "Install all tools"

# do all tests
tests: $(addprefix $(TES_DIR)/, font_test codec_test rectpack_test )
	@for t in $^; do \
		echo "Running $$t"; \
		./$$t || { echo "Running fail"; exit 1; } \
	done
	@echo "All Tests Passed"

$(eval $(call PROG,$(TES_DIR)/font_test, $(TFLAGS), \
		$(STB_Src) \
		$(OBJ_DIR)/main/math/vec.c.o \
		$(OBJ_DIR)/main/math/matrix.c.o \
		$(OBJ_DIR)/tests/font_test.c.o \
))
$(eval $(call PROG,$(TES_DIR)/codec_test, $(TFLAGS), \
		$(STB_Src) \
		$(OBJ_DIR)/main/math/vec.c.o \
		$(OBJ_DIR)/main/math/matrix.c.o \
		$(OBJ_DIR)/tools/codec/image.c.o \
		$(OBJ_DIR)/tests/codec_test.c.o \
))
$(eval $(call PROG,$(TES_DIR)/rectpack_test, $(TFLAGS), \
		$(STB_Src) \
		$(OBJ_DIR)/main/math/vec.c.o \
		$(OBJ_DIR)/main/math/matrix.c.o \
		$(OBJ_DIR)/tests/rectpack_test.c.o \
))
$(eval $(call PROG,$(TOL_DIR)/file_rewrite, $(OFLAGS), \
		$(STB_Src) \
		$(OBJ_DIR)/main/math/vec.c.o \
		$(OBJ_DIR)/main/math/matrix.c.o \
		$(OBJ_DIR)/tools/codec/image.c.o \
		$(OBJ_DIR)/tools/file_rewrite.c.o \
))
$(eval $(call PROG,$(TOL_DIR)/image_pack, $(OFLAGS), \
		$(STB_Src) \
		$(OBJ_DIR)/main/math/vec.c.o \
		$(OBJ_DIR)/main/math/matrix.c.o \
		$(OBJ_DIR)/tools/image_pack.c.o \
))

# internal sources
$(OBJ_DIR)/%.c.o: %.c
	@mkdir -p $(@D)
	@$(CC) -c $< -o $@ $(CFLAGS)

clean:
	@rm -rf $(BIN_DIR)

-include $(shell find $(OBJ_DIR) -type f -name "*.d")
