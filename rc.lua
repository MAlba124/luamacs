local em = require("emacs/emacs")
local ui = require("emacs/ui")
local pakage = require("emacs/pakage")

pakage.add_pkg_archive({ name = "melpa", url = "https://melpa.org/packages"})
pakage.init()

ui.menu_bar_mode(em.Mode.DISABLE)
ui.scroll_bar_mode(em.Mode.DISABLE)
ui.tool_bar_mode(em.Mode.DISABLE)

em.set(em.intern("make-backup-files"), nil)

ui.require_theme(em.intern("modus-themes"))
ui.load_theme(em.intern("modus-operandi"))
ui.blink_cursor_mode(em.Mode.DISABLE)

em.message(em.cons("Test", "asd")["car"]) -- prints `Test`

cons_cell = em.cons("the-car", "the-cdr")
test_cons_value = em.intern("test-cons-value")
em.set(test_cons_value, cons_cell)

local pkg_evil = em.intern("evil")
if not pakage.is_installed(pkg_evil) then
    pakage.install(em.intern("evil"))
else
   em.message("evil is already installed")
end
em.require(em.intern("evil"))
functioncall(emacs_environment, "evil-mode", 1, {1})

function luamacs_test()
   print("It is working!")
end

expose_function(
   emacs_environment, -- env     : -6
   "luamacs-test", -- lisp name  : -5
   "docstring", -- docstring     : -4
   0, -- takes n args            : -3
   false, -- returns             : -2
   luamacs_test -- function      : -1
)
