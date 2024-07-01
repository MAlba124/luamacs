(module-load"./luamacs.so")

(let ((state (luamacs-state-init)))
  (luamacs-exec-str state (luamacs-read-file-to-str "rc.lua"))
  (message (car test-cons-value)))
