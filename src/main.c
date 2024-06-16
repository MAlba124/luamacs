#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <emacs-module.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

int plugin_is_GPL_compatible;

// TODO: Make function get_string_length for elisp strings

#define LOG(msg) printf("LUAMACS(%s): "msg"\n", __func__)
#define NIL(env) env->intern(env, "nil")
#define ELISP_IS_TYPE(env, type, str) env->eq(env, env->intern(env, str), type)

static void
lua_state_deinit(void *arg)
{
    lua_State *L = (lua_State*)arg;
    lua_close(L);
    LOG("Deinitialized lua state");
}

static emacs_value
state_init(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data)
{
    (void)nargs;
    (void)args;
    (void)data;

    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    LOG("Lua state initialized");
    return env->make_user_ptr(env, lua_state_deinit, L);
}

emacs_value
lua_to_emacs_val(emacs_env *env, lua_State *L, size_t stack_index)
{
    switch (lua_type(L, stack_index))
    {
        case LUA_TNIL:
        {
            return NIL(env);
        }
        case LUA_TNUMBER:
        {
            lua_Integer integer = lua_tointeger(L, stack_index);
            lua_Number num = lua_tonumber(L, stack_index);
            return integer != num ? env->make_float(env, num) : env->make_integer(env, integer);
        }
        case LUA_TBOOLEAN:
        {
            int boolean = lua_toboolean(L, stack_index);
            return boolean ? env->intern(env, "t") : NIL(env);
        }
        case LUA_TSTRING:
        {
            const char *string = lua_tostring(L, stack_index);
            return env->make_string(env, string, strlen(string));
        }
        case LUA_TUSERDATA:
        {
            emacs_value val = lua_touserdata(L, stack_index);
            return val;
        }
        case LUA_TTABLE:
        {
            lua_getfield(L, -1, "type");
            const char *type = lua_tostring(L, -1);

            if (strcmp(type, "cons") == 0)
            {
                lua_getfield(L, -2, "car");
                emacs_value car = lua_to_emacs_val(env, L, -1);

                lua_getfield(L, -3, "cdr");
                emacs_value cdr = lua_to_emacs_val(env, L, -1);

                emacs_value args[] = { car, cdr };
                return env->funcall(env, env->intern(env, "cons"), 2, args);
            }
            else
            {
                LOG("Unknown table type");
                return NIL(env);
            }
        }
        default:
        {
            LOG("Unknown type");
            return NIL(env);
        }
    }
}

// Convert a emacs lisp value to lua and push it onto the stack
int
emacs_to_lua_val(emacs_env *env, emacs_value eval, lua_State *L)
{
    if (!env->is_not_nil(env, eval))
    {
        LOG("Return value from emacs is `nil`");
        lua_pushnil(L);
        return 0;
    }

    emacs_value type = env->type_of(env, eval);
    if (ELISP_IS_TYPE(env, type, "string"))
    {
        LOG("Return value from emacs is `string`");
        ptrdiff_t str_len = 0;
        if (!env->copy_string_contents(env, eval, NULL, &str_len))
        {
            LOG("Failed to get string length");
            return -1;
        }

        char *str = malloc(str_len);
        if (!str)
        {
            LOG("Failed to allocate str");
            return -1;
        }

        if (!env->copy_string_contents(env, eval, str, &str_len))
        {
            LOG("Failed to copy string");
            return -1;
        }

        lua_pushstring(L, str);
        free(str);
    }
    else if (ELISP_IS_TYPE(env, type, "integer"))
    {
        LOG("Return value from emacs is `integer`");
        lua_pushinteger(L, env->extract_integer(env, eval));
    }
    else if (ELISP_IS_TYPE(env, type, "float"))
    {
        LOG("Return value from emacs is `float`");
        lua_pushnumber(L, env->extract_float(env, eval));
    }
    else if (ELISP_IS_TYPE(env, type, "boolean"))
    {
        LOG("Return value from emacs is `boolean`");
        // Redundant?
        if (env->eq(env, eval, env->intern(env, "nil")))
        {
            lua_pushboolean(L, 0);
        }
        else
        {
            lua_pushboolean(L, 1);
        }
    }
    else if (ELISP_IS_TYPE(env, type, "symbol"))
    {
        LOG("Return value from emacs is `symbol`");
        emacs_value *l_udata = lua_newuserdata(L, sizeof(emacs_value));
        memcpy(l_udata, eval, sizeof(emacs_value));
        return 0;
    }
    else if (ELISP_IS_TYPE(env, type, "cons"))
    {
        emacs_value args[] = { eval };
        emacs_value car = env->funcall(env, env->intern(env, "car"), 1, args);
        emacs_value cdr = env->funcall(env, env->intern(env, "cdr"), 1, args);

        lua_createtable(L, 3, 0); // table_stack_index = -1

        lua_pushstring(L, "cons"); // table_stack_index = -2
        lua_setfield(L, -2, "type"); // table_stack_index = -1

        emacs_to_lua_val(env, car, L); // stack_index = -1 , table_stack_index = -2
        lua_setfield(L, -2, "car"); // table_stack_index = -1

        emacs_to_lua_val(env, cdr, L); // stack_index = -1 , table_stack_index = -2
        lua_setfield(L, -2, "cdr"); // table_stack_index = -1

        LOG("Return value from emacs is `cons`");
        return 0;
    }
    else
    {
        LOG("Unsupported type");
        return -1;
    }

    return 0;
}

