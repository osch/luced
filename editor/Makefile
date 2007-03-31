####################################################################################
#
#   LucED - The Lucid Editor
#
#   Copyright (C) 2005-2006 Oliver Schmidt, oliver at luced dot de
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


CPPCOMP_PRG     := g++
LINK_PRG         = $(CPPCOMP_PRG)
CPPCOMP_OPTS    := -O2

BUILD_DIR       := build

LINK_X11_OPTS   := -L /usr/X11R6/lib/ -lX11 -lXpm
LINK_PCRE_OPTS  := -lpcre
LINK_LUA_OPTS   := $(shell if type lua-config >/dev/null 2>&1; then lua-config --libs; else echo "-llualib -llua"; fi)
LINK_EXTRA_OPTS := 
LINK_OPTS       :=

INCL_LUA_OPTS   := $(shell if type lua-config >/dev/null 2>&1; then lua-config --include; fi)
INCL_EXTRA_OPTS :=
INCL_OPTS       :=

# include sandbox-specific local settins in sandbox.mk, the
# above variables may be modified in this file:
#
include sandbox.mk


LINK_OPTS       += $(LINK_X11_OPTS) $(LINK_PCRE_OPTS) $(LINK_LUA_OPTS) $(LINK_EXTRA_OPTS)
INCL_OPTS       += $(INCL_LUA_OPTS) $(INCL_EXTRA_OPTS)

                 
MODULES = EditorTopWin           EventDispatcher     GuiRoot            GuiWidget \
          HeapMem                HilitingBase        KeyPressRepeater   ScrollBar \
                                 TextData            TextEditorWidget   TextStyle \
          TextWidget             TopWin              HilitedText        SyntaxPatterns \
          HilitingBuffer         LuaException        File               Regex \
          LuaObject              LuaInterpreter      ConfigException    GlobalConfig \
          LanguageModes          RegexException      BackliteBuffer     Clipboard \
          SelectionOwner         PasteDataReceiver   StatusLine         GuiLayoutRow \
          Button                 GuiLayoutColumn     GuiLayoutWidget    LuaStoredObject \
          GotoLinePanel          SingletonKeeper     TopWinList         TopWinOwner \
          DialogWin              GuiLayoutSpacer     MultiLineEditorWidget \
          SingleLineEditorWidget StandardEditActions HeapObject         LuaStackChecker \
          SingleLineEditField    LabelWidget         DialogPanel        PanelDialogWin \
          FindPanel              CheckBox            MessageBox         SearchHistory \
          FileException          EditorServer        ClientServerUtil   EditorClient  \
          CommandlineException
          

LIB_MBRS = $(patsubst %, $(BUILD_DIR)/libluced.a(%.o), $(MODULES))
LIB_OBJS = $(patsubst %, $(BUILD_DIR)/%.o,             $(MODULES))

.PHONY:        default clean
.INTERMEDIATE: $(LIB_OBJS)

default: luced lc

luced: $(BUILD_DIR)/luced.o $(BUILD_DIR)/libluced.a
	$(LINK_RUN)

lc: $(BUILD_DIR)/lc.o $(BUILD_DIR)/libluced.a
	$(LINK_RUN)

$(BUILD_DIR)/%.o: %.cpp
	$(COMPILE_RUN)

$(BUILD_DIR)/libluced.a: $(LIB_MBRS)

$(BUILD_DIR)/libluced.a(%.o): $(BUILD_DIR)/%.o
	ar -r $(BUILD_DIR)/libluced.a $<

clean:
	rm -rf $(BUILD_DIR)


##########################################################################
#
# Dependencies
#

# Spezialregel verhindert, dass bei fehlenden Includes,
# die von den Dependencies noch benoetigt werden,
# gar nicht mehr compiliert wird:
%.h: ;

# bei folgenden Goals keine Dependencies inkludieren:
IGNOREDEPS_GOALS := clean 

# damit es keine Warnung bei leerer Kommandozeile gibt:
MAKECMDGOALS ?=

ifeq ($(strip $(filter $(IGNOREDEPS_GOALS),$(MAKECMDGOALS))),)
  DEPS := $(wildcard $(BUILD_DIR)/deps/*.dep)
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
	$(CPPCOMP_PRG) -g $(CPPCOMP_OPTS) $(CPP_DEFINES) $(INCL_OPTS) -c $<  -o "$@" \
	-MMD -MF $(BUILD_DIR)/deps/$(*F).dep 
endef

define LINK_RUN
	@mkdir -p $(@D)
	$(LINK_PRG) -g $^ $(LINK_OPTS) -o $@
endef

