#lang scribble/manual
@(require (for-label racket))

@title{My Library}

@defmodule[archicad/objects]

@defproc[(wall [guide list?]
               [#:bottom-level bottom-level (current-level)]
               [#:top-level top-level (upper-level #:level bottom-level)]
               [#:alignment alignment string? (default-wall-alignment)]
               [#:thickness thickness number? (default-wall-thickness)]
               [#:angle angle number? 0]
               [#:type-of-material type-of-material string? (default-wall-type-of-material)]
               [#:material material string? "GENERIC - STRUCTURAL"]
               [#:type-of-profile type-of-profile string? (default-wall-profile)]
               [#:alpha-angle alpha-angle number? (/ pi 2)]
               [#:beta-angle beta-angle number? (/ pi 2)]
               [#:height height number? null]
               [#:profile-name profile-name string? ""])
         wall-object]{
 
 Creates a wall or multiple walls, given list of points @racket[guide].
 If the @racket[guide] contains more than two points, it will create more than one wall.
 Returns a wall object @racket[wall-object].
 All other arguments are optional.

 Parameter @racket[bottom-level] determines the bottom level of the created wall/s.

 Parameter @racket[top-level] determines the top level of the created wall/s,
 it will link the wall to that level and control its height.

 Parameter @racket[alignment] determines the reference-line of wall,
 i.e., @racket["Center"], @racket["Inside"], @racket["Outside"].
 
 Parameter @racket[thickness] determines the thickness of the created wall/s.

 Parameter @racket[angle] determines the of each individual created wall. Given in radians (CONFIRM!).

 Parameter @racket[type-of-material] determines the type of material. either @racket["Basic"] or @racket["Composite"].

 Parameter @racket[material] determines which material to use, see ArchiCAD's materials.
 It is supported the use of user created material by giving their name.

 Parameter @racket[type-of-profile] determines the type of profile for the wall/s.
 Can be @racket["Normal"], @racket["Slanted"] or @racket["DoubleSlanted"]
 
 Parameter @racket[alpha-angle] determines the alpha angle of a slanted wall.

 Parameter @racket[beta-angle] determines the beta angle of a double slanted wall.

 Parameter @racket[height] determines the height of the wall.
 The use of this parameter disregards the @racket[top-level], and unlinks the wall from its top level.

 Parameter @racket[profile-name] determines the profile to be used for the wall.
 These profiles can be user created or the default ones in ArchiCAD.
 Both identified by their name, i.e., @racket[string?]. 
}