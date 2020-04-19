PREFIX ?= /usr/local
BUILD_CONFIG ?= Release
EXECUTABLES = $(BUILD_CONFIG)/schnapsen_fds $(BUILD_CONFIG)/schnapsen_server $(BUILD_CONFIG)/schnapsen_client
DEPENDENCIES = $(foreach exe,$(EXECUTABLES),$(exe).d)

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),install)
-include $(DEPENDENCIES)
endif
endif

all: $(EXECUTABLES)

$(BUILD_CONFIG)/schnapsen_fds: schnapsen_fds.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
ifeq ($(BUILD_CONFIG), Debug)
	gcc -Og -ggdb -Wall -Wextra -fmessage-length=0 -MMD -MP -MF"$(@).d" -MT"$(@)" -o "$@" "$<"
else ifeq ($(BUILD_CONFIG), Release)
	gcc -DNDEBUG -O3 -Wall -Wextra -fmessage-length=0 -MMD -MP -MF"$(@).d" -MT"$(@)" -o "$@" "$<"
else
	$(error invalid build config $(BUILD_CONFIG))
endif
	@echo 'Finished building: $<'
	@echo ' '

$(BUILD_CONFIG)/schnapsen_client: schnapsen_client.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
ifeq ($(BUILD_CONFIG), Debug)
	gcc $(if $(PORT) ,-DPORT=$(PORT),) -Og -ggdb -Wall -Wextra -fmessage-length=0 -pthread -MMD -MP -MF"$(@).d" -MT"$(@)" -o "$@" "$<"
else ifeq ($(BUILD_CONFIG), Release)
	gcc $(if $(PORT) ,-DPORT=$(PORT),) -O3 -Wall -Wextra -fmessage-length=0 -pthread -MMD -MP -MF"$(@).d" -MT"$(@)" -o "$@" "$<"
else
	$(error invalid build config $(BUILD_CONFIG))
endif
	@echo 'Finished building: $<'
	@echo ' '
	
$(BUILD_CONFIG)/schnapsen_server: schnapsen_server.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
ifeq ($(BUILD_CONFIG), Debug)
	gcc $(if $(PORT) ,-DPORT=$(PORT),) -Og -ggdb -Wall -Wextra -fmessage-length=0 -MMD -MP -MF"$(@).d" -MT"$(@)" -o "$@" "$<"
else ifeq ($(BUILD_CONFIG), Release)
	gcc $(if $(PORT) ,-DPORT=$(PORT),) -O3 -Wall -Wextra -fmessage-length=0 -MMD -MP -MF"$(@).d" -MT"$(@)" -o "$@" "$<"
else
	$(error invalid build config $(BUILD_CONFIG))
endif
	@echo 'Finished building: $<'
	@echo ' '

clean:
	rm -rf $(EXECUTABLES) $(DEPENDENCIES)

install: $(EXECUTABLES)
	install -d $(DESTDIR)$(PREFIX)/bin/
	install $? $(DESTDIR)$(PREFIX)/bin/

.PHONY: all clean install