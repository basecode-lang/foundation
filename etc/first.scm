(load "basic.scm")

(define test-runner (cvar-ref *test-runner*))
(define product-name (cvar-ref *product-name*))
(print (format "Product:  {}" product-name))
(print (format "Version:  {}.{}" (cvar-ref *version-major*) (cvar-ref *version-minor*)))
(print (format "Platform: {}" (cvar-ref *platform*)))

(cond
    [(is product-name "Basecode Foundation Core Library")           (load "core.scm")]
    [(is product-name "Basecode Foundation Binary Format Library")  (load "core.scm"
                                                                          "binfmt.scm")])
