#lang racket/base
;; This file is part of Protocol Buffers for Racket.
;; Copyright (c) 2012 by Thomas Chust <chust@web.de>
;;
;; Protocol Buffers for Racket is free software: you can redistribute
;; it and/or modify it under the terms of the GNU Lesser General
;; Public License as published by the Free Software Foundation, either
;; version 3 of the License, or (at your option) any later version.
;;
;; Protocol Buffers for Racket is distributed in the hope that it will
;; be useful, but WITHOUT ANY WARRANTY; without even the implied
;; warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
;; PURPOSE. See the GNU Lesser General Public License for more
;; details.
;;
;; You should have received a copy of the GNU Lesser General Public
;; License along with Protocol Buffers for Racket. If not, see
;; <http://www.gnu.org/licenses/>.
(require
 srfi/26
 (only-in srfi/13 string-null? string-prefix? string-index-right)
 racket/contract
 racket/match
 racket/list
 racket/path
 racket/port
 racket/pretty
 "main.rkt"
 "google/protobuf/descriptor.rkt"
 "google/protobuf/compiler/plugin.rkt"
 "extend/protobuf/bigint.rkt")

(define-syntax doto
  (syntax-rules ()
    [(doto v op0 ops ...)
     (doto (op0 v) ops ...)]
    [(doto v)
     v]))

