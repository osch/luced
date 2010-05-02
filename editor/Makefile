####################################################################################
#
#   LucED - The Lucid Editor
#
#   Copyright (C) 2005-2010 Oliver Schmidt, oliver at luced dot de
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

ifneq (,)
This makefile requires GNU Make.
endif

CPPCOMP_PRG          = $(CXX)
LINK_PRG             = $(CPPCOMP_PRG)
CPPCOMP_INVOKE_OPTS  = $(CPPFLAGS) $(CXXFLAGS)
LINK_INVOKE_OPTS     = $(CXXFLAGS) $(LDFLAGS)

#### BEGIN-DEFINES
#

CPP_DEFINES         := -D HAVE_CONFIG_H

#CPP_DEFINES        += -D DEBUG


#
##### END-DEFINES

CPPCOMP_FAST_OPTS := -O2
CPPCOMP_SLOW_OPTS := -Os

BUILD_DIR       := build
RELEASE_DIR     := release

#LINK_X11_OPTS   := -L /usr/X11R6/lib/ -lX11 -lXpm -lXext
LINK_X11_OPTS   := 
LINK_PCRE_OPTS  := 
LINK_EXTRA_OPTS := 
LINK_OPTS       :=

INCL_OPTS       := -I .
INCL_LUA_OPTS   := -I ../lua/src
INCL_PCRE_OPTS  := -I ../pcre/
INCL_EXTRA_OPTS :=

# command parameters for configure script
CONFIGURE_OPTS  :=

MAKECMDGOALS   ?=
NONBUILD_GOALS := clean configure config

.PHONY: default
default: luced

WAS_SANDBOX_MK_INCLUDED := false
WAS_MAKEFILE2_INCLUDED  := false
WAS_HEADERS1_GENERATED  := false
WAS_HEADERS2_GENERATED  := false

# include sandbox-specific local settings from sandbox.mk,
# the above variables may be modified in this file:
#
include sandbox.mk

WAS_SANDBOX_MK_INCLUDED := $(shell ( test -f sandbox.mk && echo "true" ) || echo "false")

###############################################################
#                                                             #
# Assure that autoconf was run before anything else is        #
# built:                                                      #
#                                                             #
ifeq ($(WAS_SANDBOX_MK_INCLUDED),true)                        #
ifeq ($(filter $(NONBUILD_GOALS),$(MAKECMDGOALS)),)           #
#                                                             #
# Makefile2.mk contains variables that are set from the       #
# autoconf's configure script                                 #
#                                                             #
include $(BUILD_DIR)/autoconf/Makefile2.mk
#                                                             #
endif                                                         #
endif                                                         #
###############################################################




LINK_OPTS       += $(LINK_X11_OPTS) $(LINK_PCRE_OPTS) $(LINK_EXTRA_OPTS)
INCL_OPTS       += $(INCL_LUA_OPTS) $(INCL_PCRE_OPTS) $(INCL_EXTRA_OPTS)

LUA_SCRIPTS            := functions             BuiltinActionDefinitions  BuiltinClassDefinitions \
                          ConfigDefinition
                          
GENERATED_HEADERS      := CallbackContainer     Callback                  ActionId \
                          LuaClassRegistry      LuaCClosure               ConfigData \
                          LucedLuaInterface     ViewLuaInterface \
                          
GENERATED_SLOW_MODULES := ActionId              ActionMethodBinding       DefaultConfig \
                          ConfigData
                          
GENERATED_FAST_MODULES := LuaClassRegistry


