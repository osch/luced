####################################################################################
#
# Makefile for transfering files to the LucED web server.
#
####################################################################################

BASE_DIR      ?= ../web
TIMESTAMP_DIR ?= $(BASE_DIR)/.transport-timestamps-tmp
SFTP_SCRIPT   ?= $(BASE_DIR)/.transport-sftp-script

HTDOCS_FILES := $(patsubst $(BASE_DIR)/%,%, $(wildcard $(BASE_DIR)/*.lhtml \
                                                       $(BASE_DIR)/*.ico   \
                                                       $(BASE_DIR)/*.png   \
                 )) \
                 index.shtml
                                             

CGIBIN_FILES := $(patsubst $(BASE_DIR)/%,%, $(wildcard $(BASE_DIR)/*.lua \
                 )) \
                 lua \
                 page_processor.sh
                                            

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


#####
# scripts
#
define QUEUE_HTDOCS_RUN
	@echo "queueing for htdocs:  $^"; \
	 echo "put $^ htdocs/$(patsubst $(TIMESTAMP_DIR)/%.htdocs.timestamp,%,$@)" \
	      >> $(SFTP_SCRIPT); \
	 touch $@
endef
define QUEUE_CGIBIN_RUN
	@echo "queueing for cgibin:  $^"; \
	 echo "put  $^ cgi-bin/$(patsubst $(TIMESTAMP_DIR)/%.cgibin.timestamp,%,$@)" \
	      >> $(SFTP_SCRIPT); \
	 touch $@
endef
