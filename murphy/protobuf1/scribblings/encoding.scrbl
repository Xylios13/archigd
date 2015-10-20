#lang scribble/manual
@require[(for-label "../encoding.rkt")]

@title{Encoding}

@defmodule[murphy/protobuf1/encoding]{

  Low-level functions dealing with the binary wire format of protocol
  buffers.

}

@deftogether[(
  @defproc[(read-uint* [in input-port? (current-input-port)] [max-size (or/c exact-positive-integer? #f) 10]) (or/c exact-nonnegative-integer? eof-object?)]{}
  @defproc[(write-uint* [n exact-nonnegative-integer?] [out output-port? (current-output-port)] [max-size (or/c exact-positive-integer? #f) 10]) void?]{}
)]{

  Read/write a variable length unsigned integer. If @racket[max-size]
  is not false, it defines the maximum number of bytes that the
  encoded number may use.

}

@deftogether[(
  @defproc[(read-sint* [in input-port? (current-input-port)] [max-size (or/c exact-positive-integer? #f) 10]) (or/c exact-integer? eof-object?)]{}
  @defproc[(write-sint* [i exact-integer?] [out output-port? (current-output-port)] [max-size (or/c exact-positive-integer? #f) 10]) void?]{}
)]{

  Read/write a variable length signed integer in zigzag encoding. If
  @racket[max-size] is not false, it defines the maximum number of
  bytes that the encoded number may use.

}

@deftogether[(
  @defproc[(read-int* [in input-port? (current-input-port)]) (or/c exact-integer? eof-object?)]{}
  @defproc[(write-int* [i exact-integer?] [out output-port? (current-output-port)]) void?]{}
)]{

  Read/write a variable length signed integer in tows-complement
  encoding. The maximum number of bytes used by the encoded number is
  10, the maximum bit length of the number is 64.

}

@deftogether[(
  @defproc[(read-bool [in input-port? (current-input-port)]) (or/c bool? eof-object?)]{}
  @defproc[(write-bool [v any/c] [out output-port? (current-output-port)]) void?]{}
)]{

  Read/write a boolean as a one byte integer.

}

@deftogether[(
  @defproc[(read-fixed32 [in input-port? (current-input-port)]) (or/c exact-nonnegative-integer? eof-object?)]{}
  @defproc[(read-fixed64 [in input-port? (current-input-port)]) (or/c exact-nonnegative-integer? eof-object?)]{}
  @defproc[(write-fixed32 [n exact-nonnegative-integer?] [out output-port? (current-output-port)]) void?]{}
  @defproc[(write-fixed64 [n exact-nonnegative-integer?] [out output-port? (current-output-port)]) void?]{}
)]{

  Read/write unsigned integers as fixed length 32/64-bit values with little
  endian byte order.

}

@deftogether[(
  @defproc[(read-sfixed32 [in input-port? (current-input-port)]) (or/c exact-integer? eof-object?)]{}
  @defproc[(read-sfixed64 [in input-port? (current-input-port)]) (or/c exact-integer? eof-object?)]{}
  @defproc[(write-sfixed32 [i exact-integer?] [out output-port? (current-output-port)]) void?]{}
  @defproc[(write-sfixed64 [i exact-integer?] [out output-port? (current-output-port)]) void?]{}
)]{

  Read/write signed integers as fixed length 32/64-bit values in
  twos-complement encoding with little endian byte order.

}

@deftogether[(
  @defproc[(read-float [in input-port? (current-input-port)]) (or/c real? eof-object?)]{}
  @defproc[(read-double [in input-port? (current-input-port)]) (or/c real? eof-object?)]{}
  @defproc[(write-float [x real?] [out output-port? (current-output-port)]) void?]{}
  @defproc[(write-double [x real?] [out output-port? (current-output-port)]) void?]{}
)]{

  Read/write real numbers as fixed length 32/64-bit IEEE floating
  point values with little endian byte order.

}

@deftogether[(
  @defproc[(read-sized-bytes [in input-port? (current-input-port)]) (or/c bytes? eof-object?)]{}
  @defproc[(write-sized-bytes [bstr bytes?] [out output-port? (current-output-port)]) void?]{}
)]{

  Read/write a byte string with size prefix. The size is read and
  written using @racket[read-uint*] and @racket[write-uint*].

}

@deftogether[(
  @defproc[(read-sized-string [in input-port? (current-input-port)]) (or/c string? eof-object?)]{}
  @defproc[(write-sized-string [str string?] [out output-port? (current-output-port)]) void?]{}
)]{

  Read/write a UTF-8 encoded string with size prefix. The size in
  bytes is read and written using @racket[read-uint*] and
  @racket[write-uint*].

}

@deftogether[(
  @defproc[(read-sized [read (-> input-port? any/c)] [in input-port? (current-input-port)]) any/c]{}
  @defproc[(write-sized [write (-> any/c output-port? any)] [v any/c] [out output-port? (current-output-port)]) any]{}
)]{

  Read/write any object with size prefix. The size in bytes is read
  and written using @racket[read-uint*] and @racket[write-uint*].

  On input, @racket[read] is called on an input port limited according
  to the size prefix. On output, @racket[write] is called on
  @racket[v] and a byte string output port; the buffered output's
  length is then written to the actual output port.

}

@deftogether[(
  @defproc[(read-tag/type [in input-port? (current-input-port)]) (values (or/c tag/c eof-object?) (or/c exact-nonnegative-integer? eof-object?))]{}
  @defproc[(write-tag/type [tag tag/c] [type exact-nonnegative-integer?] [out output-port? (current-output-port)]) void?]{}
)]{

  Read/write the tag and type of a protocol buffer message field.

}

@defthing[type/c flat-contract?]{

  Contract matching any of the protocol buffer message field wire type
  enumeration items:
  @itemize[
    @item{
      @racket['int*]: The field value is encoded as a variable length
      integer.
    }
    @item{
      @racket['64bit]: The field value is always encoded in 64 bits.
    }
    @item{
      @racket['32bit]: The field value is always encoded in 32 bits.
    }
    @item{
      @racket['sized]: The field value is encoded with a byte size
      prefix.
    }
  ]

}
