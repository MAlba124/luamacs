require "emacs.lua"

menu_bar_mode(Mode.DISABLE)
scroll_bar_mode(Mode.DISABLE)
tool_bar_mode(Mode.DISABLE)

set(intern("make-backup-files"), nil)

require_theme(intern("modus-themes"))
load_theme(intern("modus-operandi"))

message(cons("Test", "asd")["car"]) -- prints `Test`
