(print (or nil 2 nil) "\n")

(print (and nil 2 nil) "\n")

(print (and 1 1) "\n")

(if nil (print "hi\n") (print "not hi\n"))

(if (eq (* 2 (+ 1 4)) (- (* 4 3) 2))
    (print "2(1 + 4) == 4*3 - 2\n")
    (print "2(1 + 4) != 4*3 - 2\n"))

