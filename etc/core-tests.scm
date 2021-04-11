(define fizz-buzz-test (fn ()
    (define fizz-buzz  0)
    (define fizz       0)
    (define buzz       0)
    (define numbers    0)
    (for n (range 1 50 1)
        (cond
            [(is (mod n 15) 0)      (set! fizz-buzz (add1 fizz-buzz))]
            [(is (mod n 3) 0)       (set! fizz      (add1 fizz))]
            [(is (mod n 5) 0)       (set! buzz      (add1 buzz))]
            [else                   (set! numbers   (add1 numbers))]))
    `((fizz-buzz . ,fizz-buzz) (fizz . ,fizz) (buzz . ,buzz) (numbers . ,numbers))))

(define core-tests (fn ()
    ; basic assertions around scheme terp
    (test-suite "scheme terp basic assertions"
        (assert '(is    '((fizz-buzz . 3) (fizz . 13) (buzz . 7) (numbers . 27)) (fizz-buzz-test)))
        (assert '(<     "a" "b"))
        (assert '(>     "j"  "i"))
        (assert '(is    15  (and 3 5 15)))
        (assert '(is    2   (or nil 2 nil 4 nil 6)))
        (assert '(is    5   (length '(1 2 3 4 5))))
        (assert '(is    2   2))
        (assert '(not   (> 2 9)))
        (assert '(>=    2   2))
        (assert '(is        (mod 10 2) 0))
        (assert '(is        '(10 20)
                            (begin
                                (define a 10)
                                (define b 20)
                                `(,a ,b))))
        (assert '(is        '(10 20 30)
                            (begin
                                (define a 10)
                                (define b 20)
                                `(,a ,b ,(+ a b)))))
        (assert '(is        '(0 1 2 3 4 5)
                            `(0 1 ,@(list 2 3 4) 5)))
        (assert '(is 300    (eval '(+ 100 200))))
        (assert '(is 42     (begin (define a 42) (eval 'a))))
        (assert '(is 15     (begin
                                (define sum 0)
                                (define expr '(for x (list 1 2 3 4 5)
                                                (set! sum (+ sum x))))
                                (eval expr)
                                sum)))
        (assert '(>         (begin
                                (define count 0)
                                (for arg (current-command-line)
                                    (set! count (add1 count)))
                                count)
                            0)))

    (test-suite "scheme + config module integration"
        (assert '(is "Basecode Foundation Core Library" (cvar-ref *product-name*)))
        (assert '(is 0 (cvar-ref *version-major*)))
        (assert '(is 1 (cvar-ref *version-minor*)))
        (assert '(is 0 (cvar-ref *version-revision*)))
        (assert '(begin
                    (define build-type (cvar-ref *build-type*))
                    (or (is "Debug"             build-type)
                        (is "RelWithDebInfo"    build-type)
                        (is "Release"           build-type))))
        (assert '(begin
                    (define platform (cvar-ref *platform*))
                    (or (is "Windows" platform)
                        (is "Linux" platform))))
        (assert '(is #t (cvar-ref *test-runner*)))

        ; N.B. these initial values come from libcore-test's main.cpp
        ;       when it sets up the test runner.
        (assert '(is #t (cvar-ref *enable-console-color*)))
        (assert '(is "/var/log" (cvar-ref *log-path*)))
        (assert '(is 47.314 (cvar-ref *magick-weight*)))

        (cvar-set! *enable-console-color*   #f)
        (cvar-set! *log-path*               "/var/log/basecode")
        (cvar-set! *magick-weight*          12.6566)

        (assert '(is #f (cvar-ref *enable-console-color*)))
        (assert '(is "/var/log/basecode" (cvar-ref *log-path*)))
        (assert '(is 12.6566 (cvar-ref *magick-weight*)))
        (assert '(is #t (begin
                            (localized-string 5000 'en_US "US: test localized string: 0={} 1={} 2={}")
                            (localized-string 5001 'en_US "duplicate cvar")
                            (localized-string 5002 'en_US "cvar not found")
                            (localized-string 5003 'en_US "invalid modification of constant: {}")
                            (localized-string 5004 'en_GB "GB: test localized string: 0={} 1={} 2={}")

                            (define str:undecl-msg (localized-string 5005 'en_US "undeclared identifier: {}"))
                            (localized-error 5000 'en_US 'BC001 str:undecl-msg)
                            #t)))
        )))

(core-tests)
