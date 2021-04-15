;; -------------------------------------------------------------
;;
;; localization
;;
;; -------------------------------------------------------------

;; strings
(define read-error               (localized-string 20000 'en_US "container read error"))
(define write-error              (localized-string 20001 'en_US "container write error"))
(define import-failure           (localized-string 20002 'en_US "section import failure"))
(define duplicate-import         (localized-string 20003 'en_US "imported duplicate symbol"))
(define duplicate-symbol         (localized-string 20004 'en_US "duplicate symbol"))
(define symbol-not-found         (localized-string 20005 'en_US "symbol not found"))
(define config-eval-error        (localized-string 20006 'en_US "unable to read/eval config"))
(define invalid-section-type     (localized-string 20007 'en_US "invalid section type"))
(define container-init-error     (localized-string 20008 'en_US "container init error"))
(define invalid-container-type   (localized-string 20009 'en_US "invalid container type"))
(define spec-section-custom-name (localized-string 20010 'en_US "specification section cannot use custom name"))
(define not-implemented          (localized-string 20011 'en_US "not implemented"))
(define invalid-machine          (localized-string 20012 'en_US "invalid machine type"))
(define invalid-output-type      (localized-string 20013 'en_US "invalid output type"))
(define cannot-map-section-name  (localized-string 20014 'en_US "cannot map section name"))
(define section-not-found        (localized-string 20015 'en_US "section not found"))
(define invalid-input-type       (localized-string 20016 'en_US "invalid input type"))
(define not-ar-long-name         (localized-string 20017 'en_US "not ar long name"))
(define sect-entry-out-of-bounds (localized-string 20018 'en_US "section entry out-of-bounds"))
(define bad-cv-signature         (localized-string 20019 'en_US "bad CV signature"))
(define missing-linked-section   (localized-string 20020 'en_US "missing linked section"))
(define custom-sect-missing-sym  (localized-string 20021 'en_US "custom section missing symbol"))
(define invalid-file-type        (localized-string 20022 'en_US "invalid file type"))
(define elf-unsupported-section  (localized-string 20023 'en_US "elf unsupported section"))
(define invalid-reloc-type       (localized-string 20024 'en_US "invalid relocation type"))

;; errors
(localized-error read-error               'en_US 'BF001 read-error)
(localized-error write-error              'en_US 'BF002 write-error)
(localized-error import-failure           'en_US 'BF003 import-failure)
(localized-error duplicate-import         'en_US 'BF004 duplicate-import)
(localized-error duplicate-symbol         'en_US 'BF005 duplicate-symbol)
(localized-error symbol-not-found         'en_US 'BF006 symbol-not-found)
(localized-error config-eval-error        'en_US 'BF007 config-eval-error)
(localized-error invalid-section-type     'en_US 'BF008 invalid-section-type)
(localized-error container-init-error     'en_US 'BF009 container-init-error)
(localized-error invalid-container-type   'en_US 'BF010 invalid-container-type)
(localized-error spec-section-custom-name 'en_US 'BF011 spec-section-custom-name)
(localized-error not-implemented          'en_US 'BF012 not-implemented)
(localized-error invalid-machine          'en_US 'BF013 invalid-machine)
(localized-error invalid-output-type      'en_US 'BF014 invalid-output-type)
(localized-error cannot-map-section-name  'en_US 'BF015 cannot-map-section-name)
(localized-error section-not-found        'en_US 'BF016 section-not-found)
(localized-error invalid-input-type       'en_US 'BF017 invalid-input-type)
(localized-error not-ar-long-name         'en_US 'BF018 not-ar-long-name)
(localized-error sect-entry-out-of-bounds 'en_US 'BF019 sect-entry-out-of-bounds)
(localized-error bad-cv-signature         'en_US 'BF020 bad-cv-signature)
(localized-error missing-linked-section   'en_US 'BF021 missing-linked-section)
(localized-error custom-sect-missing-sym  'en_US 'BF022 custom-sect-missing-sym)
(localized-error invalid-file-type        'en_US 'BF023 invalid-file-type)
(localized-error elf-unsupported-section  'en_US 'BF024 elf-unsupported-section)
(localized-error invalid-reloc-type       'en_US 'BF025 invalid-reloc-type)
