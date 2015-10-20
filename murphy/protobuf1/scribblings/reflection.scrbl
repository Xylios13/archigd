#lang scribble/manual
@require[(for-label "../reflection.rkt" "../syntax.rkt" "../encoding.rkt")]

@title{Reflection}

@defmodule[murphy/protobuf1/reflection]{

  Generic functionality for protocol buffer data and types.

}

@defstruct*[type-info
            ([name symbol?])
            #:transparent]{

  Common base of protocol buffer type information records.

}

@defstruct*[(primitive-info type-info)
            ([type type/c]
             [reader (-> input-port? any/c)]
             [writer (-> any/c output-port? any)])]{

  Information about a primitive protocol buffer type. The
  @racket[type] indicates the wire type of the primitive as used by
  @racket[read-tag/type] and @racket[write-tag/type].

  The @racket[reader] and @racket[writer] are responsible for
  (de-)serialization of values of this type.

}

@defstruct*[(enum-info type-info)
            ([integer->enum (-> exact-integer? any/c)]
             [enum->integer (-> any/c exact-integer?)])]{

  Information about a user defined protocol buffer enumeration
  type.

  The @racket[integer->enum] and @racket[enum->integer] conversions
  are responsible to make the Racket representation of the enumeration
  compatible with (de-)serialization using @racket[read-int*] and
  @racket[write-int*].

  The @racket[define-enum-type] syntax can be used to synthesize
  conversion procedures between symbols and integers as well as an
  instance of this structure type.

}

@deftogether[(
  @defthing[prop:protobuf struct-type-property?]{}
  @defproc[(protobuf? [v any/c]) boolean?]{}
  @defproc[(protobuf-ref [v any/c] [default any/c ...]) any/c]{}
)]{

  This property is used to attach a promise of a @racket[message-info]
  structure to the structure type corresponding to a protocol buffer
  message type.

  The procedure @racket[protobuf?] checks for the presence of the
  property on a structure type or instance; @racket[protobuf-ref]
  extracts the value of the property.

}

@defstruct*[(message-info type-info)
            ([constructor (-> any/c)]
             [fields (hash/c exact-nonnegative-integer? field?)]
             [required (set/c exact-nonnegative-integer?)])]{

  Information about a user defined protocol buffer message type.

  The @racket[constructor] is used to obtain fresh instances of the
  structure type representing the message type which have all fields
  initialized to the default value @racket[(void)] for missing
  fields.

  The hash @racket[fields] maps wire tags to field descriptors, the
  set @racket[required] indicates which wire tags must be present in a
  valid message.

  The @racket[define-message-type] syntax can be used to synthesize a
  structure type and an instance of this descriptor type.

}

@defstruct*[field-info
            ([type (or/c primitive-info? enum-info? struct-type?)]
             [repeated? any/c]
             [packed? any/c]
             [accessor (->* (any/c) (any/c) any/c)]
             [mutator (-> any/c any/c any)])]{

  Information about a field in a user defined protocol buffer
  message. The @racket[type] of the field can contain primitive
  protocol buffer type information, enumeration type information or a
  record type with associated message type information.

  Iff the field can be present any number of times, @racket[repeated?]
  should be true; @racket[packed?] may be true for repeated fields of
  non-length-delimited primitive types to specify a more efficient
  wire format.

  The @racket[accessor] and @racket[mutator] are used to extract and
  modify the values of the field in instances of the message
  structure.

}

@defstruct*[message
            ([extensions (hash/c exact-nonnegative-integer? any/c)]
             [unknown bytes?])
            #:mutable #:transparent]{

  The common base of all protocol buffer message types. While regular
  fields of a message are stored in fields of @racket[message]'s
  subtypes, known extensions are stored in the @racket[extensions]
  hash under their field tags. Fields with unknown tags are copied
  verbatim to or from the @racket[unknown] byte string.

}
