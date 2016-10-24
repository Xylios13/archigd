#lang racket
;(require rosetta/revit)
;(require rosetta/autocad)
;(require rosetta/archicad)
(require "main.rkt")
;(delete-all-shapes)

;(define beam-family (load-beam-family "C:\\ProgramData\\Autodesk\\RVT 2015\\Libraries\\US Metric\\Structural Framing\\Wood\\M_Timber.rfa" #:width "b" #:height "d"))

;(current-level (level 0))

(define (right-cuboid2 p w h a)
  #;(right-cuboid (+yz p (/ h -2) (/ a 2))
                w a
                (+yz p (/ h +2) (/ a 2)))
  (beam (+yz p (/ h -2) (/ a 1))
        (+yz p (/ h +2) (/ a 1))
        #:beam-width w
        #:beam-height a))

(define union list)
(define (empty-shape) #f)

(define j 0.01)
;FUNCAO QUE CRIA QUATRO TIJOLO PEQUENOS NO ESPAÇO DE 1 TIJOLO GRANDE (30x10cm)
(define (brick4 p lado alt e r)
  (let* ((lado2 (/ (- lado j) 2))
         (alt2 (/ (- alt j) 2))
         (pp (+xy p (/ e 2) (+ (/ lado2 2) j)))
         (p1 (+y pp lado2))
         (p2 (+z pp alt2))
         (p3 (+yz pp lado2 alt2)))
    (union
     (right-cuboid2 pp (* (random-range 1 r) e) lado2 alt2)
     (right-cuboid2 p1 (* (random-range 1 r) e) lado2 alt2)
     (right-cuboid2 p2 (* (random-range 1 r) e) lado2 alt2)
     (right-cuboid2 p3 (* (random-range 1 r) e) lado2 alt2))))

;FUNCOES QUE CRIAM TRES TIJOLOS PEQUENOS NO ESPAÇO DE UM GRANDE
(define (brick3 p lado alt e r)
  (let* ((lado2 (/ (- lado j) 2))
         (alt2 (/ (- alt j) 2))
         (pp (+xy p (/ e 2) (+ (/ lado2 2) j)))
         (p1 (+y pp lado2))
         (p2 (+z pp alt2))
         (p3 (+yz pp lado2 alt2)))
    (union
     (right-cuboid2 pp (* (random-range 1 r) e) lado2 alt2)
     (right-cuboid2 p1 (* (random-range 1 r) e) lado2 alt2)
     (right-cuboid2 p3 (* (random-range 1 r) e) lado2 alt2))))

;FACHADA NORTE
;DIMENSOES
(define length 28.8)
(define height 3)
;DIMENSOES BRICKS
(define c-brick 0.6) ;largura dos tijolos grandes
(define h-brick 0.2) ;altura dos tijolos grandes
(define e-bricks 0.03) ;espessura dos tijolos
(define n-bricks 48) ;numero em x
(define m-bricks 15) ;numero em z
;FATORES DENSIDADE
(define r50 2) ;para as zonas de opacidade 100%
(define r75 3)
(define r100 4)

;ZONA OPACA 100%
;intervalos de ocurrência
;0m-3m dos 12.6m-17.4m e dos 25.8m-28.8m
(define a1% (/ 3 c-brick))
(define a2% (/ 11.4 c-brick))
(define a3% (/ 16.2 c-brick))
(define a4% (/ 25.8 c-brick))

;ZONA OPACIDADE 80%
;dos 3-6.6m dos 9.6-12.6m dos 17.4-20.4m dos22.8-25.8m
(define b0% a1%)
(define b1% (/ 6 c-brick))
(define b2% (/ 8.4 c-brick))
(define b3% a2%)
(define b4% a3%)
(define b5% (/ 19.2 c-brick))
(define b6% (/ 22.2 c-brick))
(define b7% a4%)

;ZONA OPACA 60%: o restante dos intervalos

;FUNÇÃO PARA FAZER A PRIMEIRA E ULTIMA FILEIRA DA FACHADA
(define (rectExt p lado alt e fator-e n)
  (if (= n 0)
      (empty-shape)
      (union
       (brick4 p lado alt e fator-e)
       (rectExt (+y p lado) lado alt e fator-e (- n 1)))))

(define (rect p lado alt e fator-e n)
  (let ((pp (+xy p (/ e 2) (/ (+ j lado) 2)))
        (lado1 (- lado j))
        (alt1 (- alt j))
        (fator (random-range 2
                             (random-range 3 5) ;percentagem de relevo 15%
                             #;4) ;percentagem de relevo 30%
               #;3));percentagem de relevo 50%
    (cond ((= n 0)
           (empty-shape))
          ;totalmente opaca
          ((or (or (< n a1%) (and (> n a2%) (< n a3%))) (> n a4%))
             (if (= (random-range 0 r50) 1) ;probabilidade de 50% pecas grandes
                 (union 
                  (right-cuboid2 pp (* (random-range 1 fator) e) lado1 alt1)
                  (rect (+y p lado) lado alt e fator-e (- n 1)))
                 (union ;probabilidade 50% de criar pecas pequenas
                  (brick4 p lado alt e fator)
                  (rect (+y p lado) lado alt e fator-e (- n 1)))))
          
          ;opacidade a 80%
          ((or (or (or (and (>= n b0%) (<= n b1%))
                       (and (>= n b2%) (<= n b3%)))
                   (and (>= n b4%) (<= n b5%)))
               (and (>= n b6%) (<= n b7%)))
             (if (= (random-range 0 r75) 1) ;probabilidade 30% pecas grandes 
                 (union 
                  (right-cuboid2 pp (* (random-range 1 fator) e) lado1 alt1)
                  (rect (+y p lado) lado alt e fator-e (- n 1)))
                 (if (= (random-range 0 4) 1) ;probabilidade 70% pecas pequenas
                     (union
                      (brick3 p lado alt e fator) ;ausencias 25%
                      (rect (+y p lado) lado alt e fator-e (- n 1)))
                     (union
                      (brick4 p lado alt e fator) ;sem ausencias 75%
                      (rect (+y p lado) lado alt e fator-e (- n 1))))))
          
          ;Opacidade 60%, com algumas ausencias
          ((if (= (random-range 0 r100) 1) ;probabilidade 25% pecas grandes
                 (union
                  (right-cuboid2 pp (* (random-range 1 fator) e) lado1 alt1)
                  (rect (+y p lado) lado alt e fator-e (- n 1)))
                 (if (< (random-range 0 4) 3) ;probabilidade 75% pecas pequenas
                     (union
                      (brick3 p lado alt e fator) ;75% ausencias
                      (rect (+y p lado) lado alt e fator-e (- n 1)))
                     (union
                      (brick4 p lado alt e fator) ;25% sem ausencias
                      (rect (+y p lado) lado alt e fator-e (- n 1)))))))))


(define (rects p lado alt e fator-e n m m-max)
  (cond ((= m 1)
         (rectExt p lado alt e fator-e n))
        ((= m m-max)
         (union
          (rectExt p lado alt e fator-e n)
          (rects (+z p alt) lado alt e fator-e n (- m 1) m-max)))
        ((union
          (rect p lado alt e fator-e n)
          (rects (+z p alt) lado alt e fator-e n (- m 1) m-max)))))

;(rects (z 0) 0.6 0.2 0.05 2 48 15 15)
;(disconnect)