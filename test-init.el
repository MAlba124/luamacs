(module-load"./luamacs.so")

(let ((state (luamacs-state-init)))
  (luamacs-exec-str state (with-temp-buffer (insert-file-contents "rc.lua") (buffer-string)))
  (message (car test-cons-value)))
