####################################################################################
#
# Makefile for transfering files to the LucED web server.
#
####################################################################################

BASE_DIR      ?= ../web
TIMESTAMP_DIR ?= $(BASE_DIR)/.transport-timestamps-tmp
SFTP_SCRIPT   ?= $(BASE_DIR)/.transport-sftp-script

# if lua-min exists in ../editor, then transfer it to the server
# lua-min can be build by invoking "make lua-min" in ../editor
HAVE_LUA_INTERPRETER = $(shell if [ -f $(BASE_DIR)/../editor/lua-min ]; \
                               then echo yes; fi)

HTDOCS_FILES := $(patsubst $(BASE_DIR)/%,%, $(wildcard $(BASE_DIR)/*.lhtml \
                                                       $(BASE_DIR)/*.ico   \
                                                       $(BASE_DIR)/*.png   \
                 )) \
                 index.shtml
                                             

CGIBIN_FILES := $(patsubst $(BASE_DIR)/%,%, $(wildcard $(BASE_DIR)/*.lua \
                 )) \
                 page_processor.sh
                                            
ifeq ($(HAVE_LUA_INTERPRETER),yes)
  CGIBIN_FILES += lua
endif


HTDOCS_TIMESTAMPS  := $(patsubst %, $(TIMESTAMP_DIR)/%.htdocs.timestamp, $(HTDOCS_FILES))
CGIBIN_TIMESTAMPS  := $(patsubst %, $(TIMESTAMP_DIR)/%.cgibin.timestamp, $(CGIBIN_FILES))


.PHONY: default transfer

default: transfer

transfer: $(HTDOCS_TIMESTAMPS) \
          $(CGIBIN_TIMESTAMPS)

#####
# transport htdocs
#
$(TIMESTAMP_DIR)/index.shtml.htdocs.timestamp: $(BASE_DIR)/server-index.shtml
	$(QUEUE_HTDOCS_RUN)
$(TIMESTAMP_DIR)/%.htdocs.timestamp: $(BASE_DIR)/%
	$(QUEUE_HTDOCS_RUN)


#####
# transport cgibin
#
$(TIMESTAMP_DIR)/page_processor.sh.cgibin.timestamp: $(BASE_DIR)/server-page_processor.sh
	$(QUEUE_CGIBIN_RUN)
$(TIMESTAMP_DIR)/%.cgibin.timestamp: $(BASE_DIR)/%
	$(QUEUE_CGIBIN_RUN)

ifeq ($(HAVE_LUA_INTERPRETER),yes)
$(BASE_DIR)/lua: $(BASE_DIR)/../editor/lua-min
	cp $^ $@
	chmod a+rx $@
endif


#####
# scripts
#
define QUEUE_HTDOCS_RUN
	@echo "queueing for htdocs:  $^"; \
	 echo "put -P $^ htdocs/$(patsubst $(TIMESTAMP_DIR)/%.htdocs.timestamp,%,$@)" \
	      >> $(SFTP_SCRIPT); \
	 touch $@
endef
define QUEUE_CGIBIN_RUN
	@echo "queueing for cgibin:  $^"; \
	 echo "put -P $^ cgi-bin/$(patsubst $(TIMESTAMP_DIR)/%.cgibin.timestamp,%,$@)" \
	      >> $(SFTP_SCRIPT); \
	 touch $@
endef
