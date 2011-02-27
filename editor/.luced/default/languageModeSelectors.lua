return
{
    {
        languageMode              = "cplusplus",
        fileNameRegex             = "^.*\\.(?:CPP|HPP|H|cpp|hpp|cxx|hxx|c|h|cc|hh)(?:\\.emlua)?$",
    },
    {
        languageMode              = "jsp",
        fileNameRegex             = "^.*\\.(jspf?|html?)$",
    },
    {
        languageMode              = "lua",
        fileNameRegex             = "^.*\\.lua$|^.*\\.lua\\.emlua$",
    },
    {
        languageMode              = "rexx",
        fileNameRegex             = ".*\\.rexx?|.*\\.r",
    },
    {
        languageMode              = "emlua",
        fileNameRegex             = "^(.*\\.emlua|.*\\.(jcl))$",
    },
    {
        languageMode              = "makefile",
        fileNameRegex             = "^.*/(Makefile|.*\\.h?mk(.in)?|.*\\.dep)$",
    },
    {
        languageMode              = "java",
        fileNameRegex             = "^.*\\.(?:java)$",
    },
    {
        languageMode              = "shell",
        fileNameRegex             = "^.*\\.(?:sh)$",
    },
    {
        languageMode              = "xml",
        fileContentRegex           = [[^\<\?xml[^\n]*(encoding="(?P<ENCODING>[^"]*)")\?\>]],
    },
    {
        languageMode              = "xml",
        fileNameRegex             = "^.*\\.(?:xml)$",
    },
    {
        languageMode              = "shell",
        fileContentRegex           = [[^\#\!/(?:usr/)?(?:local/)?bin/(?:ba)?sh]],
    },
}
