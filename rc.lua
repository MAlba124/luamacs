local em = require("emacs/emacs")
local ui = require("emacs/ui")

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

-- em.add_to_list(em.intern("package-archives"), em.cons("gnu", "https://elpa.gnu.org/packages"))
