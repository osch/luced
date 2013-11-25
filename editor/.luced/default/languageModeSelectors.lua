return
{
    {
        languageMode              = "cplusplus",
        fileNameRegex             = "^.*\\.(?:CPP|HPP|H|cpp|hpp|cxx|hxx|c|h|cc|mm|m|hh)(?:\\.emlua)?$",
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
        languageMode              = "makefile",
        fileNameRegex             = "^.*/(Makefile.*|.*\\.h?mk(.in)?|.*\\.dep)$",
        fileContentRegex          = nil
    },
    {
        languageMode              = "emlua",
        fileNameRegex             = "^(.*\\.emlua|.*\\.(jcl))$",
        fileContentRegex          = nil
    },
    {
        languageMode              = "java",
        fileNameRegex             = "^.*\\.(?:java)$",
    },
    {
        languageMode              = "csharp",
        fileNameRegex             = "^.*\\.(?:cs)$",
        fileContentRegex          = nil
    },
    {
        languageMode              = "antlrJavaGrammar",
        fileNameRegex             = "^.*\\.(?:g)$",
        fileContentRegex          = nil
    },
    {
        languageMode              = "shell",
        fileNameRegex             = "^.*\\.(?:sh|bashrc)$",
    },
    {
        languageMode              = "xml",
        fileContentRegex           = [[^\<\?xml[^\n]*(encoding="(?P<ENCODING>[^"]*)").*?\?\>]],
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
