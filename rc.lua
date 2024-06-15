function set(name, value)
   return functioncall(emacs_environment, "set", 2, {name, value})
end

function message(msg)
   functioncall(emacs_environment, "message", 1, {msg})
end

function toggle_scroll_bar(toggle)
   functioncall(emacs_environment, "toggle-scroll-bar", 1, {toggle})
end

function tool_bar_mode(mode)
   functioncall(emacs_environment, "tool-bar-mode", 1, {mode})
end

function package_installed_p(pkg)
   if (functioncall(emacs_environment, "package-installed-p", 1, {pkg}) == nil) then
      return false
   else
      return true
   end
end

function emacs_init_time()
   return functioncall(emacs_environment, "emacs-init-time", 0, {})
end

function create_symbol(sym_name)
   return functioncall(emacs_environment, "intern", 1, {sym_name});
end

message("Hello world")
toggle_scroll_bar(-1)
tool_bar_mode(-1)

set(create_symbol("variable-for-testing"), "test")

if (package_installed_p(create_symbol("package"))) then
   message("`package` is installed")
else
   message("`package` is not installed")
end