// Lua function
// Call emacs lisp function from lua
int
functioncall(lua_State *L)
{
    emacs_env *env = lua_touserdata(L, -4);
    const char *func_name = lua_tostring(L, -3);
    printf("LUAMACS: func name: %s\n", func_name);
    size_t nargs = (size_t)lua_tonumber(L, -2);

    if (nargs == 0)
    {
        emacs_value ret = env->funcall(env, env->intern(env, func_name), 0, NULL);
        emacs_to_lua_val(env, ret, L); // TODO: Check error
        return 1;
    }

    emacs_value *evalues = malloc(sizeof(emacs_value) * nargs);
    if (!evalues)
    {
        LOG("Failed to allocate evalues");
        return 0;
    }

    for (size_t i = 0; i < nargs; i++)
    {
        lua_rawgeti(L, -1 - i, i + 1);
        lua_to_emacs_val(env, L, -1);
        evalues[i] = lua_to_emacs_val(env, L, -1);
    }

    emacs_value ret = env->funcall(env, env->intern(env, func_name), nargs, evalues);
    emacs_to_lua_val(env, ret, L); // TODO: Check error

    free(evalues);

    return 1;
}

static emacs_value
execute_lua_str(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data)
{
    (void)env;
    (void)data;

    if (nargs < 2)
    {
        LOG("Missing arguments");
        return NIL(env);
    }

    lua_State *L = env->get_user_ptr(env, args[0]);

    ptrdiff_t code_len = 0;
    if (!env->copy_string_contents(env, args[1], NULL, &code_len))
    {
        LOG("Failed to get code len");
        return NIL(env);
    }

    char *lua_code = malloc(code_len);
    if (!lua_code)
    {
        LOG("Failed to allocate lua_code");
        return NIL(env);
    }

    if (!env->copy_string_contents(env, args[1], lua_code, &code_len))
    {
        LOG("Failed to copy lua code");
        return NIL(env);
    }

    // Expose the emacs environment for use in Lua
    lua_pushlightuserdata(L, env);
    lua_setglobal(L, "emacs_environment");
    lua_pop(L, -1);

    lua_pushcfunction(L, functioncall);
    lua_setglobal(L, "functioncall");

    if (luaL_dostring(L, lua_code))
    {
        LOG("Error occured running lua code");
        return NIL(env);
    }

    free(lua_code);

    return NIL(env);
}

static emacs_value
read_file_to_str(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data)
{
    (void)data;

    if (nargs < 1)
    {
        LOG("Missing arguments");
        return NIL(env);
    }

    ptrdiff_t buf_len = 0;
    if (!env->copy_string_contents(env, args[0], NULL, &buf_len))
    {
        LOG("Failed to get path length");
        return NIL(env);
    }

    char *buf = malloc(buf_len);
    if (!buf)
    {
        LOG("Failed to allocate path buf");
        return NIL(env);
    }

    if (!env->copy_string_contents(env, args[0], buf, &buf_len))
    {
        LOG("Failed to get path");
        return NIL(env);
    }

    FILE *f = fopen(buf, "r");
    if (!f)
    {
        perror("LUAMACS: fopen");
        return NIL(env);
    }

    free(buf);

    fseek(f, 0, SEEK_END);
    long f_len = ftell(f);
    rewind(f);

    char *str_buf = malloc(f_len + 1);
    if (!str_buf)
    {
        LOG("Failed to allocate str_buf");
        return NIL(env);
    }

    if (fread(str_buf, 1, f_len, f)!= (size_t)f_len)
    {
        LOG("Failed to read from file");
        return NIL(env);
    }
    str_buf[f_len] = '\0';

    fclose(f);

    emacs_value read_str = env->make_string(env, str_buf, f_len);

    free(str_buf);

    return read_str;
}

void
defun(emacs_env *env, int mm_arity, emacs_function func, const char *docstring, const char *symbol_name)
{
    emacs_value efunc = env->make_function(env, mm_arity, mm_arity, func, docstring, NULL);
    emacs_value symbol = env->intern(env, symbol_name);
    emacs_value args[] = {symbol, efunc};
    env->funcall(env, env->intern(env, "defalias"), 2, args);
}

int
emacs_module_init(struct emacs_runtime *runtime)
{
    emacs_env *env = runtime->get_environment(runtime);

    defun(env, 0, state_init, "Initialize the lua state", "luamacs-state-init");
    defun(env, 1, read_file_to_str, "Read a file to string", "luamacs-read-file-to-str");
    defun(env, 2, execute_lua_str, "Execute a given string containing lua code", "luamacs-exec-str");

    LOG("Initialized");

    return 0;
}
