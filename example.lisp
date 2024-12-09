(format "Hello, world!\n")
(format (* 3 (+ 2 2 3)) "\n")
;(format (quote (a b c d e)) "\n")
;(format '(1 2 3 4) "\n")
;(format 'a " " 'b " " 'c "\n")

;; Simple lists
(format (cons 1 2) "\n")
(format (cons (cons 1 2) nil) "\n")
(format (cons 1 nil) "\n")
;(format (cons '(1 2 3) nil) "\n")
;(format (cons nil '(1 2 3)) "\n")
;(format (cons nil (cons '(1 2 3) nil)) "\n")

(format ((lambda (a b) (+ a b)) 3 4) "\n")
;(format ((lambda (&rest l) l) 1 2 3) "\n")

