#lang racket
(require rosetta/rhino)
;;Door for CAD
(define (casing p L l h)
  (define casing-length 0.06)
  (define casing-width 0.12)
  (sweep (line (+x p (- (/ L -2) (/ casing-length 2)))
               (+xz p (- (/ L -2) (/ casing-length 2)) (+ h (/ casing-length 2)))
               (+xz p (+ (/ L +2) (/ casing-length 2)) (+ h (/ casing-length 2)))
               (+x p (+ (/ L +2) (/ casing-length 2))))
         (surface-rectangle p casing-length casing-width)))

(define (box-door p L l h)
  (box (+xy p (/ L -2) (/ l -2)) L l h))

(define (glass p L l1 h)
  (define door-margin-sides 0.1)
  (define door-margin-up 0.17)
  (define door-margin-bottom 0.3)
  (define door-middle-horizontal 0.2)
  (define door-middle-vertical 0.07)
  (define n-glasses-horizontal 2)
  (define n-glasses-vertical 2)
  (define glass-area-length (- L
                               (* door-margin-sides 2)
                               (* door-middle-vertical (- n-glasses-horizontal 1))))
  (define glass-area-height (- h
                               door-margin-up
                               door-margin-bottom
                               (* door-middle-horizontal (- n-glasses-vertical 1))))
  (define glass-length (/ glass-area-length n-glasses-horizontal))
  (define glass-height (/ glass-area-height n-glasses-vertical))
  (map-division (lambda (a b) (box (+xz p a b) glass-length l1 glass-height))
                (+ (/ L -2) door-margin-sides)
                (+ (/ L -2)
                   door-margin-sides
                   glass-area-length
                   (* door-middle-vertical n-glasses-horizontal))
                n-glasses-horizontal
                #f
                door-margin-bottom
                (+ door-margin-bottom
                   glass-area-height
                   (* door-middle-horizontal
                      n-glasses-vertical))
                n-glasses-vertical
                #f))

(define (door p L l h)
  (define glass-thickness 0.005)
  (define door-margin 0.08)
  (casing p L l h)
  (subtraction (box-door p L l h)
               (glass (+y p (- l)) L (* l 2) h))
  (glass (+y p (/ glass-thickness -2)) L glass-thickness h))

(subtraction (box (x -5)(xyz 5 1 4))
               (box (x -1)(xyz 1 1 4)))
(door (x 0) 2 1 3)
