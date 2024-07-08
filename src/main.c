#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <emacs-module.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

int plugin_is_GPL_compatible;

#ifdef DEBUG
#define LOG(msg) printf("LUAMACS(%s): "msg"\n", __func__)
#else
#define LOG(msg)
#endif
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
  switch (lua_type(L, stack_index)) {
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

static ptrdiff_t
emacs_get_string_length(emacs_env *env, emacs_value eval)
{
    ptrdiff_t str_len = 0;
    if (!env->copy_string_contents(env, eval, NULL, &str_len))
      {
        return -1;
      }

    return str_len;
}


// Convert a emacs lisp value to lua and push it onto the stack
static int
emacs_to_lua_val(emacs_env *env, emacs_value eval, lua_State *L)
{
  if (!env->is_not_nil(env, eval))
    {
      lua_pushnil(L);
      return 0;
    }

  emacs_value type = env->type_of(env, eval);
  if (ELISP_IS_TYPE(env, type, "string"))
    {
      ptrdiff_t str_len;
      if ((str_len = emacs_get_string_length(env, eval)) < 0)
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
      lua_pushinteger(L, env->extract_integer(env, eval));
    }
  else if (ELISP_IS_TYPE(env, type, "float"))
    {
      lua_pushnumber(L, env->extract_float(env, eval));
    }
  else if (ELISP_IS_TYPE(env, type, "boolean"))
    {
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
      emacs_value *l_udata = lua_newuserdata(L, sizeof(emacs_value));
      memcpy(l_udata, eval, sizeof(emacs_value));
      return 0;
    }
  else if (ELISP_IS_TYPE(env, type, "cons"))
    {
      emacs_value args[] = { eval };
      emacs_value car = env->funcall(env, env->intern(env, "car"), 1, args);
      emacs_value cdr = env->funcall(env, env->intern(env, "cdr"), 1, args);

      lua_createtable(L, 3, 0);

      lua_pushstring(L, "cons");
      lua_setfield(L, -2, "type");

      emacs_to_lua_val(env, car, L);
      lua_setfield(L, -2, "car");

      emacs_to_lua_val(env, cdr, L);
      lua_setfield(L, -2, "cdr");

      return 0;
    }
  else
    {
      LOG("Unsupported type returned from emacs");
      return -1;
    }

    return 0;
}

// Lua function
// Call emacs lisp function from lua
static int
functioncall(lua_State *L)
{
  emacs_env *env = lua_touserdata(L, -4);
  const char *func_name = lua_tostring(L, -3);
  size_t nargs = (size_t)lua_tonumber(L, -2);

  if (nargs == 0)
    {
      emacs_value ret = env->funcall(env, env->intern(env, func_name), 0, NULL);
      if (emacs_to_lua_val(env, ret, L) < 0)
          {
            return 0;
          }
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
        evalues[i] = lua_to_emacs_val(env, L, -1);
      }

    emacs_value ret = env->funcall(env, env->intern(env, func_name), nargs, evalues);
    if (emacs_to_lua_val(env, ret, L) < 0)
      {
        free(evalues);
        return 0;
      }

    free(evalues);
    return 1;
}

static int
functioncall_no_return(lua_State *L)
{
    emacs_env *env = lua_touserdata(L, -4);
    const char *func_name = lua_tostring(L, -3);
    size_t nargs = (size_t)lua_tonumber(L, -2);

    if (nargs == 0)
      {
        env->funcall(env, env->intern(env, func_name), 0, NULL);
        return 0;
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
        evalues[i] = lua_to_emacs_val(env, L, -1);
      }

    env->funcall(env, env->intern(env, func_name), nargs, evalues);
    free(evalues);

    return 0;
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

    ptrdiff_t code_len;
    if ((code_len = emacs_get_string_length(env, args[1])) < 0) {
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

    lua_pushcfunction(L, functioncall_no_return);
    lua_setglobal(L, "functioncall_no_return");

    if (luaL_dostring(L, lua_code))
      {
        printf("LUAMACS(execute_lua_str) Error occured running lua code: %s\n", lua_tostring(L, -1));
        return NIL(env);
      }

    free(lua_code);
    return NIL(env);
}

static void
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
    defun(env, 2, execute_lua_str, "Execute a given string containing lua code", "luamacs-exec-str");
    LOG("Initialized");

    return 0;
}
