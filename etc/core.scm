;; -------------------------------------------------------------
;;
;; support macros / functions
;;
;; -------------------------------------------------------------
(= append (fn (l m)
    (if (not l)
        '()
        (cons (car l) (append (cdr l) m)))))

(= for (mac (item lst . body)
    (list 'do
        (list 'let 'for-iter lst)
        (list 'while 'for-iter
            (list 'let item '(car for-iter))
            '(= for-iter (cdr for-iter))
            (cons 'do body)))))

(= test-suite (mac (title . body)
    (list 'do
        (list 'let 'pass-count 0)
        (list 'let 'fail-count 0)
        (list 'let 'sys-print 'print)
        (list 'let 'print '(fn (args)
                               (sys-print " "
                                          (car args)
                                          (car (cdr args)))))
        (list 'sys-print "TEST SUITE:" title)
        (list 'sys-print "----------------------------------------------------")
        (list 'sys-print " PASS | FAIL | Assert Expression                    ")
        (list 'sys-print "----------------------------------------------------")

        (list 'let 'results (cons 'list body))
        (list 'for 'x 'results
            (list 'if 'x
                '(= pass-count (+ 1 pass-count))
                '(= fail-count (+ 1 fail-count))))

        (list 'sys-print "====================================================")
        (list 'sys-print "PASS:" 'pass-count " FAIL:" 'fail-count)
        (list 'sys-print ""))))

(= assert (mac (expr)
    (list 'if (car (cdr expr))
        (list 'do
            (list 'print (list 'list " P          " expr))
            #t)
        (list 'do
            (list 'print (list 'list "        F   " expr))
            #f))))

;; -------------------------------------------------------------
;;
;; localization
;;
;; -------------------------------------------------------------

;; strings
(= str:ok                       (localized-string 0 'en_US "ok"))

(= str:buf-unable_to_open_file  (localized-string 100 'en_US "buf: unable to open file"))

(= str:eav-error                (localized-string 101 'en_US "eav: error"))
(= str:eav-not-found            (localized-string 102 'en_US "eav: not found"))
(= str:eav-sql-error            (localized-string 103 'en_US "eav: sql error"))
(= str:eav-invalid-rowid        (localized-string 104 'en_US "eav: invalid rowid"))
(= str:eav-invalid-entity       (localized-string 105 'en_US "eav: invalid entity"))

(= str:event-error              (localized-string 106 'en_US "event: error"))
(= str:event-timeout            (localized-string 107 'en_US "event: timeout"))

(= str:ffi-addr-null            (localized-string 108 'en_US "ffi: address null"))
(= str:ffi-proto-null           (localized-string 109 'en_US "ffi: prototype null"))
(= str:ffi-lib-not-loaded       (localized-string 110 'en_US "ffi: lib not loaded"))
(= str:ffi-symbol-not-found     (localized-string 111 'en_US "ffi: symbol not found"))
(= str:ffi-invalid-int-sz       (localized-string 112 'en_US "ffi: invalid int size"))
(= str:ffi-invalid-float-sz     (localized-string 113 'en_US "ffi: invalid float size"))
(= str:ffi-load-failure         (localized-string 114 'en_US "ffi: load library failure"))
(= str:ffi-sbv-not-impl         (localized-string 115 'en_US "ffi: struct by value not implemented"))

(= str:fs-not-dir               (localized-string 116 'en_US "filesys: not dir"))
(= str:fs-not-file              (localized-string 117 'en_US "filesys: not file"))
(= str:fs-not-exists            (localized-string 118 'en_US "filesys: not exists"))
(= str:fs-invalid-dir           (localized-string 119 'en_US "filesys: invalid dir"))
(= str:fs-chdir-failure         (localized-string 120 'en_US "filesys: chdir failure"))
(= str:fs-file-writable         (localized-string 121 'en_US "filesys: file writable"))
(= str:fs-mkdir-failure         (localized-string 122 'en_US "filesys: mkdir failure"))
(= str:fs-getcwd-failure        (localized-string 123 'en_US "filesys: getcwd failure"))
(= str:fs-rename-failure        (localized-string 124 'en_US "filesys: rename failure"))
(= str:fs-remove-failure        (localized-string 125 'en_US "filesys: remove failure"))
(= str:fs-not-equivalent        (localized-string 126 'en_US "filesys: not equivalent"))
(= str:fs-mkdtemp-failure       (localized-string 127 'en_US "filesys: mkdtemp failure"))
(= str:fs-not-implemented       (localized-string 128 'en_US "filesys: not implemented"))
(= str:fs-unexpected-path       (localized-string 129 'en_US "filesys: unexpected path"))
(= str:fs-realpath-failure      (localized-string 130 'en_US "filesys: realpath failure"))
(= str:fs-cannot-modify-root    (localized-string 131 'en_US "filesys: cannot modify root"))
(= str:fs-empty-path            (localized-string 132 'en_US "filesys: unexpected empty path"))
(= str:fs-rename-to-existing    (localized-string 133 'en_US "filesys: cannot rename to existing file"))

(= str:forth-error              (localized-string 134 'en_US "forth: error"))

(= str:intern-no-bucket         (localized-string 135 'en_US "intern: no bucket"))
(= str:intern-not-found         (localized-string 136 'en_US "intern: not found"))

(= str:ipc-error                (localized-string 137 'en_US "ipc: error"))

(= str:job-busy                 (localized-string 138 'en_US "job: busy"))
(= str:job-error                (localized-string 139 'en_US "job: error"))
(= str:job-invalid-job-id       (localized-string 140 'en_US "job: invalid job id"))
(= str:job-invalid-job-state    (localized-string 141 'en_US "job: invalid job state"))
(= str:job-label-intern-failure (localized-string 142 'en_US "job: label intern failure"))

(= str:mutex-busy               (localized-string 143 'en_US "mutex: busy"))
(= str:mutex-error              (localized-string 144 'en_US "mutex: error"))
(= str:mutex-invalid-mutex      (localized-string 145 'en_US "mutex: invalid mutex"))
(= str:mutex-out-of-memory      (localized-string 146 'en_US "mutex: out of memory"))
(= str:mutex-create-failure     (localized-string 147 'en_US "mutex: create mutex failure"))
(= str:mutex-insufficient-priv  (localized-string 148 'en_US "mutex: insufficient privilege"))
(= str:mutex-thread-owns-lock   (localized-string 149 'en_US "mutex: thread already owns lock"))

(= str:net-bind-failure         (localized-string 150 'en_US "net: bind failure"))
(= str:net-listen-failure       (localized-string 151 'en_US "net: listen failure"))
(= str:net-connect-failure      (localized-string 152 'en_US "net: connect failure"))
(= str:net-dgram-error          (localized-string 153 'en_US "net: socket dgram error"))
(= str:net-already-open         (localized-string 154 'en_US "net: socket already open"))
(= str:net-winsock-init-failure (localized-string 155 'en_US "net: winsock init failure"))
(= str:net-socket-closed        (localized-string 156 'en_US "net: socket already closed"))
(= str:net-invalid-bind         (localized-string 157 'en_US "net: invalid address and port"))
(= str:net-opt-broadcast-err    (localized-string 158 'en_US "net: socket option broadcast error"))

(= str:path-too-long            (localized-string 159 'en_US "path: too long"))
(= str:path-no-parent           (localized-string 160 'en_US "path: no parent path"))
(= str:path-empty               (localized-string 161 'en_US "path: unexpected empty path"))
(= str:path-expected-relative   (localized-string 162 'en_US "path: expected relative path"))
(= str:path-empty-ext           (localized-string 163 'en_US "path: unexpected empty extension"))

(= str:prof-no_cpu_rtdscp       (localized-string 164 'en_US "profiler: no cpu rtdscp support"))
(= str:prof-no_cpu_invariant    (localized-string 165 'en_US "profiler: no cpu invariant tsc support"))

(= str:thread-error             (localized-string 167 'en_US "thread: error"))
(= str:thread-deadlock          (localized-string 168 'en_US "thread: deadlock"))
(= str:thread-not-joinable      (localized-string 169 'en_US "thread: not joinable"))
(= str:thread-invalid-state     (localized-string 170 'en_US "thread: invalid state"))
(= str:thread-name-too-long     (localized-string 171 'en_US "thread: name too long"))
(= str:thread-invalid-thread    (localized-string 172 'en_US "thread: invalid thread"))
(= str:thread-already-joined    (localized-string 173 'en_US "thread: already joined"))
(= str:thread-not-cancelable    (localized-string 174 'en_US "thread: not cancelable"))
(= str:thread-already-canceled  (localized-string 175 'en_US "thread: already canceled"))
(= str:thread-already-detached  (localized-string 176 'en_US "thread: already detached"))
(= str:thread-create-thread     (localized-string 177 'en_US "thread: create thread failure"))
(= str:thread-insuff-privilege  (localized-string 178 'en_US "thread: insufficient privilege"))

(= str:timer-error              (localized-string 179 'en_US "timer: error"))

(= str:cxx-error                (localized-string 180 'en-US "cxx: error"))
(= str:cxx-lhs-not-found        (localized-string 181 'en-US "cxx: lhs not found"))
(= str:cxx-rhs-not-found        (localized-string 182 'en-US "cxx: rhs not found"))
(= str:cxx-pgm-not-found        (localized-string 183 'en-US "cxx: pgm not found"))
(= str:cxx-list-not-found       (localized-string 184 'en-US "cxx: list not found"))
(= str:cxx-scope-not-found      (localized-string 185 'en-US "cxx: scope not found"))
(= str:cxx-label-not-found      (localized-string 186 'en-US "cxx: label not found"))
(= str:cxx-child-not-found      (localized-string 187 'en-US "cxx: child not found"))
(= str:cxx-invalid-pp-type      (localized-string 188 'en-US "cxx: invalid pp type"))
(= str:cxx-intern-not-found     (localized-string 189 'en-US "cxx: intern not found"))
(= str:cxx-invalid-revision     (localized-string 190 'en-US "cxx: invalid revision"))
(= str:cxx-invalid-pos-type     (localized-string 191 'en-US "cxx: invalid pos type"))
(= str:cxx-element-not-found    (localized-string 192 'en-US "cxx: element not found"))
(= str:cxx-invalid-meta-type    (localized-string 193 'en-US "cxx: invalid meta type"))
(= str:cxx-invalid-list-entry   (localized-string 194 'en-US "cxx: invalid list entry"))
(= str:cxx-invalid-def-element  (localized-string 195 'en-US "cxx: invalid def element"))
(= str:cxx-invalid-decl-element (localized-string 196 'en-US "cxx: invalid decl element"))
(= str:cxx-invalid-expr-element (localized-string 197 'en-US "cxx: invalid expr element"))
(= str:cxx-unsupported-revision (localized-string 198 'en-US "cxx: unsupported revision"))
(= str:cxx-not-implemented      (localized-string 199 'en-US "cxx: not implemented"))

(= str:rpn-error                (localized-string 200 'en_US "rpn: error"))
(= str:rpn-invalid-array        (localized-string 201 'en_US "rpn: invalid operator precedence array"))

;; job states
(= str:job-state-queued         (localized-string 250 'en_US "queued"))
(= str:job-state-created        (localized-string 251 'en_US "created"))
(= str:job-state-running        (localized-string 252 'en_US "running"))
(= str:job-state-finished       (localized-string 253 'en_US "finished"))

;; log types
(= str:log-type-default         (localized-string 240 'en_US "default"))
(= str:log-type-spdlog          (localized-string 241 'en_US "spdlog"))
(= str:log-type-syslog          (localized-string 242 'en_US "syslog"))

;; errors


;; -------------------------------------------------------------
;;
;; test runner support
;;
;; -------------------------------------------------------------
(= binfmt-tests (fn ()
    nil))

(= core-tests (fn ()
    (do
        (do
            (let i 0)
            (for arg (current-command-line)
                (print i "=" arg)
                (= i (+ i 1))))

        (localized-string 5000 'en_US "US: test localized string: 0={} 1={} 2={}")
        (localized-string 5001 'en_US "duplicate cvar")
        (localized-string 5002 'en_US "cvar not found")
        (localized-string 5003 'en_US "invalid modification of constant: {}")
        (localized-string 5004 'en_GB "GB: test localized string: 0={} 1={} 2={}")

        (let str:undecl-msg (localized-string 5005 'en_US "undeclared identifier: {}"))
        (localized-error 5000 'en_US 'BC001 str:undecl-msg)

        ; basic assertions around scheme terp
        (test-suite "scheme terp basic assertions"
            (assert '(is 2 2))
            (assert '(not (> 2 9)))
            (assert '(>= 2 2))
            (assert '(is 0 (mod 10 2))))

        (test-suite "scheme + config module integration"
            (assert '(is "Basecode Foundation Core Library" (cvar-ref cvar:product-name)))
            (assert '(is 0 (cvar-ref cvar:version-major)))
            (assert '(is 1 (cvar-ref cvar:version-minor)))
            (assert '(is 0 (cvar-ref cvar:version-revision)))
            (assert '(do
                        (let build-type (cvar-ref cvar:build-type))
                        (or (is "Debug" build-type)
                            (is "Release" build-type))))
            (assert '(do
                        (let platform (cvar-ref cvar:platform))
                        (or (is "Windows" platform)
                            (is "Linux" platform))))
            (assert '(is #t (cvar-ref cvar:test-runner)))

            ; N.B. these initial values come from libcore-test's main.cpp
            ;       when it sets up the test runner.
            (assert '(is #t (cvar-ref cvar:enable-console-color)))
            (assert '(is "/var/log" (cvar-ref cvar:log-path)))
            (assert '(is 47.314 (cvar-ref cvar:magick-weight)))

            (cvar-set! cvar:enable-console-color    nil)
            (cvar-set! cvar:log-path                "/var/log/basecode")
            (cvar-set! cvar:magick-weight           12.6566)

            (assert '(is #f (cvar-ref cvar:enable-console-color)))
            (assert '(is "/var/log/basecode" (cvar-ref cvar:log-path)))
            (assert '(is 12.6566 (cvar-ref cvar:magick-weight))))

        (do
            (let parent-logger (current-logger))
            (let color-logger (log-create-color 'out))
            (let syslog-logger (syslog-create "bc-libcore-tests"
                           (list 'opt-pid 'opt-ndelay 'opt-cons 'opt-perror)
                           'local0))
            (let basic-file-logger (log-create-basic-file "test.log"))
            (let daily-file-logger (log-create-daily-file "daily.log" 23 0))
            (let rotating-file-logger (log-create-rotating-file "rotating.log"
                                                                (* 256 1024)
                                                                256))

            (logger-append-child parent-logger color-logger)
            (logger-append-child parent-logger basic-file-logger)
            (logger-append-child parent-logger daily-file-logger)
            (logger-append-child parent-logger rotating-file-logger)
            (logger-append-child parent-logger syslog-logger)

            (log-info "example log message!")
            (log-warn "oh, shit!")
            (log-error "this should really stick out")))))

(if (cvar-ref cvar:test-runner)
    (do
        (let product-name (cvar-ref cvar:product-name))
        (if (is product-name "Basecode Foundation Binary Format Library") (binfmt-tests)
            (is product-name "Basecode Foundation Core Library")          (core-tests)
            nil)))
