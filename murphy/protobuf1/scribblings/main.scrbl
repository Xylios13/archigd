#lang scribble/manual
@require[(for-label "../main.rkt")]

@title{Protocol Buffers: Portable Data Serialization}

@table-of-contents[]

@defmodule[murphy/protobuf1/main]{

  Facilities to read and write @link["http://protobuf.googlecode.com/"]{Protocol Buffers}
  in binary format. The basic message type is re-exported from the
  reflection module.
  
}

@defproc[(deserialize [type/msg (or/c struct-type? struct?)] [in input-port? (current-input-port)]) any/c]{

  Read a structure instance encoded in protocol buffer format from the
  port @racket[in]. @racket[type/msg] can either represent a protocol
  buffer message type or be an existing protocol buffer message that
  will be destructively merged with the newly read data.

}

@defproc[(serialize [v struct?] [out output-port? (current-output-port)]) void?]{

  Write @racket[v] to the port @racket[out], encoding it in protocol
  buffer format. @racket[v] must be an instance of a protocol buffer
  message type.

}

@include-section{reflection.scrbl}
@include-section{syntax.scrbl}
@include-section{encoding.scrbl}
@include-section{license.scrbl}
