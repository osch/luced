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

CPPCOMP_FAST_OPTS := -O2
CPPCOMP_SLOW_OPTS := -Os

BUILD_DIR       := build

LINK_X11_OPTS   := -L /usr/X11R6/lib/ -lX11 -lXpm
LINK_PCRE_OPTS  := -lpcre
LINK_LUA_OPTS   := -llua5.1
LINK_EXTRA_OPTS := 
LINK_OPTS       :=

INCL_LUA_OPTS   := -I /usr/include/lua5.1
INCL_EXTRA_OPTS :=
INCL_OPTS       :=

# include sandbox-specific local settins in sandbox.mk, the
# above variables may be modified in this file:
#
include sandbox.mk


LINK_OPTS       += $(LINK_X11_OPTS) $(LINK_PCRE_OPTS) $(LINK_LUA_OPTS) $(LINK_EXTRA_OPTS)
INCL_OPTS       += $(INCL_LUA_OPTS) $(INCL_EXTRA_OPTS)

 
SLOW_MODULES := EditorTopWin            GuiRoot                GuiWidget              SystemException \
                TextEditorWidget        TextStyle              KeyPressRepeater       ScrollBar \
                System                  LuaException           TopWin                 \
                LuaObject               LuaInterpreter         ConfigException        GlobalConfig \
                LanguageModes           RegexException         StatusLine             \
                Button                  GuiLayoutWidget        LuaStoredObject \
                GotoLinePanel           SingletonKeeper        TopWinList             TopWinOwner \
                DialogWin               GuiLayoutSpacer        MultiLineEditorWidget  SubstitutionException \
                SingleLineEditorWidget  StandardEditActions    LuaStackChecker \
                SingleLineEditField     LabelWidget            DialogPanel            PanelDialogWin \
                FindPanel               CheckBox               MessageBox             SearchHistory \
                FileException           EditorServer           ClientServerUtil       EditorClient  \
                CommandlineException    ReplacePanel           LuaFunctionArguments   GuiElement \
                SelectionOwner          PasteDataReceiver      Clipboard              FileOpener \
                ConfigErrorHandler      WindowCloser           SaveAsPanel
		
                         

FAST_MODULES := TextWidget              TextData               HilitingBase           HilitedText \
                HilitingBuffer          HeapMem                File                   Regex       \
                BackliteBuffer          GuiLayoutRow           GuiLayoutColumn        HeapObject  \
                EventDispatcher         FindUtil               ReplaceUtil            SyntaxPatterns \
                

PRG_MODULES  := luced

MODULES = $(FAST_MODULES) $(SLOW_MODULES)

LIB_MBRS  := $(patsubst %, $(BUILD_DIR)/libluced.a(%.o), $(MODULES))
LIB_OBJS  := $(patsubst %, $(BUILD_DIR)/%.o,             $(MODULES))
FAST_OBJS := $(patsubst %, $(BUILD_DIR)/%.o,             $(FAST_MODULES))
SLOW_OBJS := $(patsubst %, $(BUILD_DIR)/%.o,             $(SLOW_MODULES))
 PRG_OBJS := $(patsubst %, $(BUILD_DIR)/%.o,              $(PRG_MODULES))

.PHONY:        default clean
.INTERMEDIATE: $(LIB_OBJS)

default: luced

luced: $(BUILD_DIR)/luced.o $(BUILD_DIR)/libluced.a
	$(LINK_RUN)


$(FAST_OBJS): $(BUILD_DIR)/%.o: %.cpp
	$(call COMPILE_RUN,$(CPPCOMP_FAST_OPTS))

$(SLOW_OBJS): $(BUILD_DIR)/%.o: %.cpp
	$(call COMPILE_RUN,$(CPPCOMP_SLOW_OPTS))

$(PRG_OBJS): $(BUILD_DIR)/%.o: %.cpp
	$(call COMPILE_RUN,$(CPPCOMP_SLOW_OPTS))

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

%.hpp: ;


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
	$(CPPCOMP_PRG) -g $1 $(CPP_DEFINES) $(INCL_OPTS) -c $<  -o "$@" \
	-MMD -MF $(BUILD_DIR)/deps/$(*F).dep 
endef

define LINK_RUN
	@mkdir -p $(@D)
	$(LINK_PRG) -g $^ $(LINK_OPTS) -o $@
endef

