;; -------------------------------------------------------------
;;
;; support macros / functions
;;
;; -------------------------------------------------------------
(define add1 (mac (e)
    `(+ ,e 1)))

(define sub1 (mac (e)
    `(- ,e 1)))

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

(define length (fn (ls)
    (define count 0)
    (while ls
        (set! count (add1 count))
        (set! ls (cdr ls)))
    count))

(define list-tail (fn (ls k)
    (while (> k 0)
        (set! ls (cdr ls))
        (set! k (sub1 k)))
    ls))

(define list-ref (fn (ls k)
    (car (list-tail ls k))))

(define reverse (fn (lst)
    (define res nil)
    (while lst
        (set! res (cons (car lst) res))
        (set! lst (cdr lst)))
    res))

(define append (fn (a b)
    (define a1 (reverse a))
    (define b1 b)
    (while a1
        (set! b1 (cons (car a1) b1))
        (set! a1 (cdr a1)))
    b1))

(define append-reverse (fn (rev tail)
    (if (not rev)
        tail
        (append-reverse (cdr rev) (cons (car rev) tail)))))

(define cons* (fn (first . rest)
    (define loop (fn (rev next rest)
        (if (not rest)
            (append-reverse rev next)
            (loop (cons next rev) (car rest) (cdr rest)))))
    (loop '() first rest)))
(define list* cons*)

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
            (if expr
                (set! pass-count (+ 1 pass-count))
                (set! fail-count (+ 1 fail-count))))

        (sys-print "====================================================")
        (sys-print "PASS:" pass-count " FAIL:" fail-count)
        (sys-print ""))))

