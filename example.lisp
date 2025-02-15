(print "Hello, world!\n")
(print (* 3 (+ 2 2 3)) "\n")
;(print (quote (a b c d e)) "\n")
;(print '(1 2 3 4) "\n")
;(print 'a " " 'b " " 'c "\n")

;; Simple lists
(print (cons 1 2) "\n")
(print (cons (cons 1 2) nil) "\n")
(print (cons 1 nil) "\n")
;(print (cons '(1 2 3) nil) "\n")
;(print (cons nil '(1 2 3)) "\n")
;(print (cons nil (cons '(1 2 3) nil)) "\n")

(print ((lambda (a b) (+ a b)) 3 4) "\n")
;(print ((lambda (&rest l) l) 1 2 3) "\n")

