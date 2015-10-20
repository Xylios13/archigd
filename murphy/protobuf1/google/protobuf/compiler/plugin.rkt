#lang racket/base
;; Generated using protoc-gen-racket v1.1
(require murphy/protobuf1/syntax "../descriptor.rkt")

(define-message-type
 code-generator-request
 ((repeated primitive:string file-to-generate 1)
  (optional primitive:string parameter 2)
  (repeated struct:file-descriptor-proto proto-file 15)))
(define-message-type
 code-generator-response
 ((optional primitive:string error 1)
  (repeated struct:code-generator-response:file file 15)))
(define-message-type
 code-generator-response:file
 ((optional primitive:string name 1)
  (optional primitive:string insertion-point 2)
  (optional primitive:string content 15)))

(provide (all-defined-out))