SLOW_MODULES := EditorTopWin             GuiRoot                GuiWidget                  SystemException \
                TextEditorWidget         TextStyle              KeyPressRepeater           ScrollBar \
                System                   LuaException           TopWin                     ProgramName \
                LuaVar                   LuaInterpreter         ConfigException            GlobalConfig \
                LanguageModes            RegexException         StatusLine                 GlobalLuaInterpreter \
                Button                   GuiLayoutWidget        LuaStoredObjectReference   LuaArgException \
                GotoLinePanel            SingletonKeeper        TopWinList                 OwnedTopWins \
                DialogWin                GuiLayoutSpacer        MultiLineEditorWidget      SubstitutionException \
                SingleLineEditorWidget   LuaStackChecker        Commandline                BaseException \
                SingleLineEditField      LabelWidget            DialogPanel                PanelDialogWin \
                FindPanel                CheckBox               MessageBox                 SearchHistory \
                FileException            EditorServer           ClientServerUtil           EditorClient  \
                CommandlineException     ReplacePanel           LuaFunctionArguments       GuiElement \
                SelectionOwner           PasteDataReceiver      Clipboard                  FileOpener \
                ConfigErrorHandler       WindowCloser           SaveAsPanel                KeyModifier \
		CurrentDirectoryKeeper   ProgramExecutor        TextDisplayGuiCompound     CommandOutputBox \
		FramedGuiCompound        EditorTopWinActions    UnknownActionNameException ScrollableTextGuiCompound \
		SearchInteraction        ActionKeyConfig        SingleLineEditActions      MultiLineEditActions \
		ActionKeySequenceHandler StackTrace             ActionIdRegistry           MultiLineDisplayActions \
		SingleLineDisplayActions LuaStateAccess         LuaCMethod                 LuaAccess \
		LucedLuaInterface        EncodingException      QualifiedName              SyntaxPatternsConfig \
		TextStyleCache           ConfigPackageLoader    FocusableWidget            FocusableElement \
		NonFocusableWidget       FocusableContainerWidget 
                         

FAST_MODULES := TextWidget              TextData               HilitingBase           HilitedText \
                HilitingBuffer          HeapMem                File                   BasicRegex  \
                BackliteBuffer          GuiLayoutRow           GuiLayoutColumn        HeapObject  \
                EventDispatcher         FindUtil               ReplaceUtil            SyntaxPatterns \
                ViewLuaInterface        LuaSerializer          ActionMethodContainer  FocusManager \
                FontInfo                EncodingConverter      String
                
ROOT_CONFIG_FILES            := .luced/config.lua 

