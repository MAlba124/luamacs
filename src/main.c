#include <stdio.h>
#include <stdlib.h>

#include <emacs-module.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

int plugin_is_GPL_compatible;

#define LOG(msg) printf("LUAMACS(%s): "msg"\n", __func__)
#define NIL(env) env->intern(env, "nil")

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
