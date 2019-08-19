all: schnapsen_fds schnapsen_server schnapsen_client

schnapsen_fds: ../schnapsen_fds.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DNDEBUG -O3 -Wall -Wextra -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

schnapsen_client: ../schnapsen_client.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O3 -Wall -Wextra -fmessage-length=0 -pthread -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '
	
schnapsen_server: ../schnapsen_server.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O3 -Wall -Wextra -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

clean:
	rm -rf schnapsen_server schnapsen_client schnapsen_fds