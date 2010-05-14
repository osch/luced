return
{
    name    = "configData",
    type    = "map",
    entries =
    {
        ------------------------------------------------------------------------
        {   name    = "generalConfig",
            type    = "map",
            entries =
            {
                {   name    = "useOwnKeyPressRepeater",
                    type    = "bool",
                },
                {   name    = "doNotUseX11XkbExtension",
                    type    = "bool",
                },
                {   name    = "keyPressRepeatFirstMilliSecs",
                    type    = "double",
                },
                {   name    = "keyPressRepeatNextMilliSecs",
                    type    = "double",
                },
                {   name    = "scrollBarWidth",
                    type    = "int",
                },
                {   name    = "scrollBarRepeatFirstMilliSecs",
                    type    = "double",
                },
                {   name    = "scrollBarRepeatNextMilliSecs",
                    type    = "double",
                },
                {   name    = "doubleClickMilliSecs",
                    type    = "int",
                },
                {   name    = "guiColor01",
                    type    = "String",
                },
                {   name    = "guiColor02",
                    type    = "String",
                },
                {   name    = "guiColor03",
                    type    = "String",
                },
                {   name    = "guiColor04",
                    type    = "String",
                },
                {   name    = "guiColor05",
                    type    = "String",
                },
                {   name    = "guiFont",
                    type    = "String",
                },
                {   name    = "guiFontColor",
                    type    = "String",
                },
                {   name    = "primarySelectionColor",
                    type    = "String",
                },
                {   name    = "pseudoSelectionColor",
                    type    = "String",
                },
                {   name    = "initialWindowWidth",
                    type    = "int",
                },
                {   name    = "initialWindowHeight",
                    type    = "int",
                },
                {   name    = "x11SelectionChunkLength",
                    type    = "long",
                },
                {   name    = "buttonInnerSpacing",
                    type    = "int",
                },
                {   name    = "guiSpacing",
                    type    = "int",
                },
                {   name    = "editorPanelOnTop",
                    type    = "bool",
                },
                {   name    = "keepRunningIfOwningClipboard",
                    type    = "bool",
                },
                {   name    = "maxRegexAssertionLength",
                    type    = "int",
                },
            }
        },
        ------------------------------------------------------------------------
        {   name    = "fonts",
            type    = "list",
            index   = "name",
            member  =
            {
                name    = "font",
                type    = "map",
                entries =
                {
                    {   name = "name",
                        type = "String"
                    },
                    {   name = "x11FontId",
                        type = "String"
                    }
                }
            }
        },
        ------------------------------------------------------------------------
        {   name    = "textStyles",
            type    = "list",
            member  =
            {
                name         = "element",
                type         = "alternative",
                alternatives =
                {
                    {   name    = "textStyle",
                        type    = "map",
                        entries =
                        {
                            {   name = "name",
                                type = "String"
                            },
                            {   name = "font",
                                type = "String"
                            },
                            {   name = "color",
                                type = "String"
                            }
                        },
                    },
                    {   name    = "referer",
                        type    = "map",
                        entries =
                        {
                            {   name    = "referToPackage",
                                type    = "String"
                            }
                        }
                    }
                }
            }
        },
        ------------------------------------------------------------------------
        {   name    = "languageModes",
            type    = "list",
            member  =
            {
                name         = "element",
                type         = "alternative",
                alternatives =
                {   
                    {   name    = "languageMode",
                        type    = "map",
                        entries =
                        {
                            {   name    = "name",
                                type    = "String"
                            },
                            {   name    = "syntaxName",
                                type    = "String"
                            },
                            {   name    = "fileNameRegex",
                                type    = "String"
                            },
                            {   name    = "approximateUnknownHiliting",
                                type    = "bool",
                                default = "true",
                            },
                            {   name    = "approximateUnknownHilitingReparseRange",
                                type    = "int",
                                default = "2000"
                            },
                            {   name    = "hilitingBreakPointDistance",
                                type    = "int",
                                default = "50"
                            },
                            {   name    = "hardTabWidth",
                                type    = "int",
                                default = "8"
                            },
                            {   name    = "softTabWidth",
                                type    = "int",
                                default = "0"
                            }
                        }
                    },
                    {   name    = "referer",
                        type    = "map",
                        entries =
                        {
                            {   name    = "referToPackage",
                                type    = "String"
                            }
                        }
                    }
                }
            }
        },
        ------------------------------------------------------------------------
        {   name    = "actionKeyBindings",
            type    = "list",
            member  =
            {
                name    = "element",
                type    = "alternative",
                alternatives =
                {
                    {   name    = "actionKeyBinding",
                        type    = "map",
                        entries =
                        {
                            {   name    = "actionName",
                                type    = "String"
                            },
                            {   name    = "keys",
                                type    = "list",
                                member  =
                                {
                                    name    = "keyCombination",
                                    type    = "String"
                                }
                            }
                        }
                    },
                    {   name    = "referer",
                        type    = "map",
                        entries =
                        {
                            {   name    = "referToPackage",
                                type    = "String"
                            }
                        }
                    }
                }
            }
        }
        ------------------------------------------------------------------------
    }
}

