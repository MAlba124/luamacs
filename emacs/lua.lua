Mode = { ENABLE = 1, DISABLE = -1 }

function set(name, value)
   return functioncall(emacs_environment, "set", 2, {name, value})
end

function message(msg)
   functioncall(emacs_environment, "message", 1, {msg})
end

function menu_bar_mode(mode)
   functioncall(emacs_environment, "menu-bar-mode", 1, {mode})
end

function scroll_bar_mode(mode)
   functioncall(emacs_environment, "scroll-bar-mode", 1, {mode})
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

function intern(sym_name)
   return functioncall(emacs_environment, "intern", 1, {sym_name})
end

function require_theme(theme)
   functioncall(emacs_environment, "require-theme", 1, {theme})
end

function load_theme(theme)
   functioncall(emacs_environment, "load-theme", 1, {theme})
end

function cons(car, cdr)
   cell = {}
   cell["type"] = "cons"
   cell["car"] = car
   cell["cdr"] = cdr
   return cell
end
