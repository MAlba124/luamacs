(module-load"./luamacs.so")

(setq luamacs-state (luamacs-state-init))

(luamacs-exec-str luamacs-state (with-temp-buffer (insert-file-contents "rc.lua") (buffer-string)))

(luamacs-test)
