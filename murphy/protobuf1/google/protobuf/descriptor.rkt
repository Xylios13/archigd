#lang racket/base
;; Generated using protoc-gen-racket v1.1
(require murphy/protobuf1/syntax)

(define-message-type
 file-descriptor-set
 ((repeated struct:file-descriptor-proto file 1)))
(define-message-type
 file-descriptor-proto
 ((optional primitive:string name 1)
  (optional primitive:string package 2)
  (repeated primitive:string dependency 3)
  (repeated struct:descriptor-proto message-type 4)
  (repeated struct:enum-descriptor-proto enum-type 5)
  (repeated struct:service-descriptor-proto service 6)
  (repeated struct:field-descriptor-proto extension 7)
  (optional struct:file-options options 8)
  (optional struct:source-code-info source-code-info 9)))
(define-message-type
 descriptor-proto
 ((optional primitive:string name 1)
  (repeated struct:field-descriptor-proto field 2)
  (repeated struct:field-descriptor-proto extension 6)
  (repeated struct:descriptor-proto nested-type 3)
  (repeated struct:enum-descriptor-proto enum-type 4)
  (repeated struct:descriptor-proto:extension-range extension-range 5)
  (optional struct:message-options options 7)))
(define-message-type
 descriptor-proto:extension-range
 ((optional primitive:int32 start 1) (optional primitive:int32 end 2)))
(define-message-type
 field-descriptor-proto
 ((optional primitive:string name 1)
  (optional primitive:int32 number 3)
  (optional enum:field-descriptor-proto:label label 4)
  (optional enum:field-descriptor-proto:type type 5)
  (optional primitive:string type-name 6)
  (optional primitive:string extendee 2)
  (optional primitive:string default-value 7)
  (optional struct:field-options options 8)))
(define-enum-type
 field-descriptor-proto:type
 ((type-double 1)
  (type-float 2)
  (type-int64 3)
  (type-uint64 4)
  (type-int32 5)
  (type-fixed64 6)
  (type-fixed32 7)
  (type-bool 8)
  (type-string 9)
  (type-group 10)
  (type-message 11)
  (type-bytes 12)
  (type-uint32 13)
  (type-enum 14)
  (type-sfixed32 15)
  (type-sfixed64 16)
  (type-sint32 17)
  (type-sint64 18)))
(define-enum-type
 field-descriptor-proto:label
 ((label-optional 1) (label-required 2) (label-repeated 3)))
(define-message-type
 enum-descriptor-proto
 ((optional primitive:string name 1)
  (repeated struct:enum-value-descriptor-proto value 2)
  (optional struct:enum-options options 3)))
(define-message-type
 enum-value-descriptor-proto
 ((optional primitive:string name 1)
  (optional primitive:int32 number 2)
  (optional struct:enum-value-options options 3)))
(define-message-type
 service-descriptor-proto
 ((optional primitive:string name 1)
  (repeated struct:method-descriptor-proto method 2)
  (optional struct:service-options options 3)))
(define-message-type
 method-descriptor-proto
 ((optional primitive:string name 1)
  (optional primitive:string input-type 2)
  (optional primitive:string output-type 3)
  (optional struct:method-options options 4)))
(define-message-type
 file-options
 ((optional primitive:string java-package 1)
  (optional primitive:string java-outer-classname 8)
  (optional primitive:bool java-multiple-files 10 #f)
  (optional primitive:bool java-generate-equals-and-hash 20 #f)
  (optional enum:file-options:optimize-mode optimize-for 9 'speed)
  (optional primitive:bool cc-generic-services 16 #f)
  (optional primitive:bool java-generic-services 17 #f)
  (optional primitive:bool py-generic-services 18 #f)
  (repeated struct:uninterpreted-option uninterpreted-option 999)))
(define-enum-type
 file-options:optimize-mode
 ((speed 1) (code-size 2) (lite-runtime 3)))
(define-message-type
 message-options
 ((optional primitive:bool message-set-wire-format 1 #f)
  (optional primitive:bool no-standard-descriptor-accessor 2 #f)
  (repeated struct:uninterpreted-option uninterpreted-option 999)))
(define-message-type
 field-options
 ((optional enum:field-options:ctype ctype 1 'string)
  (optional primitive:bool packed 2)
  (optional primitive:bool deprecated 3 #f)
  (optional primitive:string experimental-map-key 9)
  (repeated struct:uninterpreted-option uninterpreted-option 999)))
(define-enum-type field-options:ctype ((string 0) (cord 1) (string-piece 2)))
(define-message-type
 enum-options
 ((repeated struct:uninterpreted-option uninterpreted-option 999)))
(define-message-type
 enum-value-options
 ((repeated struct:uninterpreted-option uninterpreted-option 999)))
(define-message-type
 service-options
 ((repeated struct:uninterpreted-option uninterpreted-option 999)))
(define-message-type
 method-options
 ((repeated struct:uninterpreted-option uninterpreted-option 999)))
(define-message-type
 uninterpreted-option
 ((repeated struct:uninterpreted-option:name-part name 2)
  (optional primitive:string identifier-value 3)
  (optional primitive:uint64 positive-int-value 4)
  (optional primitive:int64 negative-int-value 5)
  (optional primitive:double double-value 6)
  (optional primitive:bytes string-value 7)
  (optional primitive:string aggregate-value 8)))
(define-message-type
 uninterpreted-option:name-part
 ((required primitive:string name-part 1)
  (required primitive:bool extension? 2)))
(define-message-type
 source-code-info
 ((repeated struct:source-code-info:location location 1)))
(define-message-type
 source-code-info:location
 ((packed primitive:int32 path 1) (packed primitive:int32 span 2)))

(provide (all-defined-out))
