####################################################################################
#
#   LucED - The Lucid Editor
#
#   Copyright (C) 2005-2008 Oliver Schmidt, oliver at luced dot de
#
#   This program is free software; you can redistribute it and/or modify it
#   under the terms of the GNU General Public License Version 2 as published
#   by the Free Software Foundation in June 1991.
#
#   This program is distributed in the hope that it will be useful, but WITHOUT
#   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
#   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
#   more details.
#
#   You should have received a copy of the GNU General Public License along with 
#   this program; if not, write to the Free Software Foundation, Inc., 
#   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#
####################################################################################


CPPCOMP_PRG         := g++
LINK_PRG             = $(CPPCOMP_PRG)
CPPCOMP_INVOKE_OPTS := -g
LINK_INVOKE_OPTS    := -g

CPPCOMP_FAST_OPTS := -O2
CPPCOMP_SLOW_OPTS := -Os

BUILD_DIR       := build

LINK_X11_OPTS   := -L /usr/X11R6/lib/ -lX11 -lXpm -lXext
LINK_PCRE_OPTS  := -lpcre
LINK_LUA_OPTS   := -llua5.1
LINK_EXTRA_OPTS := 
LINK_OPTS       :=

INCL_LUA_OPTS   := -I /usr/include/lua5.1
INCL_EXTRA_OPTS :=
INCL_OPTS       :=

# include sandbox-specific local settings from sandbox.mk,
# the above variables may be modified in this file:
#
include sandbox.mk

LINK_OPTS       += $(LINK_X11_OPTS) $(LINK_PCRE_OPTS) $(LINK_LUA_OPTS) $(LINK_EXTRA_OPTS)
INCL_OPTS       += $(INCL_LUA_OPTS) $(INCL_EXTRA_OPTS)

GENERATED_HEADERS := \
                CallbackContainer       Callback

SLOW_MODULES := EditorTopWin            GuiRoot                GuiWidget              SystemException \
                TextEditorWidget        TextStyle              KeyPressRepeater       ScrollBar \
                System                  LuaException           TopWin                 \
                LuaObject               LuaInterpreter         ConfigException        GlobalConfig \
                LanguageModes           RegexException         StatusLine             \
                Button                  GuiLayoutWidget        LuaStoredObject \
                GotoLinePanel           SingletonKeeper        TopWinList             OwnedTopWins \
                DialogWin               GuiLayoutSpacer        MultiLineEditorWidget  SubstitutionException \
                SingleLineEditorWidget  StandardEditActions    LuaStackChecker \
                SingleLineEditField     LabelWidget            DialogPanel            PanelDialogWin \
                FindPanel               CheckBox               MessageBox             SearchHistory \
                FileException           EditorServer           ClientServerUtil       EditorClient  \
                CommandlineException    ReplacePanel           LuaFunctionArguments   GuiElement \
                SelectionOwner          PasteDataReceiver      Clipboard              FileOpener \
                ConfigErrorHandler      WindowCloser           SaveAsPanel            KeyModifier \
		CurrentDirectoryKeeper  ProgramExecutor        TextDisplayGuiCompound CommandOutputBox \
		FramedGuiCompound       BuiltinTopWinActions   TopWinActionsRegistry  ScrollableTextGuiCompound \
		SearchInteraction       UnknownActionNameException
                         

FAST_MODULES := TextWidget              TextData               HilitingBase           HilitedText \
                HilitingBuffer          HeapMem                File                   BasicRegex  \
                BackliteBuffer          GuiLayoutRow           GuiLayoutColumn        HeapObject  \
                EventDispatcher         FindUtil               ReplaceUtil            SyntaxPatterns \
                
PRG_MODULES  := luced

MODULES   := $(FAST_MODULES) $(SLOW_MODULES)

LIB_MBRS  := $(patsubst %, $(BUILD_DIR)/libluced.a(%.o), $(MODULES))
LIB_OBJS  := $(patsubst %, $(BUILD_DIR)/%.o,             $(MODULES))
FAST_OBJS := $(patsubst %, $(BUILD_DIR)/%.o,             $(FAST_MODULES))
SLOW_OBJS := $(patsubst %, $(BUILD_DIR)/%.o,             $(SLOW_MODULES))
 PRG_OBJS := $(patsubst %, $(BUILD_DIR)/%.o,              $(PRG_MODULES))
