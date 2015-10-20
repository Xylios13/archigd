#lang scribble/manual
@require[(for-label "../syntax.rkt" "../reflection.rkt")]

@title{Syntax}

@defmodule[murphy/protobuf1/syntax]{

  Racket syntax to represent protocol buffer definitions as used by
  code output from @tt{protoc-gen-racket}.

}

@defform[(define-primitive-type name type
           reader writer)
         #:contracts
         ([reader (-> input-port? any/c)]
          [writer (-> any/c output-port? any)])]{

  Binds @racketid[primitive:]@racket[name] to an instance of
  @racket[primitive-info] using the quoted @racket[type] and the
  specified @racket[reader] and @racket[writer].

  This is an internal utility macro and should normally not be used
  since the set of primitive wire types for protocol buffers is not
  intended to be extended.

}

@deftogether[(
  @defthing[primitive:int32 primitive-info?]{}
  @defthing[primitive:int64 primitive-info?]{}
  @defthing[primitive:uint32 primitive-info?]{}
  @defthing[primitive:uint64 primitive-info?]{}
  @defthing[primitive:sint32 primitive-info?]{}
  @defthing[primitive:sint64 primitive-info?]{}
  @defthing[primitive:fixed32 primitive-info?]{}
  @defthing[primitive:fixed64 primitive-info?]{}
  @defthing[primitive:sfixed32 primitive-info?]{}
  @defthing[primitive:sfixed64 primitive-info?]{}
  @defthing[primitive:bool primitive-info?]{}
  @defthing[primitive:float primitive-info?]{}
  @defthing[primitive:double primitive-info?]{}
  @defthing[primitive:bytes primitive-info?]{}
  @defthing[primitive:string primitive-info?]{}
)]{

  The primitive protocol buffer types.

}

@deftogether[(
  @defproc[(primitive:uint* [max-size (or/c exact-positive-integer? #f)]) primitive-info?]{}
  @defproc[(primitive:sint* [max-size (or/c exact-positive-integer? #f)]) primitive-info?]{}
)]{

  Generators for variants of the primitive protocol buffer integer
  types that modify or lift the maximum encoded size restriction for
  these values.

  The @tt{protoc-gen-racket} code generator uses these types when it
  finds an @litchar{(extend.protobuf.max_size)} option on a field of
  type @litchar{uint64} or @litchar{sint64}. The @racket[max-size]
  will be the value of the option or @racket[#f] in case the option is
  zero. Protocol buffer definitions using this extension must
  @litchar{import "extend/protobuf/bigint.proto"} from this library's
  distribution.

  Note that relaxing the size restriction is not compatible with other
  implementations of protocol buffers and lifting the restriction
  entirely may also be vulnerable to denial of service attacks.

}

@defform[(define-enum-type name
           ([alt tag] ...))
         #:contracts
         ([tag exact-nonnegative-integer?])]{

  Binds @racketid[enum:]@racket[name] to an instance of
  @racket[enum-info] and defines the enumeration conversion procedures
  @racketid[integer->]@racket[name] and @racket[name]@racketid[->integer].

  Each @racket[alt] is a possible symbolic enumeration item
  corresponding to the numeric @racket[tag].

}

@defform/subs[#:literals
              (required optional repeated packed)
              (define-message-type name
                ([label type field tag . default] ...))
              ([label required optional repeated packed])
              #:contracts
              ([type (or/c primitive-info? enum-info? struct-type?)]
               [tag exact-nonnegative-integer?])]{

  Defines a structure type similar to the following code: @racketblock[
    (struct name message
      ([field #:auto] ...)
      #:transparent #:mutable
      #:auto-value (void)
      #:property prop:protobuf (delay (message-info 'name ...)))
  ]

  In addition to the unary constructor procedure @racket[name] that is
  of limited use, a variadic constructor @racket[name]@racketid[*] is
  created. This constructor accepts an optional keyword argument for
  every declared field of the message.

  The accessors generated for each field accept an additional optional
  argument to specify a default value. If the accessor is applied to
  an instance of the message type in which the field is set to
  @racket[(void)], the default value is applied in case it is a thunk
  or is returned otherwise.

  If no explicit default value is specified for a missing field, the
  accessors behave as follows:
  @itemize[
    @item{
      The accessor for a required field raises an
      @racket[exn:fail:user] error.
    }
    @item{
      The accessor for an optional field runs a default thunk with
      @racket[default].
    }
    @item{
      The accessor for a repeated (or packed repeated) field returns
      an empty list.
    }
  ]

}

@defform/subs[#:literals
              (required optional repeated packed)
              (define-message-extension name
                [label type field tag . default])
              ([label required optional repeated packed])
              #:contracts
              ([type (or/c primitive-info? enum-info? struct-type?)]
               [tag exact-nonnegative-integer?])]{

  Registers an extension field for the existing message type called
  @racket[name]. The field declaration syntax is identical to that of
  a single field in @racket[define-message-type].

}
