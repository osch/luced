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
                    default = "true"
                },
                {   name    = "doNotUseX11XkbExtension",
                    type    = "bool",
                    default = "true"
                },
                {   name    = "keyPressRepeatFirstMilliSecs",
                    type    = "double",
                    default = "200"
                },
                {   name    = "keyPressRepeatNextMilliSecs",
                    type    = "double",
                    default = "18"
                },
                {   name    = "scrollBarWidth",
                    type    = "int",
                    default = "12+2"
                },
                {   name    = "scrollBarRepeatFirstMilliSecs",
                    type    = "double",
                    default = "300"
                },
                {   name    = "scrollBarRepeatNextMilliSecs",
                    type    = "double",
                    default = "10"
                },
                {   name    = "doubleClickMilliSecs",
                    type    = "int",
                    default = "400"
                },
                {   name    = "guiColor01",
                    type    = "String",
                    default = "grey30"
                },
                {   name    = "guiColor02",
                    type    = "String",
                    default = "grey70"
                },
                {   name    = "guiColor03",
                    type    = "String",
                    default = "grey80"
                },
                {   name    = "guiColor04",
                    type    = "String",
                    default = "grey85"
                },
                {   name    = "guiColor05",
                    type    = "String",
                    default = "grey90"
                },
                {   name    = "guiFont",
                    type    = "String",
                    default = "-*-helvetica-medium-r-*-*-*-120-75-75-*-*-iso10646-1"
                },
                {   name    = "guiFontColor",
                    type    = "String",
                    default = "black"
                },
                {   name    = "primarySelectionColor",
                    type    = "String",
                    default = "grey"
                },
                {   name    = "pseudoSelectionColor",
                    type    = "String",
                    default = "rgb:f1/f1/f1"
                },
                {   name    = "initialWindowWidth",
                    type    = "int",
                    default = "100"
                },
                {   name    = "initialWindowHeight",
                    type    = "int",
                    default = "45"
                },
                {   name    = "x11SelectionChunkLength",
                    type    = "long",
                    default = "200000"
                },
                {   name    = "buttonInnerSpacing",
                    type    = "int",
                    default = "2"
                },
                {   name    = "guiSpacing",
                    type    = "int",
                    default = "2"
                },
                {   name    = "editorPanelOnTop",
                    type    = "bool",
                    default = "false"
                },
                {   name    = "keepRunningIfOwningClipboard",
                    type    = "bool",
                    default = "false"
                },
                {   name    = "maxRegexAssertionLength",
                    type    = "int",
                    default = "3000"
                },
            }
        },
        ------------------------------------------------------------------------
        {   name    = "fonts",
            type    = "list",
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

