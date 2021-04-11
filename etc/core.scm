(load "core-localization.scm")

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

(if test-runner
    (load "core-tests.scm"))