(define assert (mac (expr)
    `(begin
        (define result (eval ,expr))
        (if result
            (begin
                (print (list " P          " ,expr))
                #t)
            (begin
                (print (list "        F   " ,expr))
                #f)))))

;; -------------------------------------------------------------
;;
;; localization
;;
;; -------------------------------------------------------------

;; strings
(define str:ok                       (localized-string 0 'en_US "ok"))

(define str:buf-unable_to_open_file  (localized-string 100 'en_US "buf: unable to open file"))

(define str:eav-error                (localized-string 101 'en_US "eav: error"))
(define str:eav-not-found            (localized-string 102 'en_US "eav: not found"))
(define str:eav-sql-error            (localized-string 103 'en_US "eav: sql error"))
(define str:eav-invalid-rowid        (localized-string 104 'en_US "eav: invalid rowid"))
(define str:eav-invalid-entity       (localized-string 105 'en_US "eav: invalid entity"))

(define str:event-error              (localized-string 106 'en_US "event: error"))
(define str:event-timeout            (localized-string 107 'en_US "event: timeout"))

(define str:ffi-addr-null            (localized-string 108 'en_US "ffi: address null"))
(define str:ffi-proto-null           (localized-string 109 'en_US "ffi: prototype null"))
(define str:ffi-lib-not-loaded       (localized-string 110 'en_US "ffi: lib not loaded"))
(define str:ffi-symbol-not-found     (localized-string 111 'en_US "ffi: symbol not found"))
(define str:ffi-invalid-int-sz       (localized-string 112 'en_US "ffi: invalid int size"))
(define str:ffi-invalid-float-sz     (localized-string 113 'en_US "ffi: invalid float size"))
(define str:ffi-load-failure         (localized-string 114 'en_US "ffi: load library failure"))
(define str:ffi-sbv-not-impl         (localized-string 115 'en_US "ffi: struct by value not implemented"))

(define str:fs-not-dir               (localized-string 116 'en_US "filesys: not dir"))
(define str:fs-not-file              (localized-string 117 'en_US "filesys: not file"))
(define str:fs-not-exists            (localized-string 118 'en_US "filesys: not exists"))
(define str:fs-invalid-dir           (localized-string 119 'en_US "filesys: invalid dir"))
(define str:fs-chdir-failure         (localized-string 120 'en_US "filesys: chdir failure"))
(define str:fs-file-writable         (localized-string 121 'en_US "filesys: file writable"))
(define str:fs-mkdir-failure         (localized-string 122 'en_US "filesys: mkdir failure"))
(define str:fs-getcwd-failure        (localized-string 123 'en_US "filesys: getcwd failure"))
(define str:fs-rename-failure        (localized-string 124 'en_US "filesys: rename failure"))
(define str:fs-remove-failure        (localized-string 125 'en_US "filesys: remove failure"))
(define str:fs-not-equivalent        (localized-string 126 'en_US "filesys: not equivalent"))
(define str:fs-mkdtemp-failure       (localized-string 127 'en_US "filesys: mkdtemp failure"))
(define str:fs-not-implemented       (localized-string 128 'en_US "filesys: not implemented"))
(define str:fs-unexpected-path       (localized-string 129 'en_US "filesys: unexpected path"))
(define str:fs-realpath-failure      (localized-string 130 'en_US "filesys: realpath failure"))
(define str:fs-cannot-modify-root    (localized-string 131 'en_US "filesys: cannot modify root"))
(define str:fs-empty-path            (localized-string 132 'en_US "filesys: unexpected empty path"))
(define str:fs-rename-to-existing    (localized-string 133 'en_US "filesys: cannot rename to existing file"))

(define str:forth-error              (localized-string 134 'en_US "forth: error"))

(define str:intern-no-bucket         (localized-string 135 'en_US "intern: no bucket"))
(define str:intern-not-found         (localized-string 136 'en_US "intern: not found"))

(define str:ipc-error                (localized-string 137 'en_US "ipc: error"))

(define str:job-busy                 (localized-string 138 'en_US "job: busy"))
(define str:job-error                (localized-string 139 'en_US "job: error"))
(define str:job-invalid-job-id       (localized-string 140 'en_US "job: invalid job id"))
(define str:job-invalid-job-state    (localized-string 141 'en_US "job: invalid job state"))
(define str:job-label-intern-failure (localized-string 142 'en_US "job: label intern failure"))

(define str:mutex-busy               (localized-string 143 'en_US "mutex: busy"))
(define str:mutex-error              (localized-string 144 'en_US "mutex: error"))
(define str:mutex-invalid-mutex      (localized-string 145 'en_US "mutex: invalid mutex"))
(define str:mutex-out-of-memory      (localized-string 146 'en_US "mutex: out of memory"))
(define str:mutex-create-failure     (localized-string 147 'en_US "mutex: create mutex failure"))
(define str:mutex-insufficient-priv  (localized-string 148 'en_US "mutex: insufficient privilege"))
(define str:mutex-thread-owns-lock   (localized-string 149 'en_US "mutex: thread already owns lock"))

(define str:net-bind-failure         (localized-string 150 'en_US "net: bind failure"))
(define str:net-listen-failure       (localized-string 151 'en_US "net: listen failure"))
(define str:net-connect-failure      (localized-string 152 'en_US "net: connect failure"))
(define str:net-dgram-error          (localized-string 153 'en_US "net: socket dgram error"))
(define str:net-already-open         (localized-string 154 'en_US "net: socket already open"))
(define str:net-winsock-init-failure (localized-string 155 'en_US "net: winsock init failure"))
(define str:net-socket-closed        (localized-string 156 'en_US "net: socket already closed"))
(define str:net-invalid-bind         (localized-string 157 'en_US "net: invalid address and port"))
(define str:net-opt-broadcast-err    (localized-string 158 'en_US "net: socket option broadcast error"))

(define str:path-too-long            (localized-string 159 'en_US "path: too long"))
(define str:path-no-parent           (localized-string 160 'en_US "path: no parent path"))
(define str:path-empty               (localized-string 161 'en_US "path: unexpected empty path"))
(define str:path-expected-relative   (localized-string 162 'en_US "path: expected relative path"))
(define str:path-empty-ext           (localized-string 163 'en_US "path: unexpected empty extension"))

(define str:prof-no_cpu_rtdscp       (localized-string 164 'en_US "profiler: no cpu rtdscp support"))
(define str:prof-no_cpu_invariant    (localized-string 165 'en_US "profiler: no cpu invariant tsc support"))

(define str:thread-error             (localized-string 167 'en_US "thread: error"))
(define str:thread-deadlock          (localized-string 168 'en_US "thread: deadlock"))
(define str:thread-not-joinable      (localized-string 169 'en_US "thread: not joinable"))
(define str:thread-invalid-state     (localized-string 170 'en_US "thread: invalid state"))
(define str:thread-name-too-long     (localized-string 171 'en_US "thread: name too long"))
(define str:thread-invalid-thread    (localized-string 172 'en_US "thread: invalid thread"))
(define str:thread-already-joined    (localized-string 173 'en_US "thread: already joined"))
(define str:thread-not-cancelable    (localized-string 174 'en_US "thread: not cancelable"))
(define str:thread-already-canceled  (localized-string 175 'en_US "thread: already canceled"))
(define str:thread-already-detached  (localized-string 176 'en_US "thread: already detached"))
(define str:thread-create-thread     (localized-string 177 'en_US "thread: create thread failure"))
(define str:thread-insuff-privilege  (localized-string 178 'en_US "thread: insufficient privilege"))

(define str:timer-error              (localized-string 179 'en_US "timer: error"))

(define str:cxx-error                (localized-string 180 'en-US "cxx: error"))
(define str:cxx-lhs-not-found        (localized-string 181 'en-US "cxx: lhs not found"))
(define str:cxx-rhs-not-found        (localized-string 182 'en-US "cxx: rhs not found"))
(define str:cxx-pgm-not-found        (localized-string 183 'en-US "cxx: pgm not found"))
(define str:cxx-list-not-found       (localized-string 184 'en-US "cxx: list not found"))
(define str:cxx-scope-not-found      (localized-string 185 'en-US "cxx: scope not found"))
(define str:cxx-label-not-found      (localized-string 186 'en-US "cxx: label not found"))
(define str:cxx-child-not-found      (localized-string 187 'en-US "cxx: child not found"))
(define str:cxx-invalid-pp-type      (localized-string 188 'en-US "cxx: invalid pp type"))
(define str:cxx-intern-not-found     (localized-string 189 'en-US "cxx: intern not found"))
(define str:cxx-invalid-revision     (localized-string 190 'en-US "cxx: invalid revision"))
(define str:cxx-invalid-pos-type     (localized-string 191 'en-US "cxx: invalid pos type"))
(define str:cxx-element-not-found    (localized-string 192 'en-US "cxx: element not found"))
(define str:cxx-invalid-meta-type    (localized-string 193 'en-US "cxx: invalid meta type"))
(define str:cxx-invalid-list-entry   (localized-string 194 'en-US "cxx: invalid list entry"))
(define str:cxx-invalid-def-element  (localized-string 195 'en-US "cxx: invalid def element"))
(define str:cxx-invalid-decl-element (localized-string 196 'en-US "cxx: invalid decl element"))
(define str:cxx-invalid-expr-element (localized-string 197 'en-US "cxx: invalid expr element"))
(define str:cxx-unsupported-revision (localized-string 198 'en-US "cxx: unsupported revision"))
(define str:cxx-not-implemented      (localized-string 199 'en-US "cxx: not implemented"))

(define str:rpn-error                (localized-string 200 'en_US "rpn: error"))
(define str:rpn-invalid-array        (localized-string 201 'en_US "rpn: invalid operator precedence array"))

;; job states
(define str:job-state-queued         (localized-string 250 'en_US "queued"))
(define str:job-state-created        (localized-string 251 'en_US "created"))
(define str:job-state-running        (localized-string 252 'en_US "running"))
(define str:job-state-finished       (localized-string 253 'en_US "finished"))

;; log types
(define str:log-type-default         (localized-string 240 'en_US "default"))
(define str:log-type-spdlog          (localized-string 241 'en_US "spdlog"))
(define str:log-type-syslog          (localized-string 242 'en_US "syslog"))

;; errors


;; -------------------------------------------------------------
;;
;; test runner support
;;
;; -------------------------------------------------------------
(define binfmt-tests (fn ()
    nil))

(define core-tests (fn ()
    (begin
        (begin
            (define fizz-buzz  0)
            (define fizz       0)
            (define buzz       0)
            (define numbers    0)
            (print "fizz buzz test:")
            (for n (range 1 50 1)
                (cond
                    [(is (mod n 15) 0)      (set! fizz-buzz (add1 fizz-buzz))]
                    [(is (mod n 3) 0)       (set! fizz      (add1 fizz))]
                    [(is (mod n 5) 0)       (set! buzz      (add1 buzz))]
                    [else                   (set! numbers   (add1 numbers))]))
            (print (format "  fizz buzz: {:.7g}" fizz-buzz))
            (print (format "  fizz     : {:.7g}" fizz))
            (print (format "  buzz     : {:.7g}" buzz))
            (print (format "  numbers  : {:.7g}" numbers)))

        ; basic assertions around scheme terp
        (test-suite "scheme terp basic assertions"
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
            )

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
            (logger-append-child (current-logger) syslog-logger)

            (log-info "example log message: {}" "Hello, Sailor!")
            (log-warn "oh, noes! something bad is about to happen: {} {}" "Dirk!" 42)
            (log-error "this should really stick out: {}" "yup, it does")
            ))))

(if (cvar-ref *test-runner*)
    (begin
        (define product-name (cvar-ref *product-name*))
        (cond
            [(is product-name "Basecode Foundation Binary Format Library") (binfmt-tests)]
            [(is product-name "Basecode Foundation Core Library")          (core-tests)]
            [else                                                          (print "shit")]))
    (print "wtf!!!???"))
