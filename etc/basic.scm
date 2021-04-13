;; -------------------------------------------------------------
;;
;; basic procedures & macros
;;
;; -------------------------------------------------------------

(define add1 (mac (e)
    `(+ ,e 1)))

(define sub1 (mac (e)
    `(- ,e 1)))

(define ok? (mac (s)
    `(is s 0)))

(define let (mac (args . body)
    `(begin
        (define proc (fn ,(map (fn (p) (car p)) args)
                         ,@body))
        (proc ,@(map (fn (p) (car (cdr p))) args)))))

(define cond (mac args
    (if (not args)
        ''()
        (begin
            (define next    (car args))
            (define rest    (cdr args))
            (define test    (if (is (car next) 'else)
                                #t
                                (car next)))
            (define expr    (car (cdr next)))
            `(if ,test ,expr (cond ,@rest))))))

(define for (mac (item lst . body)
    `(begin
        (define for-iter ,lst)
        (while for-iter
            (define ,item (car for-iter))
            (set! for-iter (cdr for-iter))
            (begin ,@body)))))

(define test-suite (mac (title . body)
    `(begin
        (define pass-count 0)
        (define fail-count 0)
        (define sys-print print)
        (define print (fn (args)
            (sys-print " "
                       (car args)
                       (car (cdr args)))))
        (sys-print "TEST SUITE:" ,title)
        (sys-print "----------------------------------------------------")
        (sys-print " PASS | FAIL | Assert Expression                    ")
        (sys-print "----------------------------------------------------")

        (define results (list ,@body))
        (for expr results
            (define pass? (cdr (car expr)))
            (define test  (car (cdr expr)))
            (if (is pass? 1)
                (begin
                    (sys-print "   P           " test)
                    (set! pass-count (add1 pass-count))))
            (if (is pass? 2)
                (begin
                    (sys-print "          F    " test)
                    (set! fail-count (add1 fail-count)))))

        (sys-print "====================================================")
        (sys-print "PASS:" pass-count " FAIL:" fail-count)
        (sys-print ""))))

(define assert (mac (expr)
    `(begin
        (define result (eval ,expr))
        (if result
            (list (cons 'assert 1) ,expr)
            (list (cons 'assert 2) ,expr)))))

(define map (fn (proc lst)
    (define res nil)
    (while lst
        (set! res (cons (proc (car lst)) res))
        (set! lst (cdr lst)))
    (reverse res)))

(define range (fn (a b step)
    (define lst '())
    (while (<= a b)
        (set! lst (cons a lst))
        (set! a (+ a step)))
    (reverse lst)))

(define list-tail (fn (ls k)
    (while (> k 0)
        (set! ls (cdr ls))
        (set! k (sub1 k)))
    ls))

(define list-ref (fn (ls k)
    (car (list-tail ls k))))

