#lang scribble/manual
@require[(for-label "../generator.rkt")]

@title{@tt{protoc-gen-racket}: Generate Racket Code for Protocol Buffers}

@table-of-contents[]

The generator module serves as a plug-in executable for ProtoC to
facilitate the conversion between Protocol Buffer description files
and Racket modules.
  
To use the @tt{protoc-gen-racket} code converter, make sure the
executable is in your path as well as the @tt{protoc} compiler, then
simply invoke @litchar{protoc --racket_out=DIR FILE ...}

You can also access the generator function pragramatically, if
necessary.

@section{Programming Interface}

@defmodule[murphy/protobuf1/generator]{

  Functions to convert Protocol Buffer descriptors into Racket code.

  This module relies on the ProtoC plug-in interfaces which are
  available as racket modules generated from protocol buffer
  definitions themselves.

}

@defproc[(proto-string->symbol [str string?] [prefix any/c #f]) symbol?]{

  Reformats an identifier following protocol buffer definition
  conventions into a symbol, optionally adding a prefix.

  The conversion proceeds as follows:
  @itemize[
    @item{
      Camel case words are separated by @litchar{-} characters.
    }
    @item{
      The entire string is converted to lower case.
    }
    @item{
      Any occurrence of @litchar{_} is replaced by @litchar{-}.
    }
    @item{
      An @litchar{is-} prefix is replaced by a @litchar{?} suffix.
    }
    @item{
      The @racket[prefix], if present, is stringified and prepended to
      the identifier separated by @litchar{:}.
    }
  ]

}

@defproc[(register-types! [types (hash/c string? (cons/c symbol? symbol?) #:immutable #f)] [proto file-descriptor-proto?]) void?]{

  Register all types defined in @racket[proto] in the hash
  @racket[types] that maps the fully qualified protocol buffer names
  to pairs of @racket['enum] or @racket['struct] and an identifier.

}

@defproc[(register-enum-type! [types (hash/c string? (cons/c symbol? symbol?) #:immutable #f)] [proto enum-descriptor-proto?]) void?]{

  Register the enumeration type described by @racket[proto] in the
  hash @racket[types].

}

@defproc[(register-message-types! [types (hash/c string? (cons/c symbol? symbol?) #:immutable #f)] [proto descriptor-proto?]) void?]{

  Register the message type described by @racket[proto] and all its
  nested types in the hash @racket[types].

}

@defproc[(type-ref [types (hash/c string? (cons/c symbol? symbol?))] [package string?] [name string?]) (cons/c symbol? symbol?)]{

  Find the kind and identifier of a type in @racket[types] specified
  by its protocol buffers @racket[package] and @racket[name].

}

@defproc[(translate-types [types (hash/c string? (cons/c symbol? symbol?))] [proto file-descriptor-proto?]) list?]{

  Translate all types defined in @racket[proto] into module level
  Racket code. Use @racket[types] for lookup of type references.

}

@defproc[(translate-enum-type [types (hash/c string? (cons/c symbol? symbol?))] [package (or/c string? #f)] [proto enum-descriptor-proto?]) any/c]{

  Translate the enumeration type described by @racket[proto] into
  module level Racket code. Use @racket[types] for lookup of type
  references.

}

@defproc[(translate-message-types [types (hash/c string? (cons/c symbol? symbol?))] [package (or/c string? #f)] [proto descriptor-proto?]) list?]{

  Translate the message type described by @racket[proto] and all its
  nested types into module level Racket code. Use @racket[types] for
  lookup of type references.

}

@defproc[(translate-extension [types (hash/c string? (cons/c symbol? symbol?))] [package (or/c string? #f)] [proto field-descriptor-proto?]) any/c]{

  Translate the message type extension described by @racket[proto]
  into module level Racket code. Use @racket[types] for lookup of type
  references.

}

@defproc[(generate-racket [req code-generator-request?]) code-generator-response?]{

  Process a code generation request and generate Racket code.

}

@section{Generated Modules}

@defmodule[murphy/protobuf1/google/protobuf/descriptor]{

  Protocol buffer descriptor types. Definitions are generated from
  @litchar{google/protobuf/descriptor.proto}.

}

@defmodule[murphy/protobuf1/google/protobuf/compiler/plugin]{

  The ProtoC compiler plug-in interface. Definitions are generated from
  @litchar{google/protobuf/compiler/plugin.proto}.

}

@defmodule[murphy/protobuf1/extend/protobuf/bigint]{

  Big integer support extensions for
  @tt{protoc-gen-racket}. Definitions are generated from
  @litchar{extend/protobuf/bigint.proto}.

}

@include-section{license.scrbl}
