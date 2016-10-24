#lang racket
(require rosetta/revit)
(require "main.rkt")

(define building-length 90600)
(define building-width 33600)
(define building-height 13500)
(define building-floors 4)
(define slab-height 500)
(define column-width 600)
(define bar-thickness 200)

(define (box p l w h)
  (create-box p l w h))

;Floor plans
(define (floorplans p)
  (for/list ((h (division 0 building-height (- building-floors 1))))
    (box (+z p h) building-length building-width slab-height)))

#;(floorplans (xyz 0 0 0))
;columns

(define (columns p n m)
  (for/list ((x (division 0 (- building-length column-width) n)))
    (for/list ((y (division 0 (- building-width column-width) m)))
      (box (+xy p x y) column-width column-width building-height))))

#;(columns (xyz 0 0 0) 11 6)

;Facade

(define (facade p length height n m)
  (let ((element-length (/ length n))
        (element-height (/ height m)))
     (for/list ((r (division 0 length n #f)))
        (for/list ((z (division 0 height m #f)))
          (facade-element (+xz p r (+ z (/ element-height 2)))
                          element-length
                          element-height
                          (+ 2000 (* 1000 (cos (+ (* z 2pi (/ 1 height)) (* r 4pi (/ 1 length)))))))))))

(define (facade-element p width height dist)
  (let ((e (/ height 2)))
    (let ((p0 p)
          (p1 (+xy p (/ width 2) (- dist))))
      (let ((v (unitize (p-p p1 p0))))
        (let ((pa (+c p0 (v*r v (/ e -2))))
              (pb (+c p1 (v*r v (/ e +2)))))
          (right-cuboid pa e e pb))))
    (let ((p0 (+xz p width e))
          (p1 (+xyz p (/ width 2) (- dist) e)))
      (let ((v (unitize (p-p p1 p0))))
        (let ((pa (+c p0 (v*r v (/ e -2))))
              (pb (+c p1 (v*r v (/ e +2)))))
          (right-cuboid pa e e pb))))))

(define (facades p length width height n0 n1 m)
   (facade p length height n0 m)
   (facade (loc-from-o-phi (+x p length) pi/2) width height n1 m)
   (facade (loc-from-o-phi (+xy p length width) pi) length height n0 m)
   (facade (loc-from-o-phi (+y p width) 3pi/2) width height n1 m))

#;(facades (xy 0 0) building-length building-width building-height 50 20 40)

;;(send (floorplans (xyz 0 0 0)) (columns (xyz 0 0 0) 11 6) (facades (xy 0 0) building-length building-width building-height 50 20 40))
;;(send (floorplans (xyz 0 0 0)) (columns (xyz 0 0 0) 11 6) (facades (xy 0 0) building-length building-width building-height 50 20 10))
