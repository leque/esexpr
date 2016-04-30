(defun fact (n)
  (if (= n 0)
      1
    (* n (fact (1- n)))))

(defun fib (n)
  (cond ((= n 0)
         1)
        ((= n 1)
         1)
        (t
         (+ (fib (- n 1)) (fib (- n 2))))))
