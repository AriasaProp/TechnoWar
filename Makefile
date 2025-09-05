BUILD_DIR := build
CORE_DIR  := core
TOOLS_DIR := tools
TESTS_DIR := tests

CORE_SRCS := \
	$(CORE_DIR)/common.c \
	$(CORE_DIR)/stb/stb_image.c \
	$(CORE_DIR)/stb/stb_local.c
TOOLS_SRCS := $(shell find $(TOOLS_DIR) -type f -name "*.c")
TESTS_SRCS := $(shell find $(TESTS_DIR) -type f -name "*.c")
	

CORE_OBJS := $(CORE_SRCS:%.c=$(BUILD_DIR)/%.o)
TESTS_OBJS := $(TESTS_SRCS:%.c=$(BUILD_DIR)/%.o)
TOOLS_OBJS := $(TOOLS_SRCS:%.c=$(BUILD_DIR)/%.o)

CORE_DEPS := $(CORE_OBJS:.o=.d)
TESTS_DEPS := $(TESTS_OBJS:.o=.d)
TOOLS_DEPS := $(TOOLS_OBJS:.o=.d)

# Flags
CFLAGS := -Werror -Wall -std=c23 -MMD -MP
LDFLAGS := -lm

qtest: CFLAGS += -O3
qtest: $(BUILD_DIR)/tmain.o
	@$(CC) $^ -o $@ $(LDFLAGS)
	@chmod +x $@
	@./qtest

# target test
test: CFLAGS += -O0 -g $(addprefix -I,$(TESTS_DIR) $(TOOLS_DIR) $(CORE_DIR)) $(addprefix -D,$(ARGS))
test: $(TESTS_OBJS) $(filter-out $(BUILD_DIR)/$(TOOLS_DIR)/main.o,$(TOOLS_OBJS)) $(CORE_OBJS)
	@echo "build test ..."
	@$(CC) $^ -o $@ $(LDFLAGS)
	@chmod +x $@
	@./test

# target tool
tool: CFLAGS += -O3 $(addprefix -I,$(TOOLS_DIR) $(CORE_DIR))
tool: $(TOOLS_OBJS) $(CORE_OBJS)
	@echo "build tools ..."
	@$(CC) $^ -o $@ $(LDFLAGS)

# internal sources
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(@D)
	@$(CC) -c $< -o $@ $(CFLAGS)

clean:
	@echo "cleanup build files ..."
	@rm -rf $(BUILD_DIR) test tool

-include $(CORE_DEPS) $(TOOLS_DEPS) $(TESTS_DEPS)
