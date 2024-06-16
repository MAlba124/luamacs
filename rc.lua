require("emacs/emacs")
local ui = require("emacs/ui")

ui.menu_bar_mode(Mode.DISABLE)
ui.scroll_bar_mode(Mode.DISABLE)
ui.tool_bar_mode(Mode.DISABLE)

set(intern("make-backup-files"), nil)

ui.require_theme(intern("modus-themes"))
ui.load_theme(intern("modus-operandi"))
ui.blink_cursor_mode(Mode.DISABLE)

message(cons("Test", "asd")["car"]) -- prints `Test`

cons_cell = cons("the-car", "the-cdr")
set(intern("test-cons-value"), cons_cell["car"])