DEFAULT_PACKAGE_CONFIG_FILES := $(wildcard .luced/default/*.lua)

CONFIG_FILES := $(ROOT_CONFIG_FILES) $(DEFAULT_PACKAGE_CONFIG_FILES)

PRG_MODULES  := luced

LUA_MODULES  := lapi lcode ldebug ldo ldump lfunc lgc llex lmem \
                lobject lopcodes lparser lstate lstring ltable ltm  \
                lundump lvm lzio \
                lauxlib lbaselib ldblib liolib lmathlib loslib ltablib \
                lstrlib loadlib linit

LPOSIX_MODULES :=  lposix

PCRE_MODULES := pcre_compile   pcre_config      pcre_dfa_exec    \
                pcre_exec      pcre_fullinfo    pcre_get         \
                pcre_globals   pcre_info        pcre_maketables  \
                pcre_newline   pcre_ord2utf8    pcre_refcount    \
                pcre_study     pcre_tables      pcre_try_flipped \
                pcre_ucd       pcre_valid_utf8  pcre_version     \
                pcre_xclass    pcre_chartables

GENERATED_MODULES  := $(GENERATED_SLOW_MODULES) $(GENERATED_FAST_MODULES)

EDITOR_MODULES     := $(GENERATED_MODULES) $(FAST_MODULES) $(SLOW_MODULES)

MODULES            := $(EDITOR_MODULES) \
                      $(LUA_MODULES)    \
                      $(LPOSIX_MODULES) \
                      $(PCRE_MODULES)

  LIB_MBRS    := $(patsubst %, $(BUILD_DIR)/libluced.a(%.o),    $(MODULES))
  LIB_OBJS    := $(patsubst %, $(BUILD_DIR)/%.o,                $(MODULES))
  FAST_OBJS   := $(patsubst %, $(BUILD_DIR)/%.o,                $(FAST_MODULES))
  SLOW_OBJS   := $(patsubst %, $(BUILD_DIR)/%.o,                $(SLOW_MODULES))
   LUA_OBJS   := $(patsubst %, $(BUILD_DIR)/%.o,                 $(LUA_MODULES) \
                                                                   lua)
  PCRE_OBJS   := $(patsubst %, $(BUILD_DIR)/%.o,                $(PCRE_MODULES))
LPOSIX_OBJS   := $(patsubst %, $(BUILD_DIR)/%.o,              $(LPOSIX_MODULES))
   PRG_OBJS   := $(patsubst %, $(BUILD_DIR)/%.o,                 $(PRG_MODULES))

GEN_HDRS      := $(patsubst %, $(BUILD_DIR)/%.hpp,         $(GENERATED_HEADERS))
GEN_HDRS_DEPS := $(patsubst %, $(BUILD_DIR)/%.hpp.gen.dep, $(GENERATED_HEADERS))

GEN_SRC       := $(patsubst %, $(BUILD_DIR)/%.cpp,         $(GENERATED_MODULES))
GEN_SRC_DEPS  := $(patsubst %, $(BUILD_DIR)/%.cpp.gen.dep, $(GENERATED_MODULES))

GEN_FAST_OBJS := $(patsubst %, $(BUILD_DIR)/%.o,      $(GENERATED_FAST_MODULES))
GEN_SLOW_OBJS := $(patsubst %, $(BUILD_DIR)/%.o,      $(GENERATED_SLOW_MODULES))


LUA_SCRIPT_FILES := $(patsubst %, %.lua, $(LUA_SCRIPTS))


.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

luced: $(BUILD_DIR)/luced.o $(BUILD_DIR)/libluced.a
	$(call LINK_RUN, $(LIBS))

lua:  $(BUILD_DIR)/lua.o $(LUA_OBJS) $(LPOSIX_OBJS)
	$(call LINK_RUN, $(LIBS_WITH_READLINE))

###############################################################
# release

release_lua_headers    := $(patsubst %, lua/%,    $(shell cd ../lua/src; git ls-files *.h))
release_lposix_headers := $(patsubst %, lposix/%, $(shell cd ../lposix;  git ls-files *.h))
release_pcre_headers   := $(patsubst %, pcre/%,   $(shell cd ../pcre;    git ls-files *.h) \
                                                    pcre.h)

release_editor_headers := $(patsubst %, editor/%,   $(shell        git ls-files *.hpp) \
                                                    $(patsubst %,%.hpp,$(GENERATED_HEADERS)))

release_headers        := $(release_lua_headers)    \
                          $(release_lposix_headers) \
                          $(release_pcre_headers)   \
                          $(release_editor_headers)

RELEASE_HEADERS        := $(patsubst %, $(RELEASE_DIR)/%, $(release_headers))

 
release_lua_sources    := $(patsubst %, lua/%.cpp,    $(LUA_MODULES))
release_lposix_sources := $(patsubst %, lposix/%.cpp, $(LPOSIX_MODULES))
release_pcre_sources   := $(patsubst %, pcre/%.c,     $(PCRE_MODULES))
release_editor_sources := $(patsubst %, editor/%.cpp, $(EDITOR_MODULES))

release_sources        := $(release_lua_sources)    \
                          $(release_lposix_sources) \
                          $(release_pcre_sources)   \
                          $(release_editor_sources)

RELEASE_SOURCES        := $(patsubst %, $(RELEASE_DIR)/%, $(release_sources))

release_extras         := lua/COPYRIGHT \
                          lposix/modemuncher.c \
                          pcre/LICENCE \
                          editor/keysym2ucs.c \
                          editor/luced.xpm
                       
RELEASE_EXTRAS         := $(patsubst %, $(RELEASE_DIR)/%, $(release_extras))

release_luced_main     := editor/luced.cpp
RELEASE_LUCED_MAIN     := $(RELEASE_DIR)/$(release_luced_main)

RELEASE_FILES := $(RELEASE_HEADERS)             \
                 $(RELEASE_SOURCES)             \
                 $(RELEASE_EXTRAS)              \
                 $(RELEASE_LUCED_MAIN)          \
                 $(RELEASE_DIR)/Makefile.am     \
                 $(RELEASE_DIR)/configure.ac 
              

.PHONY: release
release: $(RELEASE_FILES)
	@mkdir -p $(RELEASE_DIR)/config-m4 \
	          $(RELEASE_DIR)/config-aux
	cd $(RELEASE_DIR); autoreconf -vi
	@echo "make release: finished";

$(RELEASE_DIR)/Makefile.am: Makefile.am.emlua Makefile
	$(call EMLUA_RUN,$@, RELEASE_HEADERS   = [[$(release_headers)]];\
	                     RELEASE_SOURCES   = [[$(release_sources)]]; \
	                     RELEASE_EXTRAS    = [[$(release_extras)]]; \
	                     LUCED_MAIN        = [[$(release_luced_main)]]; \
	 )

$(RELEASE_DIR)/configure.ac: autoconf-configure1.ac \
                             autoconf-configure3.ac
	@mkdir -p $(@D)
	cat $^ > $@
	
$(RELEASE_DIR)/lua/%:           ../lua/%
	$(COPY_RUN)
$(RELEASE_DIR)/lua/%.cpp:       ../lua/src/%.c
	$(COPY_RUN)
$(RELEASE_DIR)/lua/%:           ../lua/src/%
	$(COPY_RUN)
$(RELEASE_DIR)/pcre/%:          ../pcre/%
	$(COPY_RUN)
$(RELEASE_DIR)/lposix/%.cpp:    ../lposix/%.c
	$(COPY_RUN)
$(RELEASE_DIR)/lposix/%:        ../lposix/%
	$(COPY_RUN)
$(RELEASE_DIR)/editor/%:        %
	$(COPY_RUN)
$(RELEASE_DIR)/editor/%:        $(BUILD_DIR)/%
	$(COPY_RUN)

define COPY_RUN
	@mkdir -p $(@D)
	cp -f $< $@
endef

###############################################################
#                                                             #
# Assure that headers are generated before anything else is   #
# built, but after the configure script has generated the     #
# Makefile2.mk.                                               #
#                                                             #
ifeq ($(WAS_MAKEFILE2_INCLUDED),true)
ifeq ($(filter $(NONBUILD_GOALS),$(MAKECMDGOALS)),)
include $(BUILD_DIR)/headers1_generated.mk
$(BUILD_DIR)/headers1_generated.mk: sandbox-config.h \
                                     ../pcre/pcre.h
	@mkdir -p $(@D); echo "WAS_HEADERS1_GENERATED := true" > $@
endif
endif                                                         #

ifeq ($(WAS_HEADERS1_GENERATED),true)
ifeq ($(filter $(NONBUILD_GOALS),$(MAKECMDGOALS)),)
include $(BUILD_DIR)/headers2_generated.mk
$(BUILD_DIR)/headers2_generated.mk: $(GEN_HDRS) \
                                    $(GEN_HDRS_DEPS)
	@mkdir -p $(@D); echo "WAS_HEADERS2_GENERATED := true" > $@
endif
endif
#                                                             #
#                                                             #
ifeq ($(WAS_HEADERS2_GENERATED),true)
ifeq ($(filter $(NONBUILD_GOALS),$(MAKECMDGOALS)),)
  DEPS := $(wildcard $(BUILD_DIR)/deps/*.dep) $(GEN_HDRS_DEPS) $(GEN_SRC_DEPS)
  ifneq ($(DEPS),)
    include $(DEPS)
  endif
endif
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

EMLUA_STARTUP := CONFIG_FILES                 = { $(patsubst .luced/%,"%";, $(CONFIG_FILES)) }; \
                 ROOT_CONFIG_FILES            = { $(patsubst .luced/%,"%";, $(ROOT_CONFIG_FILES)) }; \
                 DEFAULT_PACKAGE_CONFIG_FILES = { $(patsubst .luced/%,"%";, $(DEFAULT_PACKAGE_CONFIG_FILES)) };

$(BUILD_DIR)/%.hpp.gen.dep $(BUILD_DIR)/%.new.hpp: %.hpp.emlua $(LUA_SCRIPT_FILES) $(CONFIG_FILES) lua
	$(call EMLUA_RUN,                   $(patsubst %.hpp.emlua,$(BUILD_DIR)/%.new.hpp,$<),$(EMLUA_STARTUP))
	$(call CONDITIONAL_DEPENDECY_RUN,   $(patsubst %.hpp.emlua,$(BUILD_DIR)/%.hpp.gen.dep,$<), \
	                                    $(patsubst %.hpp.emlua,$(BUILD_DIR)/%.new.hpp,$<),     \
	                                    $(patsubst %.hpp.emlua,$(BUILD_DIR)/%.hpp,$<) )
	                                    


$(BUILD_DIR)/%.cpp.gen.dep $(BUILD_DIR)/%.new.cpp: %.cpp.emlua $(LUA_SCRIPT_FILES) $(CONFIG_FILES) lua
	$(call EMLUA_RUN,                   $(patsubst %.cpp.emlua,$(BUILD_DIR)/%.new.cpp,$<),$(EMLUA_STARTUP))
	$(call CONDITIONAL_DEPENDECY_RUN,   $(patsubst %.cpp.emlua,$(BUILD_DIR)/%.cpp.gen.dep,$<), \
	                                    $(patsubst %.cpp.emlua,$(BUILD_DIR)/%.new.cpp,$<),     \
	                                    $(patsubst %.cpp.emlua,$(BUILD_DIR)/%.cpp,$<) )
	                                    

##########################################################################
#
# Rule for generating special files
#

$(BUILD_DIR)/autoconf/Makefile2.mk: autoconf-configure1.ac \
                                    autoconf-configure2.ac \
                                    autoconf-dummy.mk.am   \
                                    Makefile2.mk.in
	@ echo "================================================================================"; \
	  echo "Generating file $@...";                                                            \
	  echo "================================================================================";
	$(CONFIGURE_RUN)
	@ echo "================================================================================"; \
	  echo "Finished generating file $@";                                                      \
	  echo "================================================================================";

.PHONY: configure config
configure:
	$(CONFIGURE_RUN)
config:
	$(CONFIGURE_RUN)
	
define CONFIGURE_RUN
	@ if type autoreconf 2>&1 >/dev/null; then true; \
	                                      else echo "Error: command 'autoreconf' is missing. Is the package 'autoconf' installed?"; \
	                                           exit 1; fi; \
	mkdir -p $(BUILD_DIR)/autoconf && \
	      cd $(BUILD_DIR)/autoconf && \
	cat ../../autoconf-configure1.ac ../../autoconf-configure2.ac > configure.ac && \
	ln -sf ../../autoconf-dummy.mk.am    dummy.mk.am    && \
	ln -sf ../../Makefile2.mk.in         Makefile2.mk.in && \
	autoreconf -vi && \
	./configure $(CONFIGURE_OPTS)
endef	  

sandbox-config.h:
	@( echo "/*****************************************"; \
	   echo " * The file $@ can be used"; \
	   echo " * to manually override settings from"; \
	   echo " * $(BUILD_DIR)/autoconf/config.h"; \
	   echo " ******************************************/"; \
	   echo "" \
	 ) | tee $@
sandbox.mk:
	@( echo "############################################"; \
	   echo "# The file $@ can be used"; \
	   echo "# to manually override settings from"; \
	   echo "# $(BUILD_DIR)/autoconf/Makefile2.mk"; \
	   echo "############################################"; \
	   echo "" \
	 ) | tee $@


../pcre/pcre_chartables.c: ../pcre/pcre_chartables.c.dist
	ln -sf $<  $@

../pcre/pcre.h: ../pcre/pcre.h.generic
	ln -sf $<  $@

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

$(LUA_OBJS): $(BUILD_DIR)/%.o: ../lua/src/%.c
	$(call COMPILE_RUN,$(CPPCOMP_FAST_OPTS))

$(LPOSIX_OBJS): $(BUILD_DIR)/%.o: ../lposix/%.c
	$(call COMPILE_RUN,$(CPPCOMP_FAST_OPTS))

$(PCRE_OBJS): $(BUILD_DIR)/%.o: ../pcre/%.c
	$(call COMPILE_RUN,$(CPPCOMP_FAST_OPTS) -U DEBUG)

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



##########################################################################
#
# Runs
#

define COMPILE_RUN
	@rm -f $(BUILD_DIR)/deps/$(*F).dep
	@mkdir -p $(@D) && mkdir -p $(BUILD_DIR)/deps
	$(CPPCOMP_PRG) $(CPPCOMP_INVOKE_OPTS) -I . -I $(BUILD_DIR) $(CPP_DEFINES) $(INCL_OPTS) $1 \
	-c $<  -o "$@" 	-MMD -MF $(BUILD_DIR)/deps/$(*F).dep.tmp
	@cat $(BUILD_DIR)/deps/$(*F).dep.tmp | sed 's:^$(@F)\::$@\::' > $(BUILD_DIR)/deps/$(*F).dep
	@rm -f $(BUILD_DIR)/deps/$(*F).dep.tmp
endef

define LINK_RUN
	@mkdir -p $(@D)
	$(LINK_PRG) $(LINK_INVOKE_OPTS) $^ $1 $(LINK_OPTS) -o $@
endef



###################
# Parameters:
#
# $1:  target file
# $2:  startup code without '
#
define EMLUA_RUN
	@echo "Generating $< --> $(strip $1)"; \
	mkdir -p $(@D); \
	rm -f $1; \
	./lua -e '$2' em.lua $< > $1; rc=$$?; \
	if [ 0 -ne $$rc ]; then echo "*** emlua error ***"; rm -f $1; else chmod a-w $1; fi; \
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
                                    if [ -f $3 ] && diff $2 $3 >/dev/null 2>&1; then true; \
                                    else \
                                      echo "Copying   $(strip $2) ---> $(strip $3)"; \
	                              rm -f $3; cp $2 $3; chmod a-w $3; \
	                            fi; \
	                            echo " " > $1; \
	                           '; \
	 ) > $1
endef

