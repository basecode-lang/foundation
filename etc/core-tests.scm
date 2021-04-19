;; ----------------------------------------------------------------------------
;; ____                               _
;; |  _\                             | |
;; | |_)| __ _ ___  ___  ___ ___   __| | ___ TM
;; |  _< / _` / __|/ _ \/ __/ _ \ / _` |/ _ \
;; | |_)| (_| \__ \  __/ (_| (_) | (_| |  __/
;; |____/\__,_|___/\___|\___\___/ \__,_|\___|
;;
;;      F O U N D A T I O N   P R O J E C T
;;
;; Copyright (C) 2017-2021 Jeff Panici
;; All rights reserved.
;;
;; This software source file is licensed under the terms of MIT license.
;; For details, please read the LICENSE file.
;;
;; ----------------------------------------------------------------------------

(define fizz-buzz-test (lambda ()
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

(define core-tests (lambda ()
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
        (assert '(is "Basecode Foundation Core Library" *product-name*))
        (assert '(is 0 *version-major*))
        (assert '(is 1 *version-minor*))
        (assert '(is 0 *version-revision*))
        (assert '(begin
                    (or (is "Debug"             *build-type*)
                        (is "RelWithDebInfo"    *build-type*)
                        (is "Release"           *build-type*))))
        (assert '(begin
                    (or (is "Windows" *platform*)
                        (is "Linux" *platform*))))
        (assert '(is #t *test-runner*))

        ; N.B. these initial values come from libcore-test's main.cpp
        ;       when it sets up the test runner.
        (assert '(is #t *enable-console-color*))
        (assert '(is "/var/log" *log-path*))
        (assert '(is 47.314 *magick-weight*))

        (set! *enable-console-color*   #f)
        (set! *log-path*               "/var/log/basecode")
        (set! *magick-weight*          12.6566)

        (assert '(is #f *enable-console-color*))
        (assert '(is "/var/log/basecode" *log-path*))
        (assert '(is 12.6566 *magick-weight*))
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
(begin
    (define color-logger  (log-create-color 'out))
    (define syslog-logger (syslog-create "bc-libcore-tests"
                          (list 'opt-pid 'opt-ndelay 'opt-cons 'opt-perror)
                          'local0))
    (define basic-file-logger (log-create-basic-file "test.log"))
    (define daily-file-logger (log-create-daily-file "daily.log" 23 0))
    (define rotating-file-logger (log-create-rotating-file "rotating.log"
                                                           (* 256 1024)
                                                           256))

    (logger-append-child (current-logger) color-logger)
    (logger-append-child (current-logger) basic-file-logger)
    (logger-append-child (current-logger) daily-file-logger)
    (logger-append-child (current-logger) rotating-file-logger)
    (logger-append-child (current-logger) syslog-logger))

