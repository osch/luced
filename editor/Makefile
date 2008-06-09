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

LUA_MODULES            := functions             BuiltinActionDefinitions
GENERATED_HEADERS      := CallbackContainer     Callback                  ActionId
GENERATED_SLOW_MODULES := ActionId              ActionMethodBinding       DefaultActionKeyConfig

GENERATED_FAST_MODULES :=


SLOW_MODULES := EditorTopWin             GuiRoot                GuiWidget                  SystemException \
                TextEditorWidget         TextStyle              KeyPressRepeater           ScrollBar \
                System                   LuaException           TopWin                     \
                LuaObject                LuaInterpreter         ConfigException            GlobalConfig \
                LanguageModes            RegexException         StatusLine                 \
                Button                   GuiLayoutWidget        LuaStoredObject \
                GotoLinePanel            SingletonKeeper        TopWinList                 OwnedTopWins \
                DialogWin                GuiLayoutSpacer        MultiLineEditorWidget      SubstitutionException \
                SingleLineEditorWidget   LuaStackChecker \
                SingleLineEditField      LabelWidget            DialogPanel                PanelDialogWin \
                FindPanel                CheckBox               MessageBox                 SearchHistory \
                FileException            EditorServer           ClientServerUtil           EditorClient  \
                CommandlineException     ReplacePanel           LuaFunctionArguments       GuiElement \
                SelectionOwner           PasteDataReceiver      Clipboard                  FileOpener \
                ConfigErrorHandler       WindowCloser           SaveAsPanel                KeyModifier \
		CurrentDirectoryKeeper   ProgramExecutor        TextDisplayGuiCompound     CommandOutputBox \
		FramedGuiCompound        EditorTopWinActions    UnknownActionNameException ScrollableTextGuiCompound \
		SearchInteraction        ActionKeyConfig        SingleLineEditActions      MultiLineEditActions \
		ActionKeySequenceHandler
                         

FAST_MODULES := TextWidget              TextData               HilitingBase           HilitedText \
                HilitingBuffer          HeapMem                File                   BasicRegex  \
                BackliteBuffer          GuiLayoutRow           GuiLayoutColumn        HeapObject  \
                EventDispatcher         FindUtil               ReplaceUtil            SyntaxPatterns \
                
PRG_MODULES  := luced


GENERATED_MODULES  := $(GENERATED_SLOW_MODULES) $(GENERATED_FAST_MODULES)

MODULES            :=           $(FAST_MODULES)           $(SLOW_MODULES) \
                      $(GENERATED_FAST_MODULES) $(GENERATED_SLOW_MODULES)

LIB_MBRS      := $(patsubst %, $(BUILD_DIR)/libluced.a(%.o),    $(MODULES))
LIB_OBJS      := $(patsubst %, $(BUILD_DIR)/%.o,                $(MODULES))
FAST_OBJS     := $(patsubst %, $(BUILD_DIR)/%.o,                $(FAST_MODULES))
SLOW_OBJS     := $(patsubst %, $(BUILD_DIR)/%.o,                $(SLOW_MODULES))
 PRG_OBJS     := $(patsubst %, $(BUILD_DIR)/%.o,                 $(PRG_MODULES))

GEN_HDRS      := $(patsubst %, $(BUILD_DIR)/%.hpp,         $(GENERATED_HEADERS))
GEN_HDRS_DEPS := $(patsubst %, $(BUILD_DIR)/%.hpp.gen.dep, $(GENERATED_HEADERS))

GEN_SRC       := $(patsubst %, $(BUILD_DIR)/%.cpp,         $(GENERATED_MODULES))
GEN_SRC_DEPS  := $(patsubst %, $(BUILD_DIR)/%.cpp.gen.dep, $(GENERATED_MODULES))

GEN_FAST_OBJS := $(patsubst %, $(BUILD_DIR)/%.o,      $(GENERATED_FAST_MODULES))
GEN_SLOW_OBJS := $(patsubst %, $(BUILD_DIR)/%.o,      $(GENERATED_SLOW_MODULES))


