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

(load "basic.scm")

(print (format "Product:  {}" *product-name*))
(print (format "Version:  {}.{}" *version-major* *version-minor*))
(print (format "Platform: {}" *platform*))

(cond
    [(is *product-name* "Basecode Foundation Core Library")
     (load "core.scm"
           "core-tests.scm")]

    [(is *product-name* "Basecode Foundation Binary Format Library")
     (load "core.scm"
           "binfmt.scm"
           "binfmt-tests.scm")])