(define (proto-string->symbol str [prefix #f])
  (doto str
    (cut regexp-replace* #rx"(^|[a-z])([A-Z])" <>
         (λ (_ l u) (if (string-null? l) u (string-append l "-" u))))
    string-downcase
    (cut regexp-replace* #rx"_" <> "-")
    (cut regexp-replace #rx"^is-(.*)" <>
         (λ (_ s) (string-append s "?")))
    (if prefix
        (cut format "~a:~a" prefix <>)
        values)
    string->symbol))

(provide/contract
 [proto-string->symbol
  (->* (string?) (any/c)
       symbol?)])

(define (register-types! types proto)
  (let ([package (cond
                   [(file-descriptor-proto-package proto #f)
                    => (cut string-append "." <>)]
                   [else
                    ""])])
    (for-each
     (cut register-enum-type! types package <>)
     (file-descriptor-proto-enum-type proto))
    (for-each
     (cut register-message-types! types package <>)
     (file-descriptor-proto-message-type proto))))

(define (register-enum-type! types package proto [prefix #f])
  (let* ([name (enum-descriptor-proto-name proto)]
         [packaged (string-append package "." name)]
         [prefixed (proto-string->symbol name prefix)])
    (hash-set! types packaged (cons 'enum prefixed))))

(define (register-message-types! types package proto [prefix #f])
  (let* ([name (descriptor-proto-name proto)]
         [packaged (string-append package "." name)]
         [prefixed (proto-string->symbol name prefix)])
    (hash-set! types packaged (cons 'struct prefixed))
    (for-each
     (cut register-enum-type! types packaged <> prefixed)
     (descriptor-proto-enum-type proto))
    (for-each
     (cut register-message-types! types packaged <> prefixed)
     (descriptor-proto-nested-type proto))))

(define (type-ref types package name)
  (if (string-prefix? "." name)
      (hash-ref types name (cut raise-user-error name "unknown absolute type"))
      (let retry ([package package])
        (if (string-null? package)
            (raise-user-error name "unknown relative type")
            (or (hash-ref types (string-append package "." name) #f)
                (retry (substring
                        package
                        0 (or (string-index-right package #\.) 0))))))))

(provide/contract
 [register-types!
  (-> (hash/c string? (cons/c symbol? symbol?) #:immutable #f) file-descriptor-proto?
      any)]
 [register-enum-type!
  (->* ((hash/c string? (cons/c symbol? symbol?) #:immutable #f)
        (or/c string? #f) enum-descriptor-proto?)
       (any/c)
       any)]
 [register-message-types!
  (->* ((hash/c string? (cons/c symbol? symbol?) #:immutable #f)
        (or/c string? #f) descriptor-proto?)
       (any/c)
       any)]
 [type-ref
  (-> (hash/c string? (cons/c symbol? symbol?) #:immutable 'dont-care)
      string? string? (cons/c symbol? symbol?))])

(define (translate-types types proto)
  (let ([package (cond
                   [(file-descriptor-proto-package proto #f)
                    => (cut string-append "." <>)]
                   [else
                    ""])])
    (append
     (append*
      (map
       (cut translate-enum-type types package <>)
       (file-descriptor-proto-enum-type proto))
      (map
       (cut translate-message-types types package <>)
       (file-descriptor-proto-message-type proto)))
     (map
      (cut translate-extension types package <>)
      (file-descriptor-proto-extension proto)))))

(define (translate-enum-type types package proto [prefix #f])
  (let* ([name (enum-descriptor-proto-name proto)]
         [prefixed (proto-string->symbol name prefix)])
    `(define-enum-type ,prefixed
       ,(for/list ([val (in-list (enum-descriptor-proto-value proto))])
          (list (proto-string->symbol (enum-value-descriptor-proto-name val))
                (enum-value-descriptor-proto-number val))))))

(define (translate-message-types types package proto [prefix #f])
  (let* ([name (descriptor-proto-name proto)]
         [packaged (string-append package "." name)]
         [prefixed (proto-string->symbol name prefix)])
    (cons
     `(define-message-type ,prefixed
        ,(for/list ([field (in-list (descriptor-proto-field proto))])
           (translate-field types packaged field)))
     (append
      (append*
       (map
        (cut translate-enum-type types packaged <> prefixed)
        (descriptor-proto-enum-type proto))
       (map
        (cut translate-message-types types packaged <> prefixed)
        (descriptor-proto-nested-type proto)))
      (map
       (cut translate-extension types packaged <>)
       (descriptor-proto-extension proto))))))

(define (translate-extension types package proto)
  (match-let* ([name (field-descriptor-proto-extendee proto)]
               [(cons _ host) (type-ref types package name)])
    `(define-message-extension ,host
       ,(translate-field types package proto))))

(define (translate-field types packaged field)
  (let ([name (proto-string->symbol (field-descriptor-proto-name field))]
        [type (field-descriptor-proto-type field)]
        [options (field-descriptor-proto-options field field-options*)])
    (list*
     (case (field-descriptor-proto-label field)
       [(label-required)
        'required]
       [(label-optional)
        'optional]
       [(label-repeated)
        (if (field-options-packed options #f)
            'packed
            'repeated)])
     (cond
       [(field-descriptor-proto-type-name field #f)
        => (λ (name)
             (match-let ([(cons kind name) (type-ref types packaged name)])
               (set-field-descriptor-proto-type! field
                 (case kind
                   [(enum) 'type-enum]
                   [(struct) 'type-message]))
               (string->symbol (format "~a:~a" kind name))))]
       [else
        (case type
          [(type-int32)
           'primitive:int32]
          [(type-int64)
           'primitive:int64]
          [(type-uint32)
           'primitive:uint32]
          [(type-uint64)
           (let ([max-size (field-options-max-size options)])
             (if (= max-size 10)
                 'primitive:uint64
                 `(primitive:uint* ,(and (positive? max-size) max-size))))]
          [(type-sint32)
           'primitive:sint32]
          [(type-sint64)
           (let ([max-size (field-options-max-size options)])
             (if (= max-size 10)
                 'primitive:sint64
                 `(primitive:sint* ,(and (positive? max-size) max-size))))]
          [(type-fixed32)
           'primitive:fixed32]
          [(type-fixed64)
           'primitive:fixed64]
          [(type-sfixed32)
           'primitive:sfixed32]
          [(type-sfixed64)
           'primitive:sfixed64]
          [(type-bool)
           'primitive:bool]
          [(type-float)
           'primitive:float]
          [(type-double)
           'primitive:double]
          [(type-bytes)
           'primitive:bytes]
          [(type-string)
           'primitive:string]
          [else
           (raise-user-error
            name "unsupported field type: ~e"
            type)])])
     name
     (field-descriptor-proto-number field)
     (cond
       [(field-descriptor-proto-default-value field #f)
        => (λ (default)
             (list
              (case type
                [(type-int32 type-int64 type-uint32 type-uint64 type-sint32 type-sint64
                  type-fixed32 type-fixed64 type-sfixed32 type-sfixed64
                  type-float type-double)
                 (string->number default)]
                [(type-bool)
                 (not (equal? default "false"))]
                [(type-bytes)
                 (call-with-input-string
                  (string-append "#\"" default "\"")
                  read)]
                [(type-string)
                 default]
                [(type-enum)
                 `(quote ,(proto-string->symbol default))]
                [else
                 (raise-user-error
                  name "unsupported default value of type ~e: ~v"
                  type default)])))]
       [else
        null]))))

(provide/contract
 [translate-types
  (-> (hash/c string? (cons/c symbol? symbol?)) file-descriptor-proto?
      list?)]
 [translate-enum-type
  (->* ((hash/c string? (cons/c symbol? symbol?))
        (or/c string? #f) enum-descriptor-proto?)
       (any/c)
       any/c)]
 [translate-message-types
  (->* ((hash/c string? (cons/c symbol? symbol?))
        (or/c string? #f) descriptor-proto?)
       (any/c)
       list?)]
 [translate-extension
  (-> (hash/c string? (cons/c symbol? symbol?))
      (or/c string? #f) field-descriptor-proto?
      any/c)])

(define unix-root
  (string->some-system-path "/" 'unix))

(define (generate-racket req)
  (define types
    (make-hash))
  (define protos
    (for/hash ([file (in-list (code-generator-request-proto-file req))])
      (register-types! types file)
      (values (file-descriptor-proto-name file)
              file)))
  (with-handlers ([exn:fail:user?
                   (λ (exn)
                     (code-generator-response* #:error (exn-message exn)))])
    (code-generator-response*
     #:file
     (for/list ([proto-file (code-generator-request-file-to-generate req)]
                #:when #t
                [racket-file
                 (in-value
                  (path-replace-suffix
                   (string->some-system-path proto-file 'unix) ".rkt"))]
                [proto
                 (in-value
                  (hash-ref protos proto-file))])
       (code-generator-response:file*
        #:name (some-system-path->string racket-file)
        #:content
        (with-output-to-string
         (λ ()
           (parameterize ([print-as-expression #f])
             (display "#lang racket/base")
             (newline)
             (display ";; Generated using protoc-gen-racket v1.1")
             (newline)
             (pretty-print
              `(require
                murphy/protobuf1/syntax
                ,@(for/list ([dep (in-list (file-descriptor-proto-dependency proto))])
                    (some-system-path->string
                     (find-relative-path
                      (let-values ([(base name dir?) (split-path racket-file)])
                        (cond
                          [dir?
                           (path->complete-path racket-file unix-root)]
                          [(memq base '(relative #f))
                           unix-root]
                          [else
                           (path->complete-path base unix-root)]))
                      (path->complete-path
                       (path-replace-suffix (string->some-system-path dep 'unix) ".rkt")
                       unix-root))))))
             (newline)
             (for-each pretty-print (translate-types types proto))
             (newline)
             (pretty-print '(provide (all-defined-out)))))))))))

(define (main . args)
  (serialize (generate-racket (deserialize struct:code-generator-request))))

(provide/contract
 [generate-racket
  (-> code-generator-request? code-generator-response?)])
(provide
 main)
