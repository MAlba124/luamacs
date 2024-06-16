(require 'package)
(add-to-list 'package-archives
             '("melpa" . "https://melpa.org/packages/"))
(package-initialize)
;; (package-refresh-contents)

(use-package evil
  :ensure t
  :config
  (evil-mode 1))

(module-load"./luamacs.so")

(let ((state (luamacs-state-init)))
  (luamacs-exec-str state (luamacs-read-file-to-str "rc.lua")))

