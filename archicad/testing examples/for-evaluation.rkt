#lang racket
(require "main.rkt")

(define (floors points levels)
  (for ([lvl levels])
       (slab points #:bottom-level lvl)))

(define (walls points levels)
  (let* ((p1 (car points))
         (p2 (car (cdr points)))
         (wall-length (sqrt (+ (* (- (cx p2) (cx p1))
                                  (- (cx p2) (cx p1)))
                               (* (- (cy p2) (cy p1))
                                  (- (cy p2) (cy p1)))))))
    (for ([lvl levels])
         (for ([wall-index (wall (cons (last points) points) #:bottom-level lvl)])
              (window wall-index (/ wall-length 2))))))

(define (tower points n-floors)
  (let ((levels (for/list ([i n-floors])
            (level (* i (default-level-to-level-height))))))
    levels
    (floors points levels)
    (walls points levels)
    (roof points #:bottom-level (upper-level #:level (last levels)))))



(define (wall1 beginPoint endPoint thickness height)
  (let* ((dx (- (cx endPoint)(cx beginPoint)))
         (dy (- (cy endPoint)(cy beginPoint)))
         (hip (sqrt (+ (* dx dx)(* dy dy))))
         (cosalpha (/ dx hip))
         (sinalpha (/ dy hip)))
    (box-2points beginPoint
                 (+xyz endPoint (* sinalpha thickness) (* cosalpha thickness) height))))

#;(define (slab beginPoint length width height)
  (box beginPoint length width height))


(define (wall2 p0 p1 thickness height)
  (let* ((v (p-p p1 p0))
         (new-p1 (+xyz (+pol p0
                             (pol-rho v)
                             (pol-phi v))
                       (* (sin (pol-phi v)) thickness)
                       (* (cos (pol-phi v)) thickness)
                       height)))
    (box-2points p0 new-p1)))





#;(stairs "Stair Spiral 18"
        (u0)
        #:use-xy-fix-size #t
        #:x-ratio 37
        #:additional-parameters (list (list "zzyzx" 15)
                                      (list "nRisers" (* 36 6))
                                      (list "angle" 2pi)
                                      (list "swelldia" 35) 
                                      (list "rightRailType_m" 0)
                                      (list "leftRailType_m" 0)
                                      (list "stairBaseType_m" 0)))

#;(stairs "Stair Spiral 18"
        (u0)
        #:use-xy-fix-size #t
        #:x-ratio 37
        #:height 15
        #:properties (list "nRisers" (* 36 6)
                           "angle" 2pi
                           "swelldia" 35
                           "rightRailType_m" 0
                           "leftRailType_m" 0
                           "stairBaseType_m" 0))

(object "Rail Solid 18"
        (+x (u0) -0.75))

(object "Rail Solid 18"
        (+x (u0) -0.75)
        #:angle pi/2)

(disconnect)
