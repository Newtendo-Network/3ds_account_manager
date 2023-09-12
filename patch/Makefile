.PHONY: all clean

LUMA_PATH				:= luma
LUMA_SYSMODULE_PATH  	:= $(LUMA_PATH)/sysmodules
FRIENDS_MODULE_TID		:= 0004013000003202
FRIENDS_OUTPUT_PATH     := $(LUMA_SYSMODULE_PATH)/$(FRIENDS_MODULE_TID).ips

all: $(LUMA_SYSMODULE_PATH)

clean:
	@rm -rf $(LUMA_PATH)
	@rm friends_patch.ips friends_patched.bin

$(LUMA_SYSMODULE_PATH): $(FRIENDS_OUTPUT_PATH)
	@mkdir -p $@
	@cp friends_patch.ips $(FRIENDS_OUTPUT_PATH) 

$(FRIENDS_OUTPUT_PATH):
	@armips friends.s
	@flips -c friends.bin friends_patched.bin friends_patch.ips
	
	
