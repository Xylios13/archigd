#lang scribble/manual
@(require scribble/eval
          (for-label (except-in racket
                                send
                                random
                                box?
                                box)
                     "../backend.rkt"))

@title{Rosetta - ArchiCAD}

@(define eval (make-base-eval))

@defmodule[rosetta/archicad]

@defproc[(wall [p0 Loc]
               [p1 Loc]
               [bottom-level Level (current-level)]
               [top-level Level (upper-level bottom-level)]
               [family Any (default-wall-family)])
         wall-shape]{
 Creates a wall given two points @racket[p0] and @racket[p1].
 Returns a wall that can be used by other operations, such as @racket[window], @racket[door], among others.
 All other arguments are optional, and have default values.

 Parameter @racket[bottom-level] determines the bottom level of the created wall.

 Parameter @racket[top-level] determines the top level of the created wall,
 it will link the wall to that level and control its height.

 Parameter @racket[family] contains information regarding the wall's type.
 At the moment only thickness is supported.

 @interaction-eval[#:eval eval
                   (require "../backend.rkt")]
 
 @examples[#:eval eval
           (wall (x 0)(x 10))]
}

@defproc[(walls [vertices Locs]
                [bottom-level Level (current-level)]
                [top-level Level @(upper-level bottom-level)]
                [family Any (default-wall-family)])
         (listof wall-shapes)]{
 Creates walls along all @racket[vertices].
 Returns a list of walls.
 All other arguments are optional, and have default values.

 Parameter @racket[bottom-level] determines the bottom level of the created walls.

 Parameter @racket[top-level] determines the top level of the created walls,
 it will link the walls to that level and control their height.

 Parameter @racket[family] contains information regarding the walls' type.
 At the moment only thickness is supported. 
}

@defproc[(slab [vertices Locs]
               [level Level (current-level)]
               [family Any (default-wall-family)])
         slab-shape]{
 Creates a slab given a list of points, @racket[guide].
 The @racket[guide] is not closed, i.e., the last point does NOT have to be equal to the first.

 Parameter @racket[level] determines the level of the created slab.
 This will also determine the slab's height, i.e., it will be the same height as the level in which the slab is placed.

 Parameter @racket[family] contains information regarding the slab's type.
 At the moment only thickness is supported.
}

@defproc[(roof [vertices Locs]
               [level Level (current-level)]
               [family Any (default-wall-family)])
         roof-shape]{
 Creates a roof given a list of points, @racket[guide].
 The @racket[guide] is not closed, i.e., the last point does NOT have to be equal to the first.

 Parameter @racket[level] determines the level of the created roof.
 This will also determine the roof's height, i.e., it will be the same height as the level in which the roof is placed.

 Parameter @racket[family] contains information regarding the roof's type.
 At the moment only thickness is supported.
}

@defproc[(window [wall wall-shape]
                 [relative-pos number?])
         window-shape]{
 Creates a window in a given @racket[wall] at a position relative to that wall, @racket[relative-pos].
 The parameter @racket[relative-pos] is the middle-point of the window.                           
 Furthermore it is a distance relative to the initial point of the wall, @racket[relative-pos] = 0.
 For a wall with a length of 5, and to position a window in the middle one would give @racket[relative-pos] as 2.5.
}

@defproc[(level [height number?])
         level]{
 Creates a level at a given @racket[height].
 If the level already exists, it returns that level and there is no creation.
}

@defproc[(upper-level [lvl Level (current-level)]
                      [height Real (default-level-to-level-height)])
         level]{
 It creates a level at given @racket[height] based on a given level, @racket[lvl].
 If the level already exists, it returns the level and no level is created.
}