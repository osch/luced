

void LuaSingletonCMethod::handleException(lua_State* L, const char* className,
                                                        const char* methodName)
{
    try
    {
        throw;
    }
    catch (LuaArgException& ex)
    {
        lua_pushstring(L, (String() << "Invalid invocation arguments for function " 
                                    << className << "." << methodName).toCString());
    }
    catch (BaseException& ex)
    {
        lua_pushstring(L, ex.getMessage().toCString());
    }
    catch (std::exception& ex)
    {
        lua_pushstring(L, ex.what());
    }
    catch (...)
    {
        lua_pushstring(L, "unknown error");
    }
}