LUA_FILES     := $(patsubst %, %.lua, $(LUA_MODULES))

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
$(BUILD_DIR)/headers_generated: $(GEN_HDRS) $(GEN_HDRS_DEPS)  #
	@mkdir -p $(@D); touch $@                             #
#                                                             #
endif                                                         #
###############################################################

##########################################################################
#
# Code Generating Rules
#
#  Header files are new generated if the *.lua files have been changed. 
#  However the new generated headers are only relevant if they are 
#  different from the previously generated headers (for this *.hpp.gen.dep
#  is used).
#

$(BUILD_DIR)/%.hpp.gen.dep $(BUILD_DIR)/%.new.hpp: %.hpp.emlua $(LUA_FILES)
	$(call EMLUA_RUN,                   $(patsubst %.hpp.emlua,$(BUILD_DIR)/%.new.hpp,$<))
	$(call CONDITIONAL_DEPENDECY_RUN,   $(patsubst %.hpp.emlua,$(BUILD_DIR)/%.hpp.gen.dep,$<), \
	                                    $(patsubst %.hpp.emlua,$(BUILD_DIR)/%.new.hpp,$<),     \
	                                    $(patsubst %.hpp.emlua,$(BUILD_DIR)/%.hpp,$<) )
	                                    


$(BUILD_DIR)/%.cpp.gen.dep $(BUILD_DIR)/%.new.cpp: %.cpp.emlua $(LUA_FILES)
	$(call EMLUA_RUN,                   $(patsubst %.cpp.emlua,$(BUILD_DIR)/%.new.cpp,$<))
	$(call CONDITIONAL_DEPENDECY_RUN,   $(patsubst %.cpp.emlua,$(BUILD_DIR)/%.cpp.gen.dep,$<), \
	                                    $(patsubst %.cpp.emlua,$(BUILD_DIR)/%.new.cpp,$<),     \
	                                    $(patsubst %.cpp.emlua,$(BUILD_DIR)/%.cpp,$<) )
	                                    


##########################################################################
#
# Generic Rules
#

$(FAST_OBJS): $(BUILD_DIR)/%.o: %.cpp
	$(call COMPILE_RUN,$(CPPCOMP_FAST_OPTS))

$(GEN_FAST_OBJS): $(BUILD_DIR)/%.o: $(BUILD_DIR)/%.cpp
	$(call COMPILE_RUN,$(CPPCOMP_FAST_OPTS))


$(SLOW_OBJS): $(BUILD_DIR)/%.o: %.cpp
	$(call COMPILE_RUN,$(CPPCOMP_SLOW_OPTS))

$(GEN_SLOW_OBJS): $(BUILD_DIR)/%.o: $(BUILD_DIR)/%.cpp
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
# Generic Dependencies
#

%.h: ;

%.hpp: ;


ifeq ($(filter $(NONBUILD_GOALS),$(MAKECMDGOALS)),)
  DEPS := $(wildcard $(BUILD_DIR)/deps/*.dep) $(GEN_HDRS_DEPS) $(GEN_SRC_DEPS)
  ifneq ($(DEPS),)
    -include $(DEPS)
  endif
endif


##########################################################################
#
# Runs
#

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


###################
# Parameters:
#
# $1:  target file
#
define EMLUA_RUN
	@echo "Generating $< --> $(strip $1)"; \
	mkdir -p $(@D); \
	rm -f $1; \
	lua em.lua $< > $1; rc=$$?; \
	if [ 0 -ne $$rc ]; then rm -f $1; else chmod a-w $1; fi; \
	exit $$rc
endef


###################
# Parameters:
#
# $1:  target dependency file
# $2:  target new intermediate file
# $3:  real target file
#
define CONDITIONAL_DEPENDECY_RUN
	@( printf "%s\n"    '$3: $2'; \
	   printf "\t@%s\n"        'echo      "Comparing $(strip $2) ---> $(strip $3)"; \
	                            if [ ! -f $3 ] || ! diff $2 $3 >/dev/null 2>&1; \
	                            then echo "Copying   $(strip $2) ---> $(strip $3)"; \
	                                 rm -f $3; cp $2 $3; chmod a-w $3; \
	                            fi; \
	                            echo " " > $1; \
	                           '; \
	 ) > $1
endef

