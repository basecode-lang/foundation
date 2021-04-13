(load "basic.scm")

(print (format "Product:  {}" *product-name*))
(print (format "Version:  {}.{}" *version-major* *version-minor*))
(print (format "Platform: {}" *platform*))

(cond
    [(is *product-name* "Basecode Foundation Core Library")             (load "core.scm")]
    [(is *product-name* "Basecode Foundation Binary Format Library")    (load "core.scm"
                                                                              "binfmt.scm")])