GEN_HDRS  := $(patsubst %, $(BUILD_DIR)/%.hpp,      $(GENERATED_HEADERS))

MAKECMDGOALS   ?=
NONBUILD_GOALS := clean
.PHONY:           clean default complink

default: luced

clean:
	rm -rf $(BUILD_DIR)

luced: $(BUILD_DIR)/luced.o $(BUILD_DIR)/libluced.a
	$(LINK_RUN)

complink: luced.cpp $(patsubst %, %.cpp, $(MODULES))
	$(call COMPILE_LINK_RUN,,luced,$^)


###############################################################
#                                                             #
# Assure that headers are generated before anything else is   #
# build:                                                      #
#                                                             #
ifeq ($(filter $(NONBUILD_GOALS),$(MAKECMDGOALS)),)           #
#                                                             #
-include $(BUILD_DIR)/headers_generated                       #
$(BUILD_DIR)/headers_generated: $(GEN_HDRS)                   #
	@mkdir -p $(@D); touch $@                             #
#                                                             #
endif                                                         #
###############################################################


$(GEN_HDRS): $(BUILD_DIR)/%.hpp: %.hpp.emlua functions.lua
	$(call EMLUA_RUN)
	
$(FAST_OBJS): $(BUILD_DIR)/%.o: %.cpp
	$(call COMPILE_RUN,$(CPPCOMP_FAST_OPTS))

$(SLOW_OBJS): $(BUILD_DIR)/%.o: %.cpp
	$(call COMPILE_RUN,$(CPPCOMP_SLOW_OPTS))

$(PRG_OBJS): $(BUILD_DIR)/%.o: %.cpp
	$(call COMPILE_RUN,$(CPPCOMP_SLOW_OPTS))

$(BUILD_DIR)/libluced.a: $(LIB_OBJS)
	@rm -f $@; 
	ar -r $@ $^

##
# no rules for member creation within archive files
# because of parallel build problems.
#
#
#$(BUILD_DIR)/libluced.a: $(LIB_MBRS)
#
#$(BUILD_DIR)/libluced.a(%.o): $(BUILD_DIR)/%.o
#	ar -r $(BUILD_DIR)/libluced.a $<
##



##########################################################################
#
# Dependencies
#

%.h: ;

%.hpp: ;


ifeq ($(filter $(NONBUILD_GOALS),$(MAKECMDGOALS)),)
  DEPS := $(wildcard $(BUILD_DIR)/deps/*.dep)
  ifneq ($(DEPS),)
    -include $(DEPS)
  endif
endif


##########################################################################
#
# Runs
#

define EMLUA_RUN
	@echo "Generating $< --> $@"; \
	mkdir -p $(@D); \
	lua em.lua $< > $@; rc=$$?; \
	if [ 0 -ne $$rc ]; then rm -f $@; fi; \
	exit $$rc
endef

define COMPILE_RUN
	@mkdir -p $(@D) && mkdir -p $(BUILD_DIR)/deps
	$(CPPCOMP_PRG) $(CPPCOMP_INVOKE_OPTS) -I . -I $(BUILD_DIR) $1 $(CPP_DEFINES) $(INCL_OPTS) \
	-c $<  -o "$@" \
	-MMD -MF $(BUILD_DIR)/deps/$(*F).dep 
endef

define LINK_RUN
	@mkdir -p $(@D)
	$(LINK_PRG) $(LINK_INVOKE_OPTS) $^ $(LINK_OPTS) -o $@
endef

define COMPILE_LINK_RUN
	$(CPPCOMP_PRG) $(CPPCOMP_INVOKE_OPTS) -I . -I $(BUILD_DIR) $1 $(CPP_DEFINES) $(INCL_OPTS) \
	$(LINK_OPTS) \
	$3 -o "$2"
endef
