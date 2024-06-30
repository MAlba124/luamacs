--- General emacs functions

emacs = {}

emacs.Mode = { ENABLE = 1, DISABLE = -1 }

--- Set a symbol to a value
-- @param name Symbol to set the value for
-- @param value Value to set
-- @return value
function emacs.set(name, value)
   return functioncall(emacs_environment, "set", 2, {name, value})
end

--- Display a message inside emacs
-- @param msg The message to be displayed
function emacs.message(msg)
   functioncall(emacs_environment, "message", 1, {msg})
end

--- Check if a package is installed or not
-- @param pkg Symbol representing a package
-- @return True if the package is installed, false otherwise
function emacs.package_installed_p(pkg)
   if (functioncall(emacs_environment, "package-installed-p", 1, {pkg}) == nil) then
      return false
   else
      return true
   end
end

--- Get the time emacs took to initialize
-- @return String displayig timestamp, i.e. "1.12 seconds"
function emacs.emacs_init_time()
   return functioncall(emacs_environment, "emacs-init-time", 0, {})
end

--- Create a symbol
-- @param sym_name Name of the symbol
-- @return A userdata object. ONLY USE THE RETURNED VALUE IN OTHER EMACS FUNCTIONS
function emacs.intern(sym_name)
   return functioncall(emacs_environment, "intern", 1, {sym_name})
end

--- Create a cons cell
-- @param car The car of the cons
-- @param cdr The cdr of the cons
-- @return A table representing a cons cell
function emacs.cons(car, cdr)
   cell = {}
   cell["type"] = "cons"
   cell["car"] = car
   cell["cdr"] = cdr
   return cell
end

return emacs
