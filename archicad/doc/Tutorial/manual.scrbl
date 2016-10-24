#lang scribble/manual
@(require scribble/eval
          (for-label racket
                     "helper.rkt"))
 
@title{My Library}
 
@defmodule[my-lib/helper]
 @(define helper-eval (make-base-eval))
@defproc[(my-helper [lst list?])
         (listof
          (not/c (one-of/c 'cow)))]{
 Replaces each @racket['cow] in @racket[lst] with
 @racket['aardvark].
 
 
 @interaction-eval[#:eval helper-eval
                   (require "helper.rkt")]
 @examples[
     #:eval helper-eval
     (my-helper '())
     (my-helper '(cows such remarkable cows))
   ]}